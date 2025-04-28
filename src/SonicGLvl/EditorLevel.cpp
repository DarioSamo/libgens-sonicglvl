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
#include "AR.h"
#include "PAC.h"
#include "ObjectLibrary.h"
#include "ObjectSet.h"

EditorLevel::EditorLevel(string folder_p, string slot_name_p, string geometry_name_p, string slot_id_name_p, size_t game_mode_p) {
	folder = folder_p;
	slot_name = slot_name_p;
	geometry_name = geometry_name_p;
	slot_id_name = slot_id_name_p;
	game_mode = game_mode_p;

	level=NULL;
	terrain=NULL;
	terrain_gi_info=NULL;
	direct_light = NULL;
	terrain_autodraw=NULL;

	cache_folder				   = SONICGLVL_CACHE_PATH + slot_name + "/";
	data_cache_folder			   = SONICGLVL_CACHE_PATH + slot_name + "/" + SONICGLVL_CACHE_DATA_PATH;
	gi_cache_folder				   = SONICGLVL_CACHE_PATH + slot_name + "/" + SONICGLVL_CACHE_GI_TEMP_PATH;
	terrain_cache_folder		   = SONICGLVL_CACHE_PATH + geometry_name + "/" + SONICGLVL_CACHE_TERRAIN_PATH;
	resources_cache_folder		   = SONICGLVL_CACHE_PATH + geometry_name + "/" + SONICGLVL_CACHE_RESOURCES_PATH;
	slot_resources_cache_folder	   = SONICGLVL_CACHE_PATH + slot_id_name + "/" + SONICGLVL_CACHE_SLOT_RESOURCES_PATH;

	model_library    = new LibGens::ModelLibrary(resources_cache_folder + "/");
	material_library = NULL;

	loadHashes();
}


void EditorLevel::loadHashes() {
	data_hash = {};
	terrain_hash = {};
	resources_hash = {};

	TiXmlDocument doc(cache_folder + SONICGLVL_LEVEL_HASH_FILENAME);
	if (!doc.LoadFile()) {
		return;
	}

	TiXmlHandle hDoc(&doc);
	TiXmlElement* pElem;
	TiXmlHandle hRoot(0);

	pElem=hDoc.FirstChildElement().Element();
	if (!pElem) {
		return;
	}

	pElem=pElem->FirstChildElement();
	for(pElem; pElem; pElem=pElem->NextSiblingElement()) {
		string entry_name="";

		entry_name = pElem->ValueStr();

		int *hash_pointer = NULL;
		if (entry_name==SONICGLVL_LEVEL_HASH_DATA)      hash_pointer = (int *) &data_hash;
		if (entry_name==SONICGLVL_LEVEL_HASH_TERRAIN)   hash_pointer = (int *) &terrain_hash;
		if (entry_name==SONICGLVL_LEVEL_HASH_RESOURCES) hash_pointer = (int *) &resources_hash;

		if (hash_pointer) {
			for (size_t i=0; i<4; i++) {
				pElem->QueryIntAttribute(SONICGLVL_LEVEL_HASH_VALUE_ATTRIBUTE + ToString(i), &hash_pointer[i]);
			}
		}
	}
}


void EditorLevel::saveHashes() {
	TiXmlDocument doc;
	TiXmlDeclaration *decl = new TiXmlDeclaration( "1.0", "", "" );
	doc.LinkEndChild( decl );

	TiXmlElement *hashRoot = new TiXmlElement(SONICGLVL_LEVEL_HASH_ROOT);

	TiXmlElement *dataRoot = new TiXmlElement(SONICGLVL_LEVEL_HASH_DATA);
	for (size_t i=0; i<4; i++) {
		dataRoot->SetAttribute(SONICGLVL_LEVEL_HASH_VALUE_ATTRIBUTE + ToString(i), ((int*)&data_hash)[i]);
	}
	hashRoot->LinkEndChild(dataRoot);

	TiXmlElement *terrainRoot = new TiXmlElement(SONICGLVL_LEVEL_HASH_TERRAIN);
	for (size_t i=0; i<4; i++) {
		terrainRoot->SetAttribute(SONICGLVL_LEVEL_HASH_VALUE_ATTRIBUTE + ToString(i), ((int*)&terrain_hash)[i]);
	}
	hashRoot->LinkEndChild(terrainRoot);

	TiXmlElement *resourcesRoot = new TiXmlElement(SONICGLVL_LEVEL_HASH_RESOURCES);
	for (size_t i=0; i<4; i++) {
		resourcesRoot->SetAttribute(SONICGLVL_LEVEL_HASH_VALUE_ATTRIBUTE + ToString(i), ((int*)&resources_hash)[i]);
	}
	hashRoot->LinkEndChild(resourcesRoot);

	doc.LinkEndChild(hashRoot);
	doc.SaveFile(cache_folder + SONICGLVL_LEVEL_HASH_FILENAME);
}


void EditorLevel::cleanData() {
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	hFind = FindFirstFile((data_cache_folder+"/*.*").c_str(), &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) {} 
	else {
		do {
			const char *name=FindFileData.cFileName;
			if (name[0]=='.') continue;

			string new_filename=data_cache_folder+"/"+ToString(name);
			remove(new_filename.c_str());
		} while (FindNextFile(hFind, &FindFileData) != 0);
		FindClose(hFind);
	}
	return;
}

void EditorLevel::unpackData() {
	string main_filename=folder + "#" + slot_name + ".ar.00";

	LibGens::ArPack *level_data_ar_pack=new LibGens::ArPack(main_filename);
	XXH128_hash_t hash = level_data_ar_pack->computeHash();
	bool unpack=!XXH128_isEqual(hash, data_hash);

	if (unpack) {
		cleanData();
		CreateDirectory(data_cache_folder.c_str(), NULL);
		level_data_ar_pack->extract(data_cache_folder+"/");
		data_hash = hash;
	}
	delete level_data_ar_pack;
}


void EditorLevel::deleteTerrain() {
	if (terrain) {
		terrain->clean();
	}

	if (terrain_block) {
		terrain_block->clean();
	}

	if (terrain_gi_info) {
		terrain_gi_info->clean();
	}
}

void EditorLevel::cleanTerrain() {
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	hFind = FindFirstFile((terrain_cache_folder+"/*.*").c_str(), &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) {} 
	else {
		do {
			const char *name=FindFileData.cFileName;
			if (name[0]=='.') continue;

			string new_filename=terrain_cache_folder+"/"+ToString(name);
			remove(new_filename.c_str());
		} while (FindNextFile(hFind, &FindFileData) != 0);
		FindClose(hFind);
	}
	return;
}

void EditorLevel::cleanGI() {
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	hFind = FindFirstFile((gi_cache_folder+"/*.*").c_str(), &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) {} 
	else {
		do {
			const char *name=FindFileData.cFileName;
			if (name[0]=='.') continue;

			string new_filename=gi_cache_folder+"/"+ToString(name);
			remove(new_filename.c_str());
		} while (FindNextFile(hFind, &FindFileData) != 0);
		FindClose(hFind);
	}
	return;
}

void EditorLevel::cleanTerrainResources() {
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	hFind = FindFirstFile((resources_cache_folder+"/*.*").c_str(), &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) {} 
	else {
		do {
			const char *name=FindFileData.cFileName;
			if (name[0]=='.') continue;

			string new_filename=resources_cache_folder+"/"+ToString(name);
			bool delete_file=false;

			if (new_filename.find(LIBGENS_TERRAIN_GROUP_EXTENSION) != string::npos) {
				delete_file = true;
			}

			if (new_filename.find(LIBGENS_TERRAIN_EXTENSION) != string::npos) {
				delete_file = true;
			}

			if (new_filename.find(LIBGENS_TERRAIN_BLOCK_FILENAME) != string::npos) {
				delete_file = true;
			}

			if (delete_file) remove(new_filename.c_str());
		} while (FindNextFile(hFind, &FindFileData) != 0);
		FindClose(hFind);
	}
	return;
}

void EditorLevel::unpackTerrain() {
	string main_filename=folder + SONICGLVL_LEVEL_PACKED_FOLDER + "/" + geometry_name + "/" + SONICGLVL_LEVEL_PACKED_STAGE;
	string main_add_filename=folder + SONICGLVL_LEVEL_PACKED_FOLDER + "/" + geometry_name + "/" + SONICGLVL_LEVEL_PACKED_STAGE_ADD;
	if (game_mode == LIBGENS_LEVEL_GAME_UNLEASHED) {
		main_add_filename = folder + SONICGLVL_LEVEL_ADDITIONAL_FOLDER + "/" + geometry_name + "/" + SONICGLVL_LEVEL_PACKED_STAGE_ADD;
	}

	LibGens::ArPack *stage_data_ar_pack=new LibGens::ArPack(main_filename);
	printf("Opened %s\n", main_filename.c_str());
	LibGens::ArPack *stage_add_data_ar_pack=new LibGens::ArPack(main_add_filename);
	has_additional_gi = stage_add_data_ar_pack->getFileCount() != 0;
	printf("Opened %s\n", main_add_filename.c_str());
	stage_data_ar_pack->merge(stage_add_data_ar_pack);
	printf("Merged AR Packs\n");

	XXH128_hash_t hash = stage_data_ar_pack->computeHash();
	bool unpack=!XXH128_isEqual(hash, terrain_hash);

	if (unpack) {
		if (MessageBox(NULL, "Do you want to unpack the terrain?", "SonicGLvl", MB_YESNO) != IDYES) unpack=false;
	}

	if (unpack) {
		cleanTerrain();
		cleanGI();
		CreateDirectory(terrain_cache_folder.c_str(), NULL);
		CreateDirectory(gi_cache_folder.c_str(), NULL);

		vector<string> unpacked_files;
		stage_data_ar_pack->extract(terrain_cache_folder + "/", "", "", &unpacked_files);

		for (size_t i=0; i<unpacked_files.size(); i++) {
			string uncompressed_filename=unpacked_files[i];

			if (uncompressed_filename.find("gia-") != string::npos) {
				printf("Extracting %s\n", uncompressed_filename.c_str());

				LibGens::ArPack *gia_ar_pack=new LibGens::ArPack(uncompressed_filename);
				gia_ar_pack->extract(gi_cache_folder + "/", "", stage_data_ar_pack->getFileByIndex(i)->getName() + "-");
				delete gia_ar_pack;
			}
		}

		terrain_hash = hash;
	}
	delete stage_data_ar_pack;
}

void EditorLevel::cleanResources() {
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	hFind = FindFirstFile((resources_cache_folder+"/*.*").c_str(), &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) {} 
	else {
		do {
			const char *name=FindFileData.cFileName;
			if (name[0]=='.') continue;

			string new_filename=resources_cache_folder+"/"+ToString(name);
			remove(new_filename.c_str());
		} while (FindNextFile(hFind, &FindFileData) != 0);
		FindClose(hFind);
	}
	return;
}

void EditorLevel::unpackResources() {
	if (game_mode == LIBGENS_LEVEL_GAME_LOST_WORLD) {
		string main_filename=folder + slot_name;
		string trr_cmn_filename = main_filename + "_trr_cmn.pac";
		string sky_cmn_filename = main_filename + "_sky.pac";

		CreateDirectory(resources_cache_folder.c_str(), NULL);

		if (LibGens::File::check(trr_cmn_filename)) {
			LibGens::PacSet *pac_set = new LibGens::PacSet(trr_cmn_filename);
			pac_set->extract(resources_cache_folder + "/", true);
			delete pac_set;
		}

		if (LibGens::File::check(sky_cmn_filename)) {
			LibGens::PacSet *pac_set = new LibGens::PacSet(sky_cmn_filename);
			pac_set->extract(resources_cache_folder + "/", true);
			delete pac_set;
		}
	}
	else {
		string main_filename=folder + SONICGLVL_LEVEL_PACKED_FOLDER + "/" + geometry_name + "/" + geometry_name + LIBGENS_AR_MULTIPLE_START;
		if (game_mode == LIBGENS_LEVEL_GAME_UNLEASHED) {
			main_filename = folder + geometry_name + LIBGENS_AR_MULTIPLE_START;
		}

		LibGens::ArPack *resources_data_ar_pack=new LibGens::ArPack(main_filename);
		XXH128_hash_t hash = resources_data_ar_pack->computeHash();
		bool unpack=!XXH128_isEqual(hash, resources_hash);
		bool unpack_slot_resources = (game_mode == LIBGENS_LEVEL_GAME_UNLEASHED && !slot_id_name.empty());

		if (unpack) {
			cleanResources();
			CreateDirectory(resources_cache_folder.c_str(), NULL);
			resources_data_ar_pack->extract(resources_cache_folder + "/");

			if (unpack_slot_resources) {
				CreateDirectory(slot_resources_cache_folder.c_str(), NULL);

				WIN32_FIND_DATA FindFileData;
				HANDLE hFind;
				hFind = FindFirstFile((folder + "Cmn*" + slot_id_name + "*.ar.00").c_str(), &FindFileData);
				if (hFind == INVALID_HANDLE_VALUE) {}
				else {
					do {
						const char* name = FindFileData.cFileName;
						if (name[0] == '.') continue;

						LibGens::ArPack slot_resources_data_ar_pack(folder + name);
						slot_resources_data_ar_pack.extract(slot_resources_cache_folder + "/");

					} while (FindNextFile(hFind, &FindFileData) != 0);
					FindClose(hFind);
				}
			}

			resources_hash = hash;
		}
		delete resources_data_ar_pack;

		if (unpack_slot_resources) {
			Ogre::ResourceGroupManager::getSingleton().addResourceLocation(slot_resources_cache_folder, "FileSystem");
		}
	}
}




void EditorLevel::loadData(LibGens::ObjectLibrary *library, ObjectNodeManager *object_node_manager) {
	level = new LibGens::Level(data_cache_folder + "/", game_mode);

	// Fix anything inside the level to fit with the library
	level->learnFromLibrary(library);

	// Add any new templates from the level to the library
	library->learnFromLevel(level, library->getCategory(SONICGLVL_UNASSIGNED_OBJECT_CATEGORY));

	list<LibGens::ObjectSet *> sets=level->getSets();
	for (list<LibGens::ObjectSet *>::iterator set=sets.begin(); set!=sets.end(); set++) {
		list<LibGens::Object *> objects=(*set)->getObjects();

		for (list<LibGens::Object *>::iterator it=objects.begin(); it!=objects.end(); it++) {
			object_node_manager->createObjectNode(*it);
		}
	}
}

void EditorLevel::cleanCollision(list<HavokNode *> &havok_nodes_list) {
	for (list<HavokNode *>::iterator it=havok_nodes_list.begin(); it!=havok_nodes_list.end(); it++) {
		delete (*it);
	}
	havok_nodes_list.clear();
}

void EditorLevel::loadCollision(LibGens::HavokEnviroment *havok_enviroment, Ogre::SceneManager *scene_manager, list<HavokNode *> &havok_nodes_list) {
	if (!level) return;

	list<LibGens::LevelCollisionEntry *> collision_entries=level->getCollisionEntries();
	for (list<LibGens::LevelCollisionEntry *>::iterator it=collision_entries.begin(); it!=collision_entries.end(); it++) {
		havok_enviroment->addFolder(data_cache_folder + "/");
		
		LibGens::HavokPhysicsCache *physics_cache = havok_enviroment->getPhysics((*it)->name);

		if (physics_cache) {
			createHavokNodes(physics_cache, scene_manager, havok_nodes_list);
		}
	}
}


void EditorLevel::createHavokNodes(LibGens::HavokPhysicsCache *physics_cache, Ogre::SceneManager *scene_manager, list<HavokNode *> &havok_nodes_list) {
	hkpPhysicsData *physics_data = physics_cache->getPhysics();

	if (physics_data) {
		cout << "[+] Writing physics data..." << endl;
		const hkArray<hkpPhysicsSystem*> &systems = physics_data->getPhysicsSystems();
		cout << "# Physics Data"  << endl;
		cout << systems.getSize() << "# No. of Physics Systems" << endl;

		for (int i = 0; i < systems.getSize(); i++) {
			// Dump Physics System
			cout << "[+] Dumping physics system #" << (i + 1);

			if (systems[i]->getName() != NULL) {
				cout << " (" << systems[i]->getName() << ")";
			}

			cout << endl;

			const hkArray<hkpRigidBody*> &rigidbodies = systems[i]->getRigidBodies();
			const hkArray<hkpPhantom*> &phantoms      = systems[i]->getPhantoms();
    
			cout << "# Physics System" << endl;
			cout << rigidbodies.getSize() << "# No. of Rigidbodies" << endl;
			cout << phantoms.getSize()    << "# No. of Phantoms" << endl;
		
    
			for (int j = 0; j < rigidbodies.getSize(); j++) {
				// Dump Rigid Body
				hkpRigidBodyCinfo info;
				rigidbodies[j]->getCinfo(info);

				hkTransform transform=rigidbodies[j]->getTransform();
				hkGeometry *geometry=hkpShapeConverter::toSingleGeometry(info.m_shape);

				LibGens::Matrix4 matrix( transform(0, 0), transform(0, 1), transform(0, 2), transform(0, 3),
										 transform(1, 0), transform(1, 1), transform(1, 2), transform(1, 3),
										 transform(2, 0), transform(2, 1), transform(2, 2), transform(2, 3),
										 transform(3, 0), transform(3, 1), transform(3, 2), transform(3, 3));


				// Create Visual Editor Node
				HavokNode *havok_node=new HavokNode(rigidbodies[j]->getName(), geometry, matrix, scene_manager);
				havok_nodes_list.push_back(havok_node);
			}
		}
	}
}


void EditorLevel::loadTerrain(Ogre::SceneManager *scene_manager, list<TerrainNode *> *terrain_nodes_list) {
	if (game_mode == LIBGENS_LEVEL_GAME_LOST_WORLD) {
		string terrain_data_folder = resources_cache_folder;

		terrain          = new LibGens::Terrain();

		direct_light     = new LibGens::Light(terrain_data_folder + "/" + "Direct01.light");

		printf("Terrain Folder: %s\n", terrain_data_folder.c_str());

		material_library = new LibGens::MaterialLibrary(terrain_data_folder + "/");

		// Search for model files
		vector<LibGens::Model *> terrain_models;
		{
			WIN32_FIND_DATA FindFileData;
			HANDLE hFind;
			hFind = FindFirstFile((terrain_data_folder+"/*.terrain-model").c_str(), &FindFileData);
			if (hFind == INVALID_HANDLE_VALUE) {} 
			else {
				do {
					const char *name=FindFileData.cFileName;
					if (name[0]=='.') continue;

					string new_filename=resources_cache_folder+"/"+ToString(name);
					LibGens::Model *model = new LibGens::Model(new_filename);
					terrain_models.push_back(model);

					//model->changeVertexFormat(LIBGENS_VERTEX_FORMAT_PC);
					terrain->addModel(model);
				} while (FindNextFile(hFind, &FindFileData) != 0);
				FindClose(hFind);
			}
		}

		vector<LibGens::Model *> used_models;
		// Search for instance files
		{
			WIN32_FIND_DATA FindFileData;
			HANDLE hFind;
			hFind = FindFirstFile((terrain_data_folder+"/*.terrain-instanceinfo").c_str(), &FindFileData);
			if (hFind == INVALID_HANDLE_VALUE) {} 
			else {
				do {
					const char *name=FindFileData.cFileName;
					if (name[0]=='.') continue;

					string new_filename=resources_cache_folder+"/"+ToString(name);
					LibGens::TerrainInstance *instance = new LibGens::TerrainInstance(new_filename, ToString(name), &terrain_models);

					// Add to scene
					TerrainNode *terrain_node=new TerrainNode(instance, scene_manager, material_library);
					terrain_node->setGIQualityLevel(NULL, 0);
					//if (terrain_nodes_list) terrain_nodes_list->push_back(terrain_node);

					used_models.push_back(instance->getModel());
					terrain_nodes_list->push_back(terrain_node);

					terrain->addInstance(instance);

				} while (FindNextFile(hFind, &FindFileData) != 0);
				FindClose(hFind);
			}
		}

		for (size_t i=0; i<terrain_models.size(); i++) {
			bool found=false;

			for (size_t j=0; j<used_models.size(); j++) {
				if (terrain_models[i] == used_models[j]) {
					found = true;
					break;
				}
			}

			if (!found) {
				LibGens::TerrainInstance *instance = new LibGens::TerrainInstance(terrain_models[i]->getName(), terrain_models[i], LibGens::Matrix4());
				instance->setFilename(terrain_data_folder + "/" + terrain_models[i]->getName()+".terrain-instanceinfo");

				
				// Add to scene
				TerrainNode *terrain_node=new TerrainNode(instance, scene_manager, material_library);
				terrain_node->setGIQualityLevel(NULL, 0);
				terrain_nodes_list->push_back(terrain_node);

				terrain->addInstance(instance);
			}
		}
	}
	else {
		if (!level) return;

		string terrain_data_folder = resources_cache_folder;

		// Terrain-related data files are stored in the data folder on Unleashed
		if (game_mode == LIBGENS_LEVEL_GAME_UNLEASHED) {
			terrain_data_folder = data_cache_folder;
		}

		string ghost_filename = data_cache_folder + "/" + slot_name + LIBGENS_GHOST_EXTENSION;
		string terrain_filename    = terrain_data_folder + "/" + level->getTerrainInfo() + LIBGENS_TERRAIN_EXTENSION;
		string block_filename      = terrain_data_folder + "/" + LIBGENS_TERRAIN_BLOCK_FILENAME;
		string light_list_filename = terrain_data_folder + "/" + LIBGENS_LIGHT_LIST_FILENAME;
		string groups_folder       = terrain_data_folder + "/";
		string gi_info_filename    = terrain_data_folder + "/" + LIBGENS_GI_TEXTURE_GROUP_INFO_FILE;

		string autodraw_filename   = resources_cache_folder + "/" + LIBGENS_TERRAIN_AUTODRAW_TXT;

		terrain          = new LibGens::Terrain(terrain_filename, groups_folder, resources_cache_folder + "/", terrain_cache_folder + "/", gi_cache_folder + "/", false);
		terrain_gi_info  = new LibGens::GITextureGroupInfo(gi_info_filename, terrain_cache_folder + "/");
		terrain_block    = new LibGens::TerrainBlock(block_filename);
		light_list       = new LibGens::LightList(light_list_filename);
		terrain_autodraw = new LibGens::TerrainAutodraw(autodraw_filename);
		ghost    	     = new LibGens::Ghost(ghost_filename);

		material_library = terrain->getMaterialLibrary();

		if (light_list) {
			direct_light     = light_list->getLight(level->getDirectLight());
		}
	}
}

void EditorLevel::importTerrainFBX(LibGens::FBX *fbx) {
	if (!terrain) return;

	// Merge material libraries
	if (fbx->getMaterialLibrary()) {
		terrain->getMaterialLibrary()->merge(fbx->getMaterialLibrary(), true);
		fbx->setMaterialLibrary(NULL);
	}

	// Add unorganized instances and models for group generations
	list<LibGens::TerrainInstance *> terrain_instances=fbx->getInstances();
	for (list<LibGens::TerrainInstance *>::iterator it=terrain_instances.begin(); it!=terrain_instances.end(); it++) {
		terrain->addInstance(*it);
	}

	list<LibGens::Model *> terrain_models=fbx->getModels();
	for (list<LibGens::Model *>::iterator it=terrain_models.begin(); it!=terrain_models.end(); it++) {
		terrain->addModel(*it);
	}
}


void EditorLevel::saveData(string filename) {
	if (!level) return;

	level->saveSpawn();

	list<LibGens::ObjectSet *> sets=level->getSets();
	for (list<LibGens::ObjectSet *>::iterator set=sets.begin(); set!=sets.end(); set++) {
		(*set)->saveXML((*set)->getFilename());
	}

	LibGens::ArPack *data_ar_pack=new LibGens::ArPack(data_cache_folder + "/");
	data_ar_pack->save(filename);
	data_hash = data_ar_pack->computeHash();
	delete data_ar_pack;
}


void EditorLevel::saveTerrain() {
	if (!level) return;
	if (!terrain) return;

	if (terrain) {
		string filename = resources_cache_folder + "/" + level->getTerrainInfo() + LIBGENS_TERRAIN_EXTENSION;
		terrain->save(filename);

		vector<LibGens::TerrainGroup*> terrain_groups = terrain->getGroups();
		for (vector<LibGens::TerrainGroup*>::iterator it = terrain_groups.begin(); it != terrain_groups.end(); it++) {
			string filename = resources_cache_folder + "/" + (*it)->getName() + LIBGENS_TERRAIN_GROUP_EXTENSION;
			(*it)->save(filename);
		}
	}

	if (terrain_block) {
		string filename = resources_cache_folder + "/" + LIBGENS_TERRAIN_BLOCK_FILENAME;
		terrain_block->save(filename);
	}

	if (terrain_gi_info) {
		string filename = resources_cache_folder + "/" + LIBGENS_GI_TEXTURE_GROUP_INFO_FILE;
		terrain_gi_info->save(filename);
	}

	// Pack Stage.pfd and Stage-Add.pfd
	string main_filename=folder + SONICGLVL_LEVEL_PACKED_FOLDER + "/" + geometry_name + "/" + SONICGLVL_LEVEL_PACKED_STAGE;
	string main_add_filename=folder + SONICGLVL_LEVEL_PACKED_FOLDER + "/" + geometry_name + "/" + SONICGLVL_LEVEL_PACKED_STAGE_ADD;

	vector<string> stage_files;
	vector<string> stage_add_files;
	vector<LibGens::TerrainGroup *> terrain_groups = terrain->getGroups();
	for (vector<LibGens::TerrainGroup *>::iterator it=terrain_groups.begin(); it!=terrain_groups.end(); it++) {
		string filename=terrain_cache_folder + "/" + (*it)->getName() + LIBGENS_TERRAIN_GROUP_FOLDER_EXTENSION;
		(*it)->savePack(filename);
		stage_files.push_back(filename);
	}

	vector<LibGens::GITextureGroup *> gi_groups=terrain_gi_info->getGroups();
	for (vector<LibGens::GITextureGroup *>::iterator it=gi_groups.begin(); it!=gi_groups.end(); it++) {
		if ((*it)->getQualityLevel() == LIBGENS_GI_TEXTURE_GROUP_LOWEST_QUALITY) {
			stage_files.push_back(terrain_cache_folder+"/"+(*it)->getFilename());
		}
		else {
			stage_add_files.push_back(terrain_cache_folder+"/"+(*it)->getFilename());
		}
	}

	LibGens::ArPack *stage_ar_pack=new LibGens::ArPack();
	for (size_t i=0; i<stage_files.size(); i++) {
		string internal_name=stage_files[i];
		string target=internal_name+".cab";
		string command="makecab /D CompressionType=LZX /D CompressionMemory=18 \"" + stage_files[i] + "\" \"" + target + "\"";
		system(command.c_str());
		stage_ar_pack->addFile(target, internal_name);
	}
	stage_ar_pack->save(main_filename, 0x800);
	stage_ar_pack->savePFI(resources_cache_folder + "/" + "Stage.pfi");
	terrain_hash = stage_ar_pack->computeHash();
	delete stage_ar_pack;


	LibGens::ArPack *stage_add_ar_pack=new LibGens::ArPack();
	for (size_t i=0; i<stage_add_files.size(); i++) {
		string internal_name=stage_add_files[i];
		string target=internal_name+".cab";
		string command="makecab /D CompressionType=LZX /D CompressionMemory=18 \"" + stage_add_files[i] + "\" \"" + target + "\"";
		system(command.c_str());
		stage_add_ar_pack->addFile(target, internal_name);
	}
	stage_add_ar_pack->save(main_add_filename, 0x800);
	stage_add_ar_pack->savePFI(resources_cache_folder + "/" + "Stage-Add.pfi");
	delete stage_add_ar_pack;
}

void EditorLevel::saveResources() {
	if (!level) return;

	if (terrain) {
		LibGens::MaterialLibrary *terrain_material_library = terrain->getMaterialLibrary();
		if (terrain_material_library) {
			int root_type = LIBGENS_MATERIAL_ROOT_GENERATIONS;
			if (game_mode == LIBGENS_LEVEL_GAME_UNLEASHED) {
				root_type = LIBGENS_MATERIAL_ROOT_UNLEASHED;
			}
			terrain_material_library->save(resources_cache_folder + "/", root_type);
		}
	}

	string main_filename=folder + SONICGLVL_LEVEL_PACKED_FOLDER + "/" + geometry_name + "/" + geometry_name + LIBGENS_AR_MULTIPLE_START;
	if (game_mode == LIBGENS_LEVEL_GAME_UNLEASHED) {
		main_filename = folder + geometry_name + LIBGENS_AR_MULTIPLE_START;
	}

	LibGens::ArPack *data_ar_pack=new LibGens::ArPack(resources_cache_folder + "/");
	data_ar_pack->save(main_filename);

	resources_hash = data_ar_pack->computeHash();

	delete data_ar_pack;
}


void EditorLevel::generateTerrainGroups(unsigned int cell_size) {
	if (!terrain) return;

	terrain->generateGroups(cell_size);
}
