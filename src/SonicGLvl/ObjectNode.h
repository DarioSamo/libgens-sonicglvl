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
#include "Object.h"
#include "ObjectSet.h"
#include "ObjectProduction.h"
#include "ModelLibrary.h"

#ifndef OBJECT_NODE_H_INCLUDED
#define OBJECT_NODE_H_INCLUDED
 
#define OBJECT_NODE_OBJECT_PHYSICS                 "ObjectPhysics"
#define OBJECT_NODE_OBJECT_PHYSICS_ELEMENT_TYPE    "Type"
#define OBJECT_NODE_MODEL_ATTRIBUTE                "model"
#define OBJECT_NODE_UNKNOWN_MESH                   "unknown.mesh"
#define OBJECT_NODE_MODEL_EXTENSION                ".model"
#define OBJECT_NODE_ANIMATION_EXTENSION            ".anm.hkx"
#define OBJECT_NODE_SKELETON_EXTENSION             ".skl.hkx"
#define OBJECT_NODE_MESH_EXTENSION                 ".mesh"

#define OBJECT_NODE_EXTRA_DESCRIPTION              "description"
#define OBJECT_NODE_EXTRA_OFFSET_ROTATION_X        "offset_rotation_x"
#define OBJECT_NODE_EXTRA_OFFSET_ROTATION_Y        "offset_rotation_y"
#define OBJECT_NODE_EXTRA_OFFSET_ROTATION_Z		   "offset_rotation_z"

#define OBJECT_NODE_EXTRA_OFFSET_ROTATION_X_SPEED  "offset_rotation_x_speed"
#define OBJECT_NODE_EXTRA_OFFSET_ROTATION_Y_SPEED  "offset_rotation_y_speed"
#define OBJECT_NODE_EXTRA_OFFSET_ROTATION_Z_SPEED  "offset_rotation_z_speed"

#define OBJECT_NODE_EXTRA_PREVIEW_BOX_X            "preview_box_x"
#define OBJECT_NODE_EXTRA_PREVIEW_BOX_Y            "preview_box_y"
#define OBJECT_NODE_EXTRA_PREVIEW_BOX_Z            "preview_box_z"

#define OBJECT_NODE_EXTRA_PREVIEW_BOX_X_SCALE      "preview_box_x_scale"
#define OBJECT_NODE_EXTRA_PREVIEW_BOX_Y_SCALE      "preview_box_y_scale"
#define OBJECT_NODE_EXTRA_PREVIEW_BOX_Z_SCALE      "preview_box_z_scale"

#define VECTOR_NODE_MESH                           "vector_node.mesh"

class ObjectNode;
class MultiSetNode;

class ObjectMultiSetNode : public EditorNode {
	friend class ObjectNode;

	protected:
		LibGens::MultiSetNode *multi_set_node;
		LibGens::Object *object;

		ObjectNode* object_node;

		Ogre::Radian offset_rotation_x;
		Ogre::Radian offset_rotation_y;
		Ogre::Radian offset_rotation_z;

		Ogre::Radian offset_rotation_x_speed;
		Ogre::Radian offset_rotation_y_speed;
		Ogre::Radian offset_rotation_z_speed;

		bool offset_rotation_animation_enabled;
	public:
		ObjectMultiSetNode(LibGens::Object *object_p, LibGens::MultiSetNode *multi_set_node_p, Ogre::SceneManager *scene_manager);

		void setPosition(Ogre::Vector3 v) {
			EditorNode::setPosition(v);
			if (multi_set_node) multi_set_node->position = LibGens::Vector3(v.x, v.y, v.z);
		}

		void setRotation(Ogre::Quaternion v) {
			EditorNode::setRotation(v);

			scene_node->pitch(offset_rotation_x);
			scene_node->yaw(offset_rotation_y);
			scene_node->roll(offset_rotation_z);

			if (multi_set_node) multi_set_node->rotation = LibGens::Quaternion(v.w, v.x, v.y, v.z);
		}

		void addTime(float time_s);

		LibGens::Object* getObject()
		{
			return object;
		}

		LibGens::MultiSetNode* getMultiSetNode()
		{
			return multi_set_node;
		}

		ObjectNode* getObjectNode()
		{
			return object_node;
		}
};


class ObjectNode : public EditorNode {
	protected:
		LibGens::Object *object;

		Ogre::Radian offset_rotation_x;
		Ogre::Radian offset_rotation_y;
		Ogre::Radian offset_rotation_z;

		Ogre::Radian offset_rotation_x_speed;
		Ogre::Radian offset_rotation_y_speed;
		Ogre::Radian offset_rotation_z_speed;

		bool offset_rotation_animation_enabled;

		list<ObjectMultiSetNode *> object_msp_nodes;

		Ogre::AnimationState *animation_state;
		bool force_hide;

		Ogre::SceneNode *preview_box_node;
		Ogre::Entity *preview_box_entity;

		string current_model_name;
		string current_animation_name;
		string current_skeleton_name;
		string current_type_name;
	public:
		ObjectNode(LibGens::Object *object_p, Ogre::SceneManager *scene_manager, LibGens::ModelLibrary *model_library, LibGens::MaterialLibrary *material_library, LibGens::ObjectProduction *object_production, string slot_id_name);
		void createEntities(Ogre::SceneNode *target_node, Ogre::SceneManager *scene_manager, LibGens::ModelLibrary *model_library, LibGens::MaterialLibrary *material_library, LibGens::ObjectProduction *object_production, string slot_id_name);
		void reloadEntities(Ogre::SceneManager *scene_manager, LibGens::ModelLibrary *model_library, LibGens::MaterialLibrary *material_library, LibGens::ObjectProduction *object_production, string slot_id_name);

		void setPosition(Ogre::Vector3 v) {
			EditorNode::setPosition(v);
			if (object) object->setPosition(LibGens::Vector3(v.x, v.y, v.z));
		}

		void setRotation(Ogre::Quaternion v) {
			EditorNode::setRotation(v);

			scene_node->pitch(offset_rotation_x);
			scene_node->yaw(offset_rotation_y);
			scene_node->roll(offset_rotation_z);

			if (object) object->setRotation(LibGens::Quaternion(v.w, v.x, v.y, v.z));
		}

		LibGens::Object *getObject() {
			return object;
		}

		void removeObjectMultiSetNode(ObjectMultiSetNode* msNode);

		void addTime(float time_s);
		void createAnimationState(string animation_id);

		void show();
		void hide();

		void setSelect(bool v);

		void setForceHide(bool v) {
			force_hide = v;
		}

		bool isForceHidden() {
			return force_hide;
		}

		~ObjectNode();
};

class ObjectNodeManager {
	protected:
		list<ObjectNode *> object_nodes;
		string slot_id_name;
		Ogre::SceneManager *scene_manager;
		LibGens::ModelLibrary *model_library;
		LibGens::MaterialLibrary *material_library;
		LibGens::ObjectProduction *object_production;
	public:
		ObjectNodeManager(Ogre::SceneManager *scene_manager_p, LibGens::ModelLibrary *model_library_p, LibGens::MaterialLibrary *material_library_p, LibGens::ObjectProduction *object_production_p) {
			scene_manager     = scene_manager_p;
			model_library     = model_library_p;
			material_library  = material_library_p;
			object_production = object_production_p;
			slot_id_name = "";
		}

		void setSlotIdName(string v) {
			slot_id_name = v;
		}

		string getSlotIDName() {
			return slot_id_name;
		}

		// Object Node Managers can't delete object data, but they can add and delete nodes that reference them
		ObjectNode *createObjectNode(LibGens::Object *object);
		void deleteObjectNode(LibGens::Object *object);

		void showObjectNode(LibGens::Object *object);
		void hideObjectNode(LibGens::Object *object, bool permanent=false);
		void reloadObjectNode(LibGens::Object *object);

		list<ObjectNode *>& getObjectNodes() {
			return object_nodes;
		}

		ObjectNode* findObjectNode(LibGens::Object* object);

		void showAll();
		void hideAll();
		void updateSetVisibility(LibGens::ObjectSet *current_set, bool v);

		void addTime(float time_s);
};

#endif
