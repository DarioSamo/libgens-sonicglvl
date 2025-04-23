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

ObjectNode::ObjectNode(LibGens::Object *object_p, Ogre::SceneManager *scene_manager, LibGens::ModelLibrary *model_library, LibGens::MaterialLibrary *material_library, LibGens::ObjectProduction *object_production, string slot_id_name) {
	type = EDITOR_NODE_OBJECT;

	object = object_p;
	animation_state = NULL;
	
    scene_node = scene_manager->getRootSceneNode()->createChildSceneNode();
	preview_box_node = scene_node->createChildSceneNode();

	scene_node->getUserObjectBindings().setUserAny(EDITOR_NODE_BINDING, Ogre::Any((EditorNode *)this));
	
	selected = false;
	force_hide = false;
	offset_rotation_animation_enabled = false;

	current_model_name = "";
	current_animation_name = "";
	current_skeleton_name = "";
	current_type_name = "";

	
	// Look for offset rotations

	string offset_rotation_x_str=object->queryExtraName(OBJECT_NODE_EXTRA_OFFSET_ROTATION_X, "0.0");
	string offset_rotation_y_str=object->queryExtraName(OBJECT_NODE_EXTRA_OFFSET_ROTATION_Y, "0.0");
	string offset_rotation_z_str=object->queryExtraName(OBJECT_NODE_EXTRA_OFFSET_ROTATION_Z, "0.0");

	string offset_rotation_x_speed_str=object->queryExtraName(OBJECT_NODE_EXTRA_OFFSET_ROTATION_X_SPEED, "0.0");
	string offset_rotation_y_speed_str=object->queryExtraName(OBJECT_NODE_EXTRA_OFFSET_ROTATION_Y_SPEED, "0.0");
	string offset_rotation_z_speed_str=object->queryExtraName(OBJECT_NODE_EXTRA_OFFSET_ROTATION_Z_SPEED, "0.0");

	float offset_rotation_x_f=0;
	float offset_rotation_y_f=0;
	float offset_rotation_z_f=0;

	float offset_rotation_x_speed_f=0;
	float offset_rotation_y_speed_f=0;
	float offset_rotation_z_speed_f=0;

	FromString<float>(offset_rotation_x_f, offset_rotation_x_str, std::dec);
	FromString<float>(offset_rotation_y_f, offset_rotation_y_str, std::dec);
	FromString<float>(offset_rotation_z_f, offset_rotation_z_str, std::dec);

	FromString<float>(offset_rotation_x_speed_f, offset_rotation_x_speed_str, std::dec);
	FromString<float>(offset_rotation_y_speed_f, offset_rotation_y_speed_str, std::dec);
	FromString<float>(offset_rotation_z_speed_f, offset_rotation_z_speed_str, std::dec);

	offset_rotation_x = Ogre::Degree(offset_rotation_x_f);
	offset_rotation_y = Ogre::Degree(offset_rotation_y_f);
	offset_rotation_z = Ogre::Degree(offset_rotation_z_f);

	offset_rotation_x_speed = Ogre::Degree(offset_rotation_x_speed_f);
	offset_rotation_y_speed = Ogre::Degree(offset_rotation_y_speed_f);
	offset_rotation_z_speed = Ogre::Degree(offset_rotation_z_speed_f);

	Ogre::Radian zero_r(0.0);
	if ((offset_rotation_x_speed != zero_r) || (offset_rotation_y_speed != zero_r) || (offset_rotation_z_speed != zero_r)) {
		offset_rotation_animation_enabled = true;
	}

	// Set position and rotations
	LibGens::Vector3 v=object_p->getPosition();
	setPosition(Ogre::Vector3(v.x, v.y, v.z));
	LibGens::Quaternion q=object_p->getRotation();
	setRotation(Ogre::Quaternion(q.w, q.x, q.y, q.z));

	// Create multiset param nodes
	createObjectMultiSetNodes(object, scene_manager);

	// Load Entities for all nodes
	reloadEntities(scene_manager, model_library, material_library, object_production, slot_id_name);

	preview_box_node->setVisible(false);
}


void ObjectNode::reloadEntities(Ogre::SceneManager *scene_manager, LibGens::ModelLibrary *model_library, LibGens::MaterialLibrary *material_library, LibGens::ObjectProduction *object_production, string slot_id_name) {
	string temp_model_name     = current_model_name;
	string temp_skeleton_name  = current_skeleton_name;
	string temp_animation_name = current_animation_name;
	string temp_type_name      = current_type_name;

	if (scene_node) {
		createEntities(scene_node, scene_manager, model_library, material_library, object_production, slot_id_name);
	}

	list<LibGens::MultiSetNode *> msp_nodes = object->getMultiSetParam()->getNodes();
	for (list<ObjectMultiSetNode *>::iterator it = object_msp_nodes.begin(); it != object_msp_nodes.end(); it++) {
		current_model_name = temp_model_name;
		current_skeleton_name = temp_skeleton_name;
		current_animation_name = temp_animation_name;
		current_type_name = temp_type_name;

		createEntities((*it)->getSceneNode(), scene_manager, model_library, material_library, object_production, slot_id_name);
	}
}


void ObjectNode::createEntities(Ogre::SceneNode *target_node, Ogre::SceneManager *scene_manager, LibGens::ModelLibrary *model_library, LibGens::MaterialLibrary *material_library, LibGens::ObjectProduction *object_production, string slot_id_name) {

	string model_name=object->queryEditorModel(slot_id_name, OBJECT_NODE_UNKNOWN_MESH);
	string skeleton_name=object->queryEditorSkeleton(slot_id_name, "");
	string animation_name=object->queryEditorAnimation(slot_id_name, "");

	string scale_x_str = object->queryExtraName(OBJECT_NODE_EXTRA_SCALE_X, "1.0");
	string scale_y_str = object->queryExtraName(OBJECT_NODE_EXTRA_SCALE_Y, "1.0");
	string scale_z_str = object->queryExtraName(OBJECT_NODE_EXTRA_SCALE_Z, "1.0");

	LibGens::ObjectElementFloat* s_x = (LibGens::ObjectElementFloat*)object->getElement(scale_x_str);
	LibGens::ObjectElementFloat* s_y = (LibGens::ObjectElementFloat*)object->getElement(scale_y_str);
	LibGens::ObjectElementFloat* s_z = (LibGens::ObjectElementFloat*)object->getElement(scale_z_str);

	float scale_x_f = 1;
	float scale_y_f = 1;
	float scale_z_f = 1;

	// Try reading the scale from an existing element. If not found, use the written value
	if (s_x) scale_x_f = s_x->value;
	else FromString<float>(scale_x_f, scale_x_str, std::dec);

	if (s_y) scale_y_f = s_y->value;
	else FromString<float>(scale_y_f, scale_y_str, std::dec);

	if (s_z) scale_z_f = s_z->value;
	else FromString<float>(scale_z_f, scale_z_str, std::dec);

	setScale(Ogre::Vector3(scale_x_f, scale_y_f, scale_z_f));

	string preview_box_x = object->queryExtraName(OBJECT_NODE_EXTRA_PREVIEW_BOX_X, "");
	string preview_box_y = object->queryExtraName(OBJECT_NODE_EXTRA_PREVIEW_BOX_Y, "");
	string preview_box_z = object->queryExtraName(OBJECT_NODE_EXTRA_PREVIEW_BOX_Z, "");

	string preview_box_x_scale = object->queryExtraName(OBJECT_NODE_EXTRA_PREVIEW_BOX_X_SCALE, "");
	string preview_box_y_scale = object->queryExtraName(OBJECT_NODE_EXTRA_PREVIEW_BOX_Y_SCALE, "");
	string preview_box_z_scale = object->queryExtraName(OBJECT_NODE_EXTRA_PREVIEW_BOX_Z_SCALE, "");

	if ((preview_box_x.size()) || (preview_box_y.size()) || (preview_box_z.size())) {
		LibGens::ObjectElementFloat *p_x = (LibGens::ObjectElementFloat *) object->getElement(preview_box_x);
		LibGens::ObjectElementFloat *p_y = (LibGens::ObjectElementFloat *) object->getElement(preview_box_y);
		LibGens::ObjectElementFloat *p_z = (LibGens::ObjectElementFloat *) object->getElement(preview_box_z);

		Ogre::Vector3 new_scale = preview_box_node->getScale();
		float scale_f=0.0f;

		float scale_x = 1.0f;
		float scale_y = 1.0f;
		float scale_z = 1.0f;

		if (preview_box_x_scale.size()) FromString<float>(scale_x, preview_box_x_scale, std::dec);
		if (preview_box_y_scale.size()) FromString<float>(scale_y, preview_box_y_scale, std::dec);
		if (preview_box_z_scale.size()) FromString<float>(scale_z, preview_box_z_scale, std::dec);

		// Check if elements were found. If not, try reading the float value from the string
		if (p_x) new_scale.x = p_x->value;
		else {
			FromString<float>(scale_f, preview_box_x, std::dec);
			new_scale.x = scale_f;
		}

		if (p_y) new_scale.y = p_y->value;
		else {
			FromString<float>(scale_f, preview_box_y, std::dec);
			new_scale.y = scale_f;
		}

		if (p_z) new_scale.z = p_z->value;
		else {
			FromString<float>(scale_f, preview_box_z, std::dec);
			new_scale.z = scale_f;
		}

		// Compensate scale of parent node
		new_scale.x = new_scale.x * scale_x * (1 / scale_x_f);
		new_scale.y = new_scale.y * scale_y * (1 / scale_y_f);
		new_scale.z = new_scale.z * scale_z * (1 / scale_z_f);

		// Check for valid scaling values
		if (new_scale.x <= 0.0) new_scale.x = 0.1;
		if (new_scale.y <= 0.0) new_scale.y = 0.1;
		if (new_scale.z <= 0.0) new_scale.z = 0.1;

		preview_box_node->setScale(new_scale);
	}

	if ((object->getName() == OBJECT_NODE_OBJECT_PHYSICS) && object_production) {
		bool found_model=false;

		LibGens::ObjectElement *element=object->getElement(OBJECT_NODE_OBJECT_PHYSICS_ELEMENT_TYPE);
		if (element) {
			string type_name=static_cast<LibGens::ObjectElementString *>(element)->value;

			if (type_name != current_type_name) {
				current_type_name = type_name;
				destroyAllAttachedMovableObjects(target_node);
			}
			else {
				return;
			}

			LibGens::ObjectPhysics *object_physics = object_production->getObjectPhysics(type_name);
			if (object_physics) {
				vector<string> models=object_physics->getModels();
				vector<string> motions=object_physics->getMotions();
				vector<string> motion_skeletons=object_physics->getMotionSkeletons();

				for (size_t i=0; i<models.size(); i++) {
					string model_id=models[i];
					string skeleton_id = "";
					string animation_id = "";

					LibGens::Model *model=model_library->getModel(model_id);
					if (i < motions.size()) {
						animation_id = motions[i];
					}

					if (i < motion_skeletons.size()) {
						skeleton_id = motion_skeletons[i];
					}

					if (model) {
						prepareSkeletonAndAnimation(skeleton_id, animation_id);
						buildModel(target_node, model, model->getName(), skeleton_id, scene_manager, material_library, EDITOR_NODE_QUERY_OBJECT, GENERAL_MESH_GROUP, false, SONICGLVL_SHADER_LIBRARY);
						createAnimationState(animation_id);
						found_model = true;
					}
				}
			}
		}

		if (found_model) {
			return;
		}
		else {
			current_model_name = "";
		}
	}

	if ((model_name != current_model_name) || (skeleton_name != current_skeleton_name) || (animation_name != current_animation_name)) {
		current_model_name = model_name;
		current_skeleton_name = skeleton_name;
		current_animation_name = animation_name;

		destroyAllAttachedMovableObjects(target_node);
	}
	else {
		return;
	}

	string model_id=model_name;
	model_id.resize(model_id.size() - ((string)OBJECT_NODE_MODEL_EXTENSION).size());

	string skeleton_id="";
	if (skeleton_name.size()) {
		skeleton_id=skeleton_name;
		skeleton_id.resize(skeleton_id.size() - ((string)OBJECT_NODE_SKELETON_EXTENSION).size());
	}

	string animation_id="";
	if (animation_name.size()) {
		animation_id=animation_name;
		animation_id.resize(animation_id.size() - ((string)OBJECT_NODE_ANIMATION_EXTENSION).size());
	}

	if (model_name.find(OBJECT_NODE_MODEL_EXTENSION) != string::npos) {
		LibGens::Model *model=model_library->getModel(model_id);

		if (model) {
			prepareSkeletonAndAnimation(skeleton_id, animation_id);
			buildModel(target_node, model, model->getName(), skeleton_id, scene_manager, material_library, EDITOR_NODE_QUERY_OBJECT, GENERAL_MESH_GROUP, false, SONICGLVL_SHADER_LIBRARY);
		}
		else {
			Ogre::Entity *entity = scene_manager->createEntity(OBJECT_NODE_UNKNOWN_MESH);
			entity->setQueryFlags(EDITOR_NODE_QUERY_OBJECT);
			target_node->attachObject(entity);
		}
	}
	else if (model_name.find(OBJECT_NODE_MESH_EXTENSION) != string::npos) {
		Ogre::Entity *entity = scene_manager->createEntity(model_name);
		entity->setQueryFlags(EDITOR_NODE_QUERY_OBJECT);
		target_node->attachObject(entity);
	}

	createAnimationState(animation_id);

	// Create PreviewBox if necessary
	if ((preview_box_x.size()) || (preview_box_y.size()) || (preview_box_z.size())) {
		preview_box_entity = scene_manager->createEntity("preview_box.mesh");
		preview_box_entity->setRenderQueueGroup(Ogre::RENDER_QUEUE_MAX);
		preview_box_entity->setQueryFlags(EDITOR_NODE_QUERY_PREVIEW_BOX);
		preview_box_node->attachObject(preview_box_entity);
	}
}

void ObjectNode::createAnimationState(string animation_id) {
	animation_state = NULL;

	if (animation_id.size()) {
		unsigned short attached_objects=scene_node->numAttachedObjects();
		for (unsigned short i=0; i<attached_objects; i++) {
			Ogre::Entity *entity=static_cast<Ogre::Entity *>(scene_node->getAttachedObject(i));

			if (entity->hasAnimationState(animation_id)) {
				animation_state = entity->getAnimationState(animation_id);
				animation_state->setLoop(true);
				animation_state->setEnabled(true);
				break;
			}
		}
	}
}


ObjectMultiSetNode::ObjectMultiSetNode(LibGens::Object *object_p, LibGens::MultiSetNode *multi_set_node_p, Ogre::SceneManager *scene_manager) {
	type = EDITOR_NODE_OBJECT_MSP;

	object = object_p;
	multi_set_node = multi_set_node_p;
	
    scene_node = scene_manager->getRootSceneNode()->createChildSceneNode();
	scene_node->getUserObjectBindings().setUserAny(EDITOR_NODE_BINDING, Ogre::Any((EditorNode *)this));

	scale    = Ogre::Vector3::UNIT_SCALE;
	selected = false;

	LibGens::Vector3 v=multi_set_node->position;
	setPosition(Ogre::Vector3(v.x, v.y, v.z));
	LibGens::Quaternion q=multi_set_node->rotation;
	setRotation(Ogre::Quaternion(q.w, q.x, q.y, q.z));
	LibGens::Vector3 s = multi_set_node->scale;
	setScale(Ogre::Vector3(s.x, s.y, s.z));

	offset_rotation_x = 0;
	offset_rotation_y = 0;
	offset_rotation_z = 0;

	offset_rotation_x_speed = 0;
	offset_rotation_y_speed = 0;
	offset_rotation_z_speed = 0;

	offset_rotation_animation_enabled = false;
}

void ObjectMultiSetNode::addTime(float time_s) {
	if (offset_rotation_animation_enabled) {
		offset_rotation_x += offset_rotation_x_speed * time_s;
		offset_rotation_y += offset_rotation_y_speed * time_s;
		offset_rotation_z += offset_rotation_z_speed * time_s;

		setRotation(rotation);
	}
}


void ObjectNode::show() {
	EditorNode::show();

	for (list<ObjectMultiSetNode *>::iterator it=object_msp_nodes.begin(); it!=object_msp_nodes.end(); it++) {
		(*it)->show();
	}
}

void ObjectNode::hide() {
	EditorNode::hide();

	for (list<ObjectMultiSetNode *>::iterator it=object_msp_nodes.begin(); it!=object_msp_nodes.end(); it++) {
		(*it)->hide();
	}
}

void ObjectNode::addTime(float time_s) {
	if (animation_state) {
		animation_state->addTime(time_s);
	}

	if (offset_rotation_animation_enabled) {
		offset_rotation_x += offset_rotation_x_speed * time_s;
		offset_rotation_y += offset_rotation_y_speed * time_s;
		offset_rotation_z += offset_rotation_z_speed * time_s;

		setRotation(rotation);
	}

	for (list<ObjectMultiSetNode *>::iterator it=object_msp_nodes.begin(); it!=object_msp_nodes.end(); it++) {
		(*it)->addTime(time_s);
	}
}


void ObjectNode::setSelect(bool v) {
	EditorNode::setSelect(v);

	preview_box_node->setVisible(v);
}

void ObjectNode::createObjectMultiSetNodes(LibGens::Object* object, Ogre::SceneManager *scene_manager) {
	// remove old nodes
	clearObjectMultiSetNodes();

	list<LibGens::MultiSetNode*> msp_nodes = object->getMultiSetParam()->getNodes();
	for (list<LibGens::MultiSetNode*>::iterator it = msp_nodes.begin(); it != msp_nodes.end(); it++) {
		ObjectMultiSetNode* object_msp_node = new ObjectMultiSetNode(object, (*it), scene_manager);
		
		object_msp_node->scale = scale;

		object_msp_node->offset_rotation_x = offset_rotation_x;
		object_msp_node->offset_rotation_y = offset_rotation_y;
		object_msp_node->offset_rotation_z = offset_rotation_z;

		object_msp_node->offset_rotation_x_speed = offset_rotation_x_speed;
		object_msp_node->offset_rotation_y_speed = offset_rotation_y_speed;
		object_msp_node->offset_rotation_z_speed = offset_rotation_z_speed;

		object_msp_node->offset_rotation_animation_enabled = offset_rotation_animation_enabled;

		object_msp_node->object_node = this;

		// Update rotation with offset
		object_msp_node->setRotation(object_msp_node->getRotation());
		object_msp_node->setScale(object_msp_node->getScale());
		object_msp_nodes.push_back(object_msp_node);
	}
}

void ObjectNode::clearObjectMultiSetNodes()
{
	for (list<ObjectMultiSetNode*>::iterator it = object_msp_nodes.begin(); it != object_msp_nodes.end(); ++it)
	{
		delete* it;
		object_msp_nodes.erase(it);
	}
}

void ObjectNode::removeObjectMultiSetNode(ObjectMultiSetNode* msNode)
{
	for (list<ObjectMultiSetNode*>::iterator it = object_msp_nodes.begin(); it != object_msp_nodes.end(); ++it)
		if ((*it) == msNode)
		{
			delete* it;
			object_msp_nodes.erase(it);
			return;
		}
}


ObjectNode::~ObjectNode() {
	for (list<ObjectMultiSetNode *>::iterator it=object_msp_nodes.begin(); it!=object_msp_nodes.end(); it++) {
		delete (*it);
	}
	object_msp_nodes.clear();
}
