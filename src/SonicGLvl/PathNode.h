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

#include "EditorNode.h"
#include "Path.h"

#ifndef PATH_NODE_H_INCLUDED
#define PATH_NODE_H_INCLUDED

class PathNode : public EditorNode {
	protected:
		LibGens::Node *node;
		LibGens::Spline *spline;
		Ogre::Matrix4 transform_matrix;
	public:
		PathNode(string path_name, LibGens::Spline *spline_p, LibGens::Node *node_p, Ogre::SceneManager *scene_manager, float spline_precision=0.1f);

		void setPosition(Ogre::Vector3 v) {
			EditorNode::setPosition(v);
		}

		void setRotation(Ogre::Quaternion v) {
			EditorNode::setRotation(v);
		}
};

#endif