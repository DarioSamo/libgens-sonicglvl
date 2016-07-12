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

#include "ObjectNode.h"
#include "History.h"
#include "ObjectElement.h"

#ifndef OBJECT_NODE_HISTORY_H_INCLUDED
#define OBJECT_NODE_HISTORY_H_INCLUDED

class HistoryActionCreateObjectNode : public HistoryAction {
	protected:
		LibGens::Object *object;
		ObjectNodeManager *object_node_manager;
		bool junk_state;
	public:
		HistoryActionCreateObjectNode(LibGens::Object *object_p, ObjectNodeManager *object_node_manager_p) {
			type=HISTORY_ACTION_CREATE_OBJECT_NODE;
			object = object_p;
			object_node_manager = object_node_manager_p;
			junk_state = false;
		}

		void undo();
		void redo();

		~HistoryActionCreateObjectNode();
};

class HistoryActionDeleteObjectNode : public HistoryAction {
	protected:
		LibGens::Object *object;
		ObjectNodeManager *object_node_manager;
		bool junk_state;
	public:
		HistoryActionDeleteObjectNode(LibGens::Object *object_p, ObjectNodeManager *object_node_manager_p) {
			type=HISTORY_ACTION_DELETE_OBJECT_NODE;
			object = object_p;
			object_node_manager = object_node_manager_p;
			junk_state = true;
		}

		void undo();
		void redo();

		~HistoryActionDeleteObjectNode();
};

class HistoryActionEditObjectElementBool : public HistoryAction {
	protected:
		LibGens::Object *object;
		ObjectNodeManager *object_node_manager;
		LibGens::ObjectElementBool *object_element;
		bool previous_value;
		bool new_value;
	public:
		HistoryActionEditObjectElementBool(LibGens::Object *object_p, ObjectNodeManager *object_node_manager_p, LibGens::ObjectElementBool *object_element_p, bool previous_value_p, bool new_value_p) {
			type = HISTORY_ACTION_EDIT_OBJECT_ELEMENT_BOOL;
			object = object_p;
			object_node_manager = object_node_manager_p;
			object_element = object_element_p;
			previous_value = previous_value_p;
			new_value = new_value_p;
		}

		void undo();
		void redo();
};


class HistoryActionEditObjectElementInteger : public HistoryAction {
	protected:
		LibGens::Object *object;
		ObjectNodeManager *object_node_manager;
		LibGens::ObjectElementInteger *object_element;
		unsigned int previous_value;
		unsigned int new_value;
	public:
		HistoryActionEditObjectElementInteger(LibGens::Object *object_p, ObjectNodeManager *object_node_manager_p, LibGens::ObjectElementInteger *object_element_p, unsigned int previous_value_p, unsigned int new_value_p) {
			type = HISTORY_ACTION_EDIT_OBJECT_ELEMENT_INTEGER;
			object = object_p;
			object_node_manager = object_node_manager_p;
			object_element = object_element_p;
			previous_value = previous_value_p;
			new_value = new_value_p;
		}

		void undo();
		void redo();
};



class HistoryActionEditObjectElementFloat : public HistoryAction {
	protected:
		LibGens::Object *object;
		ObjectNodeManager *object_node_manager;
		LibGens::ObjectElementFloat *object_element;
		float previous_value;
		float new_value;
	public:
		HistoryActionEditObjectElementFloat(LibGens::Object *object_p, ObjectNodeManager *object_node_manager_p, LibGens::ObjectElementFloat *object_element_p, float previous_value_p, float new_value_p) {
			type = HISTORY_ACTION_EDIT_OBJECT_ELEMENT_FLOAT;
			object = object_p;
			object_node_manager = object_node_manager_p;
			object_element = object_element_p;
			previous_value = previous_value_p;
			new_value = new_value_p;
		}

		void undo();
		void redo();
};


class HistoryActionEditObjectElementString : public HistoryAction {
	protected:
		LibGens::Object *object;
		ObjectNodeManager *object_node_manager;
		LibGens::ObjectElementString *object_element;
		string previous_value;
		string new_value;
	public:
		HistoryActionEditObjectElementString(LibGens::Object *object_p, ObjectNodeManager *object_node_manager_p, LibGens::ObjectElementString *object_element_p, string previous_value_p, string new_value_p) {
			type = HISTORY_ACTION_EDIT_OBJECT_ELEMENT_STRING;
			object = object_p;
			object_node_manager = object_node_manager_p;
			object_element = object_element_p;
			previous_value = previous_value_p;
			new_value = new_value_p;
		}

		void undo();
		void redo();
};


class HistoryActionEditObjectElementID : public HistoryAction {
	protected:
		LibGens::Object *object;
		ObjectNodeManager *object_node_manager;
		LibGens::ObjectElementID *object_element;
		size_t previous_value;
		size_t new_value;
	public:
		HistoryActionEditObjectElementID(LibGens::Object *object_p, ObjectNodeManager *object_node_manager_p, LibGens::ObjectElementID *object_element_p, size_t previous_value_p, size_t new_value_p) {
			type = HISTORY_ACTION_EDIT_OBJECT_ELEMENT_ID;
			object = object_p;
			object_node_manager = object_node_manager_p;
			object_element = object_element_p;
			previous_value = previous_value_p;
			new_value = new_value_p;
		}

		void undo();
		void redo();
};


class HistoryActionEditObjectElementIDList : public HistoryAction {
	protected:
		LibGens::Object *object;
		ObjectNodeManager *object_node_manager;
		LibGens::ObjectElementIDList *object_element;
		vector<size_t> previous_value;
		vector<size_t> new_value;
	public:
		HistoryActionEditObjectElementIDList(LibGens::Object *object_p, ObjectNodeManager *object_node_manager_p, LibGens::ObjectElementIDList *object_element_p, vector<size_t> previous_value_p, vector<size_t> new_value_p) {
			type = HISTORY_ACTION_EDIT_OBJECT_ELEMENT_ID_LIST;
			object = object_p;
			object_node_manager = object_node_manager_p;
			object_element = object_element_p;
			previous_value = previous_value_p;
			new_value = new_value_p;
		}

		void undo();
		void redo();
};


class HistoryActionEditObjectElementVector : public HistoryAction {
	protected:
		LibGens::Object *object;
		ObjectNodeManager *object_node_manager;
		LibGens::ObjectElementVector *object_element;
		LibGens::Vector3 previous_value;
		LibGens::Vector3 new_value;
	public:
		HistoryActionEditObjectElementVector(LibGens::Object *object_p, ObjectNodeManager *object_node_manager_p, LibGens::ObjectElementVector *object_element_p, LibGens::Vector3 previous_value_p, LibGens::Vector3 new_value_p) {
			type = HISTORY_ACTION_EDIT_OBJECT_ELEMENT_VECTOR;
			object = object_p;
			object_node_manager = object_node_manager_p;
			object_element = object_element_p;
			previous_value = previous_value_p;
			new_value = new_value_p;
		}

		void undo();
		void redo();
};


class HistoryActionEditObjectElementVectorList : public HistoryAction {
	protected:
		LibGens::Object *object;
		ObjectNodeManager *object_node_manager;
		LibGens::ObjectElementVectorList *object_element;
		vector<LibGens::Vector3> previous_value;
		vector<LibGens::Vector3> new_value;
	public:
		HistoryActionEditObjectElementVectorList(LibGens::Object *object_p, ObjectNodeManager *object_node_manager_p, LibGens::ObjectElementVectorList *object_element_p, vector<LibGens::Vector3> previous_value_p, vector<LibGens::Vector3> new_value_p) {
			type = HISTORY_ACTION_EDIT_OBJECT_ELEMENT_VECTOR_LIST;
			object = object_p;
			object_node_manager = object_node_manager_p;
			object_element = object_element_p;
			previous_value = previous_value_p;
			new_value = new_value_p;
		}

		void undo();
		void redo();
};

#endif