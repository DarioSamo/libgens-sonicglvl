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
#include "ObjectSet.h"
#include <commctrl.h>
#include <windowsx.h>

#define NEW_SET_OPTION "New..."
#define DELETE_SET_OPTION "Delete..."

void EditorApplication::updateBottomSelectionGUI() {

	EnableWindow(GetDlgItem(hBottomDlg, IDB_BOTTOM_GHOST_BACK), (BOOL)editor_application->getGhostNode());
	EnableWindow(GetDlgItem(hBottomDlg, IDB_BOTTOM_GHOST_FORWARD), (BOOL)editor_application->getGhostNode());
	EnableWindow(GetDlgItem(hBottomDlg, IDB_BOTTOM_GHOST_PAUSE), (BOOL)editor_application->getGhostNode());
	EnableWindow(GetDlgItem(hBottomDlg, IDB_BOTTOM_GHOST_PLAY), (BOOL)editor_application->getGhostNode());
	EnableWindow(GetDlgItem(hBottomDlg, IDS_BOTTOM_GHOST_SEEK), (BOOL)editor_application->getGhostNode());

	if (editor_application->getGhostNode())
	{
		SendDlgItemMessage(hBottomDlg, IDS_BOTTOM_GHOST_SEEK, (UINT)TBM_SETRANGEMAX, TRUE, editor_application->getGhostNode()->getDuration() * 1000);
		SendDlgItemMessage(hBottomDlg, IDS_BOTTOM_GHOST_SEEK, (UINT)TBM_SETPOS, TRUE, editor_application->getGhostNode()->getTime() * 1000);
	}
}

INT_PTR CALLBACK BottomBarCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
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
			return false;

		case WM_HSCROLL:
		{
			if (GetDlgCtrlID((HWND)lParam) == IDS_BOTTOM_GHOST_SEEK) {
				float value = (float)SendDlgItemMessage(hDlg, IDS_BOTTOM_GHOST_SEEK, TBM_GETPOS, 0, 0);
				if (editor_application->getGhostNode())
				{
					editor_application->getGhostNode()->setPlay(true);
					editor_application->getGhostNode()->setTime(value / 1000.0f);
					editor_application->getGhostNode()->setPlay(false);
				}
			}
			break;
		}

		case WM_COMMAND:
			if (LOWORD(wParam) == IDB_BOTTOM_GHOST_PLAY) {
				if (editor_application->getGhostNode()) {
					editor_application->getGhostNode()->setPlay(true);
				}
			}
			else if (LOWORD(wParam) == IDB_BOTTOM_GHOST_PAUSE) {
				if (editor_application->getGhostNode()) {
					editor_application->getGhostNode()->setPlay(false);
				}
			}
			else if (LOWORD(wParam) == IDB_BOTTOM_GHOST_BACK) {
				if (editor_application->getGhostNode()) {
					editor_application->getGhostNode()->setPlay(true);
					editor_application->getGhostNode()->addTime(-0.01f);
					editor_application->getGhostNode()->setPlay(false);
				}
			}
			else if (LOWORD(wParam) == IDB_BOTTOM_GHOST_FORWARD) {
				if (editor_application->getGhostNode()) {
					editor_application->getGhostNode()->setPlay(true);
					editor_application->getGhostNode()->addTime(0.01f);
					editor_application->getGhostNode()->setPlay(false);
				}
			}

			break;
	}

	return false;
}
