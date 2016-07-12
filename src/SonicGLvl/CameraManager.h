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
#include "ObjectElement.h"

#ifndef CAMERA_MANAGER_H_INCLUDED
#define CAMERA_MANAGER_H_INCLUDED

#define CAMERA_MANAGER_NAME_PARALLEL                "ObjCameraParallel"
#define CAMERA_MANAGER_NAME_PAN					    "ObjCameraPan"
#define CAMERA_MANAGER_NAME_POINT				    "ObjCameraPoint"
#define CAMERA_MANAGER_NAME_OBJECT_LOOK             "ObjCameraObjectLook"

#define CAMERA_MANAGER_NAME_VOLUME_CHANGER          "ChangeVolumeCamera"

#define CAMERA_MANAGER_ELEMENT_EASE_IN              "Ease_Time_Enter"
#define CAMERA_MANAGER_ELEMENT_EASE_OUT             "Ease_Time_Leave"
#define CAMERA_MANAGER_ELEMENT_CAMERA_PRIORITY      "Priority"

#define CAMERA_MANAGER_ELEMENT_CAMERA_VOLUME_X      "Collision_Width"
#define CAMERA_MANAGER_ELEMENT_CAMERA_VOLUME_Y      "Collision_Height"
#define CAMERA_MANAGER_ELEMENT_CAMERA_VOLUME_Z      "Collision_Length"
#define CAMERA_MANAGER_ELEMENT_CAMERA_TARGET        "Target"

#define CAMERA_MANAGER_ELEMENT_DISTANCE             "Distance"
#define CAMERA_MANAGER_ELEMENT_FOVY                 "Fovy"
#define CAMERA_MANAGER_ELEMENT_PITCH                "Pitch"
#define CAMERA_MANAGER_ELEMENT_TARGET_OFFSET_FRONT  "TargetOffset_Front"
#define CAMERA_MANAGER_ELEMENT_TARGET_OFFSET_RIGHT  "TargetOffset_Right"
#define CAMERA_MANAGER_ELEMENT_TARGET_OFFSET_UP     "TargetOffset_Up"
#define CAMERA_MANAGER_ELEMENT_YAW                  "Yaw"
#define CAMERA_MANAGER_ELEMENT_ZROT                 "ZRot"

class CameraInfluence {
	protected:
		LibGens::Object *camera_object;
		bool activated;
		float influence;
		float timer;
		float ease_time_in;
		float ease_time_out;
		int priority;
		Ogre::Vector3 result_position;
		Ogre::Quaternion result_rotation;
		Ogre::Radian fovy;
		Ogre::Camera *camera;

		LibGens::ObjectElementFloat *e_distance;
		LibGens::ObjectElementFloat *e_fovy;
		LibGens::ObjectElementFloat *e_targetoffset_front;
		LibGens::ObjectElementFloat *e_targetoffset_up;
		LibGens::ObjectElementFloat *e_targetoffset_right;
		LibGens::ObjectElementFloat *e_yaw;
		LibGens::ObjectElementFloat *e_pitch;
		LibGens::ObjectElementFloat *e_zrot;

		Ogre::Vector3 new_target_position;
	public:
		CameraInfluence(Ogre::Camera *camera_p, LibGens::Object *camera_object_p, int priority_p, float ease_time_in_p, float ease_time_out_p);

		void addTime(float time_s);
		void calculateNewTargetPosition(Ogre::Vector3 target_position, Ogre::Quaternion target_rotation);
		void calculateCamera(Ogre::Vector3 target_position, Ogre::Quaternion target_rotation);
		void calculateParallelCamera(Ogre::Vector3 target_position, Ogre::Quaternion target_rotation);
		void calculatePanCamera(Ogre::Vector3 target_position, Ogre::Quaternion target_rotation);
		void calculatePointCamera(Ogre::Vector3 target_position, Ogre::Quaternion target_rotation);

		Ogre::Vector3 getPosition() {
			return result_position;
		}

		Ogre::Quaternion getRotation() {
			return result_rotation;
		}

		Ogre::Radian getFovy() {
			return fovy;
		}

		float getInfluence() {
			if (influence < 0.0f) {
				return 0.0f;
			}

			return influence;
		}

		void setActivated(bool v) {
			if (activated && !v) {
				timer = 0.0f;
			}

			activated = v;
		}

		bool isActivated() {
			return activated;
		}

		bool toDelete() {
			return ((!activated) && (influence <= 0.0f));
		}

		LibGens::Object *getCameraObject() {
			return camera_object;
		}

		int getPriority() {
			return priority;
		}
};

class CameraManager {
	protected:
		bool force_camera;
		list<Ogre::Camera *> cameras_to_force;
		list<CameraInfluence *> current_camera_influences;
		Ogre::Camera *reference_camera;
		LibGens::Level *level;
	public:
		CameraManager();

		void addCamera(Ogre::Camera *camera) {
			cameras_to_force.push_back(camera);

			if (!reference_camera) {
				reference_camera = camera;
			}
		}

		void setForceCamera(bool v) {
			force_camera = v;
		}

		bool getForceCamera() {
			return force_camera;
		}

		void setLevel(LibGens::Level *v) {
			level = v;
		}

		void update(float time_s, GhostNode *ghost_node);
		void updateCameraObjects(float time_s, Ogre::Vector3 position_p);
		void updateCameraInfluences(float time_s);
		void calculateCameraFromInfluences(GhostNode *ghost_node);

		void checkCameraVolumeChanger(LibGens::Object *object, Ogre::Vector3 position_p);
};

#endif