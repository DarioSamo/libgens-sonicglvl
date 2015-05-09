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
#include "Havok.h"

namespace LibGens {
	class HavokPhysicsCache;
	class HavokSkeletonCache;
	class HavokAnimationCache;

	class HavokEnviroment {
		protected:
			list<HavokPhysicsCache *> physics_cache;
			list<HavokSkeletonCache *> skeleton_cache;
			list<HavokAnimationCache *> animation_cache;
			list<string> search_paths;
		public:
			HavokEnviroment(int bufferSize);
			void loadHavokFile(string filename, string animation_name="");
			void dumpPhysics(hkpPhysicsData*);
			void dumpPhysicsSystem(hkpPhysicsSystem&);
			void dumpRigidbody(hkpRigidBody&);
			void addFolder(string folder);
			HavokPhysicsCache *getPhysics(string physics_name);
			HavokSkeletonCache *getSkeleton(string skeleton_name);
			HavokAnimationCache *getAnimation(string animation_name);
			bool deletePhysicsEntry(string physics_name);
	};
};