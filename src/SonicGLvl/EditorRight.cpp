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

void EditorApplication::initializeCurrentLayerGUI() {
	// fill combo box with available list of sets
	SendDlgItemMessage(hRightDlg, IDC_RIGHT_CURRENT_LAYER, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
	
	for (int i = 0; i < set_mapping.size(); i++)
	{
		SendDlgItemMessage(hRightDlg, IDC_RIGHT_CURRENT_LAYER, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)set_mapping[i]->getName().c_str());
	}

	SendDlgItemMessage(hRightDlg, IDC_RIGHT_CURRENT_LAYER, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
}

void EditorApplication::updateCurrentLayerGUI() {
	// update current layer base on selected objects
	if (selected_nodes.empty())
	{
		int selected_index = SendDlgItemMessage(hRightDlg, IDC_RIGHT_CURRENT_LAYER, CB_GETCURSEL, 0, 0);
		if (!set_mapping.count(selected_index) && !set_mapping.empty())
		{
			// reset to first layer
			SendDlgItemMessage(hRightDlg, IDC_RIGHT_CURRENT_LAYER, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
		}
		return;
	}

	LibGens::ObjectSet* set_to_display = NULL;
	for (auto node : selected_nodes)
	{
		ObjectNode* object_node = getObjectNodeFromEditorNode(node);
		if (!object_node)
		{
			// not an object, ignore
			return;
		}

		LibGens::ObjectSet* set = object_node->getObject()->getParentSet();
		if (set_to_display == NULL)
		{
			set_to_display = set;
		}
		else if (set_to_display != set)
		{
			// there are multiple sets, set as blank selection
			SendDlgItemMessage(hRightDlg, IDC_RIGHT_CURRENT_LAYER, CB_SETCURSEL, -1, (LPARAM)0);
			return;
		}
	}

	for (auto& it : set_mapping)
	{
		// find index to display
		if (set_to_display == it.second)
		{
			SendDlgItemMessage(hRightDlg, IDC_RIGHT_CURRENT_LAYER, CB_SETCURSEL, it.first, (LPARAM)0);
			return;
		}
	}
}

void EditorApplication::transferObjectsToLayer(int index) {
	if (selected_nodes.empty() || !set_mapping.count(index)) return;

	for (auto node : selected_nodes)
	{
		ObjectNode* object_node = getObjectNodeFromEditorNode(node);
		if (!object_node)
		{
			// not an object, ignore
			updateCurrentLayerGUI();
			return;
		}
	}

	LibGens::ObjectSet* new_set = set_mapping[index];
	if (selected_nodes.size() > 1)
	{
		string text = "Do you want to move " + to_string(selected_nodes.size()) + " selected objects to '" + new_set->getName() + "' layer?";
		if (MessageBox(NULL, text.c_str(), "Transfer Objects To Layer", MB_YESNO | MB_ICONWARNING) != IDYES)
		{
			updateCurrentLayerGUI();
			return;
		}
	}

	for (auto node : selected_nodes)
	{
		ObjectNode* object_node = getObjectNodeFromEditorNode(node);
		LibGens::Object* object = object_node->getObject();
		LibGens::ObjectSet* prev_set = object->getParentSet();
		prev_set->eraseObject(object);
		new_set->addObject(object);

		// Brian TODO: undo?
	}

	// update object count
	updateLayerControlGUI();
}

void EditorApplication::updateTransformGUI() {
	HWND hSelectionEditPosX = GetDlgItem(hRightDlg, IDE_RIGHT_SELECTION_POS_X);
	HWND hSelectionEditPosY = GetDlgItem(hRightDlg, IDE_RIGHT_SELECTION_POS_Y);
	HWND hSelectionEditPosZ = GetDlgItem(hRightDlg, IDE_RIGHT_SELECTION_POS_Z);
	HWND hSelectionEditRotX = GetDlgItem(hRightDlg, IDE_RIGHT_SELECTION_ROT_X);
	HWND hSelectionEditRotY = GetDlgItem(hRightDlg, IDE_RIGHT_SELECTION_ROT_Y);
	HWND hSelectionEditRotZ = GetDlgItem(hRightDlg, IDE_RIGHT_SELECTION_ROT_Z);
	HWND hSelectionSpinPosX = GetDlgItem(hRightDlg, IDS_RIGHT_SELECTION_POS_X);
	HWND hSelectionSpinPosY = GetDlgItem(hRightDlg, IDS_RIGHT_SELECTION_POS_Y);
	HWND hSelectionSpinPosZ = GetDlgItem(hRightDlg, IDS_RIGHT_SELECTION_POS_Z);
	HWND hSelectionSpinRotX = GetDlgItem(hRightDlg, IDS_RIGHT_SELECTION_ROT_X);
	HWND hSelectionSpinRotY = GetDlgItem(hRightDlg, IDS_RIGHT_SELECTION_ROT_Y);
	HWND hSelectionSpinRotZ = GetDlgItem(hRightDlg, IDS_RIGHT_SELECTION_ROT_Z);

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
		Ogre::Vector3 axis_position = axis->getPosition();
		Ogre::Quaternion axis_rotation = axis->getRotation();

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

		SetDlgItemText(hRightDlg, IDE_RIGHT_SELECTION_POS_X, ToString((float)axis_position.x).c_str());
		SetDlgItemText(hRightDlg, IDE_RIGHT_SELECTION_POS_Y, ToString((float)axis_position.y).c_str());
		SetDlgItemText(hRightDlg, IDE_RIGHT_SELECTION_POS_Z, ToString((float)axis_position.z).c_str());

		SetDlgItemText(hRightDlg, IDE_RIGHT_SELECTION_ROT_X, ToString((float)pDeg).c_str());
		SetDlgItemText(hRightDlg, IDE_RIGHT_SELECTION_ROT_Y, ToString((float)yDeg).c_str());
		SetDlgItemText(hRightDlg, IDE_RIGHT_SELECTION_ROT_Z, ToString((float)rDeg).c_str());

		is_update_pos_rot = true;
	}
	else {
		SetDlgItemText(hRightDlg, IDE_RIGHT_SELECTION_POS_X, "");
		SetDlgItemText(hRightDlg, IDE_RIGHT_SELECTION_POS_Y, "");
		SetDlgItemText(hRightDlg, IDE_RIGHT_SELECTION_POS_Z, "");

		SetDlgItemText(hRightDlg, IDE_RIGHT_SELECTION_ROT_X, "");
		SetDlgItemText(hRightDlg, IDE_RIGHT_SELECTION_ROT_Y, "");
		SetDlgItemText(hRightDlg, IDE_RIGHT_SELECTION_ROT_Z, "");
	}
}
void EditorApplication::updateSelectionPosition(float value_x, float value_y, float value_z, bool push_history) {
	rememberSelection(false);
	{
		axis->setPositionAndTranslate(Ogre::Vector3(value_x, value_y, value_z));
		translateSelection(axis->getTranslate());
	}
	if (push_history) makeHistorySelection(false);
}

void EditorApplication::updateSelectionRotation(float value_x, float value_y, float value_z, bool push_history) {
	rememberSelection(true);
	{
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
	if (push_history) makeHistorySelection(true);
}

bool EditorApplication::isUpdatePosRot()
{
	return is_update_pos_rot;
}

INT_PTR CALLBACK RightBarCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
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
		if (((LPNMUPDOWN)lParam)->hdr.code == UDN_DELTAPOS)
		{
			int delta = (LPNMUPDOWN(lParam))->iDelta;

			bool update_transform = false;
			float pos_x = GetDlgItemFloat(hDlg, IDE_RIGHT_SELECTION_POS_X);
			float pos_y = GetDlgItemFloat(hDlg, IDE_RIGHT_SELECTION_POS_Y);
			float pos_z = GetDlgItemFloat(hDlg, IDE_RIGHT_SELECTION_POS_Z);
			float rot_x = GetDlgItemFloat(hDlg, IDE_RIGHT_SELECTION_ROT_X);
			float rot_y = GetDlgItemFloat(hDlg, IDE_RIGHT_SELECTION_ROT_Y);
			float rot_z = GetDlgItemFloat(hDlg, IDE_RIGHT_SELECTION_ROT_Z);

			switch (LOWORD(wParam))
			{
			case IDS_RIGHT_SELECTION_POS_X: update_transform = true; pos_x += (float)delta * 0.01f; break;
			case IDS_RIGHT_SELECTION_POS_Y: update_transform = true; pos_y += (float)delta * 0.01f; break;
			case IDS_RIGHT_SELECTION_POS_Z: update_transform = true; pos_z += (float)delta * 0.01f; break;
			case IDS_RIGHT_SELECTION_ROT_X: update_transform = true; rot_x += (float)delta; break;
			case IDS_RIGHT_SELECTION_ROT_Y: update_transform = true; rot_y += (float)delta; break;
			case IDS_RIGHT_SELECTION_ROT_Z: update_transform = true; rot_z += (float)delta; break;
			}
			
			if (update_transform)
			{
				SetDlgItemText(hDlg, IDE_RIGHT_SELECTION_POS_X, ToString((float)pos_x).c_str());
				SetDlgItemText(hDlg, IDE_RIGHT_SELECTION_POS_Y, ToString((float)pos_y).c_str());
				SetDlgItemText(hDlg, IDE_RIGHT_SELECTION_POS_Z, ToString((float)pos_z).c_str());
				editor_application->updateSelectionPosition(pos_x, pos_y, pos_z, false);

				SetDlgItemText(hDlg, IDE_RIGHT_SELECTION_ROT_X, ToString((float)rot_x).c_str());
				SetDlgItemText(hDlg, IDE_RIGHT_SELECTION_ROT_Y, ToString((float)rot_y).c_str());
				SetDlgItemText(hDlg, IDE_RIGHT_SELECTION_ROT_Z, ToString((float)rot_z).c_str());
				editor_application->updateSelectionRotation(rot_x, rot_y, rot_z, false);
			}

			return true;
		}

		switch (LOWORD(wParam))
		{
		case IDL_RIGHT_PROPERTIES_LIST:
		{
			switch (((LPNMHDR)lParam)->code)
			{
			case NM_CLICK:
			{
				int selection_index = SendMessage(GetDlgItem(hDlg, IDL_RIGHT_PROPERTIES_LIST), LVM_GETNEXTITEM, -1, LVNI_SELECTED);
				editor_application->updateObjectPropertyIndex(selection_index);
				return true;
			}
			case NM_DBLCLK:
			{
				int selection_index = SendMessage(GetDlgItem(hDlg, IDL_RIGHT_PROPERTIES_LIST), LVM_GETNEXTITEM, -1, LVNI_SELECTED);
				editor_application->editObjectPropertyIndex(selection_index);
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
		case IDC_RIGHT_CURRENT_LAYER:
		{
			switch (HIWORD(wParam))
			{
			case CBN_SELCHANGE:
			{
				int selected_index = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
				editor_application->transferObjectsToLayer(selected_index);
				return true;
			}
			}
			break;
		}
		case IDE_RIGHT_SELECTION_POS_X:
		case IDE_RIGHT_SELECTION_POS_Y:
		case IDE_RIGHT_SELECTION_POS_Z:
		{
			switch (HIWORD(wParam))
			{
			case EN_CHANGE:
			{
				if (!editor_application->getEditorAxis()->isHolding() && editor_application->isUpdatePosRot())
				{
					char value_str[1024] = "";
					GetDlgItemText(hDlg, IDE_RIGHT_SELECTION_POS_X, value_str, 1024);
					if (ToString(value_str).empty()) return false;
					GetDlgItemText(hDlg, IDE_RIGHT_SELECTION_POS_Y, value_str, 1024);
					if (ToString(value_str).empty()) return false;
					GetDlgItemText(hDlg, IDE_RIGHT_SELECTION_POS_Z, value_str, 1024);
					if (ToString(value_str).empty()) return false;

					float value_x = GetDlgItemFloat(hDlg, IDE_RIGHT_SELECTION_POS_X);
					float value_y = GetDlgItemFloat(hDlg, IDE_RIGHT_SELECTION_POS_Y);
					float value_z = GetDlgItemFloat(hDlg, IDE_RIGHT_SELECTION_POS_Z);
					editor_application->updateSelectionPosition(value_x, value_y, value_z);
					return true;
				}
			}
			}
			break;
		}
		case IDE_RIGHT_SELECTION_ROT_X:
		case IDE_RIGHT_SELECTION_ROT_Y:
		case IDE_RIGHT_SELECTION_ROT_Z:
		{
			switch (HIWORD(wParam))
			{
			case EN_CHANGE:
			{
				if (!editor_application->getEditorAxis()->isHolding() && editor_application->isUpdatePosRot())
				{
					char value_str[1024] = "";
					GetDlgItemText(hDlg, IDE_RIGHT_SELECTION_ROT_X, value_str, 1024);
					if (ToString(value_str).empty()) return false;
					GetDlgItemText(hDlg, IDE_RIGHT_SELECTION_ROT_Y, value_str, 1024);
					if (ToString(value_str).empty()) return false;
					GetDlgItemText(hDlg, IDE_RIGHT_SELECTION_ROT_Z, value_str, 1024);
					if (ToString(value_str).empty()) return false;

					float value_x = GetDlgItemFloat(hDlg, IDE_RIGHT_SELECTION_ROT_X);
					float value_y = GetDlgItemFloat(hDlg, IDE_RIGHT_SELECTION_ROT_Y);
					float value_z = GetDlgItemFloat(hDlg, IDE_RIGHT_SELECTION_ROT_Z);
					editor_application->updateSelectionRotation(value_x, value_y, value_z);
					return true;
				}
			}
			}
			break;
		}
		}
		break;
	}
	}

	return false;
}