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

#define LIBGENS_MODEL_SUBMESH_ROOT_SLOTS          3
#define LIBGENS_MODEL_SUBMESH_SLOTS               4

namespace LibGens {
	class Submesh;
	class Vertex;
	class VRMapSample;

	class Mesh {
		friend class Mesh;

		protected:
			vector<Submesh *> submeshes[LIBGENS_MODEL_SUBMESH_SLOTS];
			AABB aabb;
			string extra;
			string water_slot_string;
		public:
			Mesh();
			Mesh(Mesh *clone, LibGens::Matrix4 transform, float uv2_left, float uv2_right, float uv2_top, float uv2_bottom);
			~Mesh();

			void write(File *file, bool unleashed2_mode=false);
			void read(File *file);
			void buildAABB();
			AABB getAABB();
			void setExtra(string v);
			string getExtra();
			bool hasExtra();
			void addSubmesh(Submesh *submesh, size_t slot);
			void removeSubmesh(Submesh *submesh, size_t slot);
			std::vector<Submesh *> getSubmeshes();
			std::vector<Submesh *> getSubmeshes(size_t slot);
			std::vector<Submesh *> *getSubmeshSlots();
			list<Vertex *> getVertexList();
			list<unsigned int> getFaceList();
			list<string> getMaterialNames();
			vector<unsigned int> getMaterialMappings(list<string> &material_names);
			void fixVertexFormatForPC();
			void createSamplePoints(list<VRMapSample *> *list, Matrix4 &matrix, Bitmap *bitmap, float unit_size=1.0f, float saturation_multiplier=1.0f, float brightness_offset=0.0f);
			void setWaterSlotString(string v);
			unsigned int getEstimatedMemorySize();
			void changeVertexFormat(int format);
	};
};