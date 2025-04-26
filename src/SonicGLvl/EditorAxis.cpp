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

EditorAxis::EditorAxis(Ogre::SceneManager *scene_manager) {
	x_axis = scene_manager->createEntity(EDITOR_AXIS_POSITION_X_AXIS_NAME, EDITOR_AXIS_POSITION_X_AXIS_FILENAME);
	y_axis = scene_manager->createEntity(EDITOR_AXIS_POSITION_Y_AXIS_NAME, EDITOR_AXIS_POSITION_Y_AXIS_FILENAME);
	z_axis = scene_manager->createEntity(EDITOR_AXIS_POSITION_Z_AXIS_NAME, EDITOR_AXIS_POSITION_Z_AXIS_FILENAME);
	w_axis = scene_manager->createEntity(EDITOR_AXIS_POSITION_W_AXIS_NAME, EDITOR_AXIS_POSITION_W_AXIS_FILENAME);

	x_axis->setQueryFlags(EDITOR_NODE_AXIS);
	y_axis->setQueryFlags(EDITOR_NODE_AXIS);
	z_axis->setQueryFlags(EDITOR_NODE_AXIS);
	w_axis->setQueryFlags(EDITOR_NODE_AXIS);

	scene_node_position = scene_manager->getRootSceneNode()->createChildSceneNode();
	scene_node_position->setVisible(false);

	// FIXME: 
	// Use proper meshes on scale
    Ogre::SceneNode* x_axis_node = scene_node_position->createChildSceneNode(EDITOR_AXIS_POSITION_X_AXIS_NAME);
	x_axis_node->attachObject(x_axis);
	x_axis_node->translate(-64, 0, 0);
	x_axis_node->roll(Ogre::Degree(90));

	Ogre::SceneNode* y_axis_node = scene_node_position->createChildSceneNode(EDITOR_AXIS_POSITION_Y_AXIS_NAME);
	y_axis_node->translate(0, 64, 0);
	y_axis_node->attachObject(y_axis);

	Ogre::SceneNode* z_axis_node = scene_node_position->createChildSceneNode(EDITOR_AXIS_POSITION_Z_AXIS_NAME);
	z_axis_node->attachObject(z_axis);
	z_axis_node->translate(0, 0, 64);
	z_axis_node->pitch(Ogre::Degree(90));

	Ogre::SceneNode* w_axis_node = scene_node_position->createChildSceneNode(EDITOR_AXIS_POSITION_W_AXIS_NAME);
	w_axis_node->attachObject(w_axis);
	//z_axis_node->translate(0, 0, 64);
	//z_axis_node->pitch(Ogre::Degree(90));
	//


	x_axis_rot = scene_manager->createEntity(EDITOR_AXIS_ROTATION_X_AXIS_NAME, EDITOR_AXIS_ROTATION_X_AXIS_FILENAME);
	y_axis_rot = scene_manager->createEntity(EDITOR_AXIS_ROTATION_Y_AXIS_NAME, EDITOR_AXIS_ROTATION_Y_AXIS_FILENAME);
	z_axis_rot = scene_manager->createEntity(EDITOR_AXIS_ROTATION_Z_AXIS_NAME, EDITOR_AXIS_ROTATION_Z_AXIS_FILENAME);

	x_axis_rot->setQueryFlags(EDITOR_NODE_AXIS);
	y_axis_rot->setQueryFlags(EDITOR_NODE_AXIS);
	z_axis_rot->setQueryFlags(EDITOR_NODE_AXIS);
	
	scene_node_rotation = scene_manager->getRootSceneNode()->createChildSceneNode();
	scene_node_rotation->setVisible(false);

    Ogre::SceneNode* x_axis_rot_node = scene_node_rotation->createChildSceneNode(EDITOR_AXIS_ROTATION_X_AXIS_NAME);
	x_axis_rot_node->attachObject(x_axis_rot);
	
	Ogre::SceneNode* y_axis_rot_node = scene_node_rotation->createChildSceneNode(EDITOR_AXIS_ROTATION_Y_AXIS_NAME);
	y_axis_rot_node->attachObject(y_axis_rot);
	
	Ogre::SceneNode* z_axis_rot_node = scene_node_rotation->createChildSceneNode(EDITOR_AXIS_ROTATION_Z_AXIS_NAME);
	z_axis_rot_node->attachObject(z_axis_rot);

	mode    = false;
	holding = false;
	visible = false;
	position = Ogre::Vector3::ZERO;
	rotation = Ogre::Quaternion::IDENTITY;

	current_w_offset = 0.0f;
}


void EditorAxis::update(EditorViewport *viewport) {
	if (!viewport) {
		return;
	}

	Ogre::Camera *camera=viewport->getCamera();
	if (!camera) {
		return;
	}

	scene_node_position->setPosition(position);
	scene_node_position->setOrientation(rotation);

	scene_node_rotation->setPosition(position);
	scene_node_rotation->setOrientation(rotation);

	float distance=camera->getPosition().distance(position);
	scene_node_position->setScale(Ogre::Vector3(distance*EDITOR_AXIS_POSITION_SCALE_MULTIPLIER));
	scene_node_rotation->setScale(Ogre::Vector3(distance*EDITOR_AXIS_ROTATION_SCALE_MULTIPLIER));

	scene_node_position->setVisible(visible && !mode);
	scene_node_rotation->setVisible(visible && mode);
}


Ogre::Vector3 EditorAxis::raycastPointToAxis(EditorViewport *viewport, float raycast_x, float raycast_y, int axis) {
	Ogre::Vector3 point(0, 0, 0);
	Ogre::Ray ray = viewport->getCamera()->getCameraToViewportRay(raycast_x, raycast_y);

	if (current_axis == LIBGENS_MATH_AXIS_W) {
		Ogre::Vector3 result(0, 0, 0);
		viewport->raycastPlacement(raycast_x, raycast_y, current_w_offset, &result, EDITOR_NODE_QUERY_TERRAIN | EDITOR_NODE_QUERY_HAVOK);
		return result;
	}
	else {
		// Calculate normal, tangent, and bitangent
		Ogre::Vector3 normal(0.0f, 0.0f, 0.0f);
		if (axis == LIBGENS_MATH_AXIS_X) normal.x = 1.0f;
		if (axis == LIBGENS_MATH_AXIS_Y) normal.y = 1.0f;
		if (axis == LIBGENS_MATH_AXIS_Z) normal.z = 1.0f;
		normal = rotation * normal;

		Ogre::Vector3 c1 = normal.crossProduct(Ogre::Vector3(0.0f, 0.0f, 1.0f));
		Ogre::Vector3 c2 = normal.crossProduct(Ogre::Vector3(0.0f, 1.0f, 0.0f));
		Ogre::Vector3 tangent;

		if(c1.length() > c2.length()) tangent = c1;
		else tangent = c2;
		tangent.normalise();
		Ogre::Vector3 bitangent = tangent.crossProduct(normal);

		// Retrieve current camera direction (sign doesn't matter since both sides of the plane will be compared)
		Ogre::Vector3 camera_direction = viewport->getCamera()->getDirection();

		// Create both tangent and bitangent planes
		Ogre::Plane plane_tangent(position, position+normal, position+tangent);
		Ogre::Plane plane_bitangent(position, position+normal, position+bitangent);

		// Calculate 4 Dot Products, 2 for each plane
		float winning_dot_product       = -1.0;
		float dot_product_tangent       = camera_direction.dotProduct(plane_tangent.normal);
		float dot_product_tangent_inv   = camera_direction.dotProduct(-plane_tangent.normal);
		float dot_product_bitangent     = camera_direction.dotProduct(plane_bitangent.normal);
		float dot_product_bitangent_inv = camera_direction.dotProduct(-plane_bitangent.normal);

		// Find the best plane with the normal that is closest to the camera direction
		Ogre::Plane result_plane;
		if (dot_product_tangent > winning_dot_product) {
			result_plane = plane_tangent;
			winning_dot_product = dot_product_tangent;
		}

		if (dot_product_tangent_inv > winning_dot_product) {
			result_plane = plane_tangent;
			winning_dot_product = dot_product_tangent_inv;
		}

		if (dot_product_bitangent > winning_dot_product) {
			result_plane = plane_bitangent;
			winning_dot_product = dot_product_bitangent;
		}

		if (dot_product_bitangent_inv > winning_dot_product) {
			result_plane = plane_bitangent;
			winning_dot_product = dot_product_bitangent_inv;
		}

		// Calculate Intersection
		std::pair<bool, Ogre::Real> result_intersection=ray.intersects(result_plane);

		Ogre::Vector3 result = position;
		Ogre::Vector3 result_point_plane;
		Ogre::Vector3 result_point_plane_fixed;

		if (result_intersection.first) {
			result_point_plane     = ray.getPoint(result_intersection.second);

			Ogre::Vector3 distance = result_point_plane - position;
			float magnitude=(distance.dotProduct(normal));
			result_point_plane_fixed  = position + normal * magnitude;
			result = result_point_plane_fixed;
		}

		return result;
	}

	return position;
}


Ogre::Radian EditorAxis::raycastAngleToAxis(EditorViewport *viewport, float raycast_x, float raycast_y, int axis) {
	Ogre::Vector3 point(0, 0, 0);
	Ogre::Ray ray = viewport->getCamera()->getCameraToViewportRay(raycast_x, raycast_y);
	Ogre::Vector3 normal(0.0f, 0.0f, 0.0f);
	if (axis == LIBGENS_MATH_AXIS_X) normal.x = 1.0f;
	if (axis == LIBGENS_MATH_AXIS_Y) normal.y = 1.0f;
	if (axis == LIBGENS_MATH_AXIS_Z) normal.z = 1.0f;
	normal = rotation * normal;

	Ogre::Vector3 c1 = normal.crossProduct(Ogre::Vector3(0.0f, 0.0f, 1.0f));
	Ogre::Vector3 c2 = normal.crossProduct(Ogre::Vector3(0.0f, 1.0f, 0.0f));
	Ogre::Vector3 tangent;

	if(c1.length() > c2.length()) tangent = c1;
	else tangent = c2;
	tangent.normalise();
	Ogre::Vector3 bitangent = tangent.crossProduct(normal);

	Ogre::Plane plane(position, position+tangent, position+bitangent);
	std::pair<bool, Ogre::Real> result = ray.intersects(plane);

	if (result.first) {
		Ogre::Vector3 result_point = ray.getPoint(result.second);
		float u = (result_point - position).dotProduct(tangent);
		float v = (result_point - position).dotProduct(bitangent);
		return Ogre::Math::ATan2(v, u);
	}

	return Ogre::Radian(0);
}


bool EditorAxis::mouseMoved(EditorViewport *viewport, const OIS::MouseEvent &arg) {
	if (!holding) {
		current_axis = -1;

		if ((viewport->getCurrentEntity() == x_axis) && !mode) {
			current_axis=LIBGENS_MATH_AXIS_X;
			x_axis->setMaterialName(EDITOR_AXIS_MATERIAL_X_AXIS_HIGHLIGHT_NAME);
		}
		else x_axis->setMaterialName(EDITOR_AXIS_MATERIAL_X_AXIS_NAME);

		if ((viewport->getCurrentEntity() == y_axis) && !mode) {
			current_axis=LIBGENS_MATH_AXIS_Y;
			y_axis->setMaterialName(EDITOR_AXIS_MATERIAL_Y_AXIS_HIGHLIGHT_NAME);
		}
		else y_axis->setMaterialName(EDITOR_AXIS_MATERIAL_Y_AXIS_NAME);

		if ((viewport->getCurrentEntity() == z_axis) && !mode) {
			current_axis=LIBGENS_MATH_AXIS_Z;
			z_axis->setMaterialName(EDITOR_AXIS_MATERIAL_Z_AXIS_HIGHLIGHT_NAME);
		}
		else z_axis->setMaterialName(EDITOR_AXIS_MATERIAL_Z_AXIS_NAME);

		if ((viewport->getCurrentEntity() == w_axis) && !mode) {
			current_axis=LIBGENS_MATH_AXIS_W;
			w_axis->setMaterialName(EDITOR_AXIS_MATERIAL_W_AXIS_HIGHLIGHT_NAME);
		}
		else w_axis->setMaterialName(EDITOR_AXIS_MATERIAL_W_AXIS_NAME);


		if ((viewport->getCurrentEntity() == x_axis_rot) && mode) {
			current_axis=LIBGENS_MATH_AXIS_X;
			x_axis_rot->setMaterialName(EDITOR_AXIS_MATERIAL_X_AXIS_HIGHLIGHT_NAME);
		}
		else x_axis_rot->setMaterialName(EDITOR_AXIS_MATERIAL_X_AXIS_NAME);

		if ((viewport->getCurrentEntity() == y_axis_rot) && mode) {
			current_axis=LIBGENS_MATH_AXIS_Y;
			y_axis_rot->setMaterialName(EDITOR_AXIS_MATERIAL_Y_AXIS_HIGHLIGHT_NAME);
		}
		else y_axis_rot->setMaterialName(EDITOR_AXIS_MATERIAL_Y_AXIS_NAME);

		if ((viewport->getCurrentEntity() == z_axis_rot) && mode) {
			current_axis=LIBGENS_MATH_AXIS_Z;
			z_axis_rot->setMaterialName(EDITOR_AXIS_MATERIAL_Z_AXIS_HIGHLIGHT_NAME);
		}
		else z_axis_rot->setMaterialName(EDITOR_AXIS_MATERIAL_Z_AXIS_NAME);

		return false;
	}
	else {
		float raycast_x=arg.state.X.abs/float(arg.state.width);
		float raycast_y=arg.state.Y.abs/float(arg.state.height);
		viewport->convertMouseToLocalScreen(raycast_x, raycast_y);

		if (!mode) {
			Ogre::Vector3 point=raycastPointToAxis(viewport, raycast_x, raycast_y, current_axis);

			if (current_axis == LIBGENS_MATH_AXIS_W) {
				translate = point - position;
				position += translate;
			}
			else {
				translate = point-holding_offset;
				holding_offset = point;
				position += translate;
			}
		}
		else {
			Ogre::Radian angle=raycastAngleToAxis(viewport, raycast_x, raycast_y, current_axis);
			Ogre::Radian rotate_a=holding_angle-angle;
			Ogre::Vector3 axis;

			if (current_axis==LIBGENS_MATH_AXIS_X) axis = Ogre::Vector3::UNIT_X;
			if (current_axis==LIBGENS_MATH_AXIS_Y) axis = Ogre::Vector3::UNIT_Y;
			if (current_axis==LIBGENS_MATH_AXIS_Z) axis = Ogre::Vector3::UNIT_Z;

			int degrees = rotate_a.valueDegrees();

			if (isRotationSnap())
			{
				if (degrees != 0 && degrees % 5 == 0)
					rotate_a = ((float)degrees * LIBGENS_MATH_PI) / 180;
				else
					return false;
			}

			rotate.FromAngleAxis(rotate_a, axis);
			rotate.normalise();
			if (!rotation_frozen) rotation = rotation * rotate;
			holding_angle = angle;
		}

		return true;
	}

	return false;
}



bool EditorAxis::mousePressed(EditorViewport *viewport, const OIS::MouseEvent &arg, OIS::MouseButtonID id) {
	if (isHighlighted() && (id == OIS::MB_Left)) {
		float raycast_x=arg.state.X.abs/float(arg.state.width);
		float raycast_y=arg.state.Y.abs/float(arg.state.height);

		viewport->convertMouseToLocalScreen(raycast_x, raycast_y);

		if (!mode) {
			holding_offset = raycastPointToAxis(viewport, raycast_x, raycast_y, current_axis);
			current_w_offset = viewport->getCamera()->getPosition().distance(position);
		}
		else {
			holding_angle  = raycastAngleToAxis(viewport, raycast_x, raycast_y, current_axis);
		}
		holding = true;
		return true;
	}
	else if (id == OIS::MB_Right && !holding) {
		mode_tap_time = std::chrono::steady_clock::now();
		return true;
	}
	return false;
}

bool EditorAxis::mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id) {
	if (id == OIS::MB_Left) {
		bool last_holding=holding;
		holding = false;
		return last_holding;
	}

	if (id == OIS::MB_Right && (std::chrono::steady_clock::now() - mode_tap_time) < std::chrono::milliseconds(200)) {
		mode = !mode;
	}

	return false;
}