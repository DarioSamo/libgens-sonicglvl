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

class hkpPhysicsData;
class hkResource;

namespace LibGens {
	class HavokPhysicsCache {
		protected:
			string name;
			hkpPhysicsData *physics;
			hkResource *data;
			string extra;
			string path;
		public:
			HavokPhysicsCache(hkResource *data_p, string path_p, string name_p, hkpPhysicsData *physics_p);
			string getName();
			hkpPhysicsData *getPhysics();
			void setExtra(string v);
			string getExtra();
			bool hasExtra();
			bool save();
			~HavokPhysicsCache();
	};
};