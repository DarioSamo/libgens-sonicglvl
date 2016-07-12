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

#include "History.h"

void History::push(HistoryAction *action) {
	if (action) {
		actions.push(action);

		while (!actions_redo.empty()) {
			HistoryAction *action=actions_redo.top();
			actions_redo.pop();
			delete action;
		}
	}
}

void History::undo() {
	if (actions.size()) {
		HistoryAction *action=actions.top();
		actions.pop();
		actions_redo.push(action);
		action->undo();
	}
}

void History::redo() {
	if (actions_redo.size()) {
		HistoryAction *action=actions_redo.top();
		actions_redo.pop();
		actions.push(action);
		action->redo();
	}
}

void History::clear() {
	while (!actions.empty()) {
		HistoryAction *action=actions.top();
		actions.pop();
		delete action;
	}

	while (!actions_redo.empty()) {
		HistoryAction *action=actions_redo.top();
		actions_redo.pop();
		delete action;
	}
}