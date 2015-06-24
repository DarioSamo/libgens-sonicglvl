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

#include "EditorNodeHistory.h"

void HistoryActionSelectNode::undo() {
	if (!node) return;

	node->setSelect(previous_selection_state);
	fixSelectedNodesList();
}

void HistoryActionSelectNode::redo() {
	if (!node) return;

	node->setSelect(new_selection_state);
	fixSelectedNodesList();
}


void HistoryActionSelectNode::fixSelectedNodesList() {
	if (!node) return;
	if (!selected_nodes) return;

	if (node->isSelected()) {
		// Check if node is already on selected nodes list
		// Add if not found

		bool found=false;
		for (list<EditorNode *>::iterator it=selected_nodes->begin(); it!=selected_nodes->end(); it++) {
			if ((*it) == node) {
				found = true;
				break;
			}
		}

		if (!found) {
			selected_nodes->push_back(node);
		}
	}
	else {
		// Search for node on the list and remove it
		for (list<EditorNode *>::iterator it=selected_nodes->begin(); it!=selected_nodes->end(); it++) {
			if ((*it) == node) {
				selected_nodes->erase(it);
				break;
			}
		}
	}
}


void HistoryActionMoveNode::undo() {
	if (!node) return;

	node->setPosition(previous_position);
	node->setRotation(previous_rotation);
}

void HistoryActionMoveNode::redo() {
	if (!node) return;

	node->setPosition(new_position);
	node->setRotation(new_rotation);
}

void HistoryActionRotateNode::undo() {
	if (!node) return;

	node->setRotation(previous_rotation);
}

void HistoryActionRotateNode::redo() {
	if (!node) return;

	node->setRotation(new_rotation);
}