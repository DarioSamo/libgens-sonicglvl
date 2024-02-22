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
#include "EditorNodeHistory.h"
#include "ObjectNodeHistory.h"
#include "ObjectLibrary.h"
#include "ObjectSet.h"

INT_PTR CALLBACK EditBoolCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK EditIntCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK EditFloatCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK EditStringCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK EditVectorCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK EditIdCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK EditIdListCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK EditVectorListCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK EditControlCallback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
WNDPROC globalEditControlOldProc;

void EditorApplication::updateObjectsPropertiesGUI() {
	list<LibGens::Object *> selected_objects;
	list<LibGens::MultiSetNode*> selected_msp;
	string object_name = "";
	bool multiple_object_types = false;
	bool multiple_multiset_types = false;
	HWND hPropertiesList = GetDlgItem(hLeftDlg, IDL_PROPERTIES_LIST);

	if (current_single_property_object) {
		updateObjectsPropertiesValuesGUI(current_single_property_object);
	}

	// Retrieve Object pointers from Object Nodes
	for (list<EditorNode *>::iterator it=selected_nodes.begin(); it!=selected_nodes.end(); it++) {
		if ((*it)->getType() == EDITOR_NODE_OBJECT) {
			ObjectNode *object_node=static_cast<ObjectNode *>(*it);

			LibGens::Object *object = object_node->getObject();

			if (object) {
				selected_objects.push_back(object);

				// Check if there's multiple object names in the current selection
				if (!multiple_object_types) {
					if (object_name.size()) {
						if (object_name != object->getName()) {
							multiple_object_types = true;
						}
					}
					else {
						object_name = object->getName();
					}
				}
			}
		}
		else if ((*it)->getType() == EDITOR_NODE_OBJECT_MSP)
		{
			ObjectMultiSetNode* object_msp_node = static_cast<ObjectMultiSetNode*>(*it);
			LibGens::MultiSetNode * msp_node = object_msp_node->getMultiSetNode();

			LibGens::Object* object = object_msp_node->getObject();
			if (object)
			{
				selected_msp.push_back(msp_node);
				if (object_name.size())
				{
					if (object_name != object->getName())
					{
						multiple_object_types = true;
						multiple_multiset_types = true;
					}
				}
				else
				{
					object_name = "MultiSetParamObject (" + object->getName() + ")";
				}
			}
		}
	}


	// Compare the current selected objects list with the last one
	bool same_list = true;

	if (selected_objects.size() != current_object_list_properties.size()) {
		same_list = false;
	}
	else {
		for (list<LibGens::Object *>::iterator it=selected_objects.begin(); it!=selected_objects.end(); it++) {
			bool found=false;

			for (list<LibGens::Object *>::iterator it_o=current_object_list_properties.begin(); it_o!=current_object_list_properties.end(); it_o++) {
				if ((*it) == (*it_o)) {
					found = true;
					break;
				}
			}

			if (!found) {
				same_list = false;
				break;
			}
		}
	}

	if (same_list) {
		return;
	}

	current_object_list_properties = selected_objects;

	// Update text
	updateHelpWithObjectGUI(NULL);

	string group_text = "";
	if (multiple_object_types || multiple_multiset_types) {
		group_text = "Multiple Objects";
	}
	else if (object_name.size()) {
		group_text = object_name;

		if (selected_objects.size() == 1) {
			LibGens::Object *first_object = (*selected_objects.begin());
			if (first_object) {
				group_text += " (ID:" + ToString(first_object->getID()) + ", " + first_object->getParentSet()->getName() + ")";
			}
		}

		LibGens::Object *template_object=library->getTemplate(object_name);
		updateHelpWithObjectGUI(template_object);
	}
	else {
		group_text = "(No selection)";
	}

	SetDlgItemText(hLeftDlg, IDG_PROPERTIES_GROUP, group_text.c_str());

	// Scan for Common Properties in the list of selected objects
	current_properties_names.clear();
	current_properties_types.clear();
	vector<string> temp_properties_names;
	vector<LibGens::ObjectElementType> temp_properties_types; 

	// Add the properties of the first object for cross-comparisons
	if (selected_objects.size()) {
		
		LibGens::Object *first_object = (*selected_objects.begin());

		list<LibGens::ObjectElement *> elements = first_object->getElements();
		for (list<LibGens::ObjectElement *>::iterator it_e=elements.begin(); it_e!=elements.end(); it_e++) {
			string element_name = (*it_e)->getName();
			LibGens::ObjectElementType element_type = (*it_e)->getType();

			temp_properties_names.push_back(element_name);
			temp_properties_types.push_back(element_type);
		}
	}

	// If the property is not found in an object in the selection, it won't get added to the new list
	for (size_t i=0; i<temp_properties_names.size(); i++) {
		bool in_objects = true;

		for (list<LibGens::Object *>::iterator it=selected_objects.begin(); it!=selected_objects.end(); it++) {
			list<LibGens::ObjectElement *> elements = (*it)->getElements();

			bool found = false;
			for (list<LibGens::ObjectElement *>::iterator it_e=elements.begin(); it_e!=elements.end(); it_e++) {
				string element_name = (*it_e)->getName();
				LibGens::ObjectElementType element_type = (*it_e)->getType();

				if ((element_name == temp_properties_names[i]) && (element_type == temp_properties_types[i])) {
					found = true;
					break;
				}
			}

			if (!found) {
				in_objects = false;
				break;
			}
		}

		if (in_objects) {
			current_properties_names.push_back(temp_properties_names[i]);
			current_properties_types.push_back(temp_properties_types[i]);
		}
	}


	// Cleanup previous list
	if (ListView_GetItemCount(hPropertiesList)!=0) {
		ListView_DeleteAllItems(hPropertiesList);
		ListView_SetItemCount(hPropertiesList, 0);
	}

	// Fill list with the common current properties
	char name_str[1024]="";
	char value_str[1024]="(shared)";

	for (size_t i=0; i<current_properties_names.size(); i++) {
		strcpy(name_str, current_properties_names[i].c_str());

		// Append property type string
		/*
		strcat(name_str, " (");

		switch (current_properties_types[i]) {
			case LibGens::OBJECT_ELEMENT_UNDEFINED :
				strcat(name_str, LIBGENS_OBJECT_ELEMENT_UNDEFINED_TEMPLATE);
				break;
			case LibGens::OBJECT_ELEMENT_BOOL :
				strcat(name_str, LIBGENS_OBJECT_ELEMENT_BOOL_TEMPLATE);
				break;
			case LibGens::OBJECT_ELEMENT_FLOAT :
				strcat(name_str, LIBGENS_OBJECT_ELEMENT_FLOAT_TEMPLATE);
				break;
			case LibGens::OBJECT_ELEMENT_STRING :
				strcat(name_str, LIBGENS_OBJECT_ELEMENT_STRING_TEMPLATE);
				break;
			case LibGens::OBJECT_ELEMENT_ID :
				strcat(name_str, LIBGENS_OBJECT_ELEMENT_ID_TEMPLATE);
				break;
			case LibGens::OBJECT_ELEMENT_ID_LIST :
				strcat(name_str, LIBGENS_OBJECT_ELEMENT_ID_LIST_TEMPLATE);
				break;
			case LibGens::OBJECT_ELEMENT_VECTOR :
				strcat(name_str, LIBGENS_OBJECT_ELEMENT_VECTOR_TEMPLATE);
				break;
			case LibGens::OBJECT_ELEMENT_VECTOR_LIST :
				strcat(name_str, LIBGENS_OBJECT_ELEMENT_VECTOR_LIST_TEMPLATE);
				break;
		};

		strcat(name_str, ")");
		*/

		LV_ITEM Item;                                 
		Item.mask = LVIF_TEXT;
		Item.pszText = name_str; 
		Item.state = 0;
		Item.cchTextMax = strlen(name_str);            
		Item.iSubItem = 0;                           
		Item.lParam = (LPARAM) NULL;                   
		Item.iItem = i; 
		ListView_InsertItem(hPropertiesList, &Item);
		ListView_SetItemText(hPropertiesList, i, 1, value_str);
	}

	EnableWindow(hPropertiesList, (current_properties_names.size() ? true : false));

	closeEditPropertyGUI();
	current_single_property_object = NULL;
	current_property_index = -1;

	// If only one object was selected, update the Values column
	if (selected_objects.size() == 1) {
		LibGens::Object *first_object = (*selected_objects.begin());
		current_single_property_object = first_object;
		updateObjectsPropertiesValuesGUI(first_object);
	}
}


void EditorApplication::updateObjectsPropertiesValuesGUI(LibGens::Object *object) {
	if (!object) {
		return;
	}

	HWND hPropertiesList = GetDlgItem(hLeftDlg, IDL_PROPERTIES_LIST);
	
	for (size_t i=0; i<current_properties_names.size(); i++) {
		string element_name = current_properties_names[i];
		LibGens::ObjectElement *element = object->getElement(element_name);

		if (element) {
			LibGens::ObjectElementType element_type = element->getType();
			string value="";

			LibGens::ObjectElementBool *element_cast_bool;
			LibGens::ObjectElementInteger *element_cast_int;
			LibGens::ObjectElementFloat *element_cast_float;
			LibGens::ObjectElementString *element_cast_string;
			LibGens::ObjectElementID *element_cast_id;
			LibGens::ObjectElementIDList *element_cast_id_list;
			LibGens::ObjectElementVector *element_cast_vector;
			LibGens::ObjectElementVectorList *element_cast_vector_list;
			
			switch (current_properties_types[i]) {
				case LibGens::OBJECT_ELEMENT_BOOL :
					element_cast_bool=static_cast<LibGens::ObjectElementBool *>(element);
					value = (element_cast_bool->value ? "true" : "false");
					break;
				case LibGens::OBJECT_ELEMENT_INTEGER :
					element_cast_int=static_cast<LibGens::ObjectElementInteger *>(element);
					value = ToString(element_cast_int->value);
					break;
				case LibGens::OBJECT_ELEMENT_FLOAT :
					element_cast_float=static_cast<LibGens::ObjectElementFloat *>(element);
					value = ToString(element_cast_float->value);
					break;
				case LibGens::OBJECT_ELEMENT_STRING :
					element_cast_string=static_cast<LibGens::ObjectElementString *>(element);
					value = element_cast_string->value;
					break;
				case LibGens::OBJECT_ELEMENT_ID :
					element_cast_id=static_cast<LibGens::ObjectElementID *>(element);

					if (current_level) {
						if (current_level->getLevel()) {
							LibGens::Object *target_object = current_level->getLevel()->getObjectByID(element_cast_id->value);

							if (target_object) {
								value = target_object->getName();
							}
						}
					}

					value += "(ID: " + ToString(element_cast_id->value) + ")";
					break;
				case LibGens::OBJECT_ELEMENT_ID_LIST :
					element_cast_id_list=static_cast<LibGens::ObjectElementIDList *>(element);
					value = "ID Count: " + ToString(element_cast_id_list->value.size());
					break;
				case LibGens::OBJECT_ELEMENT_VECTOR :
					element_cast_vector=static_cast<LibGens::ObjectElementVector *>(element);
					value = "(" + ToString(element_cast_vector->value.x) + ", " + ToString(element_cast_vector->value.y) + ", " + ToString(element_cast_vector->value.z) + ")";
					break;
				case LibGens::OBJECT_ELEMENT_VECTOR_LIST :
					element_cast_vector_list=static_cast<LibGens::ObjectElementVectorList *>(element);
					value = "Vector Count: " + ToString(element_cast_vector_list->value.size());
					break;
			};

			ListView_SetItemText(hPropertiesList, i, 1, (char *) value.c_str());
		}
	}
}


void EditorApplication::createObjectsPropertiesGUI() {
	HWND hPropertiesList = GetDlgItem(hLeftDlg, IDL_PROPERTIES_LIST);

	LVCOLUMN Col;                                   
	Col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	Col.cx = 142;
	Col.pszText = "Name";
	Col.cchTextMax = strlen(Col.pszText);
	ListView_InsertColumn(hPropertiesList, 0, &Col);


	Col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	Col.cx = 115;
	Col.pszText = "Value";
	Col.cchTextMax = strlen(Col.pszText);
	ListView_InsertColumn(hPropertiesList, 1, &Col);

	ListView_SetExtendedListViewStyleEx(hPropertiesList, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
	EnableWindow(hPropertiesList, false);
}


void EditorApplication::updateObjectPropertyIndex(int selection_index) {
	if (selection_index < 0) {
		updateHelpWithPropertyGUI(NULL);
		return;
	}

	if ((size_t)selection_index < current_properties_names.size()) {
		if (current_object_list_properties.size()) {
			LibGens::Object *first_object = (*current_object_list_properties.begin());

			if (first_object) {
				LibGens::ObjectElement *element = first_object->getElement(current_properties_names[selection_index]);
				updateHelpWithPropertyGUI(element);
			}
		}
	}
}


void EditorApplication::editObjectPropertyIndex(int selection_index) {
	if (selection_index < 0) {
		return;
	}

	if ((size_t)selection_index < current_properties_names.size()) {
		if (selection_index != current_property_index) {
			closeEditPropertyGUI();
		}

		current_property_index = selection_index;

		if (!hEditPropertyDlg) {
			history_edit_property_wrapper = new HistoryActionWrapper();

			if (current_properties_types[current_property_index] == LibGens::OBJECT_ELEMENT_BOOL) {
				// Create Dialog for Bool
				hEditPropertyDlg = CreateDialog(NULL, MAKEINTRESOURCE(IDD_EDIT_BOOL_DIALOG), hwnd, EditBoolCallback);

				SendDlgItemMessage(hEditPropertyDlg, IDC_EDIT_BOOL_VALUE, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
				SendDlgItemMessage(hEditPropertyDlg, IDC_EDIT_BOOL_VALUE, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)"false");
				SendDlgItemMessage(hEditPropertyDlg, IDC_EDIT_BOOL_VALUE, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)"true");

				COMBOBOXINFO hComboBoxInfo;
				hComboBoxInfo.cbSize = sizeof(COMBOBOXINFO);

				HWND hEditMainControl = GetDlgItem(hEditPropertyDlg, IDC_EDIT_BOOL_VALUE);
				GetComboBoxInfo(hEditMainControl, &hComboBoxInfo);
				SetFocus(hEditMainControl);
				globalEditControlOldProc = (WNDPROC) SetWindowLong(hComboBoxInfo.hwndList, GWL_WNDPROC, (LONG) EditControlCallback);

				// Set Default
				if (current_single_property_object) {
					string element_name = current_properties_names[current_property_index];
					LibGens::ObjectElement *element = current_single_property_object->getElement(element_name);

					if (element) {
						LibGens::ObjectElementBool *element_bool = static_cast<LibGens::ObjectElementBool *>(element);
						bool default_value = element_bool->value;
						SendDlgItemMessage(hEditPropertyDlg, IDC_EDIT_BOOL_VALUE, CB_SETCURSEL, (WPARAM) (default_value ? 1 : 0), (LPARAM) 0);
					}
				}
			}

			
			if (current_properties_types[current_property_index] == LibGens::OBJECT_ELEMENT_ID) {
				// Create Dialog for ID
				hEditPropertyDlg = CreateDialog(NULL, MAKEINTRESOURCE(IDD_EDIT_ID_DIALOG), hwnd, EditIdCallback);

				COMBOBOXINFO hComboBoxInfo;
				hComboBoxInfo.cbSize = sizeof(COMBOBOXINFO);

				HWND hEditMainControl = GetDlgItem(hEditPropertyDlg, IDC_EDIT_ID_VALUE);
				GetComboBoxInfo(hEditMainControl, &hComboBoxInfo);
				SetFocus(hEditMainControl);
				globalEditControlOldProc = (WNDPROC) SetWindowLong(hComboBoxInfo.hwndItem, GWL_WNDPROC, (LONG) EditControlCallback);

				// Set Default
				if (current_single_property_object) {
					string element_name = current_properties_names[current_property_index];
					LibGens::ObjectElement *element = current_single_property_object->getElement(element_name);

					if (element) {
						LibGens::ObjectElementID *element_id = static_cast<LibGens::ObjectElementID *>(element);
						unsigned int default_value = element_id->value;

						// get the target object's name
						editor_application->setTargetName(default_value);

						SetDlgItemText(hEditPropertyDlg, IDC_EDIT_ID_VALUE, ToString(default_value).c_str());
						SendDlgItemMessage(hEditPropertyDlg, IDC_EDIT_ID_VALUE, (UINT)CB_SETEDITSEL, (WPARAM)0, MAKELPARAM(0, -1));
					}
				}
			}

			if (current_properties_types[current_property_index] == LibGens::OBJECT_ELEMENT_ID_LIST) {
				// Create dialog for ID list
				hEditPropertyDlg = CreateDialog(NULL, MAKEINTRESOURCE(IDD_EDIT_ID_LIST_DIALOG), hwnd, EditIdListCallback);
				HWND hIDList = GetDlgItem(hEditPropertyDlg, IDL_EDIT_ID_LIST_LIST);

				LVCOLUMN Col;
				Col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
				Col.cx = 285;
				Col.pszText = "Object IDs";
				Col.cchTextMax = strlen(Col.pszText);
				ListView_InsertColumn(hIDList, 0, &Col);

				if (current_single_property_object) {
					string element_name = current_properties_names[current_property_index];
					LibGens::ObjectElement* element = current_single_property_object->getElement(element_name);

					if (element) {
						LibGens::ObjectElementIDList* element_id_list = static_cast<LibGens::ObjectElementIDList*>(element);
						
						for (vector<size_t>::iterator it = element_id_list->value.begin(); it != element_id_list->value.end(); ++it) {
							addIDToList(*it);
						}

						updateEditPropertyIDList(temp_property_id_list);
					}
				}
			}


			if (current_properties_types[current_property_index] == LibGens::OBJECT_ELEMENT_INTEGER) {
				// Create Dialog for Float
				hEditPropertyDlg = CreateDialog(NULL, MAKEINTRESOURCE(IDD_EDIT_FLOAT_DIALOG), hwnd, EditIntCallback);

				COMBOBOXINFO hComboBoxInfo;
				hComboBoxInfo.cbSize = sizeof(COMBOBOXINFO);

				HWND hEditMainControl = GetDlgItem(hEditPropertyDlg, IDC_EDIT_FLOAT_VALUE);
				GetComboBoxInfo(hEditMainControl, &hComboBoxInfo);
				SetFocus(hEditMainControl);
				globalEditControlOldProc = (WNDPROC) SetWindowLong(hComboBoxInfo.hwndItem, GWL_WNDPROC, (LONG) EditControlCallback);

				// Set Default
				if (current_single_property_object) {
					string element_name = current_properties_names[current_property_index];
					LibGens::ObjectElement *element = current_single_property_object->getElement(element_name);

					if (element) {
						LibGens::ObjectElementInteger *element_integer = static_cast<LibGens::ObjectElementInteger *>(element);
						unsigned int default_value = element_integer->value;
						SetDlgItemText(hEditPropertyDlg, IDC_EDIT_FLOAT_VALUE, ToString(default_value).c_str());
						SendDlgItemMessage(hEditPropertyDlg, IDC_EDIT_FLOAT_VALUE, (UINT)CB_SETEDITSEL, (WPARAM)0, MAKELPARAM(0, -1));
					}
				}
			}

			
			if (current_properties_types[current_property_index] == LibGens::OBJECT_ELEMENT_FLOAT) {
				// Create Dialog for Float
				hEditPropertyDlg = CreateDialog(NULL, MAKEINTRESOURCE(IDD_EDIT_FLOAT_DIALOG), hwnd, EditFloatCallback);

				COMBOBOXINFO hComboBoxInfo;
				hComboBoxInfo.cbSize = sizeof(COMBOBOXINFO);

				HWND hEditMainControl = GetDlgItem(hEditPropertyDlg, IDC_EDIT_FLOAT_VALUE);
				GetComboBoxInfo(hEditMainControl, &hComboBoxInfo);
				SetFocus(hEditMainControl);
				globalEditControlOldProc = (WNDPROC) SetWindowLong(hComboBoxInfo.hwndItem, GWL_WNDPROC, (LONG) EditControlCallback);

				// Set Default
				if (current_single_property_object) {
					string element_name = current_properties_names[current_property_index];
					LibGens::ObjectElement *element = current_single_property_object->getElement(element_name);

					if (element) {
						LibGens::ObjectElementFloat *element_float = static_cast<LibGens::ObjectElementFloat *>(element);
						float default_value = element_float->value;
						SetDlgItemText(hEditPropertyDlg, IDC_EDIT_FLOAT_VALUE, ToString(default_value).c_str());
						SendDlgItemMessage(hEditPropertyDlg, IDC_EDIT_FLOAT_VALUE, (UINT)CB_SETEDITSEL, (WPARAM)0, MAKELPARAM(0, -1));
					}
				}
			}


			if (current_properties_types[current_property_index] == LibGens::OBJECT_ELEMENT_STRING) {
				// Create Dialog for String
				hEditPropertyDlg = CreateDialog(NULL, MAKEINTRESOURCE(IDD_EDIT_STRING_DIALOG), hwnd, EditStringCallback);

				COMBOBOXINFO hComboBoxInfo;
				hComboBoxInfo.cbSize = sizeof(COMBOBOXINFO);

				HWND hEditMainControl = GetDlgItem(hEditPropertyDlg, IDC_EDIT_STRING_VALUE);
				GetComboBoxInfo(hEditMainControl, &hComboBoxInfo);
				SetFocus(hEditMainControl);
				globalEditControlOldProc = (WNDPROC) SetWindowLong(hComboBoxInfo.hwndItem, GWL_WNDPROC, (LONG) EditControlCallback);

				// Set Default
				if (current_single_property_object) {
					string element_name = current_properties_names[current_property_index];
					LibGens::ObjectElement *element = current_single_property_object->getElement(element_name);

					if (element) {
						LibGens::ObjectElementString *element_string = static_cast<LibGens::ObjectElementString *>(element);
						string default_value = element_string->value;
						SetDlgItemText(hEditPropertyDlg, IDC_EDIT_STRING_VALUE, default_value.c_str());
						SendDlgItemMessage(hEditPropertyDlg, IDC_EDIT_STRING_VALUE, (UINT)CB_SETEDITSEL, (WPARAM)0, MAKELPARAM(0, -1));
					}
				}

				// If ObjectPhysics & Type, pre-load all ObjectProduction entries into the ComboBox
				if (current_object_list_properties.size() && object_production) {
					LibGens::Object *first_object = (*current_object_list_properties.begin());
					
					if (first_object) {
						string object_name = first_object->getName();
						string element_name = current_properties_names[current_property_index];

						if ((object_name == OBJECT_NODE_OBJECT_PHYSICS) && (element_name == OBJECT_NODE_OBJECT_PHYSICS_ELEMENT_TYPE)) {
							object_production->readySortedEntries();

							string entry_name="";
							while (object_production->getNextEntryName(entry_name)) {
								SendDlgItemMessage(hEditPropertyDlg, IDC_EDIT_STRING_VALUE, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)entry_name.c_str());
							}
						}
					}
				}
			}

			
			if (current_properties_types[current_property_index] == LibGens::OBJECT_ELEMENT_VECTOR) {
				// Create Dialog for Vector
				hEditPropertyDlg = CreateDialog(NULL, MAKEINTRESOURCE(IDD_EDIT_VECTOR_DIALOG), hwnd, EditVectorCallback);

				HWND hEditMainControl = GetDlgItem(hEditPropertyDlg, IDE_EDIT_VECTOR_X);
				SetFocus(hEditMainControl);
				
				ObjectNode* first_selection = static_cast<ObjectNode*>(*(selected_nodes.begin()));
				LibGens::Vector3 dv = first_selection->getObject()->getPosition();

				// Set Default
				if (current_single_property_object) {
					string element_name = current_properties_names[current_property_index];
					LibGens::ObjectElement *element = current_single_property_object->getElement(element_name);

					if (element) {
						LibGens::ObjectElementVector *element_vector = static_cast<LibGens::ObjectElementVector *>(element);
						dv = element_vector->value;

						SetDlgItemText(hEditPropertyDlg, IDE_EDIT_VECTOR_X, ToString(dv.x).c_str());
						SetDlgItemText(hEditPropertyDlg, IDE_EDIT_VECTOR_Y, ToString(dv.y).c_str());
						SetDlgItemText(hEditPropertyDlg, IDE_EDIT_VECTOR_Z, ToString(dv.z).c_str());
					}
				}

				VectorNode *vector_node = new VectorNode(scene_manager);
				vector_node->setPosition(Ogre::Vector3(dv.x, dv.y, dv.z));
				property_vector_nodes.push_back(vector_node);
			}

			if (current_properties_types[current_property_index] == LibGens::OBJECT_ELEMENT_VECTOR_LIST) {
				// Create Dialog for Vector List
				hEditPropertyDlg = CreateDialog(NULL, MAKEINTRESOURCE(IDD_EDIT_VECTOR_LIST_DIALOG), hwnd, EditVectorListCallback);
				HWND hVectorList = GetDlgItem(hEditPropertyDlg, IDL_EDIT_VECTOR_LIST_LIST);

				LVCOLUMN Col;
				Col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
				Col.cx = 285;
				Col.pszText = "Vector (x, y, z)";
				Col.cchTextMax = strlen(Col.pszText);
				ListView_InsertColumn(hVectorList, 0, &Col);

				if (current_single_property_object) {
					string element_name = current_properties_names[current_property_index];
					LibGens::ObjectElement* element = current_single_property_object->getElement(element_name);

					if (element) {
						LibGens::ObjectElementVectorList* element_vector_list = static_cast<LibGens::ObjectElementVectorList*>(element);

						for (vector<LibGens::Vector3>::iterator it = element_vector_list->value.begin(); it != element_vector_list->value.end(); ++it)
							addVectorToList(*it);

						updateEditPropertyVectorList(temp_property_vector_list);
					}
				}
			}
		}
	}
}


void EditorApplication::updateEditPropertyBool(bool v) {
	string element_name = current_properties_names[current_property_index];

	for (list<LibGens::Object *>::iterator it=current_object_list_properties.begin(); it!=current_object_list_properties.end(); it++) {
		LibGens::ObjectElement *element = (*it)->getElement(element_name);

		if (element) {
			if (element->getType() == LibGens::OBJECT_ELEMENT_BOOL) {
				LibGens::ObjectElementBool *element_bool = static_cast<LibGens::ObjectElementBool *>(element);
				HistoryActionEditObjectElementBool *history_action = new HistoryActionEditObjectElementBool((*it), object_node_manager, element_bool, element_bool->value, v);
				element_bool->value = v;
				history_edit_property_wrapper->push(history_action);

				object_node_manager->reloadObjectNode((*it));
			}
		}
	}

	if (current_single_property_object) {
		updateObjectsPropertiesValuesGUI(current_single_property_object);
	}
}



void EditorApplication::updateEditPropertyInteger(unsigned int v) {
	string element_name = current_properties_names[current_property_index];

	for (list<LibGens::Object *>::iterator it=current_object_list_properties.begin(); it!=current_object_list_properties.end(); it++) {
		LibGens::ObjectElement *element = (*it)->getElement(element_name);

		if (element) {
			if (element->getType() == LibGens::OBJECT_ELEMENT_ID) {
				LibGens::ObjectElementID *element_id = static_cast<LibGens::ObjectElementID *>(element);
				HistoryActionEditObjectElementID *history_action = new HistoryActionEditObjectElementID((*it), object_node_manager, element_id, element_id->value, v);
				element_id->value = v;
				history_edit_property_wrapper->push(history_action);

				object_node_manager->reloadObjectNode((*it));
			}

			if (element->getType() == LibGens::OBJECT_ELEMENT_INTEGER) {
				LibGens::ObjectElementInteger *element_integer = static_cast<LibGens::ObjectElementInteger *>(element);
				HistoryActionEditObjectElementInteger *history_action = new HistoryActionEditObjectElementInteger((*it), object_node_manager, element_integer, element_integer->value, v);
				element_integer->value = v;
				history_edit_property_wrapper->push(history_action);

				object_node_manager->reloadObjectNode((*it));
			}
		}
	}

	if (current_single_property_object) {
		updateObjectsPropertiesValuesGUI(current_single_property_object);
	}
}


void EditorApplication::updateEditPropertyFloat(float v) {
	string element_name = current_properties_names[current_property_index];

	for (list<LibGens::Object *>::iterator it=current_object_list_properties.begin(); it!=current_object_list_properties.end(); it++) {
		LibGens::ObjectElement *element = (*it)->getElement(element_name);

		if (element) {
			if (element->getType() == LibGens::OBJECT_ELEMENT_FLOAT) {
				LibGens::ObjectElementFloat *element_float = static_cast<LibGens::ObjectElementFloat *>(element);
				HistoryActionEditObjectElementFloat *history_action = new HistoryActionEditObjectElementFloat((*it), object_node_manager, element_float, element_float->value, v);
				element_float->value = v;
				history_edit_property_wrapper->push(history_action);

				object_node_manager->reloadObjectNode((*it));
			}
		}
	}

	if (current_single_property_object) {
		updateObjectsPropertiesValuesGUI(current_single_property_object);
	}
}



void EditorApplication::updateEditPropertyString(string v) {
	string element_name = current_properties_names[current_property_index];

	for (list<LibGens::Object *>::iterator it=current_object_list_properties.begin(); it!=current_object_list_properties.end(); it++) {
		LibGens::ObjectElement *element = (*it)->getElement(element_name);

		if (element) {
			if (element->getType() == LibGens::OBJECT_ELEMENT_STRING) {
				LibGens::ObjectElementString *element_string = static_cast<LibGens::ObjectElementString *>(element);
				HistoryActionEditObjectElementString *history_action = new HistoryActionEditObjectElementString((*it), object_node_manager, element_string, element_string->value, v);
				element_string->value = v;
				history_edit_property_wrapper->push(history_action);

				object_node_manager->reloadObjectNode((*it));
			}
		}
	}

	if (current_single_property_object) {
		updateObjectsPropertiesValuesGUI(current_single_property_object);
	}
}

void EditorApplication::updateEditPropertyID(size_t v)
{
	string element_name = current_properties_names[current_property_index];

	for (list<LibGens::Object*>::iterator it = current_object_list_properties.begin(); it != current_object_list_properties.end(); ++it)
	{
		LibGens::ObjectElement* element = (*it)->getElement(element_name);

		if (element)
		{
			if (element->getType() == LibGens::OBJECT_ELEMENT_ID)
			{
				LibGens::ObjectElementID* element_id = static_cast<LibGens::ObjectElementID*>(element);
				HistoryActionEditObjectElementID* history_action = new HistoryActionEditObjectElementID((*it), object_node_manager, element_id, element_id->value, v);
				element_id->value = v;
				history_edit_property_wrapper->push(history_action);
			}
		}
	}

	if (current_single_property_object) {
		updateObjectsPropertiesValuesGUI(current_single_property_object);
	}
}

void EditorApplication::updateEditPropertyIDList(vector<size_t> v)
{
	string element_name = current_properties_names[current_property_index];

	for (list<LibGens::Object*>::iterator it = current_object_list_properties.begin(); it != current_object_list_properties.end(); ++it)
	{
		LibGens::ObjectElement* element = (*it)->getElement(element_name);

		if (element)
		{
			if (element->getType() == LibGens::OBJECT_ELEMENT_ID_LIST)
			{
				LibGens::ObjectElementIDList* element_id_list = static_cast<LibGens::ObjectElementIDList*>(element);
				HistoryActionEditObjectElementIDList* history_action = new HistoryActionEditObjectElementIDList((*it), object_node_manager, element_id_list, element_id_list->value, v);
				element_id_list->value = v;
				history_edit_property_wrapper->push(history_action);
			}
		}
	}

	if (current_single_property_object) {
		updateObjectsPropertiesValuesGUI(current_single_property_object);
	}
}


void EditorApplication::updateEditPropertyVector(LibGens::Vector3 v) {
	string element_name = current_properties_names[current_property_index];

	for (list<LibGens::Object *>::iterator it=current_object_list_properties.begin(); it!=current_object_list_properties.end(); it++) {
		LibGens::ObjectElement *element = (*it)->getElement(element_name);

		if (element) {
			if (element->getType() == LibGens::OBJECT_ELEMENT_VECTOR) {
				LibGens::ObjectElementVector *element_vector = static_cast<LibGens::ObjectElementVector *>(element);
				HistoryActionEditObjectElementVector *history_action = new HistoryActionEditObjectElementVector((*it), object_node_manager, element_vector, element_vector->value, v);
				element_vector->value = v;
				history_edit_property_wrapper->push(history_action);

				object_node_manager->reloadObjectNode((*it));
			}
		}
	}

	if (current_single_property_object) {
		updateObjectsPropertiesValuesGUI(current_single_property_object);
	}

	if (property_vector_nodes.size()) {
		property_vector_nodes[0]->setPosition(Ogre::Vector3(v.x, v.y, v.z));
	}
}

void EditorApplication::updateEditPropertyVectorList(vector<LibGens::Vector3> v)
{
	string element_name = current_properties_names[current_property_index];

	for (list<LibGens::Object*>::iterator it = current_object_list_properties.begin(); it != current_object_list_properties.end(); ++it)
	{
		LibGens::ObjectElement* element = (*it)->getElement(element_name);
		if (element)
		{
			if (element->getType() == LibGens::OBJECT_ELEMENT_VECTOR_LIST)
			{
				LibGens::ObjectElementVectorList* element_vector_list = static_cast<LibGens::ObjectElementVectorList*>(element);
				HistoryActionEditObjectElementVectorList* history_action = new HistoryActionEditObjectElementVectorList((*it), object_node_manager, element_vector_list, element_vector_list->value, v);
				element_vector_list->value = v;
				history_edit_property_wrapper->push(history_action);

				object_node_manager->reloadObjectNode((*it));
			}
		}
	}

	if (current_single_property_object) {
		updateObjectsPropertiesValuesGUI(current_single_property_object);
	}

	if (property_vector_nodes.size()) {
		for (int i = 0; i < property_vector_nodes.size(); ++i)
		{
			LibGens::Vector3 v3 = v[i];
			property_vector_nodes[i]->setPosition(Ogre::Vector3(v3.x, v3.y, v3.z));
		}
	}
}


void EditorApplication::updateEditPropertyVectorFocus(int index) {
	if (property_vector_nodes.size()) {
		viewport->focusOnPoint(property_vector_nodes[index]->getPosition());
	}
}


void EditorApplication::updateEditPropertyVectorGUI(int index, bool is_list) {
	if (property_vector_nodes.size()) {

		Ogre::Vector3 v=property_vector_nodes[index]->getPosition();

		if (hEditPropertyDlg) {

			if (!is_list)
			{
				SetDlgItemText(hEditPropertyDlg, IDE_EDIT_VECTOR_X, ToString((float)v.x).c_str());
				SetDlgItemText(hEditPropertyDlg, IDE_EDIT_VECTOR_Y, ToString((float)v.y).c_str());
				SetDlgItemText(hEditPropertyDlg, IDE_EDIT_VECTOR_Z, ToString((float)v.z).c_str());
			}
		}

		if (!is_list)
			updateEditPropertyVector(LibGens::Vector3(v.x, v.y, v.z));
		else
		{
			HWND list_view = GetDlgItem(hEditPropertyDlg, IDL_EDIT_VECTOR_LIST_LIST);
			temp_property_vector_list[index] = LibGens::Vector3(v.x, v.y, v.z);

			if (hEditPropertyDlg)
			{
				string vector_string = ToString<float>(v.x) + ", " + ToString<float>(v.y) + ", " + ToString<float>(v.z);
				char buffer[128];
				strcpy(buffer, vector_string.c_str());

				ListView_SetItemText(list_view, index, 0, buffer);
			}
		}
	}
}


void EditorApplication::updateEditPropertyVectorMode(bool mode_state, bool is_list, int index) {
	SendDlgItemMessage(hEditPropertyDlg, IDC_EDIT_VECTOR_EDITING, BM_SETCHECK, (WPARAM)mode_state, 0);
	SendDlgItemMessage(hEditPropertyDlg, IDC_EDIT_VECTOR_LIST_EDITING, BM_SETCHECK, (WPARAM)mode_state, 0);
	if (!is_list)
	{
		EnableWindow(GetDlgItem(hEditPropertyDlg, IDE_EDIT_VECTOR_X), !mode_state);
		EnableWindow(GetDlgItem(hEditPropertyDlg, IDE_EDIT_VECTOR_Y), !mode_state);
		EnableWindow(GetDlgItem(hEditPropertyDlg, IDE_EDIT_VECTOR_Z), !mode_state);

		EnableWindow(GetDlgItem(hEditPropertyDlg, IDS_EDIT_VECTOR_X), !mode_state);
		EnableWindow(GetDlgItem(hEditPropertyDlg, IDS_EDIT_VECTOR_Y), !mode_state);
		EnableWindow(GetDlgItem(hEditPropertyDlg, IDS_EDIT_VECTOR_Z), !mode_state);
	}
	else
	{
		EnableWindow(GetDlgItem(hEditPropertyDlg, IDE_EDIT_VECTOR_LIST_X), !mode_state);
		EnableWindow(GetDlgItem(hEditPropertyDlg, IDE_EDIT_VECTOR_LIST_Y), !mode_state);
		EnableWindow(GetDlgItem(hEditPropertyDlg, IDE_EDIT_VECTOR_LIST_Z), !mode_state);

		EnableWindow(GetDlgItem(hEditPropertyDlg, IDS_EDIT_VECTOR_LIST_X), !mode_state);
		EnableWindow(GetDlgItem(hEditPropertyDlg, IDS_EDIT_VECTOR_LIST_Y), !mode_state);
		EnableWindow(GetDlgItem(hEditPropertyDlg, IDS_EDIT_VECTOR_LIST_Z), !mode_state);
	}

	if (mode_state) {
		previous_selected_nodes = selected_nodes;
		for (list<EditorNode *>::iterator it=selected_nodes.begin(); it!=selected_nodes.end(); it++) {
			(*it)->setSelect(false);
		}
		selected_nodes.clear();

		setEditorMode(EDITOR_NODE_QUERY_VECTOR);
		SetFocus(hwnd);

		property_vector_history->clear();

		selected_nodes.push_back(property_vector_nodes[index]);
		property_vector_nodes[index]->setSelect(true);

		updateSelection();
		updateEditPropertyVectorFocus(index);

		// Move window to the bottom right corner of the main window
		RECT main_window_rect;
		GetWindowRect(hwnd, &main_window_rect);
		GetWindowRect(hEditPropertyDlg, &hEditPropertyDlg_old_rect);

		LONG dlg_w=hEditPropertyDlg_old_rect.right - hEditPropertyDlg_old_rect.left;
		LONG dlg_h=hEditPropertyDlg_old_rect.bottom - hEditPropertyDlg_old_rect.top;
		MoveWindow(hEditPropertyDlg, main_window_rect.right - dlg_w - 15, main_window_rect.bottom - dlg_h - SONICGLVL_GUI_BOTTOM_HEIGHT - 15, dlg_w, dlg_h, true);
	}
	else {
		for (list<EditorNode *>::iterator it=selected_nodes.begin(); it!=selected_nodes.end(); it++) {
			(*it)->setSelect(false);
		}

		selected_nodes = previous_selected_nodes;
		for (list<EditorNode *>::iterator it=selected_nodes.begin(); it!=selected_nodes.end(); it++) {
			(*it)->setSelect(true);
		}

		setEditorMode(EDITOR_NODE_QUERY_OBJECT);
		SetFocus(hEditPropertyDlg);

		property_vector_history->clear();

		updateSelection();

		// Restore Window to old position
		LONG dlg_w=hEditPropertyDlg_old_rect.right - hEditPropertyDlg_old_rect.left;
		LONG dlg_h=hEditPropertyDlg_old_rect.bottom - hEditPropertyDlg_old_rect.top;
		MoveWindow(hEditPropertyDlg, hEditPropertyDlg_old_rect.left, hEditPropertyDlg_old_rect.top, dlg_w, dlg_h, true);
	}
}


void EditorApplication::closeVectorQueryMode() {
	if (editor_mode == EDITOR_NODE_QUERY_VECTOR) {
		updateEditPropertyVectorMode(false);
	}

	editor_mode = EDITOR_NODE_QUERY_OBJECT;
	global_cursor_state = 0;
}

void EditorApplication::closeTargetQueryMode() {
	editor_mode = EDITOR_NODE_QUERY_OBJECT;
	global_cursor_state = 0;
}

void EditorApplication::openQueryTargetMode(bool mode)
{
	if (mode)
	{
		is_pick_target = true;
		SetFocus(hwnd);

		RECT main_window_rect;
		GetWindowRect(hwnd, &main_window_rect);
		GetWindowRect(hEditPropertyDlg, &hEditPropertyDlg_old_rect);

		LONG dlg_w = hEditPropertyDlg_old_rect.right - hEditPropertyDlg_old_rect.left;
		LONG dlg_h = hEditPropertyDlg_old_rect.bottom - hEditPropertyDlg_old_rect.top;
		MoveWindow(hEditPropertyDlg, main_window_rect.right - dlg_w - 15, main_window_rect.bottom - dlg_h - SONICGLVL_GUI_BOTTOM_HEIGHT - 15, dlg_w, dlg_h, true);
	}
	else
	{
		is_pick_target = false;
		LONG dlg_w = hEditPropertyDlg_old_rect.right - hEditPropertyDlg_old_rect.left;
		LONG dlg_h = hEditPropertyDlg_old_rect.bottom - hEditPropertyDlg_old_rect.top;
		MoveWindow(hEditPropertyDlg, hEditPropertyDlg_old_rect.left, hEditPropertyDlg_old_rect.top, dlg_w, dlg_h, true);
	}
}

void EditorApplication::verifySonicSpawnChange() {
	// Verify that property is valid and only a single object is selected
	if ((current_property_index < current_properties_names.size()) && current_single_property_object) {
		string element_name = current_properties_names[current_property_index];

		// Verify if SonicSpawn is the type of selected object, and if the "Active" flag was the property edited
		if ((current_single_property_object->getName() == LIBGENS_SPAWN_POINT_OBJECT_NAME) && (element_name == LIBGENS_SPAWN_POINT_OBJECT_FLAG)) {
			LibGens::ObjectElement *element_single = current_single_property_object->getElement(LIBGENS_SPAWN_POINT_OBJECT_FLAG);

			if (element_single) {
				LibGens::ObjectElementBool *element_single_bool = static_cast<LibGens::ObjectElementBool *>(element_single);

				// Only change other elements if the flag was set to "true"
				if (element_single_bool->value) {
					// Verify if level exists
					if (current_level && current_level->getLevel()) {
						// Retrieve list of all SonicSpawn objects in the level
						list<LibGens::Object *> spawn_objects;
						current_level->getLevel()->getObjectsByName(LIBGENS_SPAWN_POINT_OBJECT_NAME, spawn_objects);

						for (list<LibGens::Object *>::iterator it=spawn_objects.begin(); it!=spawn_objects.end(); it++) {
							// Skip currently selected object
							if ((*it) == current_single_property_object) continue;

							LibGens::ObjectElement *element = (*it)->getElement(LIBGENS_SPAWN_POINT_OBJECT_FLAG);

							// Verify if it has an "Active" element
							if (element) {
								// Edit Boolean value and add to history wrapper
								LibGens::ObjectElementBool *element_bool = static_cast<LibGens::ObjectElementBool *>(element);

								HistoryActionEditObjectElementBool *history_action = new HistoryActionEditObjectElementBool((*it), object_node_manager, element_bool, element_bool->value, false);
								element_bool->value = false;

								if (history_edit_property_wrapper) {
									history_edit_property_wrapper->push(history_action);
								}

								object_node_manager->reloadObjectNode((*it));
							}
						}
					}
				}
			}
		}
	}
}

void EditorApplication::confirmEditProperty() {
	// Check if a Sonic Spawn was changed to "true", and disable the flag in any other spawns
	verifySonicSpawnChange();

	if (history_edit_property_wrapper) {
		pushHistory(history_edit_property_wrapper);
		history_edit_property_wrapper = NULL;
	}
}

void EditorApplication::revertEditProperty() {
	if (history_edit_property_wrapper) {
		history_edit_property_wrapper->undo();
		delete history_edit_property_wrapper;

		if (current_single_property_object) {
			updateObjectsPropertiesValuesGUI(current_single_property_object);
		}

		history_edit_property_wrapper = NULL;
	}
}


void EditorApplication::updateHelpWithPropertyGUI(LibGens::ObjectElement *element) {
	string help_name="";
	string help_description="";

	if (element) {
		help_name = element->getName();
		help_description = element->getDescription();
	}

	SetDlgItemText(hLeftDlg, IDG_HELP_GROUP, help_name.c_str());
	SetDlgItemText(hLeftDlg, IDT_HELP_DESCRIPTION, help_description.c_str());
}

void EditorApplication::clearEditPropertyGUI() {
	hEditPropertyDlg = NULL;

	for (vector<VectorNode *>::iterator it=property_vector_nodes.begin(); it!=property_vector_nodes.end(); it++) {
		delete (*it);
	}
	property_vector_nodes.clear();
	temp_property_vector_list.clear();
	temp_property_id_list.clear();
}

void EditorApplication::closeEditPropertyGUI() {
	if (hEditPropertyDlg) {
		SendMessage(hEditPropertyDlg, WM_CLOSE, 0, 0);
	}

	if (editor_mode == EDITOR_NODE_QUERY_VECTOR) {
		editor_mode = EDITOR_NODE_QUERY_OBJECT;
	}

	if (editor_mode == EDITOR_NODE_QUERY_NODE) {
		editor_mode = EDITOR_NODE_QUERY_OBJECT;
	}

	hEditPropertyDlg = NULL;
	revertEditProperty();
}

HWND EditorApplication::getEditPropertyDlg() {
	return hEditPropertyDlg;
}

float GetDlgItemInteger(HWND hDlg, int idDlgItem) {
	char value_str[1024] = "";
	unsigned int value = 0.0f;

	GetDlgItemText(hDlg, idDlgItem, value_str, 1024);
	FromString<unsigned int>(value, ToString(value_str), std::dec);

	return value;
}

float GetDlgItemFloat(HWND hDlg, int idDlgItem) {
	char value_str[1024] = "";
	float value = 0.0f;

	GetDlgItemText(hDlg, idDlgItem, value_str, 1024);
	FromString<float>(value, ToString(value_str), std::dec);

	return value;
}


INT_PTR CALLBACK EditBoolCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
		case WM_INITDIALOG:
			return true;

		case WM_CLOSE:
			EndDialog(hDlg, false);
			editor_application->clearEditPropertyGUI();
			return true;

		case WM_COMMAND:
			if(HIWORD(wParam) == CBN_SELCHANGE) { 
				int item_index = SendMessage((HWND) lParam, (UINT) CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0);
				if (LOWORD(wParam) == IDC_EDIT_BOOL_VALUE) {
					editor_application->updateEditPropertyBool((item_index == 1));
					break;
				}
			}

			switch(LOWORD(wParam)) {
				case IDOK:
					SendMessage(hDlg, WM_CLOSE, 0, 0);
					editor_application->confirmEditProperty();
					return true;

				case IDCANCEL:
					SendMessage(hDlg, WM_CLOSE, 0, 0);
					editor_application->revertEditProperty();
					return true;
			}
			break;

		case WM_NOTIFY:
			return true;
	}
	
	return false;
}



INT_PTR CALLBACK EditIntCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
		case WM_INITDIALOG:
			return true;

		case WM_CLOSE:
			EndDialog(hDlg, false);
			editor_application->clearEditPropertyGUI();
			return true;

		case WM_COMMAND:
			if(HIWORD(wParam) == CBN_SELCHANGE) { 
				char value_str[1024] = "";
				unsigned int value = 0.0f;

				int nIndex=SendDlgItemMessage(hDlg, IDC_EDIT_FLOAT_VALUE, (UINT) CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0);
				SendDlgItemMessage(hDlg, IDC_EDIT_FLOAT_VALUE, (UINT)CB_GETLBTEXT, (WPARAM)nIndex, (LPARAM)value_str);

				FromString<unsigned int>(value, ToString(value_str), std::dec);
				editor_application->updateEditPropertyInteger(value);
				break;
			}

			if (HIWORD(wParam) == CBN_EDITCHANGE) {
				unsigned int value = 0.0f;
				value = GetDlgItemInteger(hDlg, IDC_EDIT_FLOAT_VALUE);
				editor_application->updateEditPropertyInteger(value);
				break;
			}

			switch(LOWORD(wParam)) {
				case IDOK:
					SendMessage(hDlg, WM_CLOSE, 0, 0);
					editor_application->confirmEditProperty();
					return true;

				case IDCANCEL:
					SendMessage(hDlg, WM_CLOSE, 0, 0);
					editor_application->revertEditProperty();
					return true;
			}

			break;

		case WM_NOTIFY:
			return true;
	}
	
	return false;
}



INT_PTR CALLBACK EditFloatCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
		case WM_INITDIALOG:
			return true;

		case WM_CLOSE:
			EndDialog(hDlg, false);
			editor_application->clearEditPropertyGUI();
			return true;

		case WM_COMMAND:
			if(HIWORD(wParam) == CBN_SELCHANGE) { 
				char value_str[1024] = "";
				float value = 0.0f;

				int nIndex=SendDlgItemMessage(hDlg, IDC_EDIT_FLOAT_VALUE, (UINT) CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0);
				SendDlgItemMessage(hDlg, IDC_EDIT_FLOAT_VALUE, (UINT)CB_GETLBTEXT, (WPARAM)nIndex, (LPARAM)value_str);

				FromString<float>(value, ToString(value_str), std::dec);
				editor_application->updateEditPropertyFloat(value);
				break;
			}

			if (HIWORD(wParam) == CBN_EDITCHANGE) {
				float value = 0.0f;
				value = GetDlgItemFloat(hDlg, IDC_EDIT_FLOAT_VALUE);
				editor_application->updateEditPropertyFloat(value);
				break;
			}

			switch(LOWORD(wParam)) {
				case IDOK:
					SendMessage(hDlg, WM_CLOSE, 0, 0);
					editor_application->confirmEditProperty();
					return true;

				case IDCANCEL:
					SendMessage(hDlg, WM_CLOSE, 0, 0);
					editor_application->revertEditProperty();
					return true;
			}

			break;

		case WM_NOTIFY:
			return true;
	}
	
	return false;
}



INT_PTR CALLBACK EditStringCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
		case WM_INITDIALOG:
			return true;

		case WM_CLOSE:
			EndDialog(hDlg, false);
			editor_application->clearEditPropertyGUI();
			return true;

		case WM_COMMAND:
			if(HIWORD(wParam) == CBN_SELCHANGE) { 
				char value_str[1024] = "";
				int nIndex=SendDlgItemMessage(hDlg, IDC_EDIT_STRING_VALUE, (UINT) CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0);
				SendDlgItemMessage(hDlg, IDC_EDIT_STRING_VALUE, (UINT)CB_GETLBTEXT, (WPARAM)nIndex, (LPARAM)value_str);
				editor_application->updateEditPropertyString(ToString(value_str));
				break;
			}

			if (HIWORD(wParam) == CBN_EDITCHANGE) {
				char value_str[1024] = "";
				GetDlgItemText(hDlg, IDC_EDIT_STRING_VALUE, value_str, 1024);
				editor_application->updateEditPropertyString(ToString(value_str));
				break;
			}

			switch(LOWORD(wParam)) {
				case IDOK:
					SendMessage(hDlg, WM_CLOSE, 0, 0);
					editor_application->confirmEditProperty();
					return true;

				case IDCANCEL:
					SendMessage(hDlg, WM_CLOSE, 0, 0);
					editor_application->revertEditProperty();
					return true;
			}

			break;

		case WM_NOTIFY:
			return true;
	}
	
	return false;
}


INT_PTR CALLBACK EditVectorCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
		case WM_INITDIALOG:
			return true;

		case WM_CLOSE:
			if (IsDlgButtonChecked(hDlg, IDC_EDIT_VECTOR_EDITING)) {
				editor_application->updateEditPropertyVectorMode(false);
			}

			EndDialog(hDlg, false);

			editor_application->clearEditPropertyGUI();
			return true;

		case WM_COMMAND:
			if (HIWORD(wParam) == EN_CHANGE) {
				if ((LOWORD(wParam) == IDE_EDIT_VECTOR_X) || (LOWORD(wParam) == IDE_EDIT_VECTOR_Y) || (LOWORD(wParam) == IDE_EDIT_VECTOR_Z)) {
					float value_x = 0.0f;
					float value_y = 0.0f;
					float value_z = 0.0f;

					value_x = GetDlgItemFloat(hDlg, IDE_EDIT_VECTOR_X);
					value_y = GetDlgItemFloat(hDlg, IDE_EDIT_VECTOR_Y);
					value_z = GetDlgItemFloat(hDlg, IDE_EDIT_VECTOR_Z);

					editor_application->updateEditPropertyVector(LibGens::Vector3(value_x, value_y, value_z));
				}
				break;
			}

			switch(LOWORD(wParam)) {
				case IDOK:
					SendMessage(hDlg, WM_CLOSE, 0, 0);
					editor_application->confirmEditProperty();
					return true;

				case IDCANCEL:
					SendMessage(hDlg, WM_CLOSE, 0, 0);
					editor_application->revertEditProperty();
					return true;

				case IDC_EDIT_VECTOR_EDITING:
					editor_application->updateEditPropertyVectorMode(IsDlgButtonChecked(hDlg, IDC_EDIT_VECTOR_EDITING));
					return true;

				case IDB_EDIT_VECTOR_FOCUS:
					editor_application->updateEditPropertyVectorFocus();
					return true;

				/*
				case IDB_VECTOR_SELECT_FROM_POLYGON:
					editor_application->ignoreMouseClicks(5);
					editor_application->setEditorMode(EDITOR_NODE_QUERY_VECTOR_POLYGON);
					ShowWindow(hDlg, SW_HIDE);
					return true;

				case IDB_VECTOR_SELECT_FROM_NODE:
					editor_application->ignoreMouseClicks(5);
					editor_application->setEditorMode(EDITOR_NODE_QUERY_VECTOR_NODE);
					ShowWindow(hDlg, SW_HIDE);
					return true;
				*/
			}

			break;

		case WM_NOTIFY:
			if ((LOWORD(wParam) == IDS_EDIT_VECTOR_X) || (LOWORD(wParam) == IDS_EDIT_VECTOR_Y) || (LOWORD(wParam) == IDS_EDIT_VECTOR_Z)) {
				if (((LPNMUPDOWN)lParam)->hdr.code == UDN_DELTAPOS) {
					int delta = ((LPNMUPDOWN)lParam)->iDelta;

					float value_x = 0.0f;
					float value_y = 0.0f;
					float value_z = 0.0f;

					float spin_factor = 1.0;

					value_x = GetDlgItemFloat(hDlg, IDE_EDIT_VECTOR_X);
					value_y = GetDlgItemFloat(hDlg, IDE_EDIT_VECTOR_Y);
					value_z = GetDlgItemFloat(hDlg, IDE_EDIT_VECTOR_Z);

					if (delta > 1)  delta = 1;
					if (delta < -1) delta = -1;

					if (LOWORD(wParam) == IDS_EDIT_VECTOR_X) {
						value_x += (float)-delta * spin_factor;
						SetDlgItemText(hDlg, IDE_EDIT_VECTOR_X, ToString(value_x).c_str());
					}

					if (LOWORD(wParam) == IDS_EDIT_VECTOR_Y) {
						value_y += (float)-delta * spin_factor;
						SetDlgItemText(hDlg, IDE_EDIT_VECTOR_Y, ToString(value_y).c_str());
					}

					if (LOWORD(wParam) == IDS_EDIT_VECTOR_Z) {
						value_z += (float)-delta * spin_factor;
						SetDlgItemText(hDlg, IDE_EDIT_VECTOR_Z, ToString(value_z).c_str());
					}

					editor_application->updateEditPropertyVector(LibGens::Vector3(value_x, value_y, value_z));
				}
			}

			return true;
	}
	
	return false;
}

vector<VectorNode*>& EditorApplication::getPropertyVectorNodes()
{
	return property_vector_nodes;
}

vector<LibGens::Vector3>& EditorApplication::getCurrentPropertyVectorList()
{
	return temp_property_vector_list;
}

void EditorApplication::updateVectorListSelection(int index)
{
	current_vector_list_selection = index;
	HWND viewport_edit = GetDlgItem(hEditPropertyDlg, IDC_EDIT_VECTOR_LIST_EDITING);
	HWND viewport_focus = GetDlgItem(hEditPropertyDlg, IDB_EDIT_VECTOR_LIST_FOCUS);
	HWND vector_delete = GetDlgItem(hEditPropertyDlg, IDB_EDIT_VECTOR_LIST_DELETE);
	HWND vector_up = GetDlgItem(hEditPropertyDlg, IDB_EDIT_VECTOR_LIST_MOVE_UP);
	HWND vector_down = GetDlgItem(hEditPropertyDlg, IDB_EDIT_VECTOR_LIST_MOVE_DOWN);

	if (current_vector_list_selection != last_vector_list_selection)
	{
		last_vector_list_selection = current_vector_list_selection;
		
		if (isVectorListSelectionValid())
		{
			is_update_vector_list = false;
			LibGens::Vector3 v = temp_property_vector_list[current_vector_list_selection];

			SetDlgItemText(hEditPropertyDlg, IDE_EDIT_VECTOR_LIST_X, ToString<float>(v.x).c_str());
			SetDlgItemText(hEditPropertyDlg, IDE_EDIT_VECTOR_LIST_Y, ToString<float>(v.y).c_str());
			SetDlgItemText(hEditPropertyDlg, IDE_EDIT_VECTOR_LIST_Z, ToString<float>(v.z).c_str());
		}
	}

	if (isVectorListSelectionValid())
	{
		EnableWindow(viewport_edit, true);
		EnableWindow(viewport_focus, true);
		EnableWindow(vector_delete, true);

		// Enable or disable buttons based on current selection in the list view
		if (property_vector_nodes.size() > 1)
		{
			if (current_vector_list_selection > 0)
			{
				EnableWindow(vector_up, true);

				if (current_vector_list_selection < property_vector_nodes.size() - 1)
					EnableWindow(vector_down, true);
				else
					EnableWindow(vector_down, false);
			}

			if (current_vector_list_selection == 0)
			{
				EnableWindow(vector_up, false);

				if (property_vector_nodes.size() > 1)
					EnableWindow(vector_down, true);
			}
		}
	}
	else
	{
		EnableWindow(viewport_edit, false);
		EnableWindow(viewport_focus, false);
		EnableWindow(vector_delete, false);
		EnableWindow(vector_up, false);
		EnableWindow(vector_down, false);
	}

	is_update_vector_list = true;
}

void EditorApplication::removeVectorFromList(int index)
{
	HWND list_view = GetDlgItem(hEditPropertyDlg, IDL_EDIT_VECTOR_LIST_LIST);
	ListView_DeleteItem(list_view, index);
	int count = ListView_GetItemCount(list_view);
	ListView_SetItemCount(list_view, count - 1);

	for (vector<LibGens::Vector3>::iterator it = temp_property_vector_list.begin(); it != temp_property_vector_list.end(); ++it)
	{
		if (*it == temp_property_vector_list[index])
		{
			temp_property_vector_list.erase(it);
			break;
		}
	}

	for (vector<VectorNode *>::iterator it = property_vector_nodes.begin(); it != property_vector_nodes.end(); ++it)
	{
		if (*it == property_vector_nodes[index])
		{
			delete *it;
			property_vector_nodes.erase(it);
			break;
		}
	}

	if (count)
		ListView_SetSelectionMark(list_view, index - 1);
}

void EditorApplication::moveVector(int index, bool up)
{
	HWND list_view = GetDlgItem(hEditPropertyDlg, IDL_EDIT_VECTOR_LIST_LIST);
	char buffer[128];

	LV_ITEM item;
	item.mask = LVIF_TEXT;
	item.iItem = index;
	item.iSubItem = 0;
	item.pszText = buffer;

	ListView_GetItem(list_view, &item);
	ListView_DeleteItem(list_view, index);

	if (up)
	{
		item.iItem = index - 1;
		ListView_InsertItem(list_view, &item);

		swap(temp_property_vector_list[index], temp_property_vector_list[index - 1]);
		swap(property_vector_nodes[index], property_vector_nodes[index - 1]);
	}
	else
	{
		item.iItem = index + 1;
		ListView_InsertItem(list_view, &item);

		swap(temp_property_vector_list[index], temp_property_vector_list[index + 1]);
		swap(property_vector_nodes[index], property_vector_nodes[index + 1]);
	}
}

bool EditorApplication::isVectorListSelectionValid()
{
	return current_vector_list_selection > -1 && current_vector_list_selection < temp_property_vector_list.size() && temp_property_vector_list.size();
}

bool EditorApplication::isUpdateVectorList()
{
	return is_update_vector_list;
}

INT_PTR CALLBACK EditVectorListCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {

	HWND list_view = GetDlgItem(hDlg, IDL_EDIT_VECTOR_LIST_LIST);
	int list_view_index = ListView_GetNextItem(list_view, -1, LVIS_SELECTED | LVIS_FOCUSED);
	editor_application->updateVectorListSelection(list_view_index);
	
	switch (msg) {
	case WM_INITDIALOG:
		return true;

	case WM_CLOSE:
		if (IsDlgButtonChecked(hDlg, IDC_EDIT_VECTOR_LIST_EDITING)) {
			editor_application->updateEditPropertyVectorMode(false);
		}

		EndDialog(hDlg, false);
		editor_application->clearEditPropertyGUI();
		return true;

	case WM_COMMAND:
		if (HIWORD(wParam) == EN_CHANGE)
		{
			if ((LOWORD(wParam) == IDE_EDIT_VECTOR_LIST_X) || (LOWORD(wParam) == IDE_EDIT_VECTOR_LIST_Y) || (LOWORD(wParam) == IDE_EDIT_VECTOR_LIST_Z)) {
				if (editor_application->isUpdateVectorList() && editor_application->isVectorListSelectionValid())
				{
					float value_x = 0.0f;
					float value_y = 0.0f;
					float value_z = 0.0f;

					value_x = GetDlgItemFloat(hDlg, IDE_EDIT_VECTOR_LIST_X);
					value_y = GetDlgItemFloat(hDlg, IDE_EDIT_VECTOR_LIST_Y);
					value_z = GetDlgItemFloat(hDlg, IDE_EDIT_VECTOR_LIST_Z);

					char text[128];
					string newText = ToString<float>(value_x) + ", " + ToString<float>(value_y) + ", " + ToString<float>(value_z);
					strcpy(text, newText.c_str());

					ListView_SetItemText(list_view, list_view_index, 0, (char*)newText.c_str());
					editor_application->getCurrentPropertyVectorList()[list_view_index] = LibGens::Vector3(value_x, value_y, value_z);
					editor_application->getPropertyVectorNodes()[list_view_index]->setPosition(Ogre::Vector3(value_x, value_y, value_z));
				}
			}
		}

		ObjectNode* first_selection = static_cast<ObjectNode*>(*(editor_application->getSelectedNodes().begin()));

		switch ((LPARAM)wParam)
		{
		case IDCANCEL:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			editor_application->revertEditProperty();
			return true;

		case IDOK:
			editor_application->updateEditPropertyVectorList(editor_application->getCurrentPropertyVectorList());
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			editor_application->confirmEditProperty();
			return true;

		case IDB_EDIT_VECTOR_LIST_CREATE:
			editor_application->addVectorToList(first_selection->getObject()->getPosition());
			editor_application->updateEditPropertyVectorList(editor_application->getCurrentPropertyVectorList());
			break;

		case IDB_EDIT_VECTOR_LIST_FOCUS:
			if (editor_application->isVectorListSelectionValid())
				editor_application->updateEditPropertyVectorFocus(list_view_index);
			break;

		case IDB_EDIT_VECTOR_LIST_DELETE:
			if (editor_application->isVectorListSelectionValid())
			{
				editor_application->removeVectorFromList(list_view_index);
				editor_application->updateEditPropertyVectorList(editor_application->getCurrentPropertyVectorList());
			}
			break;

		case IDC_EDIT_VECTOR_LIST_EDITING:
			if (editor_application->isVectorListSelectionValid())
				editor_application->updateEditPropertyVectorMode(IsDlgButtonChecked(hDlg, IDC_EDIT_VECTOR_LIST_EDITING), true, list_view_index);
			break;

		case IDB_EDIT_VECTOR_LIST_MOVE_UP:
			editor_application->moveVector(list_view_index, true);
			break;

		case IDB_EDIT_VECTOR_LIST_MOVE_DOWN:
			editor_application->moveVector(list_view_index, false);
		}
		break;
	}

	return false;
}

void EditorApplication::addVectorToList(LibGens::Vector3 v3)
{
	HWND hVectorList = GetDlgItem(hEditPropertyDlg, IDL_EDIT_VECTOR_LIST_LIST);
	float x = v3.x;
	float y = v3.y;
	float z = v3.z;

	string v3_combined = ToString<float>(x) + ", " + ToString<float>(y) + ", " + ToString<float>(z);
	char v[256];

	strcpy(v, v3_combined.c_str());

	LV_ITEM item;
	item.mask = LVIF_TEXT;
	item.pszText = v;
	item.cchTextMax = strlen(v);
	item.state = 0;
	item.iSubItem = 0;
	item.lParam = (LPARAM)NULL;
	item.iItem = temp_property_vector_list.size();
	ListView_InsertItem(hVectorList, &item);
	ListView_SetItemText(hVectorList, item.iItem, 0, item.pszText);

	temp_property_vector_list.push_back(v3);
	VectorNode* vector_node = new VectorNode(scene_manager);
	vector_node->setPosition(Ogre::Vector3(x, y, z));
	property_vector_nodes.push_back(vector_node);
}

void EditorApplication::addIDToList(size_t id)
{
	HWND hIDList = GetDlgItem(hEditPropertyDlg, IDL_EDIT_ID_LIST_LIST);
	
	string id_string = ToString<size_t>(id);
	char v[128];

	strcpy(v, id_string.c_str());

	LV_ITEM item;
	item.mask = LVIF_TEXT;
	item.pszText = v;
	item.cchTextMax = strlen(v);
	item.state = 0;
	item.iSubItem = 0;
	item.lParam = (LPARAM)NULL;
	item.iItem = temp_property_id_list.size();
	ListView_InsertItem(hIDList, &item);
	ListView_SetItemText(hIDList, item.iItem, 0, item.pszText);

	temp_property_id_list.push_back(id);
}

void EditorApplication::removeIDFromList(int index)
{
	HWND list_view = GetDlgItem(hEditPropertyDlg, IDL_EDIT_ID_LIST_LIST);

	for (vector<size_t>::iterator it = temp_property_id_list.begin(); it != temp_property_id_list.end(); ++it)
	{
		if (*it == temp_property_id_list[index])
		{
			temp_property_id_list.erase(it);
			ListView_DeleteItem(list_view, index);
			return;
		}
	}
}

void EditorApplication::moveID(int index, bool up)
{
	HWND list_view = GetDlgItem(hEditPropertyDlg, IDL_EDIT_ID_LIST_LIST);
	char item_text[128];

	LV_ITEM item;
	item.mask = LVIF_TEXT;
	item.iItem = index;
	item.iSubItem = 0;
	item.pszText = item_text;
	
	ListView_GetItem(list_view, &item);
	ListView_DeleteItem(list_view, index);

	if (up)
	{
		item.iItem = index - 1;
		ListView_InsertItem(list_view, &item);
		ListView_SetItemText(list_view, item.iItem, item.iSubItem, item.pszText);

		swap(temp_property_id_list[index - 1], temp_property_id_list[index]);
	}
	else
	{
		item.iItem = index + 1;
		ListView_InsertItem(list_view, &item);
		ListView_SetItemText(list_view, item.iItem, item.iSubItem, item.pszText);

		swap(temp_property_id_list[index + 1], temp_property_id_list[index]);
	}
	
}

void EditorApplication::setTargetName(size_t id, bool is_list)
{
	LibGens::Object* obj = getCurrentLevel()->getLevel()->getObjectByID(id);
	string object_name = "none";
	bool enabled = false;

	if (obj)
	{
		object_name = obj->getName();
		enabled = true;
	}

	string result = "Points to: (" + object_name + ")";
	HWND button;

	if (!is_list)
	{
		SetDlgItemText(hEditPropertyDlg, IDT_EDIT_ID_POINT, result.c_str());

		button = GetDlgItem(hEditPropertyDlg, IDB_EDIT_ID_GO_TO_TARGET);
		EnableWindow(button, enabled);
	}
	else
	{
		SetDlgItemText(hEditPropertyDlg, IDT_EDIT_ID_LIST_POINT, result.c_str());

		button = GetDlgItem(hEditPropertyDlg, IDB_EDIT_ID_LIST_GO_TO_TARGET);
		EnableWindow(button, enabled);
	}
}

INT_PTR CALLBACK EditIdCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		return true;

	case WM_CLOSE:
		EndDialog(hDlg, false);
		editor_application->openQueryTargetMode(false);
		editor_application->clearEditPropertyGUI();
		return true;

	case WM_COMMAND:
		if (HIWORD(wParam) == CBN_EDITCHANGE)
		{
			if (LOWORD(wParam) == IDC_EDIT_ID_VALUE)
			{
				size_t id = GetDlgItemFloat(hDlg, IDC_EDIT_ID_VALUE);
				editor_application->setTargetName(id);
			}
		}

		switch (LOWORD(wParam))
		{
		case IDC_EDIT_ID_SELECT_FROM_VIEWPORT:
			editor_application->openQueryTargetMode(IsDlgButtonChecked(hDlg, IDC_EDIT_ID_SELECT_FROM_VIEWPORT));
			break;

		case IDB_EDIT_ID_GO_TO_TARGET:
		{
			size_t id = GetDlgItemFloat(hDlg, IDC_EDIT_ID_VALUE);
			LibGens::Object* obj = editor_application->getCurrentLevel()->getLevel()->getObjectByID(id);
			if (obj)
			{
				editor_application->clearSelection();
				ObjectNode *obj_node = editor_application->getObjectNodeManager()->findObjectNode(obj);
				if (obj_node)
				{
					SendMessage(hDlg, WM_CLOSE, 0, 0);
					editor_application->selectNode(obj_node);
					editor_application->updateSelection();
					return true;
				}
			}
			break;
		}
		break;

		case IDOK:
		{
			size_t value = GetDlgItemFloat(hDlg, IDC_EDIT_ID_VALUE);
			editor_application->updateEditPropertyID(value);
			editor_application->confirmEditProperty();
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return true;
		}

		case IDCANCEL:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return true;

		}

		break;
	}

	return false;
}

void EditorApplication::updateIDListSelection(int index)
{
	HWND list_delete = GetDlgItem(hEditPropertyDlg, IDB_EDIT_ID_LIST_DELETE);
	HWND list_move_up = GetDlgItem(hEditPropertyDlg, IDB_EDIT_ID_LIST_MOVE_UP);
	HWND list_move_down = GetDlgItem(hEditPropertyDlg, IDB_EDIT_ID_LIST_MOVE_DOWN);
	HWND list_add = GetDlgItem(hEditPropertyDlg, IDC_EDIT_ID_LIST_ADD_FROM_VIEWPORT);
	HWND list_text = GetDlgItem(hEditPropertyDlg, IDE_EDIT_ID_LIST_VALUE);

	current_id_list_selection = index;
	if (current_id_list_selection != last_id_list_selection)
	{
		last_id_list_selection = current_id_list_selection;
		if (isIDListSelectionValid())
		{
			size_t id = temp_property_id_list[index];
			SetDlgItemText(hEditPropertyDlg, IDE_EDIT_ID_LIST_VALUE, ToString<size_t>(id).c_str());
		}
	}

	if (isIDListSelectionValid())
	{
		EnableWindow(list_delete, true);
		EnableWindow(list_add, true);
		EnableWindow(list_text, true);

		// Enable or disable buttons based on current selection in the list view
		if (temp_property_id_list.size() > 1)
		{
			if (current_id_list_selection > 0)
			{
				EnableWindow(list_move_up, true);

				if (current_id_list_selection < temp_property_id_list.size() - 1)
					EnableWindow(list_move_down, true);
				else
					EnableWindow(list_move_down, false);
			}

			if (current_id_list_selection == 0)
			{
				EnableWindow(list_move_up, false);

				if (temp_property_id_list.size() > 1)
					EnableWindow(list_move_down, true);
			}
		}
	}
	else
	{
		EnableWindow(list_delete, false);
		EnableWindow(list_move_up, false);
		EnableWindow(list_move_down, false);
		EnableWindow(list_add, false);
		EnableWindow(list_text, false);
	}
}

vector<size_t>& EditorApplication::getCurrentPropertyIDList()
{
	return temp_property_id_list;
}

bool EditorApplication::isIDListSelectionValid()
{
	return current_id_list_selection > -1 && current_id_list_selection < temp_property_id_list.size() && temp_property_id_list.size();
}

INT_PTR CALLBACK EditIdListCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND list_view = GetDlgItem(hDlg, IDL_EDIT_ID_LIST_LIST);
	int index = ListView_GetNextItem(list_view, -1, LVIS_SELECTED | LVIS_FOCUSED);
	editor_application->updateIDListSelection(index);

	switch (msg)
	{
	case WM_INITDIALOG:
		return true;

	case WM_CLOSE:
		EndDialog(hDlg, false);
		editor_application->openQueryTargetMode(false);
		editor_application->clearEditPropertyGUI();
		return true;

	case WM_COMMAND:
		if (HIWORD(wParam) == EN_CHANGE)
		{
			if (LOWORD(wParam) == IDE_EDIT_ID_LIST_VALUE)
			{
				if (editor_application->isIDListSelectionValid())
				{
					size_t id = GetDlgItemFloat(hDlg, IDE_EDIT_ID_LIST_VALUE);
					string id_string = ToString<size_t>(id);
					char buffer[128];
					strcpy(buffer, id_string.c_str());

					ListView_SetItemText(list_view, index, 0, (char*)buffer);
					editor_application->getCurrentPropertyIDList()[index] = id;
					editor_application->setTargetName(id, true);
				}
			}
		}

		switch (LOWORD(wParam))
		{
		case IDB_EDIT_ID_LIST_CREATE:
			editor_application->addIDToList(0);
			editor_application->updateEditPropertyIDList(editor_application->getCurrentPropertyIDList());
			break;

		case IDB_EDIT_ID_LIST_DELETE:
			editor_application->removeIDFromList(index);
			editor_application->updateEditPropertyIDList(editor_application->getCurrentPropertyIDList());
			break;

		case IDB_EDIT_ID_LIST_MOVE_UP:
			editor_application->moveID(index, true);
			break;

		case IDB_EDIT_ID_LIST_MOVE_DOWN:
			editor_application->moveID(index, false);
			break;

		case IDB_EDIT_ID_LIST_GO_TO_TARGET:
		{
			size_t id = GetDlgItemFloat(hDlg, IDE_EDIT_ID_LIST_VALUE);
			LibGens::Object* obj = editor_application->getCurrentLevel()->getLevel()->getObjectByID(id);
			if (obj)
			{
				editor_application->clearSelection();
				ObjectNode* obj_node = editor_application->getObjectNodeManager()->findObjectNode(obj);
				if (obj_node)
				{
					SendMessage(hDlg, WM_CLOSE, 0, 0);
					editor_application->selectNode(obj_node);
					editor_application->updateSelection();
					return true;
				}
			}
			break;
		}
		break;

		case IDC_EDIT_ID_LIST_ADD_FROM_VIEWPORT:
			editor_application->openQueryTargetMode(IsDlgButtonChecked(hDlg, IDC_EDIT_ID_LIST_ADD_FROM_VIEWPORT));
			break;

		case IDCANCEL:
			editor_application->revertEditProperty();
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return true;

		case IDOK:
			editor_application->openQueryTargetMode(false);
			editor_application->updateEditPropertyIDList(editor_application->getCurrentPropertyIDList());
			editor_application->confirmEditProperty();
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return true;
		}
		break;
	}

	return false;
}

// Subclass the edit control with the main focus to detect when Enter or Escape is pressed
LRESULT CALLBACK EditControlCallback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) { 
    switch (msg) { 
        case WM_KEYDOWN: 
            switch (wParam) { 
                case VK_ESCAPE: 
					if (editor_application->getEditPropertyDlg()) {
						PostMessage(editor_application->getEditPropertyDlg(), WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0L);
					}
                    return 0; 
                case VK_RETURN: 
					if (editor_application->getEditPropertyDlg()) {
						PostMessage(editor_application->getEditPropertyDlg(), WM_COMMAND, MAKEWPARAM(IDOK, 0), 0L); 
					}
                    return 0; 
            } 
            break; 
 
        case WM_CHAR: 
            switch (wParam) { 
                case VK_TAB: 
                case VK_ESCAPE: 
                case VK_RETURN: 
                    return 0; 
            } 
    } 
 
	return CallWindowProc(globalEditControlOldProc, hwnd, msg, wParam, lParam); 
} 