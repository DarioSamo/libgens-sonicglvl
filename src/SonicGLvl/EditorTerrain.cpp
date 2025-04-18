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

void *updateTerrainStreamerExt(void *arg) {
    ((TerrainStreamer *)(arg))->update();
	return NULL;
}

void TerrainStreamer::forceLoad() {
	terrain_streamer_distance = FLT_MAX;
}

void TerrainStreamer::update() {
	while (!finished) {
		Sleep(5);

		camera_mutex.lock();
		bool check_state=check;
		bool all_loaded=true;
		LibGens::Vector3 position_state=position;
		camera_mutex.unlock();

		LibGens::TerrainGroup *current_winner=NULL;
		float current_distance=terrain_streamer_distance;
		vector<LibGens::TerrainGroup *>::iterator winner_it=terrain_groups.end();

		for (vector<LibGens::TerrainGroup *>::iterator it=terrain_groups.begin(); it!=terrain_groups.end(); it++) {
			float distance=(*it)->getDistance(position_state)-(*it)->getRadius();
			if (distance < current_distance) {
				current_distance = distance;
				winner_it = it;
			}
		}


		if (winner_it != terrain_groups.end()) {
			(*winner_it)->load();
			printf("Terrain group %s loaded!\n", (*winner_it)->getName().c_str());

			instance_mutex.lock();
			terrain_groups_to_load.push(*winner_it);
			instance_mutex.unlock();
			terrain_groups.erase(winner_it);
		}


		camera_mutex.lock();
		check = false;
		if (!terrain_groups.size()) finished = true;
		camera_mutex.unlock();
	}
}

bool TerrainStreamer::updateScene() {
	bool scene_updated=false;
	LibGens::MaterialLibrary *terrain_material_library=terrain->getMaterialLibrary();

	instance_mutex.lock();
	while (!terrain_groups_to_load.empty()) {
		scene_updated = true;

		LibGens::TerrainGroup *terrain_group = terrain_groups_to_load.top();
		terrain_groups_to_load.pop();

		list<LibGens::TerrainInstance *> instances=terrain_group->getInstances();
		for (list<LibGens::TerrainInstance *>::iterator it=instances.begin(); it!=instances.end(); it++) {
			TerrainNode *terrain_node=new TerrainNode(*it, scene_manager, terrain_material_library);
			terrain_node->setGIQualityLevel(gi_group_info, 0);
			if (terrain_nodes_list) terrain_nodes_list->push_back(terrain_node);
		}
	}
	instance_mutex.unlock();
	return scene_updated;
}

void TerrainStreamer::createNodesFromTerrainGroup(LibGens::TerrainGroup *terrain_group) {
}

TerrainStreamer::TerrainStreamer(LibGens::Terrain *terrain_p, LibGens::GITextureGroupInfo *gi_group_info_p, Ogre::SceneManager *scene_manager_p, bool start_loading) {
	terrain = terrain_p;
	gi_group_info = gi_group_info_p;

	scene_manager = scene_manager_p;
	terrain_groups = terrain->getGroups();
	check = true;
	finished = false;
	terrain_nodes_list = NULL;
	position = LibGens::Vector3(0,0,0);
	scene_check_loops = 5;

	terrain_streamer_distance = 1000;

	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(terrain->getGIFolder(), "FileSystem");

	if (start_loading) start();
}

TerrainStreamer::~TerrainStreamer() {
	if (thread.joinable()) {
		thread.join();
	}
}

void TerrainStreamer::start() {
	thread = std::thread(updateTerrainStreamerExt, this);
}


void EditorApplication::checkTerrainStreamer() {
	if (terrain_streamer) {
		if (terrain_streamer->updateScene()) {
			updateNodeVisibility();
		}

		if (terrain_streamer->isFinished()) {
			delete terrain_streamer;
			terrain_streamer=NULL;
			SHOW_MSG("Finished loading all terrain!");
		}
	}
}


void EditorApplication::checkTerrainVisibilityAndQuality(Ogre::Real timeSinceLastFrame) {
	float update_interval = configuration->getTerrainUpdateInterval();
	terrain_update_counter += timeSinceLastFrame;

	/*
	if (terrain_update_counter > update_interval) {
		terrain_update_counter = 0;

		Ogre::Vector3 camera_center=viewport->getCamera()->getPosition();
		Ogre::Vector3 camera_direction=viewport->getCamera()->getDirection();

		camera_center += camera_direction * configuration->getTerrainCameraOffset();

		float terrain_view_distance=configuration->getTerrainViewDistance();
		float gi_level_1=configuration->getGILevel1QualityOffset();
		float gi_level_2=configuration->getGILevel2QualityOffset();

		// Square the distances for less expensive checks
		terrain_view_distance *= terrain_view_distance;
		gi_level_1 *= gi_level_1;
		gi_level_2 *= gi_level_2;

		for (list<TerrainNode *>::iterator it=terrain_nodes_list.begin(); it!=terrain_nodes_list.end(); it++) {
			Ogre::Vector3 node_center=(*it)->getRealCenter();
			float node_radius=(*it)->getRadius();

			node_radius *= node_radius;

			size_t quality_level=0;
			float distance = (node_center).squaredDistance(camera_center);

			if (distance < terrain_view_distance+node_radius) {
				(*it)->show();

				if (distance > gi_level_1+node_radius) {
					quality_level = 1;
				}

				if (distance > gi_level_2+node_radius) {
					quality_level = 2;
				}

				(*it)->setGIQualityLevel(current_level->getTerrainGIInfo(), quality_level);
			}
			else {
				(*it)->hide();
			}
		}
	}
	*/
}