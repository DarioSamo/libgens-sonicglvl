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

#define LIBGENS_MODEL_SUBMESH_SLOT_SOLID          0
#define LIBGENS_MODEL_SUBMESH_SLOT_TRANSPARENT    1
#define LIBGENS_MODEL_SUBMESH_SLOT_BOOLEAN        2
#define LIBGENS_MODEL_SUBMESH_SLOT_WATER          3

#define LIBGENS_MODEL_SUBMESH_UNKNOWN_MATERIAL    "UnknownLibGens"

namespace LibGens {
	class Vertex;
	class VertexFormat;
	class VRMapSample;

	class Submesh {
		friend class Submesh;

		protected:
			vector<Vertex *> vertices;
			vector<unsigned short> faces;
			vector<Vector3> faces_vectors;
			vector<unsigned char> bone_table;
			vector<string> texture_units;
			vector<unsigned int> texture_ids;
			string material_name;
			VertexFormat *vertex_format;
			AABB aabb;
			string extra;
			size_t mesh_slot;
			vector<Vector3> points;
		public:
			Submesh();
			Submesh(Submesh *clone, LibGens::Matrix4 transform, float uv2_left, float uv2_right, float uv2_top, float uv2_bottom);

			void write(File *file);
			void read(File *file);
			list<Vertex *> getVertexList();
			list<unsigned int> getFaceList();
			vector<Vertex *> getVertices();
			size_t getVerticesSize();
			vector<unsigned short> getFacesIndices();
			vector<Vector3> getFaces();
			void buildAABB();
			AABB getAABB();
			void setExtra(string v);
			string getExtra();
			bool hasExtra();
			string getMaterialName();
			void setMaterialName(string v);
			void setVertexFormat(VertexFormat *v);
			void createSamplePoints(list<VRMapSample *> *list, Matrix4 &matrix, Bitmap *bitmap, float unit_size=1.0f, float saturation_multiplier=1.0f, float brightness_offset=0.0f);
			void setMeshSlot(size_t v);
			size_t getMeshSlot();
			void addBone(unsigned char bone);
			unsigned char getBone(unsigned int index);
			vector<unsigned char> getBoneTable();
			void build(vector<Vertex *> vertices_p, vector<Vector3> faces_vectors_p);
			void fixVertexFormatForPC();
			~Submesh();
	};
};