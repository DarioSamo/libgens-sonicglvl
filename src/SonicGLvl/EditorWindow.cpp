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

int global_cursor_state=0;

void EditorApplication::focus() {
	SetFocus(hwnd);
}

bool EditorApplication::inFocus() {
	return GetFocus() == hwnd;
}

extern WNDPROC globalWinProc;
LRESULT APIENTRY SubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	//MINMAXINFO FAR *lpMinMaxInfo;  
	switch(uMsg)
	{
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				// File
				case IMD_OPEN_LEVEL :
					editor_application->openLevelGUI();
					break;
				//case IMD_OPEN_LOST_WORLD_LEVEL :
					//editor_application->openLostWorldLevelGUI();
					//break;
				case IMD_SAVE_LEVEL_DATA :
					editor_application->saveLevelDataGUI();
					break;
				case IMD_SAVE_LEVEL_TERRAIN :
					editor_application->saveLevelTerrainGUI();
					break;
				case IMD_EXPORT_SCENE_FBX :
					editor_application->exportSceneFBXGUI();
					break;

				// Edit
				case IMD_UNDO:
					editor_application->undoHistory();
					break;
				case IMD_REDO:
					editor_application->redoHistory();
					break;
				case IMD_CUT:
					editor_application->copySelection();
					editor_application->deleteSelection();
					break;
				case IMD_COPY:
					editor_application->copySelection();
					break;
				case IMD_PASTE:
					editor_application->pasteSelection();
					break;
				case IMD_DELETE:
					editor_application->deleteSelection();
					break;
				case IMD_SELECT_ALL:
					editor_application->selectAll();
					break;
				case IMD_FIND:
					editor_application->openFindGUI();
					break;


				// Physics
				case IMD_OPEN_PHYSICS_EDITOR :
					editor_application->openPhysicsEditorGUI();
					break;

				// Materials
				case IMD_OPEN_MATERIAL_EDITOR :
					editor_application->openMaterialEditorGUI();
					break;

				// Terrain
				case IMD_CLEAN_TERRAIN :
					editor_application->cleanLevelTerrain();
					break;
				case IMD_IMPORT_TERRAIN_FBX :
					editor_application->importLevelTerrainFBXGUI();
					break;
				case IMD_GENERATE_TERRAIN_GROUPS :
					editor_application->generateTerrainGroups();
					break;
				

				// Objects
				case IMD_RELOAD_TEMPLATES_DATABASE :
					editor_application->reloadTemplatesDatabase();
					break;
				case IMD_SAVE_TEMPLATES_DATABASE :
					editor_application->saveTemplatesDatabase();
					break;

				// Show
				case IMD_SHOW_OBJECTS :
					editor_application->toggleNodeVisibility(EDITOR_NODE_OBJECT);
					editor_application->toggleNodeVisibility(EDITOR_NODE_OBJECT_MSP);
					break;
				case IMD_SHOW_TERRAIN :
					editor_application->toggleNodeVisibility(EDITOR_NODE_TERRAIN);
					break;
				case IMD_SHOW_COLLISION :
					editor_application->toggleNodeVisibility(EDITOR_NODE_HAVOK);
					break;
				case IMD_SHOW_PATHS :
					editor_application->toggleNodeVisibility(EDITOR_NODE_PATH);
					break;
				case IMD_SHOW_GHOST :
					editor_application->toggleNodeVisibility(EDITOR_NODE_GHOST);
					break;
				case IMD_WORLD_TRANSFORM:
					editor_application->toggleWorldTransform();
					editor_application->updateSelection();
					break;
				case IMD_PLACEMENT_SNAP:
					editor_application->togglePlacementSnap();
					break;
				case IMD_LOCAL_ROTATION:
					editor_application->toggleLocalRotation();
					break;
			}
			break;
	}

	HCURSOR current_cursor = GetCursor();
	HCURSOR arrow_cursor = LoadCursor(NULL, IDC_ARROW);

	if (current_cursor == arrow_cursor) {
		switch (global_cursor_state) {
			case -1:
				break;
			case 0:
				break;
			case 1:
				SetCursor(LoadCursor(NULL, IDC_HAND));
				break;
			case 2:
				SetCursor(LoadCursor(NULL, IDC_SIZEALL));
				break;
			case 3:
				SetCursor(LoadCursor(NULL, IDC_CROSS));
				break;
		}
	}

	return CallWindowProc(globalWinProc, hwnd, uMsg, wParam, lParam);
}

