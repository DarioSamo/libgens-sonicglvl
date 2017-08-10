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

#ifndef EDITOR_LEVEL_H_INCLUDED
#define EDITOR_LEVEL_H_INCLUDED

#include "TerrainBlock.h"
#include "Light.h"
#include "FBX.h"
#include "HavokEnviroment.h"
#include "HavokPhysicsCache.h"

#define SONICGLVL_DATABASE_ERROR_FILE          "No valid level database file found: "
#define SONICGLVL_DATABASE_ERROR_FILE_ROOT     "Level database file doesn't have a valid root."
#define SONICGLVL_DATABASE_ENTRY               "Entry"
#define SONICGLVL_DATABASE_NAME_ATTRIBUTE      "name"
#define SONICGLVL_DATABASE_FILENAME_ATTRIBUTE  "filename"
#define SONICGLVL_DATABASE_ID_ATTRIBUTE        "id"
#define SONICGLVL_DATABASE_GEOMETRY_ATTRIBUTE  "geometry"
#define SONICGLVL_DATABASE_MERGE_ATTRIBUTE     "layout_merge"
#define SONICGLVL_DATABASE_SLOT_ATTRIBUTE      "slot"
#define SONICGLVL_DATABASE_GAME_ATTRIBUTE      "game"

#define SONICGLVL_LEVEL_PACKED_FOLDER          "Packed"
#define SONICGLVL_LEVEL_PACKED_STAGE           "Stage.pfd"
#define SONICGLVL_LEVEL_PACKED_STAGE_ADD       "Stage-Add.pfd"
#define SONICGLVL_LEVEL_PACKED_COMPRESSED      ".cab"
#define SONICGLVL_LEVEL_PACKED_UNCOMPRESSED    ".pre"

#define SONICGLVL_LEVEL_CAB_PACKED_COMMAND     "%WINDIR%\\System32\\expand"
#define SONICGLVL_LEVEL_XBOX_PACKED_COMMAND    "xbdecompress"

#define SONICGLVL_LEVEL_HASH_ROOT              "Hashes"
#define SONICGLVL_LEVEL_HASH_DATA              "Data"
#define SONICGLVL_LEVEL_HASH_TERRAIN           "Terrain"
#define SONICGLVL_LEVEL_HASH_RESOURCES         "Resources"
#define SONICGLVL_LEVEL_HASH_VALUE_ATTRIBUTE   "hash-"
#define SONICGLVL_LEVEL_HASH_FILENAME          "Hashes.xml"


class EditorLevelEntry;
class EditorLevelDatabase;

class EditorLevelEntry {
	friend class EditorLevelDatabase;

	protected:
		string name;
		string layout_merge;
		string geometry;
		string slot;
		string game;
	public:
		EditorLevelEntry(string name_p, string geometry_p, string layout_merge_p, string slot_p, string game_p) {
			name = name_p;
			geometry = geometry_p;
			layout_merge = layout_merge_p;
			slot = slot_p;
			game = game_p;
		}
};

class EditorLevelDatabase {
	protected:
		vector<EditorLevelEntry *> entries;
	public:
		EditorLevelDatabase(string filename);

		bool exists(string name);
		string getGeometryPath(string name);
		string getMergePath(string name);
		string getSlot(string name);
		string getGame(string name);
};

class EditorLevel {
	protected:
		LibGens::Level *level;
		LibGens::Terrain *terrain;
		LibGens::TerrainBlock *terrain_block;
		LibGens::GITextureGroupInfo *terrain_gi_info;
		LibGens::LightList *light_list;
		LibGens::Light *direct_light;
		LibGens::ModelLibrary *model_library;
		LibGens::MaterialLibrary *material_library;
		LibGens::TerrainAutodraw *terrain_autodraw;

		string folder;
		string slot_name; 
		string geometry_name;
		string merge_name;
		string game_name;

		string cache_folder;
		string data_cache_folder;
		string gi_cache_folder;
		string terrain_cache_folder;
		string resources_cache_folder;

		unsigned int data_hash[5];
		unsigned int terrain_hash[5];
		unsigned int resources_hash[5];
	public:
		EditorLevel(string folder_p, string slot_name_p, string geometry_name_p, string merge_name_p, string game_name_p);

		void cleanData();
		void unpackData();

		void deleteTerrain();
		void cleanTerrain();
		void cleanGI();
		void unpackTerrain();

		void cleanResources();
		void cleanTerrainResources();
		void unpackResources();

		void loadHashes();
		void saveHashes();

		string getDataCacheFolder() {
			return data_cache_folder;
		}

		string getResourcesFolder() {
			return resources_cache_folder;
		}

		void loadData(LibGens::ObjectLibrary *library, ObjectNodeManager *object_node_manager);
		void loadCollision(LibGens::HavokEnviroment *havok_enviroment, Ogre::SceneManager *scene_manager, list<HavokNode *> &havok_nodes_list);
		void loadTerrain(Ogre::SceneManager *scene_manager, list<TerrainNode *> *terrain_nodes_list=NULL);
		void cleanCollision(list<HavokNode *> &havok_nodes_list);

		void createHavokNodes(LibGens::HavokPhysicsCache *physics_cache, Ogre::SceneManager *scene_manager, list<HavokNode *> &havok_nodes_list);

		void importTerrainFBX(LibGens::FBX *fbx);

		void saveData(string filename);
		void saveTerrain();
		void saveResources();

		void generateTerrainGroups(unsigned int cell_size);

		LibGens::Level *getLevel() {
			return level;
		}

		LibGens::Terrain *getTerrain() {
			return terrain;
		}

		LibGens::GITextureGroupInfo *getTerrainGIInfo() {
			return terrain_gi_info;
		}

		LibGens::Light *getDirectLight() {
			return direct_light;
		}

		LibGens::LightList *getLightList() {
			return light_list;
		}

		LibGens::ModelLibrary *getModelLibrary() {
			return model_library;
		}

		LibGens::MaterialLibrary *getMaterialLibrary() {
			return material_library;
		}

		LibGens::TerrainAutodraw *getTerrainAutodraw() {
			return terrain_autodraw;
		}
};

#endif