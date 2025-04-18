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

#include "Mesh.h"

#define LIBGENS_TERRAIN_INSTANCE_ERROR_MESSAGE_NULL_FILE           "Trying to read terrain instance data from unreferenced file."
#define LIBGENS_TERRAIN_INSTANCE_ERROR_MESSAGE_WRITE_NULL_FILE     "Trying to write terrain instance data to an unreferenced file."
#define LIBGENS_TERRAIN_INSTANCE_ERROR_MESSAGE_NULL_MODEL_FILE     "Couldn't find a matching model for this instance file."
#define LIBGENS_TERRAIN_INSTANCE_ERROR_MESSAGE_NULL_MODEL          "Instance doesn't have a referenced model."

#define LIBGENS_TERRAIN_INSTANCE_AABB_EXPANSION                    0.5f

namespace LibGens {
	class TerrainInstanceElement {
		protected:
			vector<unsigned int> light_indices;
			vector<unsigned short> faces;
		public:
			TerrainInstanceElement() {
				light_indices.clear();
				faces.clear();
			}

			void read(File *file);
			void write(File *file);
	};

	class TerrainInstanceSubmesh {
		protected:
			vector<TerrainInstanceElement *> elements;
		public:
			TerrainInstanceSubmesh() {
				elements.clear();
			}

			void read(File *file);
			void write(File *file);

			void addElement(TerrainInstanceElement *element) {
				elements.push_back(element);
			}

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
				for (int i = 0; i < LIBGENS_MODEL_SUBMESH_ROOT_SLOTS; i++) {
					submeshes[i].clear();
				}
			}

			void read(File *file);
			void write(File *file);

			void addSubmesh(TerrainInstanceSubmesh *submesh, int slot) {
				if (slot < LIBGENS_MODEL_SUBMESH_ROOT_SLOTS) {
					submeshes[slot].push_back(submesh);
				}
			}

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
	class LightList;

	class TerrainInstance {
		protected:
			Model *model;
			string model_name;
			string name;
			Matrix4 matrix;
			AABB aabb;

			string filename;

			vector<TerrainInstanceMesh *> meshes;
		public:
			TerrainInstance();
			TerrainInstance(string name_p, Model *model_p, Matrix4 matrix_p);
			TerrainInstance(string filename_p, string nm="", vector<Model *> *models = NULL);
			TerrainInstance(File *file, vector<Model *> *models = NULL);
			void save(string filename_p);

			void read(File *file, vector<Model *> *models);
			void write(File *file);

			list<Vertex *> getVertexList();

			void setAABB(AABB v) {
				aabb = v;
			}

			void setMatrix(Matrix4 v) {
				matrix = v;
			}

			Matrix4 getMatrix() {
				return matrix;
			}
			void buildAABB();
			AABB getAABB() {
				return aabb;
			}

			void setName(string v) {
				name = v;
			}

			string getName() {
				return name;
			}

			Model *getModel() {
				return model;
			}

			void setModelName(string v) {
				model_name = v;
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

			void addMesh(TerrainInstanceMesh *mesh) {
				meshes.push_back(mesh);
			}

			void setPosition(Vector3 v);
			void setRotation(Quaternion v);

			~TerrainInstance();
	};
};
