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

#include "Common.h"

#ifndef EDITOR_NODE_H_INCLUDED
#define EDITOR_NODE_H_INCLUDED

enum EditorNodeType {
	EDITOR_NODE_UNDEFINED        = 0,
	EDITOR_NODE_OBJECT           = 1,
	EDITOR_NODE_OBJECT_MSP       = 2,
	EDITOR_NODE_TERRAIN          = 4,
	EDITOR_NODE_HAVOK            = 8,
	EDITOR_NODE_GHOST            = 16,
	EDITOR_NODE_PATH             = 32,
	EDITOR_NODE_TERRAIN_AUTODRAW = 64,
	EDITOR_NODE_AXIS             = 128,
	EDITOR_NODE_VECTOR           = 256
};

class EditorNode {
	protected:
		Ogre::SceneNode *scene_node;
		Ogre::Vector3 position;
		Ogre::Quaternion rotation;
		Ogre::Vector3 scale;

		Ogre::Vector3 last_position;
		Ogre::Quaternion last_rotation;

		EditorNodeType type;
		bool selected;
		bool highlighted;
	public:
		EditorNode() {
			position = Ogre::Vector3::ZERO;
			rotation = Ogre::Quaternion::IDENTITY;
			scale    = Ogre::Vector3::UNIT_SCALE;
			selected = false;
			type = EDITOR_NODE_UNDEFINED;
		}

		EditorNode(Ogre::SceneNode *scene_node_p) {
			scene_node = scene_node_p;
			scene_node->getUserObjectBindings().setUserAny(EDITOR_NODE_BINDING, Ogre::Any(this));
			if (!scene_node) return;

			position = Ogre::Vector3::ZERO;
			rotation = Ogre::Quaternion::IDENTITY;
			scale    = Ogre::Vector3::UNIT_SCALE;
			selected = false;

			type = EDITOR_NODE_UNDEFINED;
		}

		~EditorNode();

		virtual void show();
		virtual void hide();

		virtual void setSelect(bool v);
		bool isSelected() {
			return selected;
		}

		void setHighlight(bool v);
		bool isHighlighted() {
			return highlighted;
		}

		void rememberPosition() {
			last_position = position;
		}

		void rememberRotation() {
			last_rotation = rotation;
		}

		virtual void setPosition(Ogre::Vector3 v) {
			position = v;
			if (scene_node) scene_node->setPosition(position);
		}

		void translate(Ogre::Vector3 v) {
			setPosition(getPosition() + v);
		}

		void rotate(Ogre::Quaternion q) {
			setRotation(getRotation() * q);
		}

		Ogre::Vector3 getPosition() {
			return position;
		}

		Ogre::Vector3 getLastPosition() {
			return last_position;
		}

		virtual void setRotation(Ogre::Quaternion v) {
			rotation = v;
			if (scene_node) scene_node->setOrientation(rotation);
		}

		Ogre::Quaternion getRotation() {
			return rotation;
		}

		Ogre::Quaternion getLastRotation() {
			return last_rotation;
		}

		Ogre::SceneNode *getSceneNode() {
			return scene_node;
		}

		EditorNodeType getType() {
			return type;
		}
};

#endif