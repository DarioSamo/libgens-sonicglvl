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

#define LIBGENS_BONE_AFFECT_LIMIT            4
#define LIBGENS_MAX_VERTEX_FORMAT_ENTRIES    256

#define LIBGENS_VERTEX_FORMAT_PC             1
#define LIBGENS_VERTEX_FORMAT_PC_TERRAIN     2
#define LIBGENS_VERTEX_FORMAT_360            3
#define LIBGENS_VERTEX_FORMAT_360_TERRAIN    4
#define LIBGENS_VERTEX_FORMAT_FORCES         5
#define LIBGENS_VERTEX_FORMAT_FORCES_TERRAIN 6

namespace LibGens {
	// Model Vertex Declaration
	class Submesh;
	class VertexFormat;

	class Vertex {
		friend class Vertex;

		protected:
			Vector3 position;
			Vector3 normal;
			Vector3 tangent;
			Vector3 binormal;
			Vector2 uv[4];
			unsigned char bone_indices[4];
			unsigned char bone_weights[4];
			Color color;
			Submesh *parent;
		public:
			Vertex();
			Vertex(Vertex *clone, LibGens::Matrix4 transform, float uv2_left, float uv2_right, float uv2_top, float uv2_bottom);
			void setParent(Submesh *v);
			Submesh *getParent();
			bool operator == (const Vertex& vertex);
			void read(File *file, VertexFormat *vformat);
			void readElements(File *file, VertexFormat *vformat);
			void write(File *file, VertexFormat *vformat);
			void writeElements(File *file, VertexFormat *vformat);
			void fixBinormalAndTangent();
			void transform(const Matrix4& matrix);
			Vector3 getTPosition(const Matrix4& matrix);
			Vector3 getPosition();
			Vector3 getNormal();
			Vector3 getTangent();
			Vector3 getBinormal();
			Color getColor();
			unsigned char getBoneIndex(size_t index);
			unsigned char getBoneWeight(size_t index);
			Vector2 getUV(size_t channel);
			void setPosition(Vector3 v);
			void setNormal(Vector3 v);
			void setTangent(Vector3 v);
			void setBinormal(Vector3 v);
			void setUV(Vector2 v, size_t channel);
			void setBoneIndex(unsigned char v, size_t index);
			void setBoneWeight(unsigned char v, size_t index);
			void setColor(Color v);
	};
};
