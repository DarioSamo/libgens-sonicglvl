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

#define LIBGENS_MODEL_SUBMESH_SLOT_SOLID          0
#define LIBGENS_MODEL_SUBMESH_SLOT_TRANSPARENT    1
#define LIBGENS_MODEL_SUBMESH_SLOT_BOOLEAN        2
#define LIBGENS_MODEL_SUBMESH_SLOT_WATER          3

#define LIBGENS_MODEL_SUBMESH_UNKNOWN_MATERIAL    "UnknownLibGens"

namespace LibGens {
	class Vertex;
	class VertexFormat;
	enum Topology;

	struct Polygon {
		unsigned int a, b, c;
	};

	class Submesh {
		friend class Submesh;

		protected:
			vector<Vertex *> vertices;
			vector<unsigned short> faces;
			vector<Polygon> faces_vectors;
			vector<unsigned short> bone_table;
			vector<string> texture_units;
			vector<unsigned int> texture_ids;
			string material_name;
			VertexFormat *vertex_format;
			AABB aabb;
			string extra;
			vector<Vector3> points;
		public:
			Submesh();
			Submesh(Submesh *clone, LibGens::Matrix4 transform, float uv2_left, float uv2_right, float uv2_top, float uv2_bottom);

			void write(File *file);
			void read(File *file, Topology topology);
			list<Vertex *> getVertexList();
			list<unsigned int> getFaceList();
			vector<Vertex *> getVertices();
			size_t getVerticesSize();
			size_t getFacesSize();
			vector<unsigned short> getFacesIndices();
			size_t getFacesIndicesSize();
			vector<Polygon> getFaces();
			void buildAABB();
			AABB getAABB();
			void setExtra(string v);
			string getExtra();
			bool hasExtra();
			string getMaterialName();
			void setMaterialName(string v);
			void setVertexFormat(VertexFormat *v);
			void addBone(unsigned short bone);
			unsigned short getBone(unsigned int index);
			vector<unsigned short> getBoneTable();
			void build(vector<Vertex *> vertices_p, vector<Polygon> faces_vectors_p);
			void fixVertexFormatForPC();
			void addTextureUnit(string v);
			void addTextureID(unsigned int v);
			unsigned int getEstimatedMemorySize();
			void changeVertexFormat(int format);
			VertexFormat* getVertexFormat();
			~Submesh();
	};
};