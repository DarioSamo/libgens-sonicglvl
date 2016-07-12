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
#include "ObjectSet.h"

Ogre::SceneNode *camera_marker_node;
Ogre::SceneNode *camera_marker_tangent;

CameraInfluence::CameraInfluence(Ogre::Camera *camera_p, LibGens::Object *camera_object_p, int priority_p, float ease_time_in_p, float ease_time_out_p) {
	camera_object = camera_object_p;

	activated = true;
	influence = 0.0f;
	timer = 0.0f;
	ease_time_in = ease_time_in_p;
	ease_time_out = ease_time_out_p;
	priority = priority_p;
	result_position = Ogre::Vector3::ZERO;
	result_rotation = Ogre::Quaternion::IDENTITY;
	fovy = Ogre::Degree(45.0);

	// Used only for reference
	camera = camera_p;

	e_distance           = (LibGens::ObjectElementFloat *) camera_object->getElement(CAMERA_MANAGER_ELEMENT_DISTANCE);
	e_fovy               = (LibGens::ObjectElementFloat *) camera_object->getElement(CAMERA_MANAGER_ELEMENT_FOVY);
	e_targetoffset_front = (LibGens::ObjectElementFloat *) camera_object->getElement(CAMERA_MANAGER_ELEMENT_TARGET_OFFSET_FRONT);
	e_targetoffset_up    = (LibGens::ObjectElementFloat *) camera_object->getElement(CAMERA_MANAGER_ELEMENT_TARGET_OFFSET_UP);
	e_targetoffset_right = (LibGens::ObjectElementFloat *) camera_object->getElement(CAMERA_MANAGER_ELEMENT_TARGET_OFFSET_RIGHT);
	e_yaw                = (LibGens::ObjectElementFloat *) camera_object->getElement(CAMERA_MANAGER_ELEMENT_YAW);
	e_pitch              = (LibGens::ObjectElementFloat *) camera_object->getElement(CAMERA_MANAGER_ELEMENT_PITCH);
	e_zrot               = (LibGens::ObjectElementFloat *) camera_object->getElement(CAMERA_MANAGER_ELEMENT_ZROT);
}

void CameraInfluence::addTime(float time_s) {
	timer += time_s;

	if (activated) {
		if (ease_time_in > 0.0f) influence = timer / ease_time_in;
		else influence = 1.0f;

		if (influence > 1.0f) influence = 1.0f;
	}
	else {
		if (ease_time_out > 0.0f) influence = 1.0f - (timer / ease_time_out);
		else influence = 0.0f;

		if (influence < 0.0f) influence = 0.0f;
	}
}

void CameraInfluence::calculateCamera(Ogre::Vector3 target_position, Ogre::Quaternion target_rotation) {
	string camera_name = camera_object->getName();

	calculateNewTargetPosition(target_position, target_rotation);

	if (camera_name == CAMERA_MANAGER_NAME_PARALLEL) {
		calculateParallelCamera(target_position, target_rotation);
	}
	else if (camera_name == CAMERA_MANAGER_NAME_PAN) {
		calculatePanCamera(target_position, target_rotation);
	}
	else if (camera_name == CAMERA_MANAGER_NAME_POINT) {
		calculatePointCamera(target_position, target_rotation);
	}
	else if (camera_name == CAMERA_MANAGER_NAME_OBJECT_LOOK) {

	}
	else {
		SHOW_MSG(("No handling for this camera type: " + camera_name).c_str());
	}
}


void CameraInfluence::calculateNewTargetPosition(Ogre::Vector3 target_position, Ogre::Quaternion target_rotation) {
	Ogre::Vector3 target_front = target_rotation * Ogre::Vector3::UNIT_Z;
	Ogre::Vector3 target_up    = target_rotation * Ogre::Vector3::UNIT_Y;
	Ogre::Vector3 target_right = target_rotation * Ogre::Vector3::UNIT_X;

	new_target_position = target_position;
	new_target_position += target_front * e_targetoffset_front->value;
	new_target_position += target_up    * e_targetoffset_up->value;
	new_target_position += target_right * e_targetoffset_right->value;
}


void CameraInfluence::calculateParallelCamera(Ogre::Vector3 target_position, Ogre::Quaternion target_rotation) {
	camera->setPosition(new_target_position);
	camera->setDirection(Ogre::Vector3(0.0, 0.0, -1.0));
	camera->yaw(Ogre::Degree(e_yaw->value));
	camera->pitch(Ogre::Degree(-e_pitch->value));
	camera->roll(Ogre::Degree(e_zrot->value));

	camera->moveRelative(Ogre::Vector3(0.0, 0.0, e_distance->value));

	// Store Results
	result_position = camera->getPosition();
	result_rotation = camera->getOrientation();
	fovy = Ogre::Degree(e_fovy->value);
}

void CameraInfluence::calculatePanCamera(Ogre::Vector3 target_position, Ogre::Quaternion target_rotation) {
	LibGens::Vector3 camera_position_v = camera_object->getPosition();
	Ogre::Vector3 camera_position(camera_position_v.x, camera_position_v.y, camera_position_v.z);

	// Set Direction
	camera->setPosition(camera_position);
	camera->lookAt(new_target_position);

	// Reset Position to target
	camera->setPosition(new_target_position);
	camera->roll(Ogre::Degree(e_zrot->value));
	camera->moveRelative(Ogre::Vector3(0.0, 0.0, e_distance->value));


	// Store Results
	result_position = camera->getPosition();
	result_rotation = camera->getOrientation();
	fovy = Ogre::Degree(e_fovy->value);
}


void CameraInfluence::calculatePointCamera(Ogre::Vector3 target_position, Ogre::Quaternion target_rotation) {
	camera->setPosition(new_target_position);

	LibGens::Vector3 camera_position_v = camera_object->getPosition();
	Ogre::Vector3 camera_position(camera_position_v.x, camera_position_v.y, camera_position_v.z);
	camera->lookAt(camera_position);

	camera->roll(Ogre::Degree(e_zrot->value));
	camera->moveRelative(Ogre::Vector3(0.0, 0.0, e_distance->value));

	// Store Results
	result_position = camera->getPosition();
	result_rotation = camera->getOrientation();
	fovy = Ogre::Degree(e_fovy->value);
}


CameraManager::CameraManager() {
	force_camera = false;
	level = NULL;
	reference_camera = NULL;
}

void CameraManager::checkCameraVolumeChanger(LibGens::Object *object, Ogre::Vector3 position_p) {
	LibGens::ObjectElementFloat *e_x   = (LibGens::ObjectElementFloat *) object->getElement(CAMERA_MANAGER_ELEMENT_CAMERA_VOLUME_X);
	LibGens::ObjectElementFloat *e_y   = (LibGens::ObjectElementFloat *) object->getElement(CAMERA_MANAGER_ELEMENT_CAMERA_VOLUME_Y);
	LibGens::ObjectElementFloat *e_z   = (LibGens::ObjectElementFloat *) object->getElement(CAMERA_MANAGER_ELEMENT_CAMERA_VOLUME_Z);
	LibGens::ObjectElementID    *e_t   = (LibGens::ObjectElementID *)    object->getElement(CAMERA_MANAGER_ELEMENT_CAMERA_TARGET);
	LibGens::ObjectElementFloat *e_p   = (LibGens::ObjectElementFloat *) object->getElement(CAMERA_MANAGER_ELEMENT_CAMERA_PRIORITY);
	LibGens::ObjectElementFloat *e_in  = (LibGens::ObjectElementFloat *) object->getElement(CAMERA_MANAGER_ELEMENT_EASE_IN);
	LibGens::ObjectElementFloat *e_out = (LibGens::ObjectElementFloat *) object->getElement(CAMERA_MANAGER_ELEMENT_EASE_OUT);

	if (e_x && e_y && e_z && e_t && e_p && e_in && e_out) {
		LibGens::Vector3 center=object->getPosition();
		LibGens::Quaternion rotation=object->getRotation();

		LibGens::Vector3 camera_point(position_p.x, position_p.y, position_p.z);
		camera_point = rotation.inverse() * (camera_point - center);

		if ((abs(camera_point.x) < (e_x->value/2.0)) && (abs(camera_point.y) < (e_y->value/2.0)) && (abs(camera_point.z) < (e_z->value/2.0))) {
			LibGens::Object *target_object = level->getObjectByID(e_t->value);

			if (target_object) {
				// Check if object is already on influence list
				bool found=false;
				for (list<CameraInfluence *>::iterator it=current_camera_influences.begin(); it!=current_camera_influences.end(); it++) {
					if ((*it)->getCameraObject() == target_object) {
						found = true;
						break;
					}
				}

				// If not in list, add new influence
				if (!found) {
					CameraInfluence *camera_influence = new CameraInfluence(reference_camera, target_object, (int) e_p->value, e_in->value, e_out->value);

					bool added=false;

					for (list<CameraInfluence *>::iterator it=current_camera_influences.begin(); it!=current_camera_influences.end(); it++) {
						if ((*it)->getPriority() > camera_influence->getPriority()) {
							current_camera_influences.insert(it, camera_influence);
							added = true;
							break;
						}
					}

					if (!added) {
						current_camera_influences.push_back(camera_influence);
					}
				}
			}
		}
		else {
			LibGens::Object *target_object = level->getObjectByID(e_t->value);

			// Search for object in influence list, and deactivate it
			if (target_object) {
				for (list<CameraInfluence *>::iterator it=current_camera_influences.begin(); it!=current_camera_influences.end(); it++) {
					if ((*it)->getCameraObject() == target_object) {
						(*it)->setActivated(false);
						break;
					}
				}
			}
		}
	}
}

void CameraManager::update(float time_s, GhostNode *ghost_node) {
	if (!level) return;
	if (!reference_camera) return;

	// Check object sets for new camera influences
	updateCameraObjects(time_s, ghost_node->getPosition());
	updateCameraInfluences(time_s);

	if (force_camera) {
		calculateCameraFromInfluences(ghost_node);
	}
}

void CameraManager::updateCameraObjects(float time_s, Ogre::Vector3 position_p) {
	list<LibGens::ObjectSet *> sets=level->getSets();

	for (list<LibGens::ObjectSet *>::iterator set=sets.begin(); set!=sets.end(); set++) {
		list<LibGens::Object *> objects=(*set)->getObjects();

		for (list<LibGens::Object *>::iterator it=objects.begin(); it!=objects.end(); it++) {
			if ((*it)->getName() == CAMERA_MANAGER_NAME_VOLUME_CHANGER) {
				checkCameraVolumeChanger(*it, position_p);
			}
		}
	}
}

void CameraManager::updateCameraInfluences(float time_s) {
	list<CameraInfluence *>::iterator it = current_camera_influences.begin();
	while (it != current_camera_influences.end()) {
		(*it)->addTime(time_s);

		if ((*it)->toDelete()) {
			it = current_camera_influences.erase(it);
		}
		else {
			it++;
		}
	}
}

void CameraManager::calculateCameraFromInfluences(GhostNode *ghost_node) {
	Ogre::Vector3 new_target=ghost_node->getCameraPosition();

	LibGens::Vector3 new_target_v;
	new_target_v.x = new_target.x;
	new_target_v.y = new_target.y;
	new_target_v.z = new_target.z;

	new_target += Ogre::Vector3(0.0, 1.7, 0.0);

	Ogre::Vector3 target_front = ghost_node->getCameraRotation() * Ogre::Vector3::UNIT_Z;
	Ogre::Vector3 target_up    = ghost_node->getCameraRotation() * Ogre::Vector3::UNIT_Y;
	Ogre::Vector3 target_right = ghost_node->getCameraRotation() * Ogre::Vector3::UNIT_X;

	LibGens::Vector3 spline_tangent(0, 0, 0);

	LibGens::Spline *ghost_spline=ghost_node->getSpline();
	LibGens::Node *ghost_spline_node=ghost_node->getSplineNode();

	// Calculate Default Camera
	if ((ghost_node->getMovementMode() == MOVEMENT_MODE_2D) && ghost_spline && ghost_spline_node) {
		LibGens::Vector3 ghost_point = ghost_spline_node->findClosestPoint(ghost_spline, new_target_v, NULL, &spline_tangent, true);

		camera_marker_node->setPosition(ghost_point.x, ghost_point.y, ghost_point.z);
		camera_marker_tangent->setPosition(ghost_point.x + spline_tangent.x * 3.0, ghost_point.y + spline_tangent.y * 3.0, ghost_point.z + spline_tangent.z * 3.0);

		reference_camera->setPosition(new_target);
		reference_camera->move(target_front * 1.1);
		reference_camera->move(target_up    * 0.3);

		reference_camera->setDirection(Ogre::Vector3(spline_tangent.x, 0.0, spline_tangent.z));
		reference_camera->yaw(Ogre::Degree(90));

		reference_camera->moveRelative(Ogre::Vector3(0.0, 0.0, 13.2001));
	}
	else {
		reference_camera->setPosition(new_target);
		reference_camera->setDirection(target_front);
		reference_camera->moveRelative(Ogre::Vector3(0.0, 0.0, 4.5));
	}

	// Store results of Default Camera
	Ogre::Vector3    result_position = reference_camera->getPosition();
	Ogre::Quaternion result_rotation = reference_camera->getOrientation();
	Ogre::Radian     result_fovy     = Ogre::Degree(45);
	
	// Calculate Cameras for each influence, and interpolate it with the results
	for (list<CameraInfluence *>::iterator it=current_camera_influences.begin(); it!=current_camera_influences.end(); it++) {
		// Calculate Camera Influence based on the current target
		(*it)->calculateCamera(new_target, ghost_node->getCameraRotation());

		// Retrieve Camera Influence Results
		Ogre::Vector3    influence_position = (*it)->getPosition();
		Ogre::Quaternion influence_rotation = (*it)->getRotation();
		Ogre::Radian     influence_fovy     = (*it)->getFovy();
		float            influence          = (*it)->getInfluence();

		// Interpolate Position by Influence
		result_position.x = result_position.x + (influence_position.x - result_position.x) * influence;
		result_position.y = result_position.y + (influence_position.y - result_position.y) * influence;
		result_position.z = result_position.z + (influence_position.z - result_position.z) * influence;

		// Interpolate Rotation by Influence
		result_rotation = Ogre::Quaternion::Slerp(influence, result_rotation, influence_rotation, true);

		// Interpolate FOVy by Influence
		result_fovy = result_fovy + (influence_fovy - result_fovy) * influence;
	}

	// Force Cameras
	for (list<Ogre::Camera *>::iterator it=cameras_to_force.begin(); it!=cameras_to_force.end(); it++) {
		(*it)->setPosition(result_position);
		(*it)->setOrientation(result_rotation);
		(*it)->setFOVy(result_fovy);
	}
}

