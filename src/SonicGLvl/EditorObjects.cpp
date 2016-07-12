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
#include "EditorNodeHistory.h"
#include "ObjectNodeHistory.h"
#include <commctrl.h>

ObjectNode *ObjectNodeManager::createObjectNode(LibGens::Object *object) {
	ObjectNode *object_node=new ObjectNode(object, scene_manager, model_library, material_library, object_production, slot_id_name);
	object_nodes.push_back(object_node);

	return object_node;
}

void ObjectNodeManager::deleteObjectNode(LibGens::Object *object) {
	for (list<ObjectNode *>::iterator it=object_nodes.begin(); it!=object_nodes.end(); it++) {
		ObjectNode *object_node = *it;

		if (object_node->getObject() == object) {
			object_nodes.erase(it);
			delete object_node;
			return;
		}
	}
}


void ObjectNodeManager::showObjectNode(LibGens::Object *object) {
	for (list<ObjectNode *>::iterator it=object_nodes.begin(); it!=object_nodes.end(); it++) {
		ObjectNode *object_node = *it;
		if (object_node->getObject() == object) {
			object_node->show();
			object_node->setForceHide(false);
			return;
		}
	}
}

void ObjectNodeManager::hideObjectNode(LibGens::Object *object, bool permanent) {
	for (list<ObjectNode *>::iterator it=object_nodes.begin(); it!=object_nodes.end(); it++) {
		ObjectNode *object_node = *it;
		if (object_node->getObject() == object) {
			object_node->hide();
			if (permanent) object_node->setForceHide(true);
			return;
		}
	}
}


void ObjectNodeManager::reloadObjectNode(LibGens::Object *object) {
	for (list<ObjectNode *>::iterator it=object_nodes.begin(); it!=object_nodes.end(); it++) {
		ObjectNode *object_node = *it;

		if (object_node->getObject() == object) {
			object_node->reloadEntities(scene_manager, model_library, material_library, object_production, slot_id_name);
			return;
		}
	}
}


void ObjectNodeManager::addTime(float time_s) {
	for (list<ObjectNode *>::iterator it=object_nodes.begin(); it!=object_nodes.end(); it++) {
		(*it)->addTime(time_s);
	}
}

void ObjectNodeManager::showAll() {
	for (list<ObjectNode *>::iterator it=object_nodes.begin(); it!=object_nodes.end(); it++) {
		if (!(*it)->isForceHidden()) {
			(*it)->show();
		}
	}
}

void ObjectNodeManager::hideAll() {
	for (list<ObjectNode *>::iterator it=object_nodes.begin(); it!=object_nodes.end(); it++) {
		(*it)->hide();
	}
}

void ObjectNodeManager::updateSetVisibility(LibGens::ObjectSet *current_set, bool v) {
	for (list<ObjectNode *>::iterator it=object_nodes.begin(); it!=object_nodes.end(); it++) {
		if (((*it)->getObject()->getParentSet() == current_set) && current_set->hasObject((*it)->getObject())) {
			if (v) {
				(*it)->show();
				(*it)->setForceHide(false);
			}
			else {
				(*it)->hide();
				(*it)->setForceHide(true);
			}
		}
	}
}
