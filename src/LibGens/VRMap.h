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

#define LIBGENS_VRMAP_ERROR_MESSAGE_NULL_FILE        "Trying to read VRMap data from unreferenced file."
#define LIBGENS_VRMAP_ERROR_MESSAGE_NO_SAMPLES       "Can't generate lookup data. The VRMap has no samples."
#define LIBGENS_VRMAP_SAMPLE_SIZE                    84

namespace LibGens {
	class VRMapSample {
		public:
			Vector3 point;
			Color8 color;
	};


	class VRMap {
		protected:
			list<VRMapSample *> samples;
			AABB aabb;
			int w, h, d;
			vector<vector<vector<vector<VRMapSample *> > > > grid;
			float grid_size;
		public:
			VRMap() {}

			void generateLookupGrid(float gsize, float unit_size);

			list<VRMapSample *> getSampleList() {
				return samples;
			}

			list<VRMapSample *> *getSampleListPointer() {
				return &samples;
			}

			list<VRMapSample *> getSampleListAroundPoint(Vector3 point);
			list<VRMapSample *> getSampleListInAABB(AABB s_aabb);

			float getGridSize() {
				return grid_size;
			}

			AABB getAABB() {
				return aabb;
			}
	};
};
