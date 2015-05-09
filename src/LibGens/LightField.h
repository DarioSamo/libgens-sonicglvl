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

#define LIBGENS_LIGHTFIELD_CUBE_X_SPLIT				      0
#define LIBGENS_LIGHTFIELD_CUBE_Y_SPLIT				      1
#define LIBGENS_LIGHTFIELD_CUBE_Z_SPLIT				      2
#define LIBGENS_LIGHTFIELD_CUBE_NO_SPLIT			      3

#define LIBGENS_LIGHTFIELD_CUBE_SIZE                      8

#define LIBGENS_LIGHTFIELD_ERROR_MESSAGE_NULL_FILE        "Trying to read lightfield data from unreferenced file."
#define LIBGENS_LIGHTFIELD_ERROR_MESSAGE_WRITE_NULL_FILE  "Trying to write lightfield data to an unreferenced file."
#define LIBGENS_LIGHTFIELD_SAMPLING_EPSILON               0.01

#define LIBGENS_LIGHTFIELD_FILE_ROOT_TYPE                 1


namespace LibGens {
	class ColorPoint {
		public:
			unsigned char rgb[8][3];
			unsigned char flag;
			unsigned int index;

			ColorPoint() {
				for (size_t x=0; x<8; x++) {
					for (size_t y=0; y<3; y++) {
						rgb[x][y]=0xFF;
					}
				}

				flag=0xFF;
				index=0;
			}

			void read(File *file);
			void write(File *file);
	};

	class SamplingPoint {
		public:
			ColorPoint *color;
			Vector3 point;

			SamplingPoint() : point(), color(NULL) {
			}
	};

	class LightFieldCube {
		protected:
			LightFieldCube *left;
			LightFieldCube *right;
			unsigned int type;
			unsigned int value;

			SamplingPoint *corners[8];
			unsigned int index;

			Vector3 point;
		public:
			LightFieldCube() : left(NULL), right(NULL), type(3), value(0) {
				for (size_t i=0; i<8; i++) {
					corners[i]=NULL;
				}
			}

			void setCorner(SamplingPoint *point, size_t corner) {
				corners[corner]=point;
			}

			SamplingPoint *getCorner(size_t corner) {
				if (corner >= 8) return NULL;
				else return corners[corner];
			}

			void read(File *file, size_t head_address, AABB aabb);
			void write(File *file);

			void setType(unsigned int v) {
				type = v;
			}
			unsigned int getType() {
				return type;
			}

			void setValue(unsigned int v) {
				value = v;
			}
			unsigned int getValue() {
				return value;
			}

			void setIndex(unsigned int v) {
				index = v;
			}

			unsigned int getIndex() {
				return index;
			}

			void setLeft(LightFieldCube *v) {
				left=v;
			}

			LightFieldCube *getLeft() {
				return left;
			}

			void setRight(LightFieldCube *v) {
				right=v;
			}

			LightFieldCube *getRight() {
				return right;
			}

			void getCubeList(vector<LightFieldCube *> *list);

			Vector3 getPoint() {
				return point;
			}
	};

	struct CubeParameter;
	class VRMap;

	class LightField {
		protected:
			LightFieldCube *cube;
			vector<ColorPoint *> color_palette;
			list<SamplingPoint *> sampling_points;
			AABB world_aabb;
			unsigned int sample_treshold;
			float sample_affect_distance;
			float min_world_cube_size;
			float grid_size;
			VRMap *vrmap;
			stack<CubeParameter *> cube_stack;
			vector< stack<CubeParameter *> > thread_stacks;
			bool *thread_status;

			int w, h, d;
			vector<vector<vector<vector<SamplingPoint *> > > > grid;
		public:
			LightField() : world_aabb(), cube(NULL), color_palette() {
			}
			LightField(string filename);

			void read(File *file);
			void save(string filename);
			void write(File *file);

			void generate(VRMap *vrmap, Color ambient_color, unsigned int sample_treshold, float sample_affect_distance, float min_world_cube_size, unsigned int threads=1);
			void generateCubeProc(CubeParameter *object);

			SamplingPoint *createSamplingPoint(Vector3 point);
			void paintSamplingPoints(VRMap *vrmap, Color ambient_color, float sample_affect_distance);

			void setThreadStatus(unsigned int thread, bool v) {
				thread_status[thread] = v;
			}

			void pushThreadStack(unsigned int thread, CubeParameter *object) {
				thread_stacks[thread].push(object);
			}

			ColorPoint *createColorPoint(unsigned char rgb[8][3], unsigned char flag);
	};
};
