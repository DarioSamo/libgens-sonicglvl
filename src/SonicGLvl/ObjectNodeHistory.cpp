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
#include "ObjectNodeHistory.h"
#include "ObjectSet.h"

// Create
void HistoryActionCreateObjectNode::undo() {
	if (!object) return;
	junk_state = true;

	LibGens::ObjectSet *object_set = object->getParentSet();

	if (object_set) {
		object_set->eraseObject(object);
		editor_application->updateLayerControlGUI();
	}

	if (object_node_manager) {
		object_node_manager->hideObjectNode(object, true);
	}
}

void HistoryActionCreateObjectNode::redo() {
	if (!object) return;
	junk_state = false;

	LibGens::ObjectSet *object_set = object->getParentSet();

	if (object_set) {
		object_set->addObject(object);
		editor_application->updateLayerControlGUI();
	}

	if (object_node_manager) {
		object_node_manager->showObjectNode(object);
	}
}

HistoryActionCreateObjectNode::~HistoryActionCreateObjectNode() {
	if (junk_state) {
		if (object) {
			delete object;
			if (object_node_manager) {
				object_node_manager->deleteObjectNode(object);
			}
		}
	}
}

// Delete
void HistoryActionDeleteObjectNode::undo() {
	if (!object) return;
	junk_state = false;

	LibGens::ObjectSet *object_set = object->getParentSet();

	if (object_set) {
		object_set->addObject(object);
		editor_application->updateLayerControlGUI();
	}

	if (object_node_manager) {
		object_node_manager->showObjectNode(object);
	}
}

void HistoryActionDeleteObjectNode::redo() {
	if (!object) return;
	junk_state = true;

	LibGens::ObjectSet *object_set = object->getParentSet();

	if (object_set) {
		object_set->eraseObject(object);
		editor_application->updateLayerControlGUI();
	}

	if (object_node_manager) {
		object_node_manager->hideObjectNode(object, true);
	}
}

HistoryActionDeleteObjectNode::~HistoryActionDeleteObjectNode() {
	if (junk_state) {
		if (object) {
			delete object;
			if (object_node_manager) {
				object_node_manager->deleteObjectNode(object);
			}
		}
	}
}


// Edit Bool
void HistoryActionEditObjectElementBool::undo() {
	if (!object) return;
	if (!object_element) return;

	object_element->value = previous_value;

	if (object_node_manager) {
		object_node_manager->reloadObjectNode(object);
	}
}

void HistoryActionEditObjectElementBool::redo() {
	if (!object) return;
	if (!object_element) return;

	object_element->value = new_value;

	if (object_node_manager) {
		object_node_manager->reloadObjectNode(object);
	}
}


// Edit Integer
void HistoryActionEditObjectElementInteger::undo() {
	if (!object) return;
	if (!object_element) return;

	object_element->value = previous_value;

	if (object_node_manager) {
		object_node_manager->reloadObjectNode(object);
	}
}

void HistoryActionEditObjectElementInteger::redo() {
	if (!object) return;
	if (!object_element) return;

	object_element->value = new_value;

	if (object_node_manager) {
		object_node_manager->reloadObjectNode(object);
	}
}

// Edit Float
void HistoryActionEditObjectElementFloat::undo() {
	if (!object) return;
	if (!object_element) return;

	object_element->value = previous_value;

	if (object_node_manager) {
		object_node_manager->reloadObjectNode(object);
	}
}

void HistoryActionEditObjectElementFloat::redo() {
	if (!object) return;
	if (!object_element) return;

	object_element->value = new_value;

	if (object_node_manager) {
		object_node_manager->reloadObjectNode(object);
	}
}


// Edit String
void HistoryActionEditObjectElementString::undo() {
	if (!object) return;
	if (!object_element) return;

	object_element->value = previous_value;

	if (object_node_manager) {
		object_node_manager->reloadObjectNode(object);
	}
}

void HistoryActionEditObjectElementString::redo() {
	if (!object) return;
	if (!object_element) return;

	object_element->value = new_value;

	if (object_node_manager) {
		object_node_manager->reloadObjectNode(object);
	}
}

// Edit ID
void HistoryActionEditObjectElementID::undo() {
	if (!object) return;
	if (!object_element) return;

	object_element->value = previous_value;

	if (object_node_manager) {
		object_node_manager->reloadObjectNode(object);
	}
}

void HistoryActionEditObjectElementID::redo() {
	if (!object) return;
	if (!object_element) return;

	object_element->value = new_value;

	if (object_node_manager) {
		object_node_manager->reloadObjectNode(object);
	}
}

// Edit ID List
void HistoryActionEditObjectElementIDList::undo() {
	if (!object) return;
	if (!object_element) return;

	object_element->value = previous_value;

	if (object_node_manager) {
		object_node_manager->reloadObjectNode(object);
	}
}

void HistoryActionEditObjectElementIDList::redo() {
	if (!object) return;
	if (!object_element) return;

	object_element->value = new_value;

	if (object_node_manager) {
		object_node_manager->reloadObjectNode(object);
	}
}

// Edit Vector
void HistoryActionEditObjectElementVector::undo() {
	if (!object) return;
	if (!object_element) return;

	object_element->value = previous_value;

	if (object_node_manager) {
		object_node_manager->reloadObjectNode(object);
	}
}

void HistoryActionEditObjectElementVector::redo() {
	if (!object) return;
	if (!object_element) return;

	object_element->value = new_value;

	if (object_node_manager) {
		object_node_manager->reloadObjectNode(object);
	}
}

// Edit Vector List
void HistoryActionEditObjectElementVectorList::undo() {
	if (!object) return;
	if (!object_element) return;

	object_element->value = previous_value;

	if (object_node_manager) {
		object_node_manager->reloadObjectNode(object);
	}
}

void HistoryActionEditObjectElementVectorList::redo() {
	if (!object) return;
	if (!object_element) return;

	object_element->value = new_value;

	if (object_node_manager) {
		object_node_manager->reloadObjectNode(object);
	}
}