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

#define LIBGENS_MODEL_ROOT_DYNAMIC_UNLEASHED      2
#define LIBGENS_MODEL_ROOT_DYNAMIC_UNLEASHED_2    4
#define LIBGENS_MODEL_ROOT_DYNAMIC_GENERATIONS    5

#define LIBGENS_MODEL_ERROR_MESSAGE_NULL_FILE           "Trying to read model data from unreferenced file."
#define LIBGENS_MODEL_ERROR_PS3_HEADER                  "Reading PS3 models is unsupported."
#define LIBGENS_MODEL_ERROR_MESSAGE_WRITE_NULL_FILE     "Trying to write model data to an unreferenced file."

namespace LibGens {
	class Mesh;
	class MorphModel;
	class Bone;
	class Vertex;
	class SampleChunkProperty;

	enum Topology {
		TRIANGLE_LIST = 3,
		TRIANGLE_STRIP = 4
	};

	class Model {
		protected:
			vector<Mesh *> meshes;
			vector<MorphModel*> morph_models;
			vector<Bone *> bones;
			vector<SampleChunkProperty *> properties;
			string name;
			unsigned int has_instances;
			string filename;
			bool terrain_mode;
			AABB global_aabb;
			Topology topology;
		public:
			Model();
			Model(string filename_p);
			Model(File *file, bool terrain_mode_p);
			void save(string filename_p, int root_type = LIBGENS_MODEL_ROOT_DYNAMIC_GENERATIONS);
			void read(File *file);
			void readSkeleton(File *file);
			void readSampleChunkHeader(File* file);
			void write(File* file);
			void writeSampleChunkHeader(File* file);
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
			void addMesh(Mesh *mesh);
			void cloneMesh(Mesh *mesh, LibGens::Matrix4 transform, float uv2_left, float uv2_right, float uv2_top, float uv2_bottom);
			void mergeModel(Model *model, LibGens::Matrix4 transform, float uv2_left, float uv2_right, float uv2_top, float uv2_bottom);
			unsigned int getEstimatedMemorySize();
			void changeVertexFormat(int format);
			bool getPropertyValue(string name, unsigned int& value);
			void setPropertyValue(string name, unsigned int value);
			vector<MorphModel*> getMorphModels();
			void addMorphModel(MorphModel* morph_model);
			~Model();
	};
}
