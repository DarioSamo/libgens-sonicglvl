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
#include "HavokPhysicsCache.h"

namespace LibGens {
	HavokPhysicsCache::HavokPhysicsCache(hkResource *data_p, string path_p, string name_p, hkpPhysicsData *physics_p) {
		data = data_p;
		path = path_p;
		name = name_p;
		physics = physics_p;
		extra = "";
	}

	string HavokPhysicsCache::getName() {
		return name;
	}

	hkpPhysicsData *HavokPhysicsCache::getPhysics() {
		return physics;
	}

	void HavokPhysicsCache::setExtra(string v) {
		extra=v;
	}

	string HavokPhysicsCache::getExtra() {
		return extra;
	}

	bool HavokPhysicsCache::hasExtra() {
		return (extra.size() > 0);
	}
	
	bool HavokPhysicsCache::save() {
		if (!data) {
			return false;
		}

		hkRootLevelContainer *container = data->getContents<hkRootLevelContainer>();

		if (!container) {
			return false;
		}

		hkBinaryPackfileWriter writer;
		writer.setContents(container, hkRootLevelContainerClass);

		hkStructureLayout::LayoutRules rules = hkStructureLayout::MsvcWin32LayoutRules;
		hkOstream out(path.c_str());
		hkPackfileWriter::Options options;
		options.m_layout = rules;
		writer.save(out.getStreamWriter(), options);
		return true;
	}

	HavokPhysicsCache::~HavokPhysicsCache() {
		delete data;
	}
};