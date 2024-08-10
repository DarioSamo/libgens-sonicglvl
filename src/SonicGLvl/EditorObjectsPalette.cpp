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
#include "ObjectLibrary.h"
#include "ObjectSet.h"

void EditorApplication::updateObjectCategoriesGUI() {
	SendDlgItemMessage(hLeftDlg, IDC_PALETTE_CATEGORY, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);

	vector<LibGens::ObjectCategory *> categories=library->getCategories();

	for (size_t i=0; i<categories.size(); i++) {
		SendDlgItemMessage(hLeftDlg, IDC_PALETTE_CATEGORY, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)categories[i]->getName().c_str());
	}

	SendDlgItemMessage(hLeftDlg, IDC_PALETTE_CATEGORY, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
}


void EditorApplication::updateObjectsPaletteGUI(int index) {
	if (!library) return;

	current_category_index = index;

	HWND hPaletteList=GetDlgItem(hLeftDlg, IDL_PALETTE_LIST);

	// Cleanup old palette
	if(ListView_GetItemCount(hPaletteList)!=0) {
		ListView_DeleteAllItems(hPaletteList);
		while(ListView_DeleteColumn(hPaletteList,0) > 0);
		ListView_SetItemCount(hPaletteList,0);
	}

	// Create new palette
	LibGens::ObjectCategory *object_category=library->getCategoryByIndex(current_category_index);
	if (object_category) {
		vector<LibGens::Object *> objects=object_category->getTemplates();

		char temp[128];
		for (size_t i=0; i<objects.size(); i++) {
			LV_ITEM Item;
			Item.mask = LVIF_TEXT;
			strcpy(temp, objects[i]->getName().c_str());
			Item.pszText = temp;
			Item.cchTextMax = strlen(temp);            
			Item.iSubItem = 0;                           
			Item.lParam = (LPARAM) NULL;                   
			Item.iItem = ListView_GetItemCount(hPaletteList); 
			ListView_InsertItem(hPaletteList, &Item);
		}
	}
}


void EditorApplication::updateHelpWithObjectGUI(LibGens::Object *object) {
	string help_name="";
	string help_description="";

	if (object) {
		help_name = object->getName();
		help_description = object->queryExtraName(OBJECT_NODE_EXTRA_DESCRIPTION);
	}

	SetDlgItemText(hLeftDlg, IDG_HELP_GROUP, help_name.c_str());
	SetDlgItemText(hLeftDlg, IDT_HELP_DESCRIPTION, help_description.c_str());
}

void EditorApplication::updateObjectsPaletteSelection(int index) {
	if (!library) return;

	if (index < 0) {
		current_palette_selection = NULL;
	}
	else {
		LibGens::ObjectCategory *object_category=library->getCategoryByIndex(current_category_index);
	
		if (object_category) {
			LibGens::Object *target_selection=object_category->getTemplateByIndex(index);
			current_palette_selection = target_selection;
		}
	}
}

void EditorApplication::updateObjectsPalettePreview() {
	if (current_palette_selection != last_palette_selection) {
		closeVectorQueryMode();
		closeEditPropertyGUI();
		clearObjectsPalettePreview();

		// Create Object Previewing Node
		if (current_palette_selection) {
			current_palette_selection->setPosition(LibGens::Vector3(LIBGENS_AABB_MAX_START, LIBGENS_AABB_MAX_START, LIBGENS_AABB_MAX_START));

			palette_cloning_mode = true;

			ObjectNode *palette_node=new ObjectNode(current_palette_selection, scene_manager, model_library, material_library, object_production, object_node_manager->getSlotIDName());
			current_palette_nodes.push_back(palette_node);
		}
		last_palette_selection = current_palette_selection;

		updateHelpWithObjectGUI(current_palette_selection);
	}
}

void EditorApplication::overrideObjectsPalettePreview(list<LibGens::Object *> override_objects) {
	closeVectorQueryMode();
	closeEditPropertyGUI();
	clearObjectsPalettePreview();

	current_palette_selection = NULL;
	last_palette_selection = NULL;

	palette_cloning_mode = false;

	for (list<LibGens::Object *>::iterator it=override_objects.begin(); it!=override_objects.end(); it++) {
		ObjectNode *palette_node=new ObjectNode((*it), scene_manager, model_library, material_library, object_production, object_node_manager->getSlotIDName());
		current_palette_nodes.push_back(palette_node);
	}
}


void EditorApplication::mouseMovedObjectsPalettePreview(const OIS::MouseEvent &arg) {
	float mouse_x=arg.state.X.abs/float(arg.state.width);
	float mouse_y=arg.state.Y.abs/float(arg.state.height);
	viewport->convertMouseToLocalScreen(mouse_x, mouse_y);

	// Raycast from camera to viewport
	Ogre::Vector3 raycast_point(0.0f);
	viewport->raycastPlacement(mouse_x, mouse_y, 15.0f, &raycast_point, EDITOR_NODE_QUERY_TERRAIN | EDITOR_NODE_QUERY_HAVOK);

	if (placement_grid_snap > 0.0f) {
		float half_placement_grid_snap = placement_grid_snap/2.0f;

		float grid_offset = fmod(raycast_point.x, placement_grid_snap);
		if (grid_offset < half_placement_grid_snap) raycast_point.x -= grid_offset;
		else raycast_point.x += placement_grid_snap - grid_offset;

		grid_offset = fmod(raycast_point.y, placement_grid_snap);
		if (grid_offset < half_placement_grid_snap) raycast_point.y -= grid_offset;
		else raycast_point.y += placement_grid_snap - grid_offset;

		grid_offset = fmod(raycast_point.z, placement_grid_snap);
		if (grid_offset < half_placement_grid_snap) raycast_point.z -= grid_offset;
		else raycast_point.z += placement_grid_snap - grid_offset;
	}

	// Calculate current preview's center
	Ogre::Vector3 center=Ogre::Vector3::ZERO;
	for (list<ObjectNode *>::iterator it=current_palette_nodes.begin(); it!=current_palette_nodes.end(); it++) {
		center += (*it)->getPosition();
	}
	center /= current_palette_nodes.size();

	// Translate all nodes from center of the list
	Ogre::Vector3 translate = raycast_point - center;
	for (list<ObjectNode *>::iterator it=current_palette_nodes.begin(); it!=current_palette_nodes.end(); it++) {
		(*it)->translate(translate);
	}
}


void EditorApplication::mousePressedObjectsPalettePreview(const OIS::MouseEvent &arg, OIS::MouseButtonID id) {
	if (id == OIS::MB_Left) {
		// Update nodes to be under the mouse if Shift is not pressed
		if (!keyboard->isModifierDown(OIS::Keyboard::Shift)) mouseMovedObjectsPalettePreview(arg);
		clearSelection();

		HistoryActionWrapper *wrapper = new HistoryActionWrapper();
		for (list<ObjectNode *>::iterator it=current_palette_nodes.begin(); it!=current_palette_nodes.end(); it++) {
			LibGens::Object *object_from_preview = (*it)->getObject();

			if (object_from_preview) {
				LibGens::Object *new_object = new LibGens::Object(object_from_preview);

				if (current_level) {
					if (current_level->getLevel()) {
						new_object->setID(current_level->getLevel()->newObjectID());
					}
				}
			
				if (current_set) {
					current_set->addObject(new_object);

					if (!current_level) {
						new_object->setID(current_set->newObjectID());
					}
				}

				// Create
				ObjectNode *new_object_node = object_node_manager->createObjectNode(new_object);

				// Push to History
				HistoryActionCreateObjectNode *action = new HistoryActionCreateObjectNode(new_object, object_node_manager);
				wrapper->push(action);

				// Add to current selection
				HistoryActionSelectNode *action_select = new HistoryActionSelectNode(new_object_node, false, true, &selected_nodes);
				new_object_node->setSelect(true);
				selected_nodes.push_back(new_object_node);
				wrapper->push(action_select);
			}
		}
		pushHistory(wrapper);

		updateSelection();

		if (!keyboard->isModifierDown(OIS::Keyboard::Ctrl)) {
			clearObjectsPalettePreviewGUI();
		}
	}

	if (id == OIS::MB_Right) {
		clearObjectsPalettePreviewGUI();
	}
}

void EditorApplication::clearObjectsPalettePreview() {
	for (list<ObjectNode *>::iterator it=current_palette_nodes.begin(); it!=current_palette_nodes.end(); it++) {
		if (!palette_cloning_mode) {
			LibGens::Object *object=(*it)->getObject();
			if (object) {
				delete object;
			}
		}

		delete (*it);
	}
	current_palette_nodes.clear();
}

void EditorApplication::clearObjectsPalettePreviewGUI() {
	clearObjectsPalettePreview();
	ListView_SetItemState(GetDlgItem(hLeftDlg, IDL_PALETTE_LIST), -1, 0, LVIS_SELECTED | LVIS_FOCUSED);

	// Refresh selection so the help text gets fixed up when quitting placement mode
	current_object_list_properties.clear();
	updateSelection();
}

bool EditorApplication::isPalettePreviewActive() {
	return (current_palette_nodes.size() > 0);
}

bool EditorApplication::isRegularMode() {
	bool regular_mode = true;
	if (isPalettePreviewActive())              regular_mode = false;
	if (editor_mode == EDITOR_NODE_QUERY_NODE) regular_mode = false;
	return regular_mode;
}


INT_PTR CALLBACK LeftBarCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	int list_view_index = ListView_GetNextItem(GetDlgItem(hDlg, IDL_PALETTE_LIST), -1, LVIS_SELECTED | LVIS_FOCUSED);
	editor_application->updateObjectsPaletteSelection(list_view_index);
	int selection_index=0;

	switch(msg) {
		case WM_INITDIALOG:
			return true;

		case WM_ACTIVATE:
			return false;

		case WM_DESTROY:
			return true;

		case WM_CLOSE:
			return false;

		case WM_NOTIFY:
			switch(LOWORD(wParam)) {
				case IDL_PROPERTIES_LIST:
					if (((LPNMHDR)lParam)->code == NM_CLICK) {
						selection_index = SendMessage(GetDlgItem(hDlg, IDL_PROPERTIES_LIST), LVM_GETNEXTITEM, -1, LVNI_SELECTED);
						editor_application->updateObjectPropertyIndex(selection_index);
						return true;
					}

					if (((LPNMHDR)lParam)->code == NM_DBLCLK) {
						selection_index = SendMessage(GetDlgItem(hDlg, IDL_PROPERTIES_LIST), LVM_GETNEXTITEM, -1, LVNI_SELECTED);
						editor_application->editObjectPropertyIndex(selection_index);
						return true;
					}
					break;
			}

		case WM_COMMAND:
			if(HIWORD(wParam) == CBN_SELCHANGE) { 
				int item_index = SendMessage((HWND) lParam, (UINT) CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0);
				if (LOWORD(wParam)==IDC_PALETTE_CATEGORY) {
					editor_application->updateObjectsPaletteGUI(item_index);
					break;
				}
			}

			break;
	}

	return false;
}
