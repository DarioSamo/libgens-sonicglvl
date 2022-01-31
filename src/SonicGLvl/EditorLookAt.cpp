#include "EditorApplication.h"
#include "EditorNodeHistory.h"
#include "ObjectNodeHistory.h"

INT_PTR CALLBACK LookAtPointCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

void EditorApplication::lookAt(EditorNode* node, int axis, Ogre::Vector3 direction)
{
	Ogre::Quaternion node_rotation = node->getRotation();
	Ogre::Radian y_rad, p_rad, r_rad;
	Ogre::Matrix3 rot_matrix;
	node_rotation.ToRotationMatrix(rot_matrix);
	rot_matrix.ToEulerAnglesYXZ(y_rad, p_rad, r_rad);

	Ogre::Real yaw_rad = y_rad.valueRadians();
	Ogre::Real pitch_rad = p_rad.valueRadians();
	Ogre::Real roll_rad = r_rad.valueRadians();

	yaw_rad = atan2(direction.x, direction.z);
	pitch_rad = asin(-direction.y);

	switch (axis)
	{
	case LIBGENS_MATH_AXIS_X:
		yaw_rad += ((90 * LIBGENS_MATH_PI) / 180);
		break;

	case LIBGENS_MATH_AXIS_Y:
		pitch_rad += ((90 * LIBGENS_MATH_PI) / 180);
		break;

	default:
		break;
	}

	y_rad = yaw_rad;
	p_rad = pitch_rad;
	rot_matrix.FromEulerAnglesYXZ(y_rad, p_rad, r_rad);
	node_rotation = Ogre::Quaternion(rot_matrix);

	node->rememberRotation();
	node->setRotation(node_rotation);
}

void EditorApplication::lookAtEachOther(int axis)
{
	if (selected_nodes.size() != 2)
		return;

	list<EditorNode*>::iterator it = selected_nodes.begin();
	EditorNode* node1 = *it++;
	EditorNode* node2 = *it;

	if (node1->getType() == EDITOR_NODE_OBJECT || node1->getType() == EDITOR_NODE_OBJECT_MSP &&
		node2->getType() == EDITOR_NODE_OBJECT || node2->getType() == EDITOR_NODE_OBJECT_MSP)
	{
		HistoryActionWrapper* wrapper = new HistoryActionWrapper();
		HistoryActionWrapper* sub_wrapper = new HistoryActionWrapper();

		for (it = selected_nodes.begin(); it != selected_nodes.end(); ++it)
		{
			EditorNode* node = *it;
			EditorNode* other_node;
			if (it == selected_nodes.begin())
			{
				++it;
				other_node = *it;
				--it;
			}
			else
			{
				--it;
				other_node = *it;
				++it;
			}

			Ogre::Vector3 direction = other_node->getPosition() - node->getPosition();
			direction.normalise();

			lookAt(node, axis, direction);
			HistoryActionRotateNode* action_rot = new HistoryActionRotateNode(node, node->getLastRotation(), node->getRotation());
			sub_wrapper->push(action_rot);
		}

		wrapper->push(sub_wrapper);
		pushHistory(wrapper);
	}
}

void EditorApplication::lookAtPoint(int axis, Ogre::Vector3 v)
{
	if (!selected_nodes.size())
		return;

	HistoryActionWrapper* wrapper = new HistoryActionWrapper();
	HistoryActionWrapper* sub_wrapper = new HistoryActionWrapper();

	for (list<EditorNode*>::iterator it = selected_nodes.begin(); it != selected_nodes.end(); ++it)
	{
		EditorNode* node = *it;
		if (node->getType() == EDITOR_NODE_OBJECT || node->getType() == EDITOR_NODE_OBJECT_MSP)
		{
			Ogre::Vector3 direction = v - node->getPosition();
			direction.normalise();

			lookAt(node, axis, direction);
			HistoryActionRotateNode* action_rot = new HistoryActionRotateNode(node, node->getLastRotation(), node->getRotation());
			sub_wrapper->push(action_rot);
		}
	}

	wrapper->push(sub_wrapper);
	pushHistory(wrapper);
}

void EditorApplication::openLookAtPointGUI()
{
	if (!hLookAtPointDlg)
	{
		if (!selected_nodes.size()) return;
		list<EditorNode*>::iterator it = selected_nodes.begin();

		Ogre::Vector3 position = (*it)->getPosition();
		vector_node = new VectorNode(scene_manager);
		vector_node->setPosition(position);

		hLookAtPointDlg = CreateDialog(NULL, MAKEINTRESOURCE(IDD_LOOK_AT_POINT), hwnd, LookAtPointCallback);
		SetDlgItemText(hLookAtPointDlg, IDE_LOOK_AT_X, ToString<float>(position.x).c_str());
		SetDlgItemText(hLookAtPointDlg, IDE_LOOK_AT_Y, ToString<float>(position.y).c_str());
		SetDlgItemText(hLookAtPointDlg, IDE_LOOK_AT_Z, ToString<float>(position.z).c_str());
		SendDlgItemMessage(hLookAtPointDlg, IDR_LOOK_AT_AXIS_Z, BM_SETCHECK, (WPARAM)1, 0);
		SetFocus(hLookAtPointDlg);

		is_update_look_at_vector = true;
	}
}

void EditorApplication::closeLookAtPointGUI()
{
	if (hLookAtPointDlg)
	{
		queryLookAtObject(false);
		hLookAtPointDlg = NULL;
	}
	delete vector_node;
}

void EditorApplication::updateLookAtVectorMode(bool mode_state) {
	SendDlgItemMessage(hLookAtPointDlg, IDC_LOOK_AT_VIEWPORT, BM_SETCHECK, (WPARAM)mode_state, 0);

	EnableWindow(GetDlgItem(hLookAtPointDlg, IDE_LOOK_AT_X), !mode_state);
	EnableWindow(GetDlgItem(hLookAtPointDlg, IDE_LOOK_AT_Y), !mode_state);
	EnableWindow(GetDlgItem(hLookAtPointDlg, IDE_LOOK_AT_Z), !mode_state);

	EnableWindow(GetDlgItem(hLookAtPointDlg, IDS_LOOK_AT_X), !mode_state);
	EnableWindow(GetDlgItem(hLookAtPointDlg, IDS_LOOK_AT_Y), !mode_state);
	EnableWindow(GetDlgItem(hLookAtPointDlg, IDS_LOOK_AT_Z), !mode_state);

	if (mode_state) {
		previous_selected_nodes = selected_nodes;
		for (list<EditorNode*>::iterator it = selected_nodes.begin(); it != selected_nodes.end(); it++) {
			(*it)->setSelect(false);
		}
		selected_nodes.clear();

		setEditorMode(EDITOR_NODE_QUERY_VECTOR);
		SetFocus(hwnd);

		look_at_vector_history->clear();

		selected_nodes.push_back(vector_node);
		vector_node->setSelect(true);

		updateSelection();
		focusLookAtPointVector();

		// Move window to the bottom right corner of the main window
		RECT main_window_rect;
		GetWindowRect(hwnd, &main_window_rect);
		GetWindowRect(hLookAtPointDlg, &hLookAtPointDlg_old_rect);

		LONG dlg_w = hLookAtPointDlg_old_rect.right - hLookAtPointDlg_old_rect.left;
		LONG dlg_h = hLookAtPointDlg_old_rect.bottom - hLookAtPointDlg_old_rect.top;
		MoveWindow(hLookAtPointDlg, main_window_rect.right - dlg_w - 15, main_window_rect.bottom - dlg_h - SONICGLVL_GUI_BOTTOM_HEIGHT - 15, dlg_w, dlg_h, true);
	}
	else {
		for (list<EditorNode*>::iterator it = selected_nodes.begin(); it != selected_nodes.end(); it++) {
			(*it)->setSelect(false);
		}

		selected_nodes = previous_selected_nodes;
		for (list<EditorNode*>::iterator it = selected_nodes.begin(); it != selected_nodes.end(); it++) {
			(*it)->setSelect(true);
		}

		setEditorMode(EDITOR_NODE_QUERY_OBJECT);
		SetFocus(hLookAtPointDlg);

		look_at_vector_history->clear();
		updateSelection();

		// Restore Window to old position
		LONG dlg_w = hLookAtPointDlg_old_rect.right - hLookAtPointDlg_old_rect.left;
		LONG dlg_h = hLookAtPointDlg_old_rect.bottom - hLookAtPointDlg_old_rect.top;
		MoveWindow(hLookAtPointDlg, hLookAtPointDlg_old_rect.left, hLookAtPointDlg_old_rect.top, dlg_w, dlg_h, true);
	}
}

void EditorApplication::updateLookAtVectorGUI()
{
	is_update_look_at_vector = false;
	if (!vector_node || !hLookAtPointDlg) return;

	Ogre::Vector3 position = vector_node->getPosition();
	SetDlgItemText(hLookAtPointDlg, IDE_LOOK_AT_X, ToString<float>(position.x).c_str());
	SetDlgItemText(hLookAtPointDlg, IDE_LOOK_AT_Y, ToString<float>(position.y).c_str());
	SetDlgItemText(hLookAtPointDlg, IDE_LOOK_AT_Z, ToString<float>(position.z).c_str());

	is_update_look_at_vector = true;
}

void EditorApplication::updateLookAtPointVectorNode(Ogre::Vector3 v)
{
	vector_node->setPosition(v);
}

void EditorApplication::focusLookAtPointVector()
{
	if (vector_node)
		viewport->focusOnPoint(vector_node->getPosition());
}

bool EditorApplication::isUpdateLookAtVector()
{
	return is_update_look_at_vector;
}

void EditorApplication::queryLookAtObject(bool mode)
{
	SendDlgItemMessage(hLookAtPointDlg, IDC_LOOK_AT_OBJECT, BM_SETCHECK, (WPARAM)mode, 0);

	if (mode)
	{
		is_pick_target_position = true;
		SetFocus(hwnd);

		RECT main_window_rect;
		GetWindowRect(hwnd, &main_window_rect);
		GetWindowRect(hLookAtPointDlg, &hLookAtPointDlg_old_rect);

		LONG dlg_w = hLookAtPointDlg_old_rect.right - hLookAtPointDlg_old_rect.left;
		LONG dlg_h = hLookAtPointDlg_old_rect.bottom - hLookAtPointDlg_old_rect.top;
		MoveWindow(hLookAtPointDlg, main_window_rect.right - dlg_w - 15, main_window_rect.bottom - dlg_h - SONICGLVL_GUI_BOTTOM_HEIGHT - 15, dlg_w, dlg_h, true);
	}
	else
	{
		is_pick_target_position = false;
		LONG dlg_w = hLookAtPointDlg_old_rect.right - hLookAtPointDlg_old_rect.left;
		LONG dlg_h = hLookAtPointDlg_old_rect.bottom - hLookAtPointDlg_old_rect.top;
		MoveWindow(hLookAtPointDlg, hLookAtPointDlg_old_rect.left, hLookAtPointDlg_old_rect.top, dlg_w, dlg_h, true);
	}
}

INT_PTR CALLBACK LookAtPointCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		return true;

	case WM_CLOSE:
		EndDialog(hDlg, false);
		editor_application->closeLookAtPointGUI();
		return true;

	case WM_COMMAND:
		if (HIWORD(wParam) == EN_CHANGE) {
			if ((LOWORD(wParam) == IDE_LOOK_AT_X) || (LOWORD(wParam) == IDE_LOOK_AT_Y) || (LOWORD(wParam) == IDE_LOOK_AT_Z)) {
				if (editor_application->isUpdateLookAtVector())
				{
					float value_x = 0.0f;
					float value_y = 0.0f;
					float value_z = 0.0f;

					value_x = GetDlgItemFloat(hDlg, IDE_LOOK_AT_X);
					value_y = GetDlgItemFloat(hDlg, IDE_LOOK_AT_Y);
					value_z = GetDlgItemFloat(hDlg, IDE_LOOK_AT_Z);

					editor_application->updateLookAtPointVectorNode(Ogre::Vector3(value_x, value_y, value_z));
				}
			}
		}

		switch (LPARAM(wParam))
		{
		case IDC_LOOK_AT_VIEWPORT:
		{
			if (IsDlgButtonChecked(hDlg, IDC_LOOK_AT_OBJECT))
				editor_application->queryLookAtObject(false);
			
			editor_application->updateLookAtVectorMode(IsDlgButtonChecked(hDlg, IDC_LOOK_AT_VIEWPORT));
		}
		break;

		case IDC_LOOK_AT_OBJECT:
		{
			if (IsDlgButtonChecked(hDlg, IDC_LOOK_AT_VIEWPORT))
				editor_application->updateLookAtVectorMode(false);

			editor_application->queryLookAtObject(IsDlgButtonChecked(hDlg, IDC_LOOK_AT_OBJECT));
		}
		break;

		case IDB_LOOK_AT_FOCUS:
		{
			if (IsDlgButtonChecked(hDlg, IDC_LOOK_AT_OBJECT))
				editor_application->queryLookAtObject(IsDlgButtonChecked(hDlg, IDC_LOOK_AT_OBJECT));

			editor_application->focusLookAtPointVector();
		}
		break;

		case IDOK:
		{
			int axis;
			if (IsDlgButtonChecked(hDlg, IDR_LOOK_AT_AXIS_X))
				axis = LIBGENS_MATH_AXIS_X;
			else if (IsDlgButtonChecked(hDlg, IDR_LOOK_AT_AXIS_Y))
				axis = LIBGENS_MATH_AXIS_Y;
			else
				axis = LIBGENS_MATH_AXIS_Z;

			float value_x = GetDlgItemFloat(hDlg, IDE_LOOK_AT_X);
			float value_y = GetDlgItemFloat(hDlg, IDE_LOOK_AT_Y);
			float value_z = GetDlgItemFloat(hDlg, IDE_LOOK_AT_Z);
			Ogre::Vector3 position(value_x, value_y, value_z);

			if (IsDlgButtonChecked(hDlg, IDC_LOOK_AT_VIEWPORT))
				editor_application->updateLookAtVectorMode(false);

			editor_application->lookAtPoint(axis, position);
			editor_application->updateSelection();
			SendMessage(hDlg, WM_CLOSE, NULL, NULL);
		}
		break;

		case IDCANCEL:
			SendMessage(hDlg, WM_CLOSE, NULL, NULL);
			break;
		}

		break;

	case WM_NOTIFY:
		if ((LOWORD(wParam) == IDS_LOOK_AT_X) || (LOWORD(wParam) == IDS_LOOK_AT_Y) || (LOWORD(wParam) == IDS_LOOK_AT_Z)) {
			if (((LPNMUPDOWN)lParam)->hdr.code == UDN_DELTAPOS) {
				int delta = ((LPNMUPDOWN)lParam)->iDelta;

				float value_x = 0.0f;
				float value_y = 0.0f;
				float value_z = 0.0f;

				float spin_factor = 1.0;

				value_x = GetDlgItemFloat(hDlg, IDE_LOOK_AT_X);
				value_y = GetDlgItemFloat(hDlg, IDE_LOOK_AT_Y);
				value_z = GetDlgItemFloat(hDlg, IDE_LOOK_AT_Z);

				if (delta > 1)  delta = 1;
				if (delta < -1) delta = -1;

				if (LOWORD(wParam) == IDS_LOOK_AT_X) {
					value_x += (float)-delta * spin_factor;
					SetDlgItemText(hDlg, IDE_LOOK_AT_X, ToString(value_x).c_str());
				}

				if (LOWORD(wParam) == IDS_LOOK_AT_Y) {
					value_y += (float)-delta * spin_factor;
					SetDlgItemText(hDlg, IDE_LOOK_AT_Y, ToString(value_y).c_str());
				}

				if (LOWORD(wParam) == IDS_LOOK_AT_Z) {
					value_z += (float)-delta * spin_factor;
					SetDlgItemText(hDlg, IDE_LOOK_AT_Z, ToString(value_z).c_str());
				}

				editor_application->updateLookAtPointVectorNode(Ogre::Vector3(value_x, value_y, value_z));
			}
		}
		break;
	}

	return false;
}