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

#pragma once

#define LIBGENS_TERRAIN_BLOCK_ERROR_MESSAGE_NULL_FILE       "Trying to read terrain block data from unreferenced file."
#define LIBGENS_TERRAIN_BLOCK_ERROR_MESSAGE_WRITE_NULL_FILE "Trying to write terrain block data to an unreferenced file."
#define LIBGENS_TERRAIN_BLOCK_FILENAME                      "terrain-block.tbst"
#define LIBGENS_TERRAIN_BLOCK_ROOT_GENERATIONS              0


namespace LibGens {
	class TerrainBlockInstance {
		protected:
			Vector3 center;
			float radius;
			unsigned int type;
			unsigned int identifier_a;
			unsigned int identifier_b;
		public:
			TerrainBlockInstance() {
			}

			void read(File *file);
			void write(File *file);
			void setType(unsigned int v);
			void setIdentifierA(unsigned int v);
			void setIdentifierB(unsigned int v);
			void setCenter(Vector3 v);
			void setRadius(float v);
	};

	class TerrainBlock {
		protected:
			vector<TerrainBlockInstance *> blocks;
		public:
			TerrainBlock() {
			}

			TerrainBlock(string filename);
			void save(string filename);

			void read(File *file);
			void write(File *file);
			void addBlockInstance(TerrainBlockInstance *instance);

			void clean() {
				for (vector<TerrainBlockInstance *>::iterator it=blocks.begin(); it!=blocks.end(); it++) {
					delete (*it);
				}

				blocks.clear();
			}
	};
};
