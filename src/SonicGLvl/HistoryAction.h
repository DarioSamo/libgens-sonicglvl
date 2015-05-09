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

#include "Common.h"

#ifndef HISTORY_ACTION_H_INCLUDED
#define HISTORY_ACTION_H_INCLUDED

enum HistoryActionType {
	HISTORY_ACTION_UNDEFINED,
	HISTORY_ACTION_WRAPPER,
	HISTORY_ACTION_MOVE_NODE,
	HISTORY_ACTION_ROTATE_NODE,
	HISTORY_ACTION_SELECT_NODE,
	HISTORY_ACTION_CREATE_OBJECT_NODE,
	HISTORY_ACTION_DELETE_OBJECT_NODE,
	HISTORY_ACTION_EDIT_OBJECT_ELEMENT_BOOL,
	HISTORY_ACTION_EDIT_OBJECT_ELEMENT_FLOAT,
	HISTORY_ACTION_EDIT_OBJECT_ELEMENT_INTEGER,
	HISTORY_ACTION_EDIT_OBJECT_ELEMENT_STRING,
	HISTORY_ACTION_EDIT_OBJECT_ELEMENT_VECTOR,
	HISTORY_ACTION_EDIT_OBJECT_ELEMENT_VECTOR_LIST,
	HISTORY_ACTION_EDIT_OBJECT_ELEMENT_ID,
	HISTORY_ACTION_EDIT_OBJECT_ELEMENT_ID_LIST
};

class HistoryAction {
	protected:
		HistoryActionType type;
	public:
		HistoryAction() {
			type = HISTORY_ACTION_UNDEFINED;
		}

		virtual void undo()=0;
		virtual void redo()=0;
};


class HistoryActionWrapper : public HistoryAction {
	protected:
		list<HistoryAction *> actions;
	public:
		HistoryActionWrapper() {
			type = HISTORY_ACTION_WRAPPER;
		}

		void push(HistoryAction *action) {
			actions.push_back(action);
		}

		void undo();
		void redo();

		~HistoryActionWrapper() {
			for (list<HistoryAction *>::iterator it=actions.begin(); it!=actions.end(); it++) {
				delete *it;
			}

			actions.clear();
		}
};


#endif