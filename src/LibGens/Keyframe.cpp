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

#include "Keyframe.h"

namespace LibGens {
	Keyframe::Keyframe() {
				frame = 0;
				value = 0;
			}

	void Keyframe::read(File *file) {
		file->readFloat32BE(&frame);
		file->readFloat32BE(&value);
	}

	float Keyframe::getFrame() {
		return frame;
	}

	float Keyframe::getValue() {
		return value;
	}
};