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

#include "EditorNode.h"
#include "Ghost.h"
#include "Path.h"
#include "Object.h"
#include "ModelLibrary.h"
#include "Level.h"

#ifndef GHOST_NODE_H_INCLUDED
#define GHOST_NODE_H_INCLUDED

#define MOVEMENT_MODE_CHANGER_3DTO2D_NAME           "ChangeMode_3Dto2D"
#define MOVEMENT_MODE_CHANGER_3DTODASH_NAME         "ChangeMode_3DtoDash"
#define MOVEMENT_MODE_CHANGER_3DTOQUICKSTEP_NAME    "ChangeMode_3DtoForward"

enum GhostMovementMode {
	MOVEMENT_MODE_3D,
	MOVEMENT_MODE_2D,
	MOVEMENT_MODE_DASH,
	MOVEMENT_MODE_QUICKSTEP,
	MOVEMENT_MODE_DIVE
};

class GhostNode : public EditorNode {
	protected:
		LibGens::Ghost *ghost;
		float current_time;
		bool play;
		Ogre::Entity *ghost_entity;
		Ogre::Entity *ghost_spin_entity;
		Ogre::Vector3 camera_position;
		Ogre::Quaternion camera_rotation;
		unordered_map<string, string> animation_mappings;

		string current_animation_name;
		Ogre::AnimationState *current_animation_state;
		GhostMovementMode current_movement_mode;
		Ogre::Vector3 current_movement_delta;
		LibGens::Object *current_changer_object;
		LibGens::Node *current_spline_node;
		LibGens::Spline *current_spline;
	public:
		GhostNode(LibGens::Ghost *ghost_p, Ogre::SceneManager *scene_manager, LibGens::ModelLibrary *model_library, LibGens::MaterialLibrary *material_library);

		void setAnimationMappings(unordered_map<string, string> v) {
			animation_mappings = v;
		}

		void setPosition(Ogre::Vector3 v) {
			EditorNode::setPosition(v);
			camera_position = v;
		}

		void setRotation(Ogre::Quaternion v) {
			EditorNode::setRotation(v);
		}

		Ogre::Vector3 getCameraPosition() {
			return camera_position;
		}

		Ogre::Quaternion getCameraRotation() {
			return camera_rotation;
		}

		void setMovementMode(GhostMovementMode v) {
			current_movement_mode = v;
		}

		GhostMovementMode getMovementMode() {
			return current_movement_mode;
		}

		Ogre::Vector3 getMovementDelta() {
			return current_movement_delta;
		}

		LibGens::Object *getModeChangerObject() {
			return current_changer_object;
		}

		LibGens::Node *getSplineNode() {
			return current_spline_node;
		}

		LibGens::Spline *getSpline() {
			return current_spline;
		}

		void checkModeChangers(LibGens::Level *level);
		void checkSplines(LibGens::Level *level);

		bool checkModeChanger(LibGens::Object *object);

		void setPlay(bool v) {
			play = v;
		}

		void setTime(float v) {
			current_time = v;
			addTime(0);
		}

		void addTime(float time);
};

#endif