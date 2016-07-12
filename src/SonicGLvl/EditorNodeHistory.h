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
#include "History.h"

#ifndef EDITOR_NODE_HISTORY_H_INCLUDED
#define EDITOR_NODE_HISTORY_H_INCLUDED

class HistoryActionSelectNode : public HistoryAction {
	protected:
		EditorNode *node;
		bool previous_selection_state;
		bool new_selection_state;
		list<EditorNode *> *selected_nodes;
	public:
		HistoryActionSelectNode(EditorNode *node_p, bool previous_selection_state_p, bool new_selection_state_p, list<EditorNode *> *selected_nodes_p) {
			type=HISTORY_ACTION_SELECT_NODE;
			node=node_p;
			previous_selection_state = previous_selection_state_p;
			new_selection_state      = new_selection_state_p;
			selected_nodes = selected_nodes_p;
		}

		void undo();
		void redo();
		void fixSelectedNodesList();
};

class HistoryActionMoveNode : public HistoryAction {
	protected:
		EditorNode *node;
		Ogre::Vector3 previous_position;
		Ogre::Vector3 new_position;
	public:
		HistoryActionMoveNode(EditorNode *node_p, Ogre::Vector3 previous_position_p, Ogre::Vector3 new_position_p) {
			type=HISTORY_ACTION_MOVE_NODE;
			node=node_p;
			previous_position=previous_position_p;
			new_position=new_position_p;
		}

		void undo();
		void redo();
};


class HistoryActionRotateNode : public HistoryAction {
	protected:
		EditorNode *node;
		Ogre::Quaternion previous_rotation;
		Ogre::Quaternion new_rotation;
	public:
		HistoryActionRotateNode(EditorNode *node_p, Ogre::Quaternion previous_rotation_p, Ogre::Quaternion new_rotation_p) {
			type=HISTORY_ACTION_ROTATE_NODE;
			node=node_p;
			previous_rotation=previous_rotation_p;
			new_rotation=new_rotation_p;
		}

		void undo();
		void redo();
};

#endif