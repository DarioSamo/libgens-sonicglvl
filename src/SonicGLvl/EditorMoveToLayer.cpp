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

INT_PTR CALLBACK MoveToLayerCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

void EditorApplication::openMoveToLayerGUI() {
	if (selected_nodes.empty() || set_mapping.empty()) return;

	DialogBox(NULL, MAKEINTRESOURCE(IDD_MOVE_LAYER_DIALOG), hwnd, MoveToLayerCallback);
}

void EditorApplication::populateMoveToLayerTargets(HWND hDlg) {
	for (int i = 0; i < set_mapping.size(); i++)
	{
		SendDlgItemMessage(hDlg, IDC_MOVE_LAYER_TARGETS, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)set_mapping[i]->getName().c_str());
	}

	SendDlgItemMessage(hDlg, IDC_MOVE_LAYER_TARGETS, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
}

void EditorApplication::moveObjectsToLayer(int index) {
	if (selected_nodes.empty() || !set_mapping.count(index)) return;

	for (auto node : selected_nodes)
	{
		ObjectNode* object_node = getObjectNodeFromEditorNode(node);
		if (!object_node)
		{
			// not an object, ignore
			return;
		}
	}

	LibGens::ObjectSet* new_set = set_mapping[index];
	HistoryActionWrapper* wrapper = new HistoryActionWrapper();
	for (auto node : selected_nodes)
	{
		ObjectNode* object_node = getObjectNodeFromEditorNode(node);
		LibGens::Object* object = object_node->getObject();
		LibGens::ObjectSet* prev_set = object->getParentSet();
		prev_set->eraseObject(object);
		new_set->addObject(object);

		// Push to History
		HistoryActionMoveObjectToLayer* action = new HistoryActionMoveObjectToLayer(object, prev_set, new_set);
		wrapper->push(action);
	}
	pushHistory(wrapper);

	// update object count
	updateLayerControlGUI();
	updateSelection();
}

INT_PTR CALLBACK MoveToLayerCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		editor_application->populateMoveToLayerTargets(hDlg);
		return true;
	}
	case WM_COMMAND:
	{
		switch (LPARAM(wParam))
		{
		case IDOK:
		{
			int selected_index = SendDlgItemMessage(hDlg, IDC_MOVE_LAYER_TARGETS, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
			editor_application->moveObjectsToLayer(selected_index);
			EndDialog(hDlg, wParam);
			return true;
		}
		case IDCANCEL:
		{
			EndDialog(hDlg, wParam);
			return true;
		}
		}
		break;
	}
	}
	return false;
}
