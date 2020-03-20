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

#include "EditorApplication.h"	

void EditorApplication::checkGhost(Ogre::Real timeSinceLastFrame) {
	if (ghost_node) {
		if (editor_mode == EDITOR_NODE_QUERY_GHOST) {
			camera_manager->update(timeSinceLastFrame, ghost_node);
			ghost_node->addTime(timeSinceLastFrame);

			if (current_level) {
				ghost_node->checkModeChangers(current_level->getLevel());
				ghost_node->checkSplines(current_level->getLevel());
			}

			camera_manager->update(abs(timeSinceLastFrame), ghost_node);
		}
	}
}