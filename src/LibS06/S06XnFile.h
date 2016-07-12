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

#define LIBGENS_S06_XNINFO_ERROR_MESSAGE_NULL_FILE         "Trying to read xninfo data from unreferenced file."
#define LIBGENS_S06_XNINFO_ERROR_MESSAGE_WRITE_NULL_FILE   "Trying to write xninfo data to an unreferenced file."

#define LIBGENS_XNSECTION_HEADER_INFO_XNO              "NXIF"
#define LIBGENS_XNSECTION_HEADER_TEXTURE_XNO           "NXTL"
#define LIBGENS_XNSECTION_HEADER_EFFECT_XNO            "NXEF"
#define LIBGENS_XNSECTION_HEADER_OBJECT_XNO            "NXOB"
#define LIBGENS_XNSECTION_HEADER_BONES_XNO             "NXNN"
#define LIBGENS_XNSECTION_HEADER_MOTION_XNO            "NXMO"

#define LIBGENS_XNSECTION_HEADER_INFO_ZNO              "NZIF"
#define LIBGENS_XNSECTION_HEADER_TEXTURE_ZNO           "NZTL"
#define LIBGENS_XNSECTION_HEADER_EFFECT_ZNO            "NZEF"
#define LIBGENS_XNSECTION_HEADER_OBJECT_ZNO            "NZOB"
#define LIBGENS_XNSECTION_HEADER_BONES_ZNO             "NZNN"
#define LIBGENS_XNSECTION_HEADER_MOTION_ZNO            "NZMO"

#define LIBGENS_XNSECTION_HEADER_INFO_YNO              "NYIF"

#define LIBGENS_XNSECTION_HEADER_INFO_ENO              "NEIF"
#define LIBGENS_XNSECTION_HEADER_TEXTURE_ENO           "NETL"
#define LIBGENS_XNSECTION_HEADER_EFFECT_ENO            "NEEF"
#define LIBGENS_XNSECTION_HEADER_OBJECT_ENO            "NEOB"
#define LIBGENS_XNSECTION_HEADER_BONES_ENO             "NENN"
#define LIBGENS_XNSECTION_HEADER_MOTION_ENO            "NEMO"

#define LIBGENS_XNSECTION_HEADER_INFO_GNO              "NGIF"
#define LIBGENS_XNSECTION_HEADER_TEXTURE_GNO           "NGTL"
#define LIBGENS_XNSECTION_HEADER_EFFECT_GNO            "NGEF"
#define LIBGENS_XNSECTION_HEADER_OBJECT_GNO            "NGOB"
#define LIBGENS_XNSECTION_HEADER_BONES_GNO             "NGNN"
#define LIBGENS_XNSECTION_HEADER_MOTION_GNO            "NGMO"

#define LIBGENS_XNSECTION_HEADER_INFO_INO              "NIIF"
#define LIBGENS_XNSECTION_HEADER_TEXTURE_INO           "NITL"
#define LIBGENS_XNSECTION_HEADER_EFFECT_INO            "NIEF"
#define LIBGENS_XNSECTION_HEADER_OBJECT_INO            "NIOB"
#define LIBGENS_XNSECTION_HEADER_BONES_INO             "NINN"
#define LIBGENS_XNSECTION_HEADER_MOTION_INO            "NIMO"


#define LIBGENS_XNSECTION_HEADER_OFFSET_TABLE          "NOF0"
#define LIBGENS_XNSECTION_HEADER_FOOTER                "NFN0"
#define LIBGENS_XNSECTION_HEADER_END                   "NEND"

#define LIBGENS_XNSECTION_HEADER_SIZE                  8
#define LIBGENS_XNSECTION_PADDING                      16

#define LIBGENS_XNO_EXTENSION                          ".xno"
#define LIBGENS_XNM_EXTENSION                          ".xnm"
#define LIBGENS_ZNO_EXTENSION                          ".zno"
#define LIBGENS_ZNM_EXTENSION                          ".znm"
#define LIBGENS_INO_EXTENSION                          ".ino"
#define LIBGENS_INM_EXTENSION                          ".inm"
#define LIBGENS_GNO_EXTENSION                          ".gno"
#define LIBGENS_GNM_EXTENSION                          ".gnm"
#define LIBGENS_GNA_EXTENSION                          ".gna"
#define LIBGENS_ENO_EXTENSION                          ".eno"

#define LIBGENS_XNO_VERTEX_TABLE_TERRAIN_FLAG          0x2000B

#define LIBGENS_XNMOTION_TYPE_X_COORDINATE_LINEAR      0x101
#define LIBGENS_XNMOTION_TYPE_Y_COORDINATE_LINEAR      0x201
#define LIBGENS_XNMOTION_TYPE_Z_COORDINATE_LINEAR      0x401
#define LIBGENS_XNMOTION_TYPE_X_ANGLE_BETA             0x809
#define LIBGENS_XNMOTION_TYPE_Y_ANGLE_BETA             0x1009
#define LIBGENS_XNMOTION_TYPE_Z_ANGLE_BETA             0x2009
#define LIBGENS_XNMOTION_TYPE_COORDINATES_LINEAR       0x701
#define LIBGENS_XNMOTION_TYPE_X_ANGLE_LINEAR		   0x812
#define LIBGENS_XNMOTION_TYPE_Y_ANGLE_LINEAR           0x1012
#define LIBGENS_XNMOTION_TYPE_Z_ANGLE_LINEAR           0x2012
#define LIBGENS_XNMOTION_TYPE_ANGLES_LINEAR            0x3812
#define LIBGENS_XNMOTION_TYPE_X_SCALE_LINEAR           0x8001
#define LIBGENS_XNMOTION_TYPE_Y_SCALE_LINEAR           0x10001
#define LIBGENS_XNMOTION_TYPE_Z_SCALE_LINEAR           0x20001

namespace LibGens {
	enum XNFileMode {
		MODE_AUTODETECT,
		MODE_XNO,
		MODE_ZNO,
		MODE_INO,
		MODE_GNO,
		MODE_ENO,
		MODE_YNO
	};

	class SonicXNObject;

	class SonicXNSection {
		protected:
			size_t head_address;
			unsigned int section_size;
			string header;
			XNFileMode file_mode;
			bool big_endian;
		public:
			SonicXNSection() {
				big_endian = false;
			}

			void setFileMode(XNFileMode v) {
				file_mode = v;
			}

			XNFileMode getFileMode() {
				return file_mode;
			}

			void setHeader(string v) {
				header = v;
			}

			void setBigEndian(bool v) {
				big_endian = v;
			}

			virtual void read(File *file);
			void write(File *file);
			virtual void writeBody(File *file) {
			}


			size_t getAddress() {
				return head_address;
			}

			unsigned int getSectionSize() {
				return section_size;
			}

			string getHeader() {
				return header;
			}

			void goToEnd(File *file);
	};

	class SonicXNInfo : public SonicXNSection {
		protected:
			unsigned int section_count;
			size_t offset_table_address;
			unsigned int offset_table_address_raw;
			unsigned int offset_table_size;
		public:
			SonicXNInfo() {
			}

			void read(File *file);
			void writeBody(File *file);
			void writeFixed(File *file);

			void setOffsetTableAddress(size_t v) {
				offset_table_address = v;
				offset_table_address_raw = v;
			}

			void setOffsetTableSize(unsigned int v) {
				offset_table_size = v;
			}

			unsigned int getSectionCount() {
				return section_count;
			}

			void setSectionCount(unsigned int v) {
				section_count = v;
			}
	};

	class SonicXNTexture : public SonicXNSection {
		protected:
			vector<size_t> texture_addresses;
			vector<string> textures;
			vector<unsigned int> sizes;
		public:
			SonicXNTexture() {
			}

			void read(File *file);
			void writeBody(File *file);
			unsigned int addTexture(string name);

			vector<string> getTextures() {
				return textures;
			}

			string getTexture(size_t i) {
				if (i >= textures.size()) {
					return "blank";
				}

				return textures[i];
			}

			unsigned int getTextureUnitsSize() {
				return textures.size();
			}

			void clear() {
				textures.clear();
				sizes.clear();
			}

	};

	class SonicXNEffect : public SonicXNSection {
		protected:
			vector<string> material_names;
			vector<string> material_shaders;
			vector<unsigned int> material_indices;
			vector<unsigned short> extras;

			vector<size_t> material_names_addresses;
			vector<size_t> material_shaders_addresses;
		public:
			SonicXNEffect() {
			}

			void read(File *file);
			void writeBody(File *file);

			size_t addMaterialName(string v, size_t shader_index=0xFFFFFFFF) {
				for (size_t i=0; i<material_names.size(); i++) {
					if (material_names[i] == v) {
						return i;
					}
				}

				if (shader_index != 0xFFFFFFFF) {
					material_indices.push_back(shader_index);
				}

				material_names.push_back(v);
				return material_names.size()-1;
			}

			size_t addMaterialShader(string v) {
				for (size_t i=0; i<material_shaders.size(); i++) {
					if (material_shaders[i] == v) {
						return i;
					}
				}

				material_shaders.push_back(v);
				return material_shaders.size()-1;
			}

			void addMaterialIndex(unsigned int v) {
				material_indices.push_back(v);
			}

			void addExtra(unsigned short v) {
				extras.push_back(v);
			}

			void clear() {
				material_names.clear();
				material_shaders.clear();
				material_indices.clear();
				extras.clear();
			}

			void clearExtras() {
				extras.clear();
			}

			size_t getMaterialShadersSize() {
				return material_shaders.size();
			}

			size_t getMaterialNamesSize() {
				return material_names.size();
			}
	};

	class SonicVertex {
		public:
			Vector3 position;
			Vector3 normal;
			Vector2 uv[4];
			float bone_weights_f[4];
			unsigned char bone_indices[4];
			unsigned char rgba[4];
			unsigned char rgba_2[4];
			Vector3 tangent;
			Vector3 binormal;

			SonicVertex() {

			}

			void zero() {
				position = Vector3(0.0f, 0.0f, 0.0f);
				normal = Vector3(0.0f, 0.0f, 0.0f);
				tangent = Vector3(0.0f, 0.0f, 0.0f);
				binormal = Vector3(0.0f, 0.0f, 0.0f);
				uv[0] = Vector2(0.0f, 0.0f);
				uv[1] = Vector2(0.0f, 0.0f);
				uv[2] = Vector2(0.0f, 0.0f);
				uv[3] = Vector2(0.0f, 0.0f);
				bone_weights_f[0] = 1.0f;
				bone_weights_f[1] = 0.0f;
				bone_weights_f[2] = 0.0f;
				bone_weights_f[3] = 0.0f;
				bone_indices[0] = 0;
				bone_indices[1] = 0;
				bone_indices[2] = 0;
				bone_indices[3] = 0;
				rgba[0] = 0xFF;
				rgba[1] = 0xFF;
				rgba[2] = 0xFF;
				rgba[3] = 0xFF;
				rgba_2[0] = 0xFF;
				rgba_2[1] = 0xFF;
				rgba_2[2] = 0xFF;
				rgba_2[3] = 0xFF;
			}

			bool operator == (const SonicVertex& vertex) {
				if (position != vertex.position) return false;
				if (normal != vertex.normal) return false;
				if (uv[0] != vertex.uv[0]) return false;
				if (uv[1] != vertex.uv[1]) return false;
				if (uv[2] != vertex.uv[2]) return false;
				if (uv[3] != vertex.uv[3]) return false;
				if (bone_weights_f[0] != vertex.bone_weights_f[0]) return false;
				if (bone_weights_f[1] != vertex.bone_weights_f[1]) return false;
				if (bone_weights_f[2] != vertex.bone_weights_f[2]) return false;
				if (bone_weights_f[3] != vertex.bone_weights_f[3]) return false;
				if (bone_indices[0] != vertex.bone_indices[0]) return false;
				if (bone_indices[1] != vertex.bone_indices[1]) return false;
				if (bone_indices[2] != vertex.bone_indices[2]) return false;
				if (bone_indices[3] != vertex.bone_indices[3]) return false;
				if (rgba[0] != vertex.rgba[0]) return false;
				if (rgba[1] != vertex.rgba[1]) return false;
				if (rgba[2] != vertex.rgba[2]) return false;
				if (rgba[3] != vertex.rgba[3]) return false;
				if (rgba_2[0] != vertex.rgba_2[0]) return false;
				if (rgba_2[1] != vertex.rgba_2[1]) return false;
				if (rgba_2[2] != vertex.rgba_2[2]) return false;
				if (rgba_2[3] != vertex.rgba_2[3]) return false;
				if (tangent != vertex.tangent) return false;
				if (binormal != vertex.binormal) return false;
	            return true;
			}

			void read(File *file, unsigned int vertex_size, bool big_endian, unsigned int vertex_flag, XNFileMode file_mode);
			void write(File *file, unsigned int vertex_size, bool big_endian, unsigned int vertex_flag, XNFileMode file_mode);

			void copy(SonicVertex &vertex) {
				position = vertex.position;
				normal = vertex.normal;
				tangent = vertex.tangent;
				binormal = vertex.binormal;
				uv[0] = vertex.uv[0];
				uv[1] = vertex.uv[1];
				uv[2] = vertex.uv[2];
				uv[3] = vertex.uv[3];
				bone_weights_f[0] = vertex.bone_weights_f[0];
				bone_weights_f[1] = vertex.bone_weights_f[1];
				bone_weights_f[2] = vertex.bone_weights_f[2];
				bone_weights_f[3] = vertex.bone_weights_f[3];
				bone_indices[0] = vertex.bone_indices[0];
				bone_indices[1] = vertex.bone_indices[1];
				bone_indices[2] = vertex.bone_indices[2];
				bone_indices[3] = vertex.bone_indices[3];
				rgba[0] = vertex.rgba[0];
				rgba[1] = vertex.rgba[1];
				rgba[2] = vertex.rgba[2];
				rgba[3] = vertex.rgba[3];

				rgba_2[0] = vertex.rgba_2[0];
				rgba_2[1] = vertex.rgba_2[1];
				rgba_2[2] = vertex.rgba_2[2];
				rgba_2[3] = vertex.rgba_2[3];
			}

			void setScale(float scale) {
				position = position * scale;
			}
	};

	class SonicTextureUnitZNO {
		public:
			unsigned int flag;
			unsigned int index;
			unsigned int enviroment_mode;
			Vector2 offset;
			Vector2 scale;
			unsigned int wrap_s;
			unsigned int wrap_t;
			float lod_bias;


			void read(File *file, bool big_endian);
			void write(File *file);
	};

	class SonicTextureUnit {
		public:
			float flag_f;
			unsigned int index;
			float flag_2_f;
			unsigned int flag;
			unsigned int flag_2;
			float flag_3_f;
			unsigned int flag_3;

			void read(File *file, bool big_endian);
			void write(File *file);
			bool compare(SonicTextureUnit *t);
	};

	class SonicMaterialColor {
		public:
			unsigned int flag;
			Color ambient;
			Color diffuse;
			Color specular;
			Color emission;
			float shininess;
			float specular_intensity;

			void read(File *file, XNFileMode file_mode, bool big_endian);
			void write(File *file, XNFileMode file_mode);
			bool compare(SonicMaterialColor *color);
	};

	class SonicMaterialProperties {
		public:
			char data[28];

			void read(File *file, XNFileMode file_mode, bool big_endian);
			void write(File *file, XNFileMode file_mode);
	};


	class SonicMaterialTable {
		public:
			unsigned int count;
			unsigned int flag_table;
			size_t head_address;
			size_t table_address;
			size_t data_block_1_address;
			size_t data_block_2_address;
			size_t texture_units_address;

			unsigned int texture_unit_flag;
			unsigned int texture_unit_flag_2;
			unsigned int user_flag;

			size_t data_block_1_length;
			size_t data_block_2_length;

			float first_floats[20];
			int first_ints[16];

			SonicMaterialColor *colors;
			SonicMaterialProperties *properties;

			vector<SonicTextureUnit *> texture_units;
			vector<SonicTextureUnitZNO *> texture_units_zno;

			SonicMaterialTable() {
			}

			void read(File *file, XNFileMode file_mode, bool big_endian);
			void write(File *file, XNFileMode file_mode);
			void writeTable(File *file, XNFileMode file_mode);
			void writeDataBlock1(File *file, XNFileMode file_mode);
			void writeDataBlock2(File *file, XNFileMode file_mode);
			void writeTextureUnits(File *file, XNFileMode file_mode);

			bool compareDataBlock1(SonicMaterialTable *table, XNFileMode file_mode);
			bool compareDataBlock2(SonicMaterialTable *table, XNFileMode file_mode);
			bool compareTextureUnits(SonicMaterialTable *table, XNFileMode file_mode);

			size_t getAddress() {
				return head_address;
			}
	};


	class SonicOldMaterialTable {
		public:
			unsigned int texture_unit;

			SonicOldMaterialTable() {
			}

			void read(File *file, XNFileMode file_mode, bool big_endian);
	};


	class SonicVertexBoneData {
		public:
			unsigned char bone_1;
			unsigned char bone_2;
			unsigned short weight;

			SonicVertexBoneData() {
			}
	};

	
	class SonicVertexResourceTable {
		public:
			vector<Vector3>				positions;
			vector<Vector3>				normals;
			vector<Vector2>				uvs;
			vector<Vector2>				uvs_2;
			vector<SonicVertexBoneData> bones;
			vector<Color>               colors;

			SonicVertexResourceTable() {
			}

			void read(File *file, XNFileMode file_mode, bool big_endian);
	};

	class SonicPolygonPoint {
		public:
			unsigned short position_index;
			unsigned short normal_index;
			unsigned short color_index;
			unsigned short uv_index;
			unsigned short uv2_index;

			SonicPolygonPoint() {
				position_index = 0;
				normal_index = 0;
				color_index = 0;
				uv_index = 0;
				uv2_index = 0;
			}

			SonicPolygonPoint& operator = (const SonicPolygonPoint& p) {
				position_index = p.position_index;
	            normal_index = p.normal_index;
				uv_index = p.uv_index;
				uv2_index = p.uv2_index;
				color_index = p.color_index;
				return *this;
			}

			inline bool operator == (const SonicPolygonPoint& p) {
	            return ((position_index == p.position_index) && (normal_index == p.normal_index) && (uv_index == p.uv_index) && (uv2_index == p.uv2_index) && (color_index == p.color_index));
			}

			void read(File *file, bool big_endian, unsigned char format_flag);
	};

	class SonicPolygon {
		public:
			SonicPolygonPoint points[3];

			SonicPolygon(SonicPolygonPoint p1, SonicPolygonPoint p2, SonicPolygonPoint p3) {
				points[0] = p1;
				points[1] = p2;
				points[2] = p3;
			}
	};

	class SonicPolygonTable {
		public:
			unsigned int flag;
			vector<SonicPolygon *> faces;

			SonicPolygonTable() {
			}

			void read(File *file, bool big_endian);
	};

	class SonicVertexTable {
		public:
			vector<SonicVertex *> vertices;
			vector<unsigned int> bone_table;

			unsigned int vertex_size;
			unsigned int flag_1;
			unsigned int flag_2;

			size_t vertex_buffer_address;
			size_t vertex_table_address;

			SonicVertexTable() {
			}

			void read(File *file, XNFileMode file_mode, bool big_endian);
			void writeVertices(File *file, XNFileMode file_mode);
			void writeTable(File *file);
			void writeTableFixed(File *file);
			void write(File *file);

			void setScale(float scale);
	};

	class SonicIndexTable {
		public:
			unsigned int flag;
			vector<unsigned short> indices;
			vector<unsigned short> strip_sizes;
			vector<Vector3> indices_vector;

			size_t strip_sizes_address_data;
			size_t indices_address_data;
			size_t indices_table_address;

			SonicIndexTable() {
			}

			void read(File *file, bool big_endian);
			void writeIndices(File *file);
			void writeTable(File *file);
			void write(File *file);
	};


	class SonicSubmesh {
		public:
			Vector3 center;
			float radius;
			unsigned int node_index;
			unsigned int matrix_index;
			unsigned int material_index;
			unsigned int vertex_index;
			unsigned int indices_index;
			unsigned int indices_index_2;

			void read(File *file, bool big_endian, XNFileMode file_mode);
			void write(File *file);
	};

	
	class SonicMesh {
		public:
			vector<SonicSubmesh *> submeshes;
			vector<unsigned int> extras;
			unsigned int flag;

			size_t submesh_table_address;
			size_t extra_table_address;

			string name;

			void read(File *file, bool big_endian, XNFileMode file_mode);

			void writeSubmeshes(File *file);
			void writeExtras(File *file);
			void write(File *file);

			vector<unsigned int> getExtras() {
				return extras;
			}

			void setExtras(vector<unsigned int> v) {
				extras=v;
			}

			unsigned int getExtrasSize() {
				return extras.size();
			}
	};

	class SonicBone {
		public:
			unsigned int flag;
			unsigned short matrix_index;
			unsigned short parent_index;
			unsigned short child_index;
			unsigned short sibling_index;
			Vector3 translation;
			unsigned int rotation_x;
			unsigned int rotation_y;
			unsigned int rotation_z;
			Vector3 scale;
			Quaternion orientation;
			Matrix4 matrix;
			Matrix4 current_matrix;
			Vector3 center;
			float radius;
			unsigned int user;
			Vector3 bounding_box;

			float scale_animation_mod;

			SonicBone() {
				flag = 0xCF;

				matrix_index = 0xFFFF;
				parent_index = 0xFFFF;
				child_index = 0xFFFF;
				sibling_index = 0xFFFF;

				bounding_box = LibGens::Vector3(0, 0, 0);
				center = LibGens::Vector3(0, 0, 0);
				radius = 0.0f;
				user = 0;
				scale_animation_mod = 1.0f;
			}

			void zero();
			void read(File *file, bool big_endian, XNFileMode file_mode);
			void write(File *file);

			void setScale(float scale);
	};

	class SonicXNBones : public SonicXNSection {
		protected:
			vector<string> bone_names;
			vector<unsigned int> bone_indices;
			vector<size_t> bone_names_addresses;
		public:
			SonicXNBones() {
			}

			void read(File *file);
			void writeBody(File *file);

			void setName(size_t i, string name) {
				bone_names[i] = name;
			}

			string getName(size_t i) {
				return bone_names[i];
			}

			void addBone(string name, size_t index) {
				bone_names.push_back(name);
				bone_indices.push_back(index);
			}

			void clear() {
				bone_names.clear();
				bone_indices.clear();
			}
	};

	class SonicFrameValue {
		public:
			float frame;
			float value;

			void read(File *file, bool big_endian);
			void write(File *file);
	};

	class SonicFrameValueIntBeta {
		public:
			float frame;
			unsigned short value;

			void read(File *file, bool big_endian);
			void write(File *file);
	};

	class SonicFrameValueInt {
		public:
			unsigned short frame;
			unsigned short value;

			void read(File *file, bool big_endian);
			void write(File *file);
	};

	class SonicFrameValueFloatsGroup {
		public:
			float frame;
			unsigned int flag;
			float unknown_1;
			float unknown_2;
			float unknown_3;
			float unknown_4;

			void read(File *file, bool big_endian);
			void write(File *file);
	};

	class SonicFrameValueFloats {
		public:
			float frame;
			Vector3 value;

			void read(File *file, bool big_endian);
			void write(File *file);
	};

	class SonicFrameValueAngles {
		public:
			unsigned short frame;
			unsigned short value_x;
			unsigned short value_y;
			unsigned short value_z;

			void read(File *file, bool big_endian);
			void write(File *file);
	};

	class SonicMotionControl {
		public:
			vector<SonicFrameValue *> frame_values;
			vector<SonicFrameValueInt *> frame_values_int;
			vector<SonicFrameValueIntBeta *> frame_values_int_beta;
			vector<SonicFrameValueFloats *> frame_values_floats;
			vector<SonicFrameValueAngles *> frame_values_angles;
			vector<SonicFrameValueFloatsGroup *> frame_values_floats_groups;

			unsigned int bone_index;
			unsigned int type;
			unsigned int flag;
			float start_frame;
			float end_frame;
			float start_key_frame;
			float end_key_frame;
			unsigned int element_size;

			size_t frame_value_address;

			SonicMotionControl() {
			}

			Vector3 getFrameVector(float frame, Vector3 reference);
			float getFrameValue(float frame, float reference);

			void read(File *file, bool big_endian);
			void write(File *file);
			void writeFrameValues(File *file);

			float getFirstFrameValue() {
				SonicFrameValue *frame_value=(frame_values.size() ? frame_values[0] : NULL);
				if (frame_value) return frame_value->value;
				else return 0.0f;
			}

			unsigned short getFirstFrameValueInt() {
				SonicFrameValueInt *frame_value=(frame_values_int.size() ? frame_values_int[0] : NULL);
				if (frame_value) return frame_value->value;
				else return 0;
			}

			bool onlyOneFrameValue() {
				if (element_size == 16) {
					return (frame_values_floats.size()==1);
				}
				else if (element_size == 8) {
					if (type == LIBGENS_XNMOTION_TYPE_ANGLES_LINEAR) return (frame_values_angles.size()==1);
					else return (frame_values.size()==1);
				}
				else if (element_size == 4) {
					return (frame_values_int.size()==1);
				}

				return false;
			}

			void optimize();
			void optimizeInt();
			void optimizeAngles();

			void setScale(float scale);
	};

	class SonicXNMotion : public SonicXNSection {
		protected:
			vector<SonicMotionControl *> motion_controls;
			unsigned int flag;
			float start_frame;
			float end_frame;
			float fps;
		public:
			SonicXNMotion() {
			}

			void read(File *file);
			void writeBody(File *file);
			void writeDAE(TiXmlElement *root, SonicXNObject *object, SonicXNBones *bones, float unit_scale);

			float getFPS() {
				return fps;
			}

			float getDuration() {
				return end_frame / fps;
			}

			void setLength(float v) {
				end_frame = v;
			}

			void setFPS(float v) {
				fps = v;
			}

			void pushMotionControl(SonicMotionControl *motion_control);
			void clearMotionControls();
			void deleteMotionControl(SonicMotionControl *motion_control);
			void optimizeMotionControls(unsigned int bone_index, float bone_x, float bone_y, float bone_z, 
										float bone_scale_x, float bone_scale_y, float bone_scale_z, 
										unsigned short bone_rot_x, unsigned short bone_rot_y, unsigned short bone_rot_z);

			SonicMotionControl *getMotionControl(unsigned int type, unsigned int bone_index);
			SonicMotionControl *getPositionMotionControl(unsigned int bone_index);
			SonicMotionControl *getAnglesMotionControl(unsigned int bone_index);
			SonicMotionControl *getPositionXMotionControl(unsigned int bone_index);
			SonicMotionControl *getPositionYMotionControl(unsigned int bone_index);
			SonicMotionControl *getPositionZMotionControl(unsigned int bone_index);
			SonicMotionControl *getAngleXMotionControl(unsigned int bone_index);
			SonicMotionControl *getAngleYMotionControl(unsigned int bone_index);
			SonicMotionControl *getAngleZMotionControl(unsigned int bone_index);
			SonicMotionControl *getAngleBetaXMotionControl(unsigned int bone_index);
			SonicMotionControl *getAngleBetaYMotionControl(unsigned int bone_index);
			SonicMotionControl *getAngleBetaZMotionControl(unsigned int bone_index);
			SonicMotionControl *getScaleXMotionControl(unsigned int bone_index);
			SonicMotionControl *getScaleYMotionControl(unsigned int bone_index);
			SonicMotionControl *getScaleZMotionControl(unsigned int bone_index);

			void updateScaleMod(SonicXNObject *object);
	};

	class SonicXNObject : public SonicXNSection {
		public:
			vector<SonicMaterialTable *> material_tables;
			vector<SonicVertexTable *> vertex_tables;
			vector<SonicIndexTable *> index_tables;

			// GNO
			vector<SonicVertexResourceTable *> vertex_resource_tables;
			vector<SonicPolygonTable *> polygon_tables;
			vector<SonicOldMaterialTable *> old_material_tables;
			

			vector<SonicMesh *> meshes;
			vector<SonicBone *> bones;
			Vector3 center;
			float radius;

			unsigned int type;
			unsigned int version;
			Vector3 bounding_box;

			unsigned int bone_max_depth;
			unsigned int total_texture_count;
			unsigned int bone_matrix_count;
			unsigned int header_flag;

			SonicXNTexture *texture;
			SonicXNEffect *effect;
			SonicXNBones *bones_names;

			AABB aabb;

			string name;

			SonicXNObject(SonicXNTexture *texture_p, SonicXNEffect *effect_p, SonicXNBones *bone_p) {
				texture       = texture_p;
				effect        = effect_p;
				bones_names   = bone_p;
			}

			void read(File *file);
			void writeBody(File *file);
			bool getBoneIndexByName(string name_search, unsigned int &index);

			void writeMaterialDAE(TiXmlElement *root);
			void writeMaterialBindDAE(TiXmlElement *root);
			void writeEffectsDAE(TiXmlElement *root, SonicXNTexture *texture);
			void writeEffectTextureDAE(TiXmlElement *root, string tex_name);
			void writeEffectTechniqueDAE(TiXmlElement *root, string tex_name);
			void writeBonesDAE(TiXmlElement *root, size_t current, float unit_scale);
			void writeControllerDAE(TiXmlElement *root, float unit_scale);
			void writeMeshesDAE(TiXmlElement *root, float unit_scale);
			void writeDAE(TiXmlElement *root, bool only_bones=false, float unit_scale=1.0f);

			void calculateMaxBoneDepth(size_t parent, size_t depth=0);
			void calculateBoneMatrixCount();

			void setNames(string v) {
				name = v;
				for (size_t i=0; i<meshes.size(); i++) {
					meshes[i]->name = v + ToString(i);
				}
			}

			void setScale(float scale);
			void setBoneScale(unsigned short current_index, float scale, bool dont_scale=false);
			void calculateSkinningMatrix(unsigned short current_index, LibGens::Matrix4 parent_matrix);
			void calculateSkinningMatrices();
			void calculateSkinningIDs();
	};


	class SonicXNOffsetTable : public SonicXNSection {
		protected:
			vector<size_t> addresses;
		public:
			SonicXNOffsetTable() {
			}

			void read(File *file);

			void writeBody(File *file);

			void clear() {
				addresses.clear();
			}

			void push(size_t v) {
				addresses.push_back(v);
			}
	};

	class SonicXNFooter : public SonicXNSection {
		public:
			string name;

			SonicXNFooter() {
			}

			void read(File *file);
			void writeBody(File *file);
	};

	class SonicXNEnd : public SonicXNSection {
		public:
			SonicXNEnd() {
			}
	};

	class SonicXNFile {
		protected:
			SonicXNInfo *info;
			SonicXNOffsetTable *offset_table;
			SonicXNFooter *footer;
			SonicXNEnd *end;
			vector<SonicXNSection *> sections;
			string folder;
			XNFileMode file_mode;
			bool big_endian;

			string header_info;
			string header_texture;
			string header_effect;
			string header_bones;
			string header_object;
			string header_motion;
		public:
			SonicXNFile(string filename, XNFileMode file_mode_parameter=MODE_AUTODETECT);

			SonicXNFile(XNFileMode file_mode_parameter);

			SonicXNSection *readSection(File *file);
			void save(string filename);
			void write(File *file);

			void deleteSection(SonicXNSection *section) {
				for (size_t i=0; i<sections.size(); i++) {
					if (sections[i] == section) {
						sections.erase(sections.begin()+i);
						return;
					}
				}
			}

			SonicXNObject *getObject() {
				for (size_t i=0; i<sections.size(); i++) {
					if (sections[i]->getHeader() == header_object) {
						return static_cast<SonicXNObject *>(sections[i]);
					}
				}

				return NULL;
			}

			SonicXNTexture *getTexture() {
				for (size_t i=0; i<sections.size(); i++) {
					if (sections[i]->getHeader() == header_texture) {
						return static_cast<SonicXNTexture *>(sections[i]);
					}
				}

				return NULL;
			}

			void setTexture(SonicXNTexture *texture) {
				bool found=false;
				if (!texture) return;

				for (size_t i=0; i<sections.size(); i++) {
					if (sections[i]->getHeader() == header_texture) {
						delete sections[i];
						sections[i] = texture;
						return;
					}
				}

				sections.push_back(texture);
			}

			SonicXNEffect *getEffect() {
				for (size_t i=0; i<sections.size(); i++) {
					if (sections[i]->getHeader() == header_effect) {
						return static_cast<SonicXNEffect *>(sections[i]);
					}
				}

				return NULL;
			}

			void setEffect(SonicXNEffect *effect) {
				bool found=false;
				if (!effect) return;

				// Try to replace effect
				for (size_t i=0; i<sections.size(); i++) {
					if (sections[i]->getHeader() == header_effect) {
						delete sections[i];
						sections[i] = effect;
						return;
					}
				}

				// Try to place behind object
				for (size_t i=0; i<sections.size(); i++) {
					if (sections[i]->getHeader() == header_object) {
						vector<SonicXNSection *>::iterator it=sections.begin() + i;
						sections.insert(it, effect);
						return;
					}
				}

				// Push to the back by default
				sections.push_back(effect);
			}

			SonicXNBones *getBones() {
				for (size_t i=0; i<sections.size(); i++) {
					if (sections[i]->getHeader() == header_bones) {
						return static_cast<SonicXNBones *>(sections[i]);
					}
				}

				return NULL;
			}

			void setBones(SonicXNBones *bones) {
				bool found=false;
				if (!bones) return;

				for (size_t i=0; i<sections.size(); i++) {
					if (sections[i]->getHeader() == header_bones) {
						delete sections[i];
						sections[i] = bones;
						return;
					}
				}

				// Try to place behind object
				for (size_t i=0; i<sections.size(); i++) {
					if (sections[i]->getHeader() == header_object) {
						vector<SonicXNSection *>::iterator it=sections.begin() + i;
						sections.insert(it, bones);
						return;
					}
				}

				sections.push_back(bones);
			}

			SonicXNMotion *getMotion() {
				for (size_t i=0; i<sections.size(); i++) {
					if (sections[i]->getHeader() == header_motion) {
						return static_cast<SonicXNMotion *>(sections[i]);
					}
				}

				return NULL;
			}

			void setMotion(SonicXNMotion *motion) {
				bool found=false;
				if (!motion) return;

				for (size_t i=0; i<sections.size(); i++) {
					if (sections[i]->getHeader() == header_motion) {
						delete sections[i];
						sections[i] = motion;
						return;
					}
				}

				sections.push_back(motion);
			}

			SonicXNFooter *getFooter() {
				return footer;
			}

			void setFooter(SonicXNFooter *v) {
				footer=v;
			}
			
			void setFileMode(XNFileMode target_file_mode);
			void saveDAE(string filename, bool only_animation=false, float unit_scale=1.0f);

			void setHeaders();

			void importFBX(FBX *fbx);
			void addFBXNode(FbxNode *lNode);
			void addFBXMaterial(FbxSurfaceMaterial *lMaterial);
			void addFBXMaterialProperty(FbxProperty *lProperty, SonicMaterialTable *sonic_material_table);
			void addFBXSubmesh(FbxNode *lNode, FbxMesh *lMesh, SonicMesh *sonic_mesh, int material_index, int material_base_index, bool single_material, FbxAMatrix transform_matrix);

			void createTextureSection();
			void createEffectSection();
			void createBoneSection();
			void createObjectSection();
	};

	extern const unsigned char xno_constant_floats[80];
	extern const unsigned char xno_constant_ints[64];
};