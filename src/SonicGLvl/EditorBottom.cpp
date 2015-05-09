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

#include "EditorApplication.h"
#include <commctrl.h>

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

		SetDlgItemText(hBottomDlg, IDE_BOTTOM_SELECTION_POS_X, ToString((float)axis_position.x).c_str());
		SetDlgItemText(hBottomDlg, IDE_BOTTOM_SELECTION_POS_Y, ToString((float)axis_position.y).c_str());
		SetDlgItemText(hBottomDlg, IDE_BOTTOM_SELECTION_POS_Z, ToString((float)axis_position.z).c_str());

		SetDlgItemText(hBottomDlg, IDE_BOTTOM_SELECTION_ROT_X, ToString((float)pDeg).c_str());
		SetDlgItemText(hBottomDlg, IDE_BOTTOM_SELECTION_ROT_Y, ToString((float)yDeg).c_str());
		SetDlgItemText(hBottomDlg, IDE_BOTTOM_SELECTION_ROT_Z, ToString((float)rDeg).c_str());
	}
	else {
		SetDlgItemText(hBottomDlg, IDE_BOTTOM_SELECTION_POS_X, "");
		SetDlgItemText(hBottomDlg, IDE_BOTTOM_SELECTION_POS_Y, "");
		SetDlgItemText(hBottomDlg, IDE_BOTTOM_SELECTION_POS_Z, "");

		SetDlgItemText(hBottomDlg, IDE_BOTTOM_SELECTION_ROT_X, "");
		SetDlgItemText(hBottomDlg, IDE_BOTTOM_SELECTION_ROT_Y, "");
		SetDlgItemText(hBottomDlg, IDE_BOTTOM_SELECTION_ROT_Z, "");
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

		case WM_COMMAND:
			if (HIWORD(wParam) == EN_CHANGE) {
				if (!editor_application->getEditorAxis()->isHolding()) {
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

			break;
	}

	return false;
}
