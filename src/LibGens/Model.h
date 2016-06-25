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

#define LIBGENS_MODEL_ROOT_DYNAMIC_UNLEASHED      2
#define LIBGENS_MODEL_ROOT_DYNAMIC_UNLEASHED_2    4
#define LIBGENS_MODEL_ROOT_DYNAMIC_GENERATIONS    5
#define LIBGENS_MODEL_ROOT_DYNAMIC_LOST_WORLD     6

#define LIBGENS_MODEL_ERROR_MESSAGE_NULL_FILE           "Trying to read model data from unreferenced file."
#define LIBGENS_MODEL_ERROR_MESSAGE_WRITE_NULL_FILE     "Trying to write model data to an unreferenced file."

namespace LibGens {
	class Mesh;
	class Bone;
	class Vertex;
	class VRMapSample;

	class Model {
		protected:
			vector<Mesh *> meshes;
			vector<Bone *> bones;
			string name;
			unsigned int model_flag;
			string filename;
			bool terrain_mode;
			AABB global_aabb;
		public:
			Model();
			Model(string filename_p);
			Model(File *file, bool terrain_mode_p);
			void save(string filename_p, int root_type = LIBGENS_MODEL_ROOT_DYNAMIC_GENERATIONS);
			void write(File *file);
			void readRootNode(File *file);
			void readRootNodeDynamicUnleashed2(File *file);
			void readRootNodeDynamicGenerations(File *file);
			void readRootNodeDynamicLostWorld(File *file);
			void readSkeleton(File *file);
			void writeRootNodeDynamicGenerations(File *file);
			void writeRootNodeDynamicUnleashed2(File *file);
			void buildAABB();
			AABB getAABB();
			void setTerrainMode(bool v);
			vector<Bone *> getBones();
			void fixVertexFormatForPC();
			int getBoneIndexByName(string bone_name);
			vector<Mesh *> getMeshes();
			void setName(string v);
			string getName();
			string getFilename();
			list<Vertex *> getVertexList();
			list<unsigned int> getFaceList();
			list<string> getMaterialNames();
			vector<unsigned int> getMaterialMappings(list<string> &material_names);
			void getTotalData(list<Vertex *> &vertex_list, list<unsigned int> &face_list, list<string> &material_names, vector<unsigned int> &material_mappings);
			void createSamplePoints(list<VRMapSample *> *list, Matrix4 &matrix, Bitmap *bitmap, float unit_size=1.0f, float saturation_multiplier=1.0f, float brightness_offset=0.0f);
			void addMesh(Mesh *mesh);
			void cloneMesh(Mesh *mesh, LibGens::Matrix4 transform, float uv2_left, float uv2_right, float uv2_top, float uv2_bottom);
			void mergeModel(Model *model, LibGens::Matrix4 transform, float uv2_left, float uv2_right, float uv2_top, float uv2_bottom);
			unsigned int getEstimatedMemorySize();
			void changeVertexFormat(int format);
			~Model();
	};
}
