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

INT_PTR CALLBACK PhysicsEditorCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

void EditorApplication::openPhysicsEditorGUI() {
	if (!current_level) {
		return;
	}

	if (!current_level->getLevel()) {
		return;
	}

	if (!hPhysicsEditorDlg) {
		hPhysicsEditorDlg = CreateDialog(NULL, MAKEINTRESOURCE(IDD_PHYSICS_EDITOR), hwnd, PhysicsEditorCallback);

		// Create Collision List
		HWND hCollisionList = GetDlgItem(hPhysicsEditorDlg, IDL_COLLISION_LIST);

		LVCOLUMN Col;                                   
		Col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
		Col.cx = 151;
		Col.pszText = "Filename";
		Col.cchTextMax = strlen(Col.pszText);
		ListView_InsertColumn(hCollisionList, 0, &Col);

		Col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
		Col.cx = 90;
		Col.pszText = "Rigid Bodies";
		Col.cchTextMax = strlen(Col.pszText);
		ListView_InsertColumn(hCollisionList, 1, &Col);

		ListView_SetExtendedListViewStyleEx(hCollisionList, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

		// Insert items into Collision List
		list<LibGens::LevelCollisionEntry *> entries = current_level->getLevel()->getCollisionEntries();
		for (list<LibGens::LevelCollisionEntry *>::iterator it=entries.begin(); it!=entries.end(); it++) {
			addPhysicsEditorEntryGUI(*it);
		}

	}
	SetFocus(hPhysicsEditorDlg);
}

void EditorApplication::addPhysicsEditorEntryGUI(LibGens::LevelCollisionEntry *entry) {
	char name_str[256];
	char body_str[256];
	HWND hCollisionList = GetDlgItem(hPhysicsEditorDlg, IDL_COLLISION_LIST);

	LV_ITEM Item;
	Item.mask = LVIF_TEXT;
	strcpy(name_str, entry->name.c_str());
	strcpy(body_str, "");

	Item.pszText = name_str;
	Item.cchTextMax = strlen(name_str);            
	Item.iSubItem = 0;                           
	Item.lParam = (LPARAM) NULL;                   
	Item.iItem = ListView_GetItemCount(hCollisionList); 
	ListView_InsertItem(hCollisionList, &Item);
	ListView_SetItemText(hCollisionList, Item.iItem, 1, body_str);
	ListView_SetCheckState(hCollisionList, Item.iItem, entry->rendering);
}

void EditorApplication::importPhysicsEditorGUI() {
	char *filename = (char *) malloc(1024);
	strcpy(filename, "");

	OPENFILENAME    ofn;
    memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize     = sizeof(ofn);
	ofn.lpstrFilter     = "Havok Physics File(*.phy.hkx)\0*.phy.hkx\0";
	ofn.nFilterIndex    = 1;
	ofn.lpstrFile       = filename;
    ofn.nMaxFile        = 1024;
    ofn.lpstrTitle      = "Choose the Havok Physics File";
    ofn.Flags           = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST |
                          OFN_LONGNAMES     | OFN_EXPLORER |
                          OFN_HIDEREADONLY  | OFN_ENABLESIZING;

    if(GetOpenFileName(&ofn)) {
		chdir(exe_path.c_str());
		
		// Check if Havok Entry is on level
		LibGens::Level *level = current_level->getLevel();
		string entry_name = LibGens::File::nameFromFilenameNoExtension(ToString(filename));
		LibGens::LevelCollisionEntry *entry = level->getCollisionEntry(entry_name);

		// If entry exists, overwrite the file and reload the collision
		if (entry) {
			// Copy the file to the current data cache folder
			string data_cache_folder = current_level->getDataCacheFolder();
			LibGens::File collision_file(ToString(filename), LIBGENS_FILE_READ_BINARY);

			if (collision_file.valid()) {
				collision_file.clone(data_cache_folder + "/" + entry_name + LIBGENS_HAVOK_PHYSICS_EXTENSION);

				// Erase current loaded file from Havok Cache
				havok_enviroment->deletePhysicsEntry(entry_name);

				// Load Physics Entry into Cache
				LibGens::HavokPhysicsCache *physics_cache = havok_enviroment->getPhysics(entry_name);
				detectAndTagHavokPhysics(physics_cache);

				// Reload collision
				current_level->cleanCollision(havok_nodes_list);
				current_level->loadCollision(havok_enviroment, scene_manager, havok_nodes_list);
			}
		}
		// If entry doesn't exist, add to list and load the new collision
		else {
		}
	}

	chdir(exe_path.c_str());
    free(filename);
}

void EditorApplication::detectAndTagHavokPhysics(LibGens::HavokPhysicsCache *physics_cache) {
	hkpPhysicsData *physics_data = physics_cache->getPhysics();

	if (physics_data) {
		const hkArray<hkpPhysicsSystem*> &systems = physics_data->getPhysicsSystems();
		for (int i = 0; i < systems.getSize(); i++) {
			const hkArray<hkpRigidBody*> &rigidbodies = systems[i]->getRigidBodies();
			for (int j = 0; j < rigidbodies.getSize(); j++) {
				havok_property_database->applyProperties(rigidbodies[j]);
			}
		}
	}

	physics_cache->save();
}

void EditorApplication::clearPhysicsEditorGUI() {
	hPhysicsEditorDlg = NULL;
}


INT_PTR CALLBACK PhysicsEditorCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	int list_view_index = ListView_GetNextItem(GetDlgItem(hDlg, IDL_COLLISION_LIST), -1, LVIS_SELECTED | LVIS_FOCUSED);
	//editor_application->updatePhysicsEditorIndex(list_view_index);

	switch(msg) {
		case WM_INITDIALOG:
			return true;

		case WM_CLOSE:
			EndDialog(hDlg, false);
			editor_application->clearPhysicsEditorGUI();
			return true;

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDCANCEL:
					SendMessage(hDlg, WM_CLOSE, 0, 0);
					return true;

				case IDB_IMPORT_PHYSICS:
					editor_application->importPhysicsEditorGUI();
					return true;
			}

			break;

		case WM_NOTIFY:
			return true;
	}
	
	return false;
}


