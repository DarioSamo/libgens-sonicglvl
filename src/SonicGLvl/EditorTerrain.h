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

#include "Common.h"
#include "Terrain.h"
#include "TerrainGroup.h"

#ifndef TERRAIN_STREAMER_H_INCLUDED
#define TERRAIN_STREAMER_H_INCLUDED

class TerrainGroupStreamNode {
	protected:
		LibGens::TerrainGroup *terrain_group;
		bool data_loaded;
		bool resources_loaded;
	public:
		TerrainGroupStreamNode(LibGens::TerrainGroup *terrain_group_p) {
			terrain_group = terrain_group_p;
			data_loaded = false;
			resources_loaded = false;
		}
};

class TerrainStreamer {
	protected:
		LibGens::Terrain *terrain;
		LibGens::GITextureGroupInfo *gi_group_info;

		std::thread thread;
		std::mutex camera_mutex;
		std::mutex instance_mutex;
		vector<LibGens::TerrainGroup *> terrain_groups;
		LibGens::Vector3 position;
		bool check;
		unsigned int scene_check_loops;
		bool finished;

		Ogre::SceneManager *scene_manager;
		stack<LibGens::TerrainGroup *> terrain_groups_to_load;
		stack<LibGens::TerrainGroup *> terrain_groups_to_unload;
		list<TerrainNode *> *terrain_nodes_list;

		float terrain_streamer_distance;
	public:
		TerrainStreamer(LibGens::Terrain *terrain_p, LibGens::GITextureGroupInfo *gi_group_info_p, Ogre::SceneManager *scene_manager_p, bool start_loading=true);
		~TerrainStreamer();
		void createNodesFromTerrainGroup(LibGens::TerrainGroup *terrain_group);

		void start();
		void update();
		bool updateScene();

		void forceLoad();

		void setExternalList(list<TerrainNode *> *v) {
			terrain_nodes_list=v;
		}

		std::mutex &getMutex() {
			return camera_mutex;
		}

		std::mutex &getInstanceMutex() {
			return instance_mutex;
		}

		void setPosition(LibGens::Vector3 v) {
			position = v;
		}

		void setCheck(bool v) {
			check = v;
		}

		bool isFinished() {
			return finished;
		}
};

#endif