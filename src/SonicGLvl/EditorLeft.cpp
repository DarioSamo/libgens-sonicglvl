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
		// Update nodes to be under the mouse
		mouseMovedObjectsPalettePreview(arg);
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
					updateLayerControlGUI();

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

		if (!keyboard->isModifierDown(OIS::Keyboard::Ctrl) && !keyboard->isModifierDown(OIS::Keyboard::Shift)) {
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

void EditorApplication::createLayerControlGUI() {
	HWND hLayersList = GetDlgItem(hLeftDlg, IDL_LAYER_LIST);
	HWND hLayersNew = GetDlgItem(hLeftDlg, IDB_LAYER_NEW);
	HWND hLayersDelete = GetDlgItem(hLeftDlg, IDB_LAYER_DELETE);

	LVCOLUMN Col;
	Col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	Col.cx = 142;
	Col.pszText = "Name";
	Col.cchTextMax = strlen(Col.pszText);
	ListView_InsertColumn(hLayersList, 0, &Col);


	Col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	Col.cx = 80;
	Col.pszText = "Objects";
	Col.cchTextMax = strlen(Col.pszText);
	ListView_InsertColumn(hLayersList, 1, &Col);

	ListView_SetExtendedListViewStyleEx(hLayersList, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
	EnableWindow(hLayersList, false);
	EnableWindow(hLayersNew, false);
	EnableWindow(hLayersDelete, false);
}

void EditorApplication::updateLayerControlGUI() {
	set_mapping.clear();
	set_visibility.clear();
	
	HWND hLayersList = GetDlgItem(hLeftDlg, IDL_LAYER_LIST);
	HWND hLayersNew = GetDlgItem(hLeftDlg, IDB_LAYER_NEW);
	HWND hLayersDelete = GetDlgItem(hLeftDlg, IDB_LAYER_DELETE);

	if (ListView_GetItemCount(hLayersList) != 0)
	{
		ListView_DeleteAllItems(hLayersList);
		ListView_SetItemCount(hLayersList, 0);
	}

	char name_str[1024] = "";
	char value_str[1024] = "";

	int i = 0;
	list<LibGens::ObjectSet*> sets = current_level->getLevel()->getSets();
	for (list<LibGens::ObjectSet*>::iterator it = sets.begin(); it != sets.end(); it++)
	{
		string set_name = (*it)->getName();
		strcpy(name_str, set_name.c_str());
		string set_size = to_string((*it)->getObjects().size());
		strcpy(value_str, set_size.c_str());

		LV_ITEM Item;
		Item.mask = LVIF_TEXT;
		Item.pszText = name_str;
		Item.cchTextMax = strlen(name_str);
		Item.iSubItem = 0;
		Item.lParam = (LPARAM)NULL;
		Item.iItem = i;
		ListView_InsertItem(hLayersList, &Item);
		ListView_SetItemText(hLayersList, i, 1, value_str);
		ListView_SetCheckState(hLayersList, i, true);

		set_mapping[i] = *it;
		set_visibility[*it] = true;

		i++;
	}

	EnableWindow(hLayersList, true);
	EnableWindow(hLayersNew, true);
	EnableWindow(hLayersDelete, false);
}

void EditorApplication::setLayerVisibility(int index, bool v) {
	if (set_mapping.count(index))
	{
		LibGens::ObjectSet* set = set_mapping[index];
		set_visibility[set] = v;
		object_node_manager->updateSetVisibility(set, v);
	}
}

void EditorApplication::renameLayer(int index, string name) {
	if (current_level->getLevel()->getSet(name))
	{
		MessageBox(NULL, "A set with that name already exists!", "SonicGLvl", MB_OK | MB_ICONWARNING);
		return;
	}
	
	if (set_mapping.count(index))
	{
		LibGens::ObjectSet* set = set_mapping[index];
		string folder = LibGens::File::folderFromFilename(set->getFilename());
		string new_filename;
		if (current_level->getGameMode() == LIBGENS_LEVEL_GAME_UNLEASHED) 
		{
			new_filename = folder + name + LIBGENS_OBJECT_SET_EXTENSION;
		}
		else
		{
			// generations
			new_filename = folder + LIBGENS_OBJECT_SET_NAME + name + LIBGENS_OBJECT_SET_EXTENSION;
		}
		LibGens::File::remove(set->getFilename());
		set->setFilename(new_filename);
		set->setName(name);
		set->saveXML(new_filename);

		HWND hLayersList = GetDlgItem(hLeftDlg, IDL_LAYER_LIST);
		char name_str[1024] = "";
		strcpy(name_str, name.c_str());
		ListView_SetItemText(hLayersList, index, 0, name_str);

		// Brian TODO: rename current object layer
	}
}

void EditorApplication::enableLayerDelete() {
	int index = SendMessage(GetDlgItem(hLeftDlg, IDL_LAYER_LIST), LVM_GETNEXTITEM, -1, LVNI_SELECTED);
	HWND hLayersDelete = GetDlgItem(hLeftDlg, IDB_LAYER_DELETE);
	EnableWindow(hLayersDelete, set_mapping.count(index) && set_mapping.size() > 1);
}

void EditorApplication::deleteLayer() {
	int index = SendMessage(GetDlgItem(hLeftDlg, IDL_LAYER_LIST), LVM_GETNEXTITEM, -1, LVNI_SELECTED);
	if (set_mapping.count(index) && set_mapping.size() > 1)
	{
		LibGens::ObjectSet* set = set_mapping[index];
		list<LibGens::Object*> objects = set->getObjects();
		string text = "Do you want to delete'" + set->getName() + "' layer and " + to_string(objects.size()) + " objects in it?";
		text += "\nThis action CANNOT be undone.";
		if (MessageBox(NULL, text.c_str(), "Delete Layer", MB_YESNO | MB_ICONWARNING) == IDYES)
		{
			clearSelection();

			// permanently remove objects in this layer
			for (auto it : objects)
			{
				object_node_manager->deleteObjectNode(it);
			}

			LibGens::File::remove(set->getFilename());
			current_level->getLevel()->removeSet(set);
			delete set;

			updateLayerControlGUI();
			history->clear();

			// Brian TODO: update current object layer
		}
	}
}

void EditorApplication::newLayer() {
	if (current_level->getLevel()->getSet("rename_me")) 
	{
		MessageBox(NULL, "Rename the object set called \"rename_me\" first before creating a new object set.", "SonicGLvl", MB_OK | MB_ICONINFORMATION);
	}
	else 
	{
		LibGens::ObjectSet* set = new LibGens::ObjectSet();
		string folder = current_level->getLevel()->getFolder();
		set->setName("rename_me");
		if (current_level->getGameMode() == LIBGENS_LEVEL_GAME_UNLEASHED) 
		{
			set->setFilename(folder + set->getName() + LIBGENS_OBJECT_SET_EXTENSION);
		}
		else {
			set->setFilename(folder + LIBGENS_OBJECT_SET_NAME + set->getName() + LIBGENS_OBJECT_SET_EXTENSION);
		}

		current_level->getLevel()->addSet(set);
		updateLayerControlGUI();

		// Brian TODO: update current object layer
	}
}

INT_PTR CALLBACK LeftBarCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	int list_view_index = ListView_GetNextItem(GetDlgItem(hDlg, IDL_PALETTE_LIST), -1, LVIS_SELECTED | LVIS_FOCUSED);
	editor_application->updateObjectsPaletteSelection(list_view_index);
	int selection_index=0;

	switch (msg)
	{
	case WM_INITDIALOG:
		return true;

	case WM_ACTIVATE:
		return false;

	case WM_DESTROY:
		return true;

	case WM_CLOSE:
		return false;

	case WM_NOTIFY:
	{
		switch (LOWORD(wParam))
		{
		case IDL_PROPERTIES_LIST:
			switch (((LPNMHDR)lParam)->code)
			{
			case NM_CLICK:
			{
				selection_index = SendMessage(GetDlgItem(hDlg, IDL_PROPERTIES_LIST), LVM_GETNEXTITEM, -1, LVNI_SELECTED);
				editor_application->updateObjectPropertyIndex(selection_index);
				return true;
			}
			case NM_DBLCLK:
			{
				selection_index = SendMessage(GetDlgItem(hDlg, IDL_PROPERTIES_LIST), LVM_GETNEXTITEM, -1, LVNI_SELECTED);
				editor_application->editObjectPropertyIndex(selection_index);
				return true;
			}
			}
			break;
		case IDL_LAYER_LIST:
		{
			switch (((LPNMHDR)lParam)->code)
			{
			case LVN_ITEMCHANGED:
			{
				// detect layer checkbox changes
				LPNMLISTVIEW listview = ((LPNMLISTVIEW)lParam);
				int oldState = listview->uOldState & LVIS_STATEIMAGEMASK;
				int newState = listview->uNewState & LVIS_STATEIMAGEMASK;
				if (oldState != newState && (newState != INDEXTOSTATEIMAGEMASK(0)))
				{
					bool checked = (newState == INDEXTOSTATEIMAGEMASK(2));
					editor_application->setLayerVisibility(listview->iItem, checked);
				}
				return true;
			}
			case LVN_ENDLABELEDITA:
			{
				// detect layer name edit
				LPNMLVDISPINFOA info = ((LPNMLVDISPINFOA)lParam);
				if (info->item.pszText)
				{
					editor_application->renameLayer(info->item.iItem, info->item.pszText);
				}
				return true;
			}
			case NM_CLICK:
			{
				// check if selected layer can be deleted
				editor_application->enableLayerDelete();
				return true;
			}
			}
			break;
		}
		}
		break;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_PALETTE_CATEGORY:
		{
			switch (HIWORD(wParam))
			{
			case CBN_SELCHANGE:
			{
				int item_index = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
				editor_application->updateObjectsPaletteGUI(item_index);
				return true;
			}
			}
			break;
		}
		case IDB_LAYER_DELETE:
		{
			editor_application->deleteLayer();
			return true;
		}
		case IDB_LAYER_NEW:
		{
			editor_application->newLayer();
			return true;
		}
		}
		break;
	}
	}

	return false;
}
