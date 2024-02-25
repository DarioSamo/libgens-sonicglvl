#include "EditorApplication.h"

INT_PTR CALLBACK findCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

bool matchesQuery(string str1, string str2, bool exactly)
{
	// convert search query string and current object's name to lowercase
	// for case insensetive comparison

	for (size_t i = 0; i < str1.size(); ++i)
		str1[i] = tolower(str1[i]);

	for (size_t i = 0; i < str2.size(); ++i)
		str2[i] = tolower(str2[i]);

	if (exactly)
		return (!strcmp(str1.c_str(), str2.c_str()));
	else
		return str1.find(str2, 0) != string::npos;
}


bool valueMatches(LibGens::Object *object, string element_name, string value_string)
{
	LibGens::ObjectElement *element = object->getElement(element_name);
	
	if (!element)
		return false;

	LibGens::ObjectElementType element_type = element->getType();
	string element_value = "";

	// cast the object element to the appropriate type

	switch (element_type)
	{
	case LibGens::OBJECT_ELEMENT_BOOL:
	{
		LibGens::ObjectElementBool* element_bool = static_cast<LibGens::ObjectElementBool*>(element);
		bool value = element_bool->value;
		element_value = value ? "true" : "false";
		break;
	}

	case LibGens::OBJECT_ELEMENT_FLOAT:
	{
		LibGens::ObjectElementFloat* element_float = static_cast<LibGens::ObjectElementFloat*>(element);
		float value = element_float->value;
		element_value = ToString<float>(value);
		break;
	}

	case LibGens::OBJECT_ELEMENT_ID:
	{
		LibGens::ObjectElementID* element_id = static_cast<LibGens::ObjectElementID*>(element);
		size_t value = element_id->value;
		element_value = ToString<size_t>(value);
		break;
	}

	case LibGens::OBJECT_ELEMENT_INTEGER:
	{
		LibGens::ObjectElementInteger* element_int = static_cast<LibGens::ObjectElementInteger*>(element);
		unsigned int value = element_int->value;
		element_value = ToString<unsigned int>(value);
		break;
	}

	case LibGens::OBJECT_ELEMENT_STRING:
	{
		LibGens::ObjectElementString* element_string = static_cast<LibGens::ObjectElementString*>(element);
		element_value = element_string->value;
		break;
	}

	case LibGens::OBJECT_ELEMENT_VECTOR:
	{
		LibGens::ObjectElementVector* element_vector = static_cast<LibGens::ObjectElementVector*>(element);
		LibGens::Vector3 v3 = element_vector->value;
		element_value = ToString<float>(v3.x) + ", " + ToString<float>(v3.y) + ", " + ToString<float>(v3.z);
		break;
	}

	case LibGens::OBJECT_ELEMENT_ID_LIST:
	{
		LibGens::ObjectElementIDList* element_id_list = static_cast<LibGens::ObjectElementIDList*>(element);
		vector<size_t> id_list = element_id_list->value;
		vector<string> id_list_str;
		for (size_t count = 0; count < id_list.size(); ++count)
		{
			id_list_str.push_back(ToString<size_t>(id_list[count]));
		}

		for (size_t count = 0; count < id_list_str.size(); ++count)
		{
			if (matchesQuery(id_list_str[count], value_string, true))
				return true;
		}

		return false;
	}

	case LibGens::OBJECT_ELEMENT_VECTOR_LIST:
	{
		LibGens::ObjectElementVectorList* element_vector_list = static_cast<LibGens::ObjectElementVectorList*>(element);
		vector<LibGens::Vector3> vector_list = element_vector_list->value;
		vector<string> vector_list_str;

		for (size_t count = 0; count < vector_list.size(); ++count)
		{
			LibGens::Vector3 v3 = vector_list[count];
			string v3_str = ToString<float>(v3.x) + ", " + ToString<float>(v3.y) + ", " + ToString<float>(v3.z);
			vector_list_str.push_back(v3_str);
		}

		for (size_t count = 0; count < vector_list_str.size(); ++count)
		{
			if (matchesQuery(vector_list_str[count], value_string, true))
				return true;
		}

		return false;
	}

	default:
		break;
	}

	return matchesQuery(element_value, value_string, true);
}

void EditorApplication::openFindGUI()
{
	if (!hFindObjectDlg)
	{
		find_position = object_node_manager->getObjectNodes().begin();

		hFindObjectDlg = CreateDialog(NULL, MAKEINTRESOURCE(IDD_FIND_DIALOG), hwnd, findCallback);
		HWND main_control = GetDlgItem(hFindObjectDlg, IDE_FIND_VALUE);
		HWND property_text = GetDlgItem(hFindObjectDlg, IDE_FIND_PROPERTY_VALUE);
		HWND value_text = GetDlgItem(hFindObjectDlg, IDE_FIND_VALUE_VALUE);

		EnableWindow(property_text, false);
		EnableWindow(value_text, false);
		SetFocus(main_control);
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

	for (list<ObjectNode*>::iterator it = find_position; it != object_node_manager->getObjectNodes().end(); ++it)
	{
		ObjectNode* object_node = *it;
		LibGens::Object* object = object_node->getObject();

		if (object_node->isForceHidden() || !set_visibility[object->getParentSet()])
			continue;

		if (matchesQuery(object->getName(), obj_name, exact))
		{
			if (param != "" && value != "")
				if (!valueMatches(object, param, value))
					continue;

			clearSelection();
			selectNode(object_node);

			find_position = it;
			if (it != object_node_manager->getObjectNodes().end())
				find_position++;

			found = true;
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

		if (object_node->isForceHidden() || !set_visibility[object->getParentSet()])
			continue;

		if (matchesQuery(object->getName(), obj_name, exact))
		{
			if (param != "" && value != "")
				if (!valueMatches(object, param, value))
					continue;

			object_node->setSelect(true);
			selected_nodes.push_back(object_node);
		}
	}

	updateSelection();
}

INT_PTR CALLBACK findCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		return true;

	case WM_CLOSE:
		DestroyWindow(hDlg);
		editor_application->closeFindGUI();
		return true;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDB_FIND:
		{
			char name[1024] = "";
			char property[1024] = "";
			char value[1024] = "";
			GetDlgItemText(hDlg, IDE_FIND_VALUE, name, 1024);

			bool filtered = IsDlgButtonChecked(hDlg, IDC_FIND_FILTERED);
			if (filtered)
			{
				GetDlgItemText(hDlg, IDE_FIND_PROPERTY_VALUE, property, 1024);
				GetDlgItemText(hDlg, IDE_FIND_VALUE_VALUE, value, 1024);
			}

			if (!IsDlgButtonChecked(hDlg, IDC_FIND_ALL))
				editor_application->findNext(name, property, value);
			else
				editor_application->findAll(name, property, value);

			break;
		}

		case IDC_FIND_FILTERED:
		{
			bool enabled = IsDlgButtonChecked(hDlg, IDC_FIND_FILTERED);
			HWND property_text = GetDlgItem(hDlg, IDE_FIND_PROPERTY_VALUE);
			HWND value_text = GetDlgItem(hDlg, IDE_FIND_VALUE_VALUE);

			EnableWindow(property_text, enabled);
			EnableWindow(value_text, enabled);
			break;
		}

		case IDCLOSE:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			break;
		}
		break;
	}

	return false;
}
