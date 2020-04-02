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
	HWND hSelectionEditPosX = GetDlgItem(hBottomDlg, IDE_BOTTOM_SELECTION_POS_X);
	HWND hSelectionEditPosY = GetDlgItem(hBottomDlg, IDE_BOTTOM_SELECTION_POS_Y);
	HWND hSelectionEditPosZ = GetDlgItem(hBottomDlg, IDE_BOTTOM_SELECTION_POS_Z);
	HWND hSelectionEditRotX = GetDlgItem(hBottomDlg, IDE_BOTTOM_SELECTION_ROT_X);
	HWND hSelectionEditRotY = GetDlgItem(hBottomDlg, IDE_BOTTOM_SELECTION_ROT_Y);
	HWND hSelectionEditRotZ = GetDlgItem(hBottomDlg, IDE_BOTTOM_SELECTION_ROT_Z);
	HWND hSelectionSpinPosX = GetDlgItem(hBottomDlg, IDS_BOTTOM_SELECTION_POS_X);
	HWND hSelectionSpinPosY = GetDlgItem(hBottomDlg, IDS_BOTTOM_SELECTION_POS_Y);
	HWND hSelectionSpinPosZ = GetDlgItem(hBottomDlg, IDS_BOTTOM_SELECTION_POS_Z);
	HWND hSelectionSpinRotX = GetDlgItem(hBottomDlg, IDS_BOTTOM_SELECTION_ROT_X);
	HWND hSelectionSpinRotY = GetDlgItem(hBottomDlg, IDS_BOTTOM_SELECTION_ROT_Y);
	HWND hSelectionSpinRotZ = GetDlgItem(hBottomDlg, IDS_BOTTOM_SELECTION_ROT_Z);

	bool objects_selected = selected_nodes.size() > 0;

	EnableWindow(hSelectionEditPosX, objects_selected);
	EnableWindow(hSelectionEditPosY, objects_selected);
	EnableWindow(hSelectionEditPosZ, objects_selected);
	EnableWindow(hSelectionEditRotX, objects_selected);
	EnableWindow(hSelectionEditRotY, objects_selected);
	EnableWindow(hSelectionEditRotZ, objects_selected);
	EnableWindow(hSelectionSpinPosX, objects_selected);
	EnableWindow(hSelectionSpinPosY, objects_selected);
	EnableWindow(hSelectionSpinPosZ, objects_selected);
	EnableWindow(hSelectionSpinRotX, objects_selected);
	EnableWindow(hSelectionSpinRotY, objects_selected);
	EnableWindow(hSelectionSpinRotZ, objects_selected);

	EnableWindow(GetDlgItem(hBottomDlg, IDB_BOTTOM_GHOST_BACK), (BOOL)editor_application->getGhostNode());
	EnableWindow(GetDlgItem(hBottomDlg, IDB_BOTTOM_GHOST_FORWARD), (BOOL)editor_application->getGhostNode());
	EnableWindow(GetDlgItem(hBottomDlg, IDB_BOTTOM_GHOST_PAUSE), (BOOL)editor_application->getGhostNode());
	EnableWindow(GetDlgItem(hBottomDlg, IDB_BOTTOM_GHOST_PLAY), (BOOL)editor_application->getGhostNode());
	EnableWindow(GetDlgItem(hBottomDlg, IDS_BOTTOM_GHOST_SEEK), (BOOL)editor_application->getGhostNode());

	if (objects_selected) {
		Ogre::Vector3 axis_position=axis->getPosition();
		Ogre::Quaternion axis_rotation=axis->getRotation();

		/*
		Ogre::Matrix3 mat;
	   quat.ToRotationMatrix(mat);
	   mat.ToEulerAnglesYXZ(yRad, pRad, rRad);
	   yDeg = yRad;
	   pDeg = pRad;
	   rDeg = rRad;

	   yDeg +=Ogre::Degree(1);

	   mat.FromEulerAnglesYXZ(yDeg, pDeg, rDeg);
	   quat.FromRotationMatrix(mat);
	   */

		Ogre::Radian yRad, pRad, rRad;
		Ogre::Matrix3 mat;
		axis_rotation.ToRotationMatrix(mat);
		mat.ToEulerAnglesYXZ(yRad, pRad, rRad);
		Ogre::Real yDeg = yRad.valueDegrees();
		Ogre::Real pDeg = pRad.valueDegrees();
		Ogre::Real rDeg = rRad.valueDegrees();

		is_update_pos_rot = false;

		SetDlgItemText(hBottomDlg, IDE_BOTTOM_SELECTION_POS_X, ToString((float)axis_position.x).c_str());
		SetDlgItemText(hBottomDlg, IDE_BOTTOM_SELECTION_POS_Y, ToString((float)axis_position.y).c_str());
		SetDlgItemText(hBottomDlg, IDE_BOTTOM_SELECTION_POS_Z, ToString((float)axis_position.z).c_str());

		SetDlgItemText(hBottomDlg, IDE_BOTTOM_SELECTION_ROT_X, ToString((float)pDeg).c_str());
		SetDlgItemText(hBottomDlg, IDE_BOTTOM_SELECTION_ROT_Y, ToString((float)yDeg).c_str());
		SetDlgItemText(hBottomDlg, IDE_BOTTOM_SELECTION_ROT_Z, ToString((float)rDeg).c_str());

		is_update_pos_rot = true;
	}
	else {
		SetDlgItemText(hBottomDlg, IDE_BOTTOM_SELECTION_POS_X, "");
		SetDlgItemText(hBottomDlg, IDE_BOTTOM_SELECTION_POS_Y, "");
		SetDlgItemText(hBottomDlg, IDE_BOTTOM_SELECTION_POS_Z, "");

		SetDlgItemText(hBottomDlg, IDE_BOTTOM_SELECTION_ROT_X, "");
		SetDlgItemText(hBottomDlg, IDE_BOTTOM_SELECTION_ROT_Y, "");
		SetDlgItemText(hBottomDlg, IDE_BOTTOM_SELECTION_ROT_Z, "");
	}

	if (editor_application->getGhostNode())
	{
		SendDlgItemMessage(hBottomDlg, IDS_BOTTOM_GHOST_SEEK, (UINT)TBM_SETRANGEMAX, TRUE, editor_application->getGhostNode()->getDuration() * 1000);
		SendDlgItemMessage(hBottomDlg, IDS_BOTTOM_GHOST_SEEK, (UINT)TBM_SETPOS, TRUE, editor_application->getGhostNode()->getTime() * 1000);
	}
}


void EditorApplication::updateBottomSelectionPosition(float value_x, float value_y, float value_z) {
	axis->setPositionAndTranslate(Ogre::Vector3(value_x, value_y, value_z));
	translateSelection(axis->getTranslate());
}


void EditorApplication::updateBottomSelectionRotation(float value_x, float value_y, float value_z) {
	Ogre::Radian yRad = Ogre::Degree(value_y);
	Ogre::Radian pRad = Ogre::Degree(value_x);
	Ogre::Radian rRad = Ogre::Degree(value_z);

	Ogre::Matrix3 mat;
	mat.FromEulerAnglesYXZ(yRad, pRad, rRad);

	Ogre::Quaternion rotation(mat);

	if (!rotation.isNaN() && (rotation.Norm() > 0)) {
		axis->setRotationAndTranslate(rotation);
		setSelectionRotation(rotation);
	}
}

bool EditorApplication::isUpdatePosRot()
{
	return is_update_pos_rot;
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
			if (LOWORD(wParam) == IDC_BOTTOM_CURRENT_OBJECT_SET_VISIBLE) {
				bool current_set_visible = IsDlgButtonChecked(hDlg, IDC_BOTTOM_CURRENT_OBJECT_SET_VISIBLE);
				editor_application->updateCurrentSetVisible(current_set_visible);
			}
			else if (HIWORD(wParam) == EN_CHANGE) {
				if (!editor_application->getEditorAxis()->isHolding() && editor_application->isUpdatePosRot()) {
					if ((LOWORD(wParam) == IDE_BOTTOM_SELECTION_POS_X) || (LOWORD(wParam) == IDE_BOTTOM_SELECTION_POS_Y) || (LOWORD(wParam) == IDE_BOTTOM_SELECTION_POS_Z)) {
						float value_x = GetDlgItemFloat(hDlg, IDE_BOTTOM_SELECTION_POS_X);
						float value_y = GetDlgItemFloat(hDlg, IDE_BOTTOM_SELECTION_POS_Y);
						float value_z = GetDlgItemFloat(hDlg, IDE_BOTTOM_SELECTION_POS_Z);

						editor_application->updateBottomSelectionPosition(value_x, value_y, value_z);
					}

					if ((LOWORD(wParam) == IDE_BOTTOM_SELECTION_ROT_X) || (LOWORD(wParam) == IDE_BOTTOM_SELECTION_ROT_Y) || (LOWORD(wParam) == IDE_BOTTOM_SELECTION_ROT_Z)) {
						float value_x = GetDlgItemFloat(hDlg, IDE_BOTTOM_SELECTION_ROT_X);
						float value_y = GetDlgItemFloat(hDlg, IDE_BOTTOM_SELECTION_ROT_Y);
						float value_z = GetDlgItemFloat(hDlg, IDE_BOTTOM_SELECTION_ROT_Z);

						editor_application->updateBottomSelectionRotation(value_x, value_y, value_z);
					}
				}
			}
			else if(HIWORD(wParam) == CBN_SELCHANGE) { 
				char value_str[1024] = "";
				int nIndex=SendDlgItemMessage(hDlg, LOWORD(wParam), (UINT) CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0);
				SendDlgItemMessage(hDlg, LOWORD(wParam), (UINT)CB_GETLBTEXT, (WPARAM)nIndex, (LPARAM)value_str);

				if (LOWORD(wParam) == IDC_BOTTOM_CURRENT_OBJECT_SET)  {
					string change_set = ToString(value_str);
					if (change_set == NEW_SET_OPTION) {
						editor_application->newCurrentSet();
					}
					else if (change_set == DELETE_SET_OPTION) {
						editor_application->deleteCurrentSet();
					}
					else {
						editor_application->changeCurrentSet(change_set);
					}
				}
			}
			else if (HIWORD(wParam) == CBN_EDITCHANGE) {
				char value_str[1024] = "";
				GetDlgItemText(hDlg, LOWORD(wParam), value_str, 1024);

				if (LOWORD(wParam) == IDC_BOTTOM_CURRENT_OBJECT_SET)  {
					int index = ComboBox_FindStringExact(GetDlgItem(hDlg, IDC_BOTTOM_CURRENT_OBJECT_SET), 0, editor_application->getCurrentSet()->getName().c_str());
					if (index >= 0) {
						SendDlgItemMessage(hDlg, IDC_BOTTOM_CURRENT_OBJECT_SET, (UINT)CB_DELETESTRING, (WPARAM)index, (LPARAM)0);
						SendDlgItemMessage(hDlg, IDC_BOTTOM_CURRENT_OBJECT_SET, (UINT)CB_INSERTSTRING, (WPARAM)index, (LPARAM)value_str);
						editor_application->renameCurrentSet(ToString(value_str));
					}
				}
			}
			else if (LOWORD(wParam) == IDB_BOTTOM_GHOST_PLAY) {
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

void EditorApplication::updateSetsGUI() {
	set_indices.clear();
	SendDlgItemMessage(hBottomDlg, IDC_BOTTOM_CURRENT_OBJECT_SET, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);

	int index = 0;
	list<LibGens::ObjectSet *> sets = current_level->getLevel()->getSets();
	for (list<LibGens::ObjectSet *>::iterator it = sets.begin(); it != sets.end(); it++) {
		string set_name = (*it)->getName();
		SendDlgItemMessage(hBottomDlg, IDC_BOTTOM_CURRENT_OBJECT_SET, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)set_name.c_str());
		set_indices[*it] = index++;
		set_visibility[*it] = true;
	}

	SendDlgItemMessage(hBottomDlg, IDC_BOTTOM_CURRENT_OBJECT_SET, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM) NEW_SET_OPTION);
	SendDlgItemMessage(hBottomDlg, IDC_BOTTOM_CURRENT_OBJECT_SET, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM) DELETE_SET_OPTION);
}

void EditorApplication::updateSelectedSetGUI() {
	SendDlgItemMessage(hBottomDlg, IDC_BOTTOM_CURRENT_OBJECT_SET, CB_SETCURSEL, (WPARAM)set_indices[current_set], (LPARAM)0);
	SendDlgItemMessage(hBottomDlg, IDC_BOTTOM_CURRENT_OBJECT_SET_VISIBLE, BM_SETCHECK, (WPARAM) set_visibility[current_set], 0);
}