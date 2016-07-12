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

#pragma once

#include "FBX.h"

#define LIBGENS_S06_COLLISION_ERROR_MESSAGE_NULL_FILE       "Trying to read collision data from unreferenced file."
#define LIBGENS_S06_COLLISION_ERROR_MESSAGE_WRITE_NULL_FILE "Trying to write collision data to an unreferenced file."

namespace LibGens {
	class SonicCollisionFace {
		public:
			unsigned short v1;
			unsigned short v2;
			unsigned short v3;
			unsigned int collision_flag;

			void read(File *file);
			void write(File *file);
	};

	class SonicCollision {
		public:
			vector<Vector3> vertex_pool;
			vector<SonicCollisionFace> face_pool;

			unsigned char *mopp_code_data;
			unsigned int mopp_code_size;
			Vector3 mopp_code_center;
			float mopp_code_w;

			SonicCollision(string filename);

			SonicCollision(FBX *fbx);

			void addFbxNode(FbxNode *node);
			void buildMoppCode();

			void read(File *file);
			void save(string filename);
			void write(File *file);
	};
};
