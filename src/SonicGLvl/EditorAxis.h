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
#include "EditorViewport.h"

#ifndef EDITOR_AXIS_H_INCLUDED
#define EDITOR_AXIS_H_INCLUDED

#define EDITOR_AXIS_POSITION_SCALE_MULTIPLIER        0.001f
#define EDITOR_AXIS_ROTATION_SCALE_MULTIPLIER        0.05f
#define EDITOR_AXIS_TRANSLATION_SCALE_MULTIPLIER     0.01f

#define EDITOR_AXIS_POSITION_X_AXIS_FILENAME         "x_axis.mesh"
#define EDITOR_AXIS_POSITION_Y_AXIS_FILENAME         "y_axis.mesh"
#define EDITOR_AXIS_POSITION_Z_AXIS_FILENAME         "z_axis.mesh"
#define EDITOR_AXIS_POSITION_W_AXIS_FILENAME         "w_axis.mesh"

#define EDITOR_AXIS_ROTATION_X_AXIS_FILENAME         "x_axis_rot.mesh"
#define EDITOR_AXIS_ROTATION_Y_AXIS_FILENAME         "y_axis_rot.mesh"
#define EDITOR_AXIS_ROTATION_Z_AXIS_FILENAME         "z_axis_rot.mesh"

#define EDITOR_AXIS_POSITION_X_AXIS_NAME             "x_axis"
#define EDITOR_AXIS_POSITION_Y_AXIS_NAME             "y_axis"
#define EDITOR_AXIS_POSITION_Z_AXIS_NAME             "z_axis"
#define EDITOR_AXIS_POSITION_W_AXIS_NAME             "w_axis"

#define EDITOR_AXIS_ROTATION_X_AXIS_NAME             "x_axis_rot"
#define EDITOR_AXIS_ROTATION_Y_AXIS_NAME             "y_axis_rot"
#define EDITOR_AXIS_ROTATION_Z_AXIS_NAME             "z_axis_rot"

#define EDITOR_AXIS_MATERIAL_X_AXIS_NAME             "xaxis"
#define EDITOR_AXIS_MATERIAL_Y_AXIS_NAME             "yaxis"
#define EDITOR_AXIS_MATERIAL_Z_AXIS_NAME             "zaxis"
#define EDITOR_AXIS_MATERIAL_W_AXIS_NAME             "waxis"

#define EDITOR_AXIS_MATERIAL_X_AXIS_HIGHLIGHT_NAME   "xaxis_highlight"
#define EDITOR_AXIS_MATERIAL_Y_AXIS_HIGHLIGHT_NAME   "yaxis_highlight"
#define EDITOR_AXIS_MATERIAL_Z_AXIS_HIGHLIGHT_NAME   "zaxis_highlight"
#define EDITOR_AXIS_MATERIAL_W_AXIS_HIGHLIGHT_NAME   "waxis_highlight"

class EditorAxis {
	protected:
		Ogre::SceneNode *scene_node_position;
		Ogre::SceneNode *scene_node_rotation;

		Ogre::Entity *x_axis;
		Ogre::Entity *y_axis;
		Ogre::Entity *z_axis;
		Ogre::Entity *w_axis;

		Ogre::Entity *x_axis_rot;
		Ogre::Entity *y_axis_rot;
		Ogre::Entity *z_axis_rot;

		Ogre::Vector3 holding_offset;
		Ogre::Radian holding_angle;
		Ogre::Vector3 position;
		Ogre::Quaternion rotation;

		Ogre::Vector3 translate;
		Ogre::Quaternion rotate;

		bool mode;
		bool holding;
		bool visible;
		bool rotation_frozen;
		bool rotation_snap;
		bool translation_snap;
		int current_axis;

		float current_w_offset;
	public:
		EditorAxis(Ogre::SceneManager *scene_manager);
		void update(EditorViewport *viewport);
		bool mouseMoved(EditorViewport *viewport, const OIS::MouseEvent &arg);
		bool mousePressed(EditorViewport *viewport, const OIS::MouseEvent &arg, OIS::MouseButtonID id);
		bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
		Ogre::Vector3 raycastPointToAxis(EditorViewport *vieport, float raycast_x, float raycast_y, int axis);
		Ogre::Radian raycastAngleToAxis(EditorViewport *viewport, float raycast_x, float raycast_y, int axis);

		Ogre::Vector3 getTranslate() {
			return translate;
		}

		Ogre::Quaternion getRotate() {
			return rotate;
		}

		void setPosition(Ogre::Vector3 v) {
			position = v;
		}

		void setRotation(Ogre::Quaternion v) {
			rotation = v;
		}

		void setPositionAndTranslate(Ogre::Vector3 v) {
			translate = v - position;
			position = v;
		}

		void setRotationAndTranslate(Ogre::Quaternion v) {
			rotate = v * rotation.Inverse();
			rotation = v;
		}

		Ogre::Vector3 getPosition() {
			return position;
		}

		Ogre::Quaternion getRotation() {
			return rotation;
		}

		void setMode(bool v) {
			mode=v;
		}

		bool getMode() {
			return mode;
		}

		int isHighlighted() {
			return (current_axis > -1);
		}

		void setVisible(bool v) {
			visible=v;
		}

		bool isVisible() {
			return visible;
		}

		bool isHolding() {
			return holding;
		}

		void setRotationFrozen(bool v) {
			rotation_frozen = v;
		}

		bool isRotationFrozen() {
			return rotation_frozen;
		}

		void setRotationSnap(bool v) {
			rotation_snap = v;
		}

		void setTranslationSnap(bool v) {
			translation_snap = v;
		}

		bool isRotationSnap() {
			return rotation_snap;
		}

		bool isTranslationSnap() {
			return translation_snap;
		}
};

#endif