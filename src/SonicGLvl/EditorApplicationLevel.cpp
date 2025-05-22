//=========================================================================
//	  Copyright (c) 2016 SonicGLvl
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
#include "ObjectSet.h"

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

		size_t game_mode = LIBGENS_LEVEL_GAME_GENERATIONS;
		if (game_name == LIBGENS_LEVEL_GAME_STRING_UNLEASHED) {
			game_mode = LIBGENS_LEVEL_GAME_UNLEASHED;
		}

		if ((entry_name==SONICGLVL_DATABASE_ENTRY) && level_name.size() && geometry_name.size()) {
			EditorLevelEntry *entry=new EditorLevelEntry(level_name, geometry_name, layout_merge_name, slot_name, game_mode);
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

size_t EditorLevelDatabase::getGame(string name) {
	for (size_t i=0; i<entries.size(); i++) {
		if (entries[i]->name == name) return entries[i]->game;
	}
	return LIBGENS_LEVEL_GAME_GENERATIONS;
}



void EditorApplication::openLostWorldLevel(string filename) {
	string slot_name=LibGens::File::nameFromFilenameNoExtension(filename);

	string folder = filename + "/";

	CreateDirectory((SONICGLVL_CACHE_PATH + slot_name).c_str(), NULL);

	EditorLevel *lost_world_level = new EditorLevel(folder, slot_name, slot_name, "", LIBGENS_LEVEL_GAME_LOST_WORLD);
	current_level = lost_world_level;

	lost_world_level->unpackResources();

	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(lost_world_level->getResourcesFolder(), "FileSystem");
	uv_animation_library->addFolder(lost_world_level->getResourcesFolder());

	lost_world_level->loadTerrain(scene_manager, &terrain_nodes_list);

	/*
	current_level->getTerrain()->addModels(lost_world_level->getTerrain()->getModelsToOrganize());
	current_level->getTerrain()->addInstances(lost_world_level->getTerrain()->getInstancesToOrganize());
	current_level->getTerrain()->getMaterialLibrary()->merge(lost_world_level->getMaterialLibrary(), true);
	*/
	
	LibGens::Light *direct_light=lost_world_level->getDirectLight();
	if (direct_light) {
		createDirectionalLight(direct_light);
	}

	string skybox_name = slot_name + "_sky";
	if (skybox_name.size()) {
		createSkybox(skybox_name);
	}

	terrain_streamer = NULL;
}

void EditorApplication::checkShaderLibrary(size_t game_mode) {
	if (!checked_shader_library) {
		if (game_mode == LIBGENS_LEVEL_GAME_UNLEASHED) {
			if (!unleashed_shader_library) {
				ERROR_MSG("Unable to locate Sonic Unleashed shader archives. Please copy the \"shader.ar\" file from the Xbox 360 version of the game to the \"database\\shaders\" folder.");
			}
		}
		else {
			if (!generations_shader_library) {
				ERROR_MSG("Unable to locate Sonic Generations shader archives. Please copy the following files from the 2011 Steam version of the game (located in \"bb3.cpk\") to the \"database\\shaders\" folder:\n\nshader_r.ar.00\nshader_r_add.ar.00\nshader_r_add.ar.01\nshader_r_add.ar.02");
			}
		}

		checked_shader_library = true;
	}
}

void EditorApplication::openLevel(string filename) {
	if (!level_database) {
		ERROR_MSG("You can't open any levels without a level database!");
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
	string slot_id_name=level_database->getSlot(slot_name);
	size_t game_mode=level_database->getGame(slot_name);

	if (!geometry_name.size()) geometry_name = slot_name;

	if (!level_database->exists(slot_name)) {
		INFO_MSG("This level name doesn't exist in the current database. The slot name will be used for finding the geometry.");
	}

	checkShaderLibrary(game_mode);

	CreateDirectory((SONICGLVL_CACHE_PATH + slot_name).c_str(), NULL);
	CreateDirectory((SONICGLVL_CACHE_PATH + geometry_name).c_str(), NULL);
	if (game_mode == LIBGENS_LEVEL_GAME_UNLEASHED) {
		if (!slot_id_name.empty()) {
			CreateDirectory((SONICGLVL_CACHE_PATH + slot_id_name).c_str(), NULL);
		}
		library = unleashed_library;
	}
	else {
		library = generations_library;
	}

	updateObjectCategoriesGUI();

	// clean up objects placed before a level is loaded
	clearSelection();
	for (auto node : object_node_manager->getObjectNodes())
	{
		LibGens::Object* obj = node->getObject();
		object_node_manager->deleteObjectNode(obj);
		delete obj;
	}
	history->clear();

	current_level_filename=filename;

	current_level = new EditorLevel(folder, slot_name, geometry_name, slot_id_name, game_mode);
	current_level->unpackData();
	printf("Unpacked data...\n");
	current_level->unpackResources();
	printf("Unpacked resources...\n");
	current_level->unpackTerrain();
	printf("Unpacked terrain...\n");
	current_level->saveHashes();

	object_node_manager->setSlotIdName(slot_id_name);
	current_level->loadData(library, object_node_manager);

	current_set = current_level->getLevel()->getSet("base");

	if (!current_set) {
		current_set = current_level->getLevel()->getSet("Base");
	}

	if (!current_set) {
		list<LibGens::ObjectSet *> sets = current_level->getLevel()->getSets();
		if (!sets.empty()) {
			current_set = sets.front();
		}
		else {
			LibGens::ObjectSet *set = new LibGens::ObjectSet();
			if (current_level->getGameMode() == LIBGENS_LEVEL_GAME_UNLEASHED) {
				set->setName("Base");
				set->setFilename(current_level->getLevel()->getFolder() + set->getName() + LIBGENS_OBJECT_SET_EXTENSION);
			}
			else {
				set->setName("base");
				set->setFilename(current_level->getLevel()->getFolder() + LIBGENS_OBJECT_SET_NAME + set->getName() + LIBGENS_OBJECT_SET_EXTENSION);
			}

			current_level->getLevel()->addSet(set);
			current_set = set;
		}
	}

	updateSetsGUI();
	updateSelectedSetGUI();
	updateLayerControlGUI();

	if (camera_manager) {
		camera_manager->setLevel(current_level->getLevel());
	}
	
	if ((game_mode == LIBGENS_LEVEL_GAME_GENERATIONS) || (game_mode == LIBGENS_LEVEL_GAME_UNLEASHED)) {
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
	if (skybox_name.size()) {
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

void EditorApplication::newCurrentSet() {
	if (current_level->getLevel()->getSet("rename_me")) {
		MessageBox(NULL, "Rename the object set called \"rename_me\" first before creating a new object set.", "SonicGLvl", MB_OK);
	}
	else {
		LibGens::ObjectSet *set = new LibGens::ObjectSet();
		set->setName("rename_me");
		if (current_level->getGameMode() == LIBGENS_LEVEL_GAME_UNLEASHED) {
			set->setFilename(current_level->getLevel()->getFolder() + set->getName() + LIBGENS_OBJECT_SET_EXTENSION);
		}
		else {
			set->setFilename(current_level->getLevel()->getFolder() + LIBGENS_OBJECT_SET_NAME + set->getName() + LIBGENS_OBJECT_SET_EXTENSION);
		}

		current_level->getLevel()->addSet(set);
		current_set = set;
		updateSetsGUI();
		updateSelectedSetGUI();
	}
}

void EditorApplication::deleteCurrentSet() {
	if (current_set) {
		if (current_set->getName() != LIBGENS_OBJECT_SET_BASE) {
			current_level->getLevel()->removeSet(current_set);
			delete current_set;
			current_set = current_level->getLevel()->getSet(LIBGENS_OBJECT_SET_BASE);
			updateSetsGUI();
			updateSelectedSetGUI();
		}
		else
			MessageBox(NULL, "You can't delete the base object set.", "SonicGLvl", MB_OK);
	}
}

void EditorApplication::changeCurrentSet(string change_set) {
	LibGens::ObjectSet *set = current_level->getLevel()->getSet(change_set);
	if (set) {
		current_set = set;
		updateSelectedSetGUI();
	}
	else
		MessageBox(NULL, "Could not change to that set because no set with that name exists on the level.", "SonicGLvl", MB_OK);
}

void EditorApplication::renameCurrentSet(string rename_set) {
	if (current_set && !current_level->getLevel()->getSet(rename_set)) {
		string folder = LibGens::File::folderFromFilename(current_set->getFilename());
		string new_filename = folder + LIBGENS_OBJECT_SET_NAME + rename_set + LIBGENS_OBJECT_SET_EXTENSION;
		if (current_level->getGameMode() == LIBGENS_LEVEL_GAME_UNLEASHED) {
			string new_filename = folder + rename_set + LIBGENS_OBJECT_SET_EXTENSION;
		}
		LibGens::File::remove(current_set->getFilename());
		current_set->setFilename(new_filename);
		current_set->setName(rename_set);
		current_set->saveXML(new_filename);
	}
	else {
		MessageBox(NULL, "A set with that name already exists!", "SonicGLvl", MB_OK);
		updateSetsGUI();
		updateSelectedSetGUI();
	}
}

void EditorApplication::updateCurrentSetVisible(bool v) {
	if (current_set) {
		set_visibility[current_set] = v;
		object_node_manager->updateSetVisibility(current_set, v);
	}
}

void EditorApplication::createDirectionalLight(LibGens::Light *direct_light) {
	LibGens::Vector3 light_direction=direct_light->getPosition();
	LibGens::Vector3 light_color=direct_light->getColor();

	global_directional_light = scene_manager->createLight(direct_light->getName());
	global_directional_light->setSpecularColour(Ogre::ColourValue::White);
	global_directional_light->setDiffuseColour(Ogre::ColourValue(light_color.x, light_color.y, light_color.z));
	global_directional_light->setType(Ogre::Light::LT_DIRECTIONAL);

	// Ogre expects light direction to be inverted as opposed to the game shaders.
	if (SONICGLVL_SHADER_LIBRARY) {
		global_directional_light->setDirection(Ogre::Vector3(-light_direction.x, -light_direction.y, -light_direction.z));
	}
	else {
		global_directional_light->setDirection(Ogre::Vector3(light_direction.x, light_direction.y, light_direction.z));
	}

	Ogre::Light *light = axis_scene_manager->createLight(direct_light->getName()+"_viewport");
	light->setSpecularColour(Ogre::ColourValue::White);
	light->setDiffuseColour(Ogre::ColourValue(light_color.x, light_color.y, light_color.z));
	light->setType(Ogre::Light::LT_DIRECTIONAL);
	light->setDirection(Ogre::Vector3(light_direction.x, light_direction.y, light_direction.z));
}


void EditorApplication::createSkybox(string skybox_name) {
	if (!SONICGLVL_SHADER_LIBRARY) {
		return;
	}

	LibGens::Model *skybox_model=current_level->getModelLibrary()->getModel(skybox_name);
	if (skybox_model) {
		Ogre::SceneNode *scene_node = scene_manager->getRootSceneNode()->createChildSceneNode();
		buildModel(scene_node, skybox_model, skybox_model->getName(), "", scene_manager, current_level->getMaterialLibrary(), 0, GENERAL_MESH_GROUP, false, SONICGLVL_SHADER_LIBRARY);


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
	INFO_MSG("Data saved!");
}

void EditorApplication::saveLevelResources() {
	if (!current_level) return;
	current_level->saveResources();
	current_level->saveHashes();
	INFO_MSG("Resources saved!");
}

void EditorApplication::saveLevelTerrain() {
	if (!current_level) return;

	current_level->saveTerrain();
	current_level->saveResources();
	current_level->saveHashes();
	INFO_MSG("Terrain saved!");
}


void EditorApplication::cleanLevelTerrain() {
	if (!current_level) {
		INFO_MSG("There's no level currently loaded.");
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

	INFO_MSG("Terrain cleaned!");
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

	library->saveDatabase(library == unleashed_library ? SONICGLVL_UNLEASHED_OBJECTS_DATABASE_PATH : SONICGLVL_GENERATIONS_OBJECTS_DATABASE_PATH);
}