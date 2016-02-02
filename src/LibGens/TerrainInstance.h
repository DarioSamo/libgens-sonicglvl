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

#include "Mesh.h"

#define LIBGENS_TERRAIN_INSTANCE_ERROR_MESSAGE_NULL_FILE           "Trying to read terrain instance data from unreferenced file."
#define LIBGENS_TERRAIN_INSTANCE_ERROR_MESSAGE_WRITE_NULL_FILE     "Trying to write terrain instance data to an unreferenced file."
#define LIBGENS_TERRAIN_INSTANCE_ERROR_MESSAGE_NULL_MODEL_FILE     "Couldn't find a matching model for this instance file."
#define LIBGENS_TERRAIN_INSTANCE_ERROR_MESSAGE_NULL_MODEL          "Instance doesn't have a referenced model."

#define LIBGENS_TERRAIN_INSTANCE_AABB_EXPANSION                    0.5f

namespace LibGens {
	class TerrainInstanceElement {
		protected:
			vector<unsigned int> identifiers;
			vector<unsigned short> faces;
		public:
			TerrainInstanceElement() {
			}

			void read(File *file);
			void write(File *file);
	};

	class TerrainInstanceSubmesh {
		protected:
			vector<TerrainInstanceElement *> elements;
		public:
			TerrainInstanceSubmesh() {
			}

			void read(File *file);
			void write(File *file);

			~TerrainInstanceSubmesh() {
				for (vector<TerrainInstanceElement *>::iterator it=elements.begin(); it!=elements.end(); it++) {
					delete (*it);
				}
			}
	};

	class TerrainInstanceMesh {
		protected:
			vector<TerrainInstanceSubmesh *> submeshes[LIBGENS_MODEL_SUBMESH_ROOT_SLOTS];
		public:
			TerrainInstanceMesh() {
			}

			void read(File *file);
			void write(File *file);

			~TerrainInstanceMesh() {
				for (size_t slot=0; slot<LIBGENS_MODEL_SUBMESH_ROOT_SLOTS; slot++) {
					for (vector<TerrainInstanceSubmesh *>::iterator it=submeshes[slot].begin(); it!=submeshes[slot].end(); it++) {
						delete (*it);
					}
					submeshes[slot].clear();
				}
			}
	};

	class Model;
	class VRMapSample;

	class TerrainInstance {
		protected:
			Model *model;
			string model_name;
			string name;
			Matrix4 matrix;
			AABB aabb;

			string filename;

			vector<TerrainInstanceMesh *> meshes;

			list<VRMapSample *> samples;
			vector<vector<vector<vector<VRMapSample *> > > > grid;
		public:
			TerrainInstance(string name_p, Model *model_p, Matrix4 matrix_p);
			TerrainInstance(string filename_p, string nm="", vector<Model *> *models=NULL);
			TerrainInstance(File *file, vector<Model *> *models);
			void save(string filename_p);

			void read(File *file, vector<Model *> *models);
			void write(File *file);

			list<Vertex *> getVertexList();
			Matrix4 getMatrix() {
				return matrix;
			}
			void buildAABB();
			AABB getAABB() {
				return aabb;
			}

			string getName() {
				return name;
			}

			Model *getModel() {
				return model;
			}

			string getModelName() {
				return model_name;
			}

			string getFilename() {
				return filename;
			}

			void setFilename(string v) {
				filename = v;
			}

			void setPosition(Vector3 v);
			void setRotation(Quaternion v);

			void createSamplePoints(list<VRMapSample *> *list, Bitmap *bitmap, float unit_size=1.0f, float saturation_multiplier=1.0f, float brightness_offset=0.0f);

			~TerrainInstance();
	};
};
