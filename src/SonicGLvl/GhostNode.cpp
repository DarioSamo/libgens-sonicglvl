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

#include "GhostNode.h"
#include "EditorApplication.h"
#include "ObjectSet.h"

GhostNode::GhostNode(LibGens::Ghost *ghost_p, Ogre::SceneManager *scene_manager, LibGens::ModelLibrary *model_library, LibGens::MaterialLibrary *material_library) {
	type = EDITOR_NODE_GHOST;

	ghost = ghost_p;
	scene_node = scene_manager->getRootSceneNode()->createChildSceneNode();

	LibGens::Model *model=model_library->getModel("chr_Sonic_HD");

	if (model) {
		buildModel(scene_node, model, model->getName(), "chr_Sonic_HD", scene_manager, material_library, EDITOR_NODE_QUERY_GHOST, GENERAL_MESH_GROUP, false);
	}
	else {
		SHOW_MSG("No model named chr_Sonic_HD was found for creating the ghost!");
	}

	/*
	ghost_spin_entity=scene_manager->createEntity("chr_Sonic_spin_HD_1.mesh");
	ghost_spin_entity->setQueryFlags(EDITOR_NODE_QUERY_GHOST);
	scene_node->attachObject(ghost_spin_entity);
	ghost_spin_entity->setVisible(false);
	*/
	
	scene_node->getUserObjectBindings().setUserAny(EDITOR_NODE_BINDING, Ogre::Any((EditorNode *)this));

	camera_position = Ogre::Vector3::ZERO;
	camera_rotation = Ogre::Quaternion::IDENTITY;
	
	position = Ogre::Vector3::ZERO;
	rotation = Ogre::Quaternion::IDENTITY;
	scale    = Ogre::Vector3::UNIT_SCALE;

	selected = false;
	play = false;
	current_time = 0;
	current_animation_name = "";
	current_animation_state = NULL;
	current_movement_mode = MOVEMENT_MODE_3D;
	current_movement_delta = Ogre::Vector3::ZERO;
	current_changer_object = NULL;
	current_spline = NULL;
	current_spline_node = NULL;
}


void GhostNode::addTime(float time) {
	if (!ghost) return;
	if (!play) return;
	current_time += time;

	LibGens::Vector3 pv;
	LibGens::Quaternion ov;
	string animation_name;
	float animation_frame;
	bool animation_ball;
	ghost->calculate(current_time, pv, ov, animation_name, animation_frame, animation_ball);

	//ghost_entity->setVisible(!animation_ball);
	//ghost_spin_entity->setVisible(animation_ball);
	
	if (!animation_ball) {
		if (current_animation_name != animation_name) {
			current_animation_name = animation_name;

			if (current_animation_state) {
				current_animation_state->setEnabled(false);
				current_animation_state = NULL;
			}

			string entity_animation_name = animation_mappings[current_animation_name];

			unsigned short attached_objects=scene_node->numAttachedObjects();
			for (unsigned short i=0; i<attached_objects; i++) {
				Ogre::Entity *entity=static_cast<Ogre::Entity *>(scene_node->getAttachedObject(i));
				if (entity->hasAnimationState(entity_animation_name)) {
					current_animation_state = entity->getAnimationState(entity_animation_name);
					current_animation_state->setLoop(true);
					current_animation_state->setEnabled(true);
					break;
				}
			}
		}

		if (current_animation_state) {
			current_animation_state->setTimePosition(animation_frame / 60.0f);
		}
	}
    
	Ogre::Vector3 new_position(pv.x, pv.y, pv.z);
	Ogre::Vector3 movement(new_position - camera_position);
	current_movement_delta = movement;

	setPosition(new_position);
	setRotation(Ogre::Quaternion(ov.w, ov.x, ov.y, ov.z));

	movement.y = 0.0;
	if (time < 0.0f) {
		movement = -movement;
	}

	Ogre::Real magnitude = movement.normalise();
	if (magnitude > 0.0) {
		Ogre::Radian movement_angle = Ogre::Degree(90) - Ogre::Math::ATan2(movement.z, movement.x);

		float speed_influence_factor = 300.0f * time;
		float speed_influence = magnitude / speed_influence_factor;

		if (speed_influence > 1.0f) speed_influence = 1.0f;

		Ogre::Quaternion new_camera_rotation = Ogre::Quaternion(movement_angle, Ogre::Vector3::UNIT_Y);
		camera_rotation = Ogre::Quaternion::Slerp(speed_influence, camera_rotation, new_camera_rotation);
	}

	/*
	Ogre::Skeleton *skeleton=ghost_entity->getSkeleton();
	Ogre::Bone *bone=skeleton->getBone("Head");
	camera_position = scene_node->getPosition() + scene_node->getOrientation()*bone->_getDerivedPosition();    
	
	if (!animation_ball) camera_direction = (scene_node->getOrientation() * bone->_getDerivedOrientation()) * Ogre::Vector3::NEGATIVE_UNIT_Z;
	else camera_direction = (scene_node->getOrientation() * bone->_getDerivedOrientation()) * Ogre::Vector3::UNIT_Z;
	*/
}


void GhostNode::checkModeChangers(LibGens::Level *level) {
	if (!level) return;

	list<LibGens::ObjectSet *> sets=level->getSets();

	for (list<LibGens::ObjectSet *>::iterator set=sets.begin(); set!=sets.end(); set++) {
		list<LibGens::Object *> objects=(*set)->getObjects();

		for (list<LibGens::Object *>::iterator it=objects.begin(); it!=objects.end(); it++) {
			if (checkModeChanger(*it)) {
				break;
			}
		}
	}
}


void GhostNode::checkSplines(LibGens::Level *level) {
	if (!level) return;

	if ((current_movement_mode == MOVEMENT_MODE_3D) || (current_movement_mode == MOVEMENT_MODE_DIVE)) {
		return;
	}

	string path_tag="";
	if (current_movement_mode == MOVEMENT_MODE_2D) {
		path_tag = LIBGENS_PATH_TAG_2D;
	}

	Ogre::Vector3 po = camera_position + Ogre::Vector3(0.0, 0.5, 0.0);
	float path_detection_treshold = 4.0;
	float closest_distance = path_detection_treshold;

	if (path_tag.size()) {
		list<LibGens::Path *> paths = level->getPaths();
		for (list<LibGens::Path *>::iterator it=paths.begin(); it!=paths.end(); it++) {
			LibGens::Library *path_library = (*it)->getLibrary();
			LibGens::Scene *path_scene = (*it)->getScene();

			list<LibGens::Node *> path_scene_nodes = path_scene->getNodes();
			for (list<LibGens::Node *>::iterator it_n = path_scene_nodes.begin(); it_n != path_scene_nodes.end(); it_n++) {
				LibGens::Node *path_node = (*it_n);
				string path_instance_name = path_node->getInstanceName();

				// Delete first character #
				path_instance_name.erase(path_instance_name.begin());

				LibGens::Spline *path_spline = path_library->getSpline(path_instance_name);

				bool has_path_tag = (path_instance_name.find(path_tag) != string::npos);
				bool has_super_sonic_tag = (path_instance_name.find(LIBGENS_PATH_TAG_SUPERSONIC) != string::npos);

				if (path_spline && has_path_tag && !has_super_sonic_tag) {
					float spline_distance=LIBGENS_AABB_MAX_START;
					path_node->findClosestPoint(path_spline, LibGens::Vector3(po.x, po.y, po.z), &spline_distance);

					if (spline_distance < closest_distance) {
						closest_distance    = spline_distance;
						current_spline_node = path_node;
						current_spline      = path_spline;
					}
				}
			}
		}
	}
}


bool GhostNode::checkModeChanger(LibGens::Object *object) {
	if (!object) return false;

	if (object->getName() == MOVEMENT_MODE_CHANGER_3DTO2D_NAME) {
		Ogre::Ray ray(camera_position+Ogre::Vector3(0.0, 0.5, 0.0), current_movement_delta);

		LibGens::ObjectElementFloat *e_x = (LibGens::ObjectElementFloat *) object->getElement("Collision_Width");
		LibGens::ObjectElementFloat *e_y = (LibGens::ObjectElementFloat *) object->getElement("Collision_Height");

		if (e_x && e_y) {
			float width = e_x->value;
			float height = e_y->value;

			LibGens::Vector3 pv = object->getPosition();
			LibGens::Quaternion rv = object->getRotation();
			Ogre::Vector3 corners[4];

			corners[0] = Ogre::Vector3(-width/2, -height/2, 0);
			corners[1] = Ogre::Vector3(+width/2, -height/2, 0);
			corners[2] = Ogre::Vector3(-width/2, +height/2, 0);
			corners[3] = Ogre::Vector3(+width/2, +height/2, 0);

			Ogre::Matrix4 transform_matrix(Ogre::Quaternion(rv.w, rv.x, rv.y, rv.z));
			transform_matrix.setTrans(Ogre::Vector3(pv.x, pv.y, pv.z));

			corners[0] = transform_matrix * corners[0];
			corners[1] = transform_matrix * corners[1];
			corners[2] = transform_matrix * corners[2];
			corners[3] = transform_matrix * corners[3];

			std::pair<bool, Ogre::Real> result_t1_positive = Ogre::Math::intersects(ray, corners[0], corners[1], corners[2], false, true);
			std::pair<bool, Ogre::Real> result_t2_positive = Ogre::Math::intersects(ray, corners[3], corners[1], corners[2], true, false);

			std::pair<bool, Ogre::Real> result_t1_negative = Ogre::Math::intersects(ray, corners[0], corners[1], corners[2], true, false);
			std::pair<bool, Ogre::Real> result_t2_negative = Ogre::Math::intersects(ray, corners[3], corners[1], corners[2], false, true);

			float movement_delta_length = current_movement_delta.length();

			// "Activation" Side
			if (result_t1_positive.first && (result_t1_positive.second <= movement_delta_length)) {
				current_movement_mode = MOVEMENT_MODE_2D;
				current_changer_object = object;
				return true;
			}

			if (result_t2_positive.first && (result_t2_positive.second <= movement_delta_length)) {
				current_movement_mode = MOVEMENT_MODE_2D;
				current_changer_object = object;
				return true;
			}

			// "Deactivation" Side
			if (result_t1_negative.first && (result_t1_negative.second <= movement_delta_length)) {
				current_movement_mode = MOVEMENT_MODE_3D;
				current_changer_object = NULL;
				return true;
			}

			if (result_t2_negative.first && (result_t2_negative.second <= movement_delta_length)) {
				current_movement_mode = MOVEMENT_MODE_3D;
				current_changer_object = NULL;
				return true;
			}
		}
	}

	return false;
}


void EditorApplication::loadGhostAnimations() {
	TiXmlDocument doc(SONICGLVL_GHOST_DATABASE_PATH);
	if (!doc.LoadFile()) {
		LibGens::Error::addMessage(LibGens::Error::FILE_NOT_FOUND, (string)SONICGLVL_DATABASE_ERROR_FILE + SONICGLVL_GHOST_DATABASE_PATH);
		return;
	}

	TiXmlHandle hDoc(&doc);
	TiXmlElement* pElem;
	TiXmlHandle hRoot(0);

	pElem=hDoc.FirstChildElement().Element();
	if (!pElem) {
		LibGens::Error::addMessage(LibGens::Error::EXCEPTION, SONICGLVL_DATABASE_ERROR_FILE_ROOT);
		return;
	}

	pElem=pElem->FirstChildElement();
	for(pElem; pElem; pElem=pElem->NextSiblingElement()) {
		string entry_name="";
		string animation_filename="";
		string animation_id="";

		entry_name = pElem->ValueStr();
		pElem->QueryValueAttribute(SONICGLVL_DATABASE_FILENAME_ATTRIBUTE, &animation_filename);
		pElem->QueryValueAttribute(SONICGLVL_DATABASE_ID_ATTRIBUTE, &animation_id);

		if ((entry_name==SONICGLVL_DATABASE_ENTRY) && animation_filename.size() && animation_id.size()) {
			prepareSkeletonAndAnimation("chr_Sonic_HD", animation_filename);
			ghost_animation_mappings[animation_id] = animation_filename;
		}
	}
}