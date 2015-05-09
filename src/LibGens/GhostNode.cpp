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

#include "GhostNode.h"

namespace LibGens {
	GhostNode::GhostNode() {
	}

	void GhostNode::read(File *file) {
		size_t header_address=file->getCurrentAddress();

		file->readFloat32BE(&timer);
		position.read(file);
		rotation.read(file);
		file->readInt16BE(&animation_index);
		file->readInt16BE(&animation_ball);
		file->readFloat32BE(&animation_frame);
	}
};