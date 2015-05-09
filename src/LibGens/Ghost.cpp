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

#include "Ghost.h"
#include "GhostNode.h"

namespace LibGens {
	Ghost::Ghost(string filename) {
		File file(filename, LIBGENS_FILE_READ_BINARY);

		if (file.valid()) {
			file.readHeader();
			read(&file);
			file.close();
		}
	}

	void Ghost::read(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_GHOST_ERROR_MESSAGE_NULL_FILE);
			return;
		}
		size_t header_address=file->getCurrentAddress();

		unsigned int animation_total=0;
		unsigned int node_total=0;
		file->readInt32BE(&animation_total);
		file->readInt32BE(&node_total);

		for (size_t i=0; i<animation_total; i++) {
			char name[32]="";
			file->read(name, 32);
			string animation_name=ToString(name);
			animation_names.push_back(animation_name);
		}

		for (size_t i=0; i<node_total; i++) {
			GhostNode *ghost_node = new GhostNode();
			ghost_node->read(file);
			ghost_nodes.push_back(ghost_node);
		}
	}

	void Ghost::calculate(float time, Vector3 &position, Quaternion &rotation, string &animation_name, float &animation_frame, bool &animation_ball) {
		GhostNode *previous_node=NULL;
		GhostNode *next_node=NULL;

		float total_time=0;
		float factor=0;

		for (size_t i=0; i<ghost_nodes.size(); i++) {
			previous_node = next_node;
			next_node = ghost_nodes[i];
			total_time += next_node->timer;

			if (total_time > time) {
				break;
			}
		}

		if (previous_node && next_node) {
			float previous_time = total_time - next_node->timer;
			factor = (time - (previous_time)) / (next_node->timer);
			position = previous_node->position + ((next_node->position - previous_node->position) * factor);
			rotation = rotation.slerp(factor, previous_node->rotation, next_node->rotation);
			animation_name = animation_names[previous_node->animation_index];
			animation_frame = previous_node->animation_frame + ((next_node->animation_frame - previous_node->animation_frame) * factor);
			animation_ball = previous_node->animation_ball;
		}
		else if (next_node) {
			position = next_node->position;
			rotation = next_node->rotation;
			animation_name = animation_names[next_node->animation_index];
			animation_frame = next_node->animation_frame;
			animation_ball = next_node->animation_ball;
		}
		else if (previous_node) {
			position = previous_node->position;
			rotation = previous_node->rotation;
			animation_name = animation_names[previous_node->animation_index];
			animation_frame = previous_node->animation_frame;
			animation_ball = previous_node->animation_ball;
		}
	}
};
