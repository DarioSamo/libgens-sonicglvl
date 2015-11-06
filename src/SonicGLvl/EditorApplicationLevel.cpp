//=========================================================================
//	  Copyright (c) 2015 SonicGLvl
//
//    This file is part of SonicGLvl, a community-created free level editor 
//    for the PC version of Sonic Generations.
//
//    SonicGLvl is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    SonicGLvl is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//    
//
//    Read AUTHORS.txt, LICENSE.txt and COPYRIGHT.txt for more details.
//=========================================================================

#include "EditorApplication.h"
#include "ObjectLibrary.h"

EditorLevelDatabase::EditorLevelDatabase(string filename) {
	TiXmlDocument doc(filename);
	if (!doc.LoadFile()) {
		LibGens::Error::addMessage(LibGens::Error::FILE_NOT_FOUND, (string)SONICGLVL_DATABASE_ERROR_FILE + filename);
		return;
	}

	TiXmlHandle hDoc(&doc);
	TiXmlElement* pElem;
	TiXmlHandle hRoot(0);

	pElem=hDoc.FirstChildElement().Element();
	if (!pElem) {
		LibGens::Error::addMessage(LibGens::Error::EXCEPTION, SONICGLVL_DATABASE_ERROR_FILE_ROOT);
		return;
	}

	pElem=pElem->FirstChildElement();
	for(pElem; pElem; pElem=pElem->NextSiblingElement()) {
		string entry_name="";
		string level_name="";
		string geometry_name="";
		string layout_merge_name="";
		string slot_name="";
		string game_name="";

		entry_name = pElem->ValueStr();
		pElem->QueryValueAttribute(SONICGLVL_DATABASE_NAME_ATTRIBUTE, &level_name);
		pElem->QueryValueAttribute(SONICGLVL_DATABASE_GEOMETRY_ATTRIBUTE, &geometry_name);
		pElem->QueryValueAttribute(SONICGLVL_DATABASE_MERGE_ATTRIBUTE, &layout_merge_name);
		pElem->QueryValueAttribute(SONICGLVL_DATABASE_SLOT_ATTRIBUTE, &slot_name);
		pElem->QueryValueAttribute(SONICGLVL_DATABASE_GAME_ATTRIBUTE, &game_name);

		if (!game_name.size()) {
			game_name = LIBGENS_LEVEL_GAME_STRING_GENERATIONS;
		}

		if ((entry_name==SONICGLVL_DATABASE_ENTRY) && level_name.size() && geometry_name.size()) {
			EditorLevelEntry *entry=new EditorLevelEntry(level_name, geometry_name, layout_merge_name, slot_name, game_name);
			entries.push_back(entry);
		}
	}
}


bool EditorLevelDatabase::exists(string name) {
	for (size_t i=0; i<entries.size(); i++) {
		if (entries[i]->name == name) return true;
	}
	return false;
}

string EditorLevelDatabase::getGeometryPath(string name) {
	for (size_t i=0; i<entries.size(); i++) {
		if (entries[i]->name == name) return entries[i]->geometry;
	}
	return "";
}

string EditorLevelDatabase::getMergePath(string name) {
	for (size_t i=0; i<entries.size(); i++) {
		if (entries[i]->name == name) return entries[i]->layout_merge;
	}
	return "";
}

string EditorLevelDatabase::getSlot(string name) {
	for (size_t i=0; i<entries.size(); i++) {
		if (entries[i]->name == name) return entries[i]->slot;
	}
	return "";
}

string EditorLevelDatabase::getGame(string name) {
	for (size_t i=0; i<entries.size(); i++) {
		if (entries[i]->name == name) return entries[i]->game;
	}
	return LIBGENS_LEVEL_GAME_STRING_GENERATIONS;
}



void EditorApplication::openLostWorldLevel(string filename) {
	string slot_name=LibGens::File::nameFromFilenameNoExtension(filename);

	string folder = filename + "/";

	CreateDirectory((SONICGLVL_CACHE_PATH + slot_name).c_str(), NULL);

	EditorLevel *lost_world_level = new EditorLevel(folder, slot_name, slot_name, "", LIBGENS_LEVEL_GAME_STRING_LOST_WORLD);
	current_level = lost_world_level;
	current_level_filename = filename;
	
	lost_world_level->unpackResources();
	lost_world_level->unpackData();

	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(lost_world_level->getResourcesFolder(), "FileSystem");
	uv_animation_library->addFolder(lost_world_level->getResourcesFolder());
	
	lost_world_level->loadData(library, object_node_manager);

	lost_world_level->loadTerrain(scene_manager, &terrain_nodes_list);

	current_set = lost_world_level->getLevel()->getSets().front();

	/*
	current_level->getTerrain()->addModels(lost_world_level->getTerrain()->getModelsToOrganize());
	current_level->getTerrain()->addInstances(lost_world_level->getTerrain()->getInstancesToOrganize());
	current_level->getTerrain()->getMaterialLibrary()->merge(lost_world_level->getMaterialLibrary(), true);
	*/
		
	LibGens::Light *direct_light=lost_world_level->getDirectLight();
	if (direct_light) {
		createDirectionalLight(direct_light);
	}
	
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	hFind = FindFirstFile((lost_world_level->getResourcesFolder() + "/*sky*.model").c_str(), &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) {}
	else {
		do {
			cout << "Creating skybox: " << FindFileData.cFileName << "\n";
			createSkybox(LibGens::File::nameFromFilenameNoExtension(FindFileData.cFileName));
		} while (FindNextFile(hFind, &FindFileData) != 0);
		FindClose(hFind);
	}
	
	loadLevelPaths();
	
	current_level->loadCollision(havok_enviroment, scene_manager, havok_nodes_list);
	terrain_streamer = NULL;
}

void EditorApplication::openLevel(string filename) {
	if (!level_database) {
		SHOW_MSG("You can't open any levels without a level database!");
		return;
	}

	string slot_name=filename;
	string folder="";

	size_t sep = slot_name.find_last_of("#");
	if (sep != string::npos) {
		folder = slot_name.substr(0, sep);
		slot_name = slot_name.substr(sep + 1, slot_name.size() - sep - 1);
	}

	sep = slot_name.find_last_of(".ar.00");
	if (sep != string::npos) {
		slot_name = slot_name.substr(0, sep-5);
	}

	string data_name=slot_name;
	string geometry_name=level_database->getGeometryPath(slot_name);
	string merge_name=level_database->getMergePath(slot_name);
	string slot_id_name=level_database->getSlot(slot_name);
	string game_name=level_database->getGame(slot_name);

	if (!geometry_name.size()) geometry_name = slot_name;

	if (!level_database->exists(slot_name)) {
		SHOW_MSG("This level name doesn't exist in the current database. The slot name will be used for finding the geometry.");
	}

	CreateDirectory((SONICGLVL_CACHE_PATH + slot_name).c_str(), NULL);
	CreateDirectory((SONICGLVL_CACHE_PATH + geometry_name).c_str(), NULL);
	if (merge_name.size()) CreateDirectory((SONICGLVL_CACHE_PATH + merge_name).c_str(), NULL);


	current_level_filename=filename;

	current_level = new EditorLevel(folder, slot_name, geometry_name, merge_name, game_name);
	current_level->unpackData();
	current_level->unpackResources();
	current_level->unpackTerrain();
	current_level->saveHashes();

	object_node_manager->setSlotIdName(slot_id_name);
	current_level->loadData(library, object_node_manager);

	current_set = current_level->getLevel()->getSet("base");

	if (!current_set) {
		current_set = current_level->getLevel()->getSet("Base");
	}

	if (camera_manager) {
		camera_manager->setLevel(current_level->getLevel());
	}
	
	if (game_name == LIBGENS_LEVEL_GAME_STRING_GENERATIONS) {
		current_level->loadCollision(havok_enviroment, scene_manager, havok_nodes_list);
	}

	current_level->loadTerrain(scene_manager);

	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(current_level->getTerrain()->getResourcesFolder(), "FileSystem");

	// Add Terrain Resources to the UV Animation Library
	uv_animation_library->addFolder(current_level->getTerrain()->getResourcesFolder());
	havok_enviroment->addFolder(current_level->getTerrain()->getResourcesFolder());

	// Create the scene lights
	LibGens::Light *direct_light=current_level->getDirectLight();
	if (direct_light) {
		createDirectionalLight(direct_light);
	}

	LibGens::LightList *light_list=current_level->getLightList();
	if (light_list) {
		vector<LibGens::Light *> omni_lights=light_list->getOmniLights(); 

		for (size_t i=0; i<omni_lights.size(); i++) {
			LibGens::Vector3 light_position=omni_lights[i]->getPosition();
			LibGens::Vector3 light_color=omni_lights[i]->getColor();

			Ogre::Light *light = scene_manager->createLight(omni_lights[i]->getName());
			light->setDiffuseColour(Ogre::ColourValue(light_color.x, light_color.y, light_color.z));
			light->setType(Ogre::Light::LT_POINT);
			light->setPosition(Ogre::Vector3(light_position.x, light_position.y, light_position.z));
			light->setAttenuation(omni_lights[i]->getOuterRange()*2, 0, omni_lights[i]->getInnerRange(), omni_lights[i]->getOuterRange());
		}
	}

	// Create Skybox
	string skybox_name = current_level->getLevel()->getSkybox();
	if (!skybox_name.empty()) {
		createSkybox(skybox_name);
	}

	// Create Paths
	loadLevelPaths();

	// Create Terrain Streamer
	if (current_level->getTerrain()) {
		terrain_streamer = new TerrainStreamer(current_level->getTerrain(), current_level->getTerrainGIInfo(), scene_manager);
		terrain_streamer->setExternalList(&terrain_nodes_list);
		printf("Created terrain streamer for %d terrain groups.\n", current_level->getTerrain()->getGroups().size());
	}

	printf("Finished loading!\n");

	updateNodeVisibility();
}


void EditorApplication::createDirectionalLight(LibGens::Light *direct_light) {
	LibGens::Vector3 light_direction=direct_light->getPosition();
	LibGens::Vector3 light_color=direct_light->getColor();

	SONICGLVL_SHADER_LIBRARY->setGlobalLightDirection(light_direction);
	SONICGLVL_SHADER_LIBRARY->setGlobalLightColor(light_color);

	global_directional_light = scene_manager->createLight(direct_light->getName());
	global_directional_light->setSpecularColour(Ogre::ColourValue::White);
	global_directional_light->setDiffuseColour(Ogre::ColourValue(light_color.x, light_color.y, light_color.z));
	global_directional_light->setType(Ogre::Light::LT_DIRECTIONAL);
	global_directional_light->setDirection(Ogre::Vector3(-light_direction.x, -light_direction.y, -light_direction.z));

	Ogre::Light *light = axis_scene_manager->createLight(direct_light->getName()+"_viewport");
	light->setSpecularColour(Ogre::ColourValue::White);
	light->setDiffuseColour(Ogre::ColourValue(light_color.x, light_color.y, light_color.z));
	light->setType(Ogre::Light::LT_DIRECTIONAL);
	light->setDirection(Ogre::Vector3(light_direction.x, light_direction.y, light_direction.z));
}


void EditorApplication::createSkybox(string skybox_name) {
	LibGens::Model *skybox_model=current_level->getModelLibrary()->getModel(skybox_name);

	if (skybox_model) {
		Ogre::SceneNode *scene_node = scene_manager->getRootSceneNode()->createChildSceneNode();
		buildModel(scene_node, skybox_model, skybox_model->getName(), "", scene_manager, current_level->getMaterialLibrary(), 0, GENERAL_MESH_GROUP, false);

		unsigned short attached_objects=scene_node->numAttachedObjects();
		for (unsigned short i=0; i<attached_objects; i++) {
			Ogre::Entity *entity=static_cast<Ogre::Entity *>(scene_node->getAttachedObject(i));
			entity->setRenderQueueGroup(Ogre::RENDER_QUEUE_SKIES_EARLY);
			unsigned int attached_entities=entity->getNumSubEntities();

			Ogre::AxisAlignedBox aabb;
			aabb.setInfinite();
			entity->getMesh()->_setBounds(aabb, false);

			for (unsigned int j=0; j<attached_entities; j++) {
				Ogre::SubEntity *sub_entity=entity->getSubEntity(j);
				Ogre::MaterialPtr material=sub_entity->getMaterial();
				Ogre::Pass *pass=material->getTechnique(0)->getPass(0);
				pass->setDepthWriteEnabled(false);
			}
		}
	}
}


void EditorApplication::saveLevelData(string filename) {
	if (!current_level) return;
	current_level->saveData(filename);
	current_level->saveHashes();
	SHOW_MSG("Data saved!");
}

void EditorApplication::saveLevelTerrain() {
	if (!current_level) return;

	current_level->saveTerrain();
	current_level->saveResources();
	current_level->saveHashes();
	SHOW_MSG("Terrain saved!");
}


void EditorApplication::cleanLevelTerrain() {
	if (!current_level) {
		SHOW_MSG("There's no level currently loaded.");
		return;
	}

	if (terrain_streamer) {
		if (CONFIRM_MSG("To do any terrain operations you have to wait for the terrain streamer to finish first. Do you want load all the terrain?") == IDYES) {
			terrain_streamer->forceLoad();
		}
		return;
	}

	current_level->deleteTerrain();
	current_level->cleanTerrain();
	current_level->cleanTerrainResources();

	clearSelection();
	for (list<TerrainNode *>::iterator it=terrain_nodes_list.begin(); it!=terrain_nodes_list.end(); it++) {
		delete (*it);
	}
	terrain_nodes_list.clear();

	Ogre::ResourceGroupManager::getSingleton().unloadUnreferencedResourcesInGroup(GENERAL_MESH_GROUP, false);
	Ogre::MeshManager::getSingleton().unloadUnreferencedResources(false);

	SHOW_MSG("Terrain cleaned!");
}


void EditorApplication::importLevelTerrainFBX(string filename) {
	if (!current_level) return;
	if (terrain_streamer) return;

	// Import and add to Terrain Data
	LibGens::FBX *fbx=fbx_manager->importFBX(filename);
	current_level->importTerrainFBX(fbx);

	// Create scene nodes from FBX
	list<LibGens::TerrainInstance *> terrain_instances=fbx->getInstances();
	for (list<LibGens::TerrainInstance *>::iterator it=terrain_instances.begin(); it!=terrain_instances.end(); it++) {
		TerrainNode *terrain_node=new TerrainNode((*it), scene_manager, current_level->getTerrain()->getMaterialLibrary());
		terrain_nodes_list.push_back(terrain_node);
	}
}


void EditorApplication::generateTerrainGroups() {
	if (!current_level) return;

	if (terrain_streamer) {
		if (CONFIRM_MSG("To do any terrain operations you have to wait for the terrain streamer to finish first. Do you want load all the terrain?") == IDYES) {
			terrain_streamer->forceLoad();
		}
		return;
	}

	current_level->generateTerrainGroups(50);
}


void EditorApplication::reloadTemplatesDatabase() {

}


void EditorApplication::saveTemplatesDatabase() {
	if (!library) return;

	library->saveDatabase(SONICGLVL_OBJECTS_DATABASE_PATH);
}