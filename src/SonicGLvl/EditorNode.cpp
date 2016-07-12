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

void EditorNode::setHighlight(bool v) {
	highlighted = v;

	/*
	unsigned short attached_objects=scene_node->numAttachedObjects();
	for (unsigned short i=0; i<attached_objects; i++) {
		Ogre::Entity *entity=static_cast<Ogre::Entity *>(scene_node->getAttachedObject(i));
		unsigned int attached_entities=entity->getNumSubEntities();
		for (unsigned int j=0; j<attached_entities; j++) {
			Ogre::SubEntity *sub_entity=entity->getSubEntity(j);
			if (highlighted) sub_entity->getMaterial()->setSelfIllumination(Ogre::ColourValue(0.5f, 0.5f, 0.5f));
			else sub_entity->getMaterial()->setSelfIllumination(Ogre::ColourValue(0.0f, 0.0f, 0.0f));
		}
	}
	*/
}

void EditorNode::setSelect(bool v) {
	selected = v;

	if (scene_node) scene_node->showBoundingBox(v);
}

void EditorNode::show() {
	if (scene_node) {
		scene_node->setVisible(true, false);
	}
}


void EditorNode::hide() {
	if (scene_node) {
		scene_node->setVisible(false, false);
	}
}

EditorNode::~EditorNode() {
	if (scene_node) destroySceneNode(scene_node, true);
}