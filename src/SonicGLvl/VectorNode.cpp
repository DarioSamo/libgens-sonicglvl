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

#include "EditorApplication.h"

VectorNode::VectorNode(Ogre::SceneManager *scene_manager) {
	type = EDITOR_NODE_VECTOR;

	scene_node = scene_manager->getRootSceneNode()->createChildSceneNode();
	scene_node->setPosition(position);

	scene_node->getUserObjectBindings().setUserAny(EDITOR_NODE_BINDING, Ogre::Any((EditorNode *)this));

	Ogre::Entity *entity = scene_manager->createEntity(VECTOR_NODE_MESH);
	entity->setQueryFlags(EDITOR_NODE_QUERY_VECTOR);
	entity->setRenderQueueGroup(Ogre::RENDER_QUEUE_MAX);
	scene_node->attachObject(entity);

	selected = false;
}