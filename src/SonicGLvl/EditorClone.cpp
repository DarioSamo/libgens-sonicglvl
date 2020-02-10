#include "StdAfx.h"
#include "EditorApplication.h"

INT_PTR CALLBACK MultiSetParamCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

void EditorApplication::openMultiSetParamDlg()
{
	if (!hMultiSetParamDlg)
	{
		hMultiSetParamDlg = CreateDialog(NULL, MAKEINTRESOURCE(131), hwnd, MultiSetParamCallback);

		SendDlgItemMessage(hMultiSetParamDlg, IDR_MULTISETPARAM_MSP, BM_SETCHECK, (WPARAM)(cloning_mode == SONICGLVL_MULTISETPARAM_MODE_MSP), 0);
		SendDlgItemMessage(hMultiSetParamDlg, IDR_MULTISETPARAM_CLONE, BM_SETCHECK, (WPARAM)(cloning_mode == SONICGLVL_MULTISETPARAM_MODE_CLONE), 0);
		EnableWindow(GetDlgItem(hMultiSetParamDlg, IDR_MULTISETPARAM_MSP_ADD), false);

		clearMultiSetParamDlg();
	}
	
	SetFocus(hMultiSetParamDlg);
}

void EditorApplication::closeMultiSetParamDlg()
{
	if (hMultiSetParamDlg)
		hMultiSetParamDlg = NULL;

	deleteTemporaryNodes();
}

void EditorApplication::clearMultiSetParamDlg()
{
	if (!hMultiSetParamDlg)
		return;

	SetDlgItemText(hMultiSetParamDlg, IDE_MULTISETPARAM_X, "0");
	SetDlgItemText(hMultiSetParamDlg, IDE_MULTISETPARAM_Y, "0");
	SetDlgItemText(hMultiSetParamDlg, IDE_MULTISETPARAM_Z, "0");
	SetDlgItemText(hMultiSetParamDlg, IDE_MULTISETPARAM_COUNT, "0");
	SetDlgItemText(hMultiSetParamDlg, IDE_MULTISETPARAM_SPACING, "0");
}

void EditorApplication::createMultiSetParamObjects()
{
	float spacing = 0, count = 0;
	float vec_x = 0, vec_y = 0, vec_z = 0;

	spacing = GetDlgItemFloat(hMultiSetParamDlg, IDE_MULTISETPARAM_SPACING);
	count = GetDlgItemInt(hMultiSetParamDlg, IDE_MULTISETPARAM_COUNT, NULL, TRUE);

	if (count < 1 || selected_nodes.size() < 1)
	{
		deleteTemporaryNodes();
		return;
	}

	vec_x = GetDlgItemFloat(hMultiSetParamDlg, IDE_MULTISETPARAM_X);
	vec_y = GetDlgItemFloat(hMultiSetParamDlg, IDE_MULTISETPARAM_Y);
	vec_z = GetDlgItemFloat(hMultiSetParamDlg, IDE_MULTISETPARAM_Z);
	LibGens::Vector3 pos_vector(vec_x, vec_y, vec_z);

	list<EditorNode*>::iterator it;

	for (it = selected_nodes.begin(); it != selected_nodes.end(); ++it)
	{
		if ((*it)->getType() == EDITOR_NODE_OBJECT)
		{
			ObjectNode* obj_node = static_cast<ObjectNode*>(*it);
			LibGens::Object* obj = obj_node->getObject();
			LibGens::Vector3 base_pos = obj->getPosition();
			LibGens::Quaternion base_rot = obj->getRotation();
			LibGens::Vector3 new_pos;

			if (cloning_mode == SONICGLVL_MULTISETPARAM_MODE_CLONE)
			{
				for (int i = 1; i <= count; ++i)
				{
					new_pos = base_pos + (pos_vector * (i * spacing));
					LibGens::Object* new_obj = new LibGens::Object(obj);
					new_obj->setPosition(new_pos);
					new_obj->setRotation(base_rot);

					if (current_level) {
						if (current_level->getLevel()) {
							new_obj->setID(current_level->getLevel()->newObjectID());
						}
					}

					if (current_set) {
						current_set->addObject(new_obj);

						if (!current_level) {
							new_obj->setID(current_set->newObjectID());
						}
					}

					ObjectNode* new_object_node = object_node_manager->createObjectNode(new_obj);
				}
			}
			else if (cloning_mode == SONICGLVL_MULTISETPARAM_MODE_MSP)
			{
				// remove old instances
				obj->getMultiSetParam()->removeAllNodes();

				for (int i = 1; i <= count; ++i)
				{
					new_pos = base_pos + (pos_vector * (i * spacing));
					LibGens::MultiSetNode* msp_node = new LibGens::MultiSetNode();

					msp_node->position = new_pos;
					msp_node->rotation = base_rot;

					obj->getMultiSetParam()->addNode(msp_node);
				}

				obj_node->createObjectMultiSetNodes(obj, scene_manager);
				obj_node->clearNames();
				object_node_manager->reloadObjectNode(obj);
			}
		}
	}

	deleteTemporaryNodes();
}

void EditorApplication::setVectorAndSpacing()
{
	list<EditorNode*>::iterator it = selected_nodes.begin();
	if ((*it)->getType() == EDITOR_NODE_OBJECT)
	{
		ObjectNode* obj_node = static_cast<ObjectNode*>(*it);
		LibGens::Vector3 tgt_pos = obj_node->getObject()->getPosition();

		it = cloning_nodes.begin();
		if ((*it)->getType() == EDITOR_NODE_OBJECT)
		{
			ObjectNode* origin_node = static_cast<ObjectNode*>(*it);
			LibGens::Vector3 originPos = origin_node->getObject()->getPosition();
			LibGens::Vector3 v = tgt_pos - originPos;

			float spacing = v.normalise();

			SetDlgItemText(hMultiSetParamDlg, IDE_MULTISETPARAM_X, ToString<float>(v.x).c_str());
			SetDlgItemText(hMultiSetParamDlg, IDE_MULTISETPARAM_Y, ToString<float>(v.y).c_str());
			SetDlgItemText(hMultiSetParamDlg, IDE_MULTISETPARAM_Z, ToString<float>(v.z).c_str());
			SetDlgItemText(hMultiSetParamDlg, IDE_MULTISETPARAM_SPACING, ToString<float>(spacing).c_str());
			SetDlgItemText(hMultiSetParamDlg, IDE_MULTISETPARAM_COUNT, "1");
		}
	}
}

void EditorApplication::getVectorFromObject()
{
	if (!selected_nodes.size())
		return;

	list<EditorNode*>::iterator it = selected_nodes.begin();

	if ((*it)->getType() == EDITOR_NODE_OBJECT)
	{
		ObjectNode* obj_node = static_cast<ObjectNode*>(*it);

		Ogre::Quaternion obj_rotation = obj_node->getRotation();
		Ogre::Radian y_rad, p_rad, r_rad;
		Ogre::Matrix3 rot_matrix;
		obj_rotation.ToRotationMatrix(rot_matrix);
		rot_matrix.ToEulerAnglesYXZ(y_rad, p_rad, r_rad);
		
		Ogre::Real yaw_rad = y_rad.valueRadians();
		Ogre::Real pitch_rad = p_rad.valueRadians();
		Ogre::Real roll_rad = r_rad.valueRadians();

		float vec_x = sin(yaw_rad);
		float vec_y = sin(pitch_rad);
		float vec_z = cos(yaw_rad);

		vec_y *= -1;

		SetDlgItemText(hMultiSetParamDlg, IDE_MULTISETPARAM_X, ToString<float>(vec_x).c_str());
		SetDlgItemText(hMultiSetParamDlg, IDE_MULTISETPARAM_Y, ToString<float>(vec_y).c_str());
		SetDlgItemText(hMultiSetParamDlg, IDE_MULTISETPARAM_Z, ToString<float>(vec_z).c_str());
	}

}

void EditorApplication::setCloningMode(size_t mode)
{
	cloning_mode = mode;
}

void EditorApplication::deleteTemporaryNodes()
{
	if (!temporary_nodes.size()) return;

	for (list<EditorNode*>::iterator it = temporary_nodes.begin(); it != temporary_nodes.end(); it++) {
		if ((*it)->getType() == EDITOR_NODE_OBJECT) {
			ObjectNode* object_node = static_cast<ObjectNode*>(*it);

			LibGens::Object* object = object_node->getObject();
			if (object) {
				LibGens::ObjectSet* object_set = object->getParentSet();
				if (object_set) {
					object_set->eraseObject(object);
				}

				(*it)->setSelect(false);
				object_node_manager->deleteObjectNode(object);
				delete object;
			}
		}
	}

	temporary_nodes.clear();
}

INT_PTR CALLBACK MultiSetParamCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		return true;

	case WM_CLOSE:
		EndDialog(hDlg, false);
		editor_application->closeMultiSetParamDlg();
		return true;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return true;

		case IDB_MULTISETPARAM_CREATE:
			editor_application->createMultiSetParamObjects();
			break;

		case IDB_MULTISETPARAM_FROM_OBJECT:
			editor_application->getVectorFromObject();
			break;

		case IDR_MULTISETPARAM_CLONE:
			editor_application->setCloningMode(SONICGLVL_MULTISETPARAM_MODE_CLONE);
			break;

		case IDR_MULTISETPARAM_MSP:
			editor_application->setCloningMode(SONICGLVL_MULTISETPARAM_MODE_MSP);
			break;

		case IDB_MULTISETPARAM_CLEAR:
			editor_application->clearMultiSetParamDlg();
			break;
		}
		break;
		
	case WM_NOTIFY:
		if (LOWORD(wParam) == IDS_MULTISETPARAM_COUNT)
		{
			if (((LPNMUPDOWN)lParam)->hdr.code == UDN_DELTAPOS)
			{
				int delta = (LPNMUPDOWN(lParam))->iDelta;
				int count = GetDlgItemFloat(hDlg, IDE_MULTISETPARAM_COUNT);

				if (delta > 1) delta = 1;
				if (delta < -1) delta = -1;

				count += -delta;
				if (count < 1) 	count = 0;

				SetDlgItemText(hDlg, IDE_MULTISETPARAM_COUNT, ToString<int>(count).c_str());
			}
		}
		break;
	}

	return false;
}