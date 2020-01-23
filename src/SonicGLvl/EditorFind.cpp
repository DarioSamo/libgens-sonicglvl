#include "EditorApplication.h"

INT_PTR CALLBACK findCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

bool matchesQuery(string object_name, string q_string, bool exactly)
{
	// convert search query string and current object's name to lowercase
	// for case insensetive comparison

	size_t object_name_length = object_name.size();
	char* object_name_lower = new char[object_name_length + 1];

	size_t obj_name_length = q_string.size();
	char* obj_name_lower = new char[obj_name_length];

	for (size_t i = 0; i < object_name_length; ++i)
		object_name_lower[i] = tolower(object_name[i]);

	object_name_lower[object_name_length] = '\0';

	for (size_t i = 0; i < obj_name_length; ++i)
		obj_name_lower[i] = tolower(q_string[i]);

	obj_name_lower[obj_name_length] = '\0';

	string object_name_string(object_name_lower);
	string obj_name_string(obj_name_lower);

	delete[] object_name_lower;
	delete[] obj_name_lower;

	if (exactly)
		return (!strcmp(object_name_string.c_str(), obj_name_string.c_str()));
	else
		return object_name_string.find(obj_name_string, 0) != string::npos;
}

void EditorApplication::openFindGUI()
{
	if (!hFindObjectDlg)
	{
		find_position = object_node_manager->getObjectNodes().begin();

		hFindObjectDlg = CreateDialog(NULL, MAKEINTRESOURCE(IDD_FIND_DIALOG), hwnd, findCallback);
		SetFocus(hFindObjectDlg);

		EnableWindow(GetDlgItem(hFindObjectDlg, IDC_FIND_FILTERED), false);
		EnableWindow(GetDlgItem(hFindObjectDlg, IDE_FIND_PROPERTY_VALUE), false);
		EnableWindow(GetDlgItem(hFindObjectDlg, IDE_FIND_VALUE_VALUE), false);
	}
}

void EditorApplication::closeFindGUI()
{
	if (hFindObjectDlg)
		hFindObjectDlg = NULL;
}

void EditorApplication::findNext(string obj_name, string param, string value)
{
	if (!obj_name.size()) return;

	bool found = false;
	bool exact = IsDlgButtonChecked(hFindObjectDlg, IDC_FIND_EXACTLY) ? true : false;

	clearSelection();

	for (list<ObjectNode*>::iterator it = find_position; it != object_node_manager->getObjectNodes().end(); ++it)
	{
		ObjectNode* object_node = *it;
		LibGens::Object* object = object_node->getObject();

		if (object)
		{
			if (matchesQuery(object->getName(), obj_name, exact))
			{
				selectNode(object_node);

				find_position = it;
				if (it != object_node_manager->getObjectNodes().end())
					find_position++;

				found = true;
			}
		}

		if (found)
			break;
	}

	if (!found)
	{
		MessageBox(NULL, "No more matches found.", "SonicGlvl", MB_OK);
		find_position = object_node_manager->getObjectNodes().begin();
	}

	updateSelection();
}

void EditorApplication::findAll(string obj_name, string param, string value)
{
	if (!obj_name.size()) return;

	list<ObjectNode*> object_nodes = object_node_manager->getObjectNodes();
	clearSelection();

	bool exact = IsDlgButtonChecked(hFindObjectDlg, IDC_FIND_EXACTLY) ? true : false;

	for (list<ObjectNode*>::iterator it = object_nodes.begin(); it != object_nodes.end(); ++it)
	{
		ObjectNode* object_node = *it;
		LibGens::Object* object = object_node->getObject();

		if (matchesQuery(object->getName(), obj_name, exact))
		{
			object_node->setSelect(true);
			selected_nodes.push_back(object_node);
		}
	}

	updateSelection();
}

bool EditorApplication::matchesPropertyAndValue(ObjectNode* object_node, string param, string value)
{
	return true;
}

INT_PTR CALLBACK findCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		return true;

	case WM_CLOSE:
		EndDialog(hDlg, true);
		return true;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDB_FIND:
		{
			char name[1024] = "";
			GetDlgItemText(hDlg, IDE_FIND_VALUE, name, 1024);

			if (!IsDlgButtonChecked(hDlg, IDC_FIND_ALL))
				editor_application->findNext(name, "", "");
			else
				editor_application->findAll(name, "", "");

			break;
		}

		case IDCLOSE:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			editor_application->closeFindGUI();
			break;
		}
		break;
	}

	return false;
}
