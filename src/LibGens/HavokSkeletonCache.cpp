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

#include "Havok.h"
#include "HavokSkeletonCache.h"

namespace LibGens {
	HavokSkeletonCache::HavokSkeletonCache(hkResource *data_p, string path_p, string name_p, hkaSkeleton *skeleton_p) {
		name = name_p;
		data = data_p;
		path = path_p;
		skeleton = skeleton_p;
		extra = "";
	}

	string HavokSkeletonCache::getName() {
		return name;
	}

	hkaSkeleton *HavokSkeletonCache::getSkeleton() {
		return skeleton;
	}

	void HavokSkeletonCache::setExtra(string v) {
		extra=v;
	}

	string HavokSkeletonCache::getExtra() {
		return extra;
	}

	bool HavokSkeletonCache::hasExtra() {
		return (extra.size() > 0);
	}
};