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

#include "HavokNode.h"

HavokNode::HavokNode(string name, hkGeometry *geometry, LibGens::Matrix4 transform, Ogre::SceneManager *scene_manager) {
	type = EDITOR_NODE_HAVOK;

	scene_node = scene_manager->getRootSceneNode()->createChildSceneNode();
	havok_name = name;

	buildHavokMesh(scene_node, havok_name, geometry, scene_manager, EDITOR_NODE_QUERY_HAVOK, GENERAL_MESH_GROUP);

	Ogre::Matrix4 matrix = Ogre::Matrix4(transform.m[0][0], transform.m[0][1], transform.m[0][2], transform.m[0][3],
		                                 transform.m[1][0], transform.m[1][1], transform.m[1][2], transform.m[1][3],
									     transform.m[2][0], transform.m[2][1], transform.m[2][2], transform.m[2][3],
										 transform.m[3][0], transform.m[3][1], transform.m[3][2], transform.m[3][3]);


	matrix.decomposition(position, scale, rotation);
	scene_node->setPosition(position);
	scene_node->setScale(scale);
	scene_node->setOrientation(rotation);

	scene_node->getUserObjectBindings().setUserAny(EDITOR_NODE_BINDING, Ogre::Any((EditorNode *)this));
		
	selected = false;
}

HavokNode::~HavokNode() {
	Ogre::MeshManager::getSingleton().remove(havok_name);
}