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
#include "AR.h"
#include "Texture.h"
#include "Parameter.h"
#include "Material.h"

INT_PTR CALLBACK MaterialEditorCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK MaterialEditorPreviewCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
bool hasScene;

void EditorApplication::openMaterialEditorGUI() {
	if (!hMaterialEditorDlg) {
		hMaterialEditorDlg = CreateDialog(NULL, MAKEINTRESOURCE(IDD_MATERIAL_EDITOR), NULL, MaterialEditorCallback);

		material_editor_model = NULL;
		material_editor_model_filename = "";
		material_editor_material = NULL;
		material_editor_skeleton_name = "";
		material_editor_animation_name = "";
		material_editor_animation_state = NULL;
		material_editor_scene_node = NULL;
		material_editor_mesh_group = PREVIEW_MESH_GROUP;
		material_editor_mode = SONICGLVL_MATERIAL_EDITOR_MODE_MODEL;

		SendDlgItemMessage(hMaterialEditorDlg, IDR_MATERIAL_MODEL_MODE, BM_SETCHECK, (WPARAM)(material_editor_mode == SONICGLVL_MATERIAL_EDITOR_MODE_MODEL), 0);
		SendDlgItemMessage(hMaterialEditorDlg, IDR_MATERIAL_MATERIAL_MODE, BM_SETCHECK, (WPARAM)(material_editor_mode == SONICGLVL_MATERIAL_EDITOR_MODE_MATERIAL), 0);
		SendDlgItemMessage(hMaterialEditorDlg, IDR_MATERIAL_TERRAIN_MODE, BM_SETCHECK, (WPARAM)(material_editor_mode == SONICGLVL_MATERIAL_EDITOR_MODE_TERRAIN), 0);

		clearSelectionMaterialEditorGUI();
		enableMaterialEditorListGUI();

		// Create Texture Units List
		HWND hMaterialTextureUnitsList = GetDlgItem(hMaterialEditorDlg, IDL_MATERIAL_TEXTURE_UNIT_LIST);
		ListView_SetExtendedListViewStyleEx(hMaterialTextureUnitsList, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

		// Only enable terrain mode if theres a level
		EnableWindow(GetDlgItem(hMaterialEditorDlg, IDR_MATERIAL_TERRAIN_MODE), current_level != NULL);

		// Disable non-functional modes for now
		EnableWindow(GetDlgItem(hMaterialEditorDlg, IDR_MATERIAL_MATERIAL_MODE), false);
	}

	SetFocus(hMaterialEditorDlg);
}

void EditorApplication::enableMaterialEditorGUI(bool enable) {
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDG_MATERIAL_PARAMETERS), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDT_MATERIAL_NAME), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_NAME), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDT_MATERIAL_SHADER), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDC_MATERIAL_SHADER), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDT_MATERIAL_MESH_SLOT), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDC_MATERIAL_MESH_SLOT), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDC_MATERIAL_BACKFACE_CULLING), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDC_MATERIAL_ADDITIVE), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDC_MATERIAL_UNKNOWN_FLAG), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDT_MATERIAL_TEXTURE_UNITS), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDL_MATERIAL_TEXTURE_UNIT_LIST), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDB_MATERIAL_NEW_TEXTURE_UNIT), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDB_MATERIAL_DELETE_TEXTURE_UNIT), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDB_MATERIAL_CREATE_TEXTURE_UNITS), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDS_MATERIAL_TEXTURE_UNIT_LIST), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDT_MATERIAL_TEXTURE_FILENAME), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_TEXTURE_FILENAME), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDB_MATERIAL_OPEN_TEXTURE_FILENAME), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDT_MATERIAL_TEXTURE_UNIT_NAME), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_TEXTURE_UNIT), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDB_MATERIAL_TEXTURE_UNIT_RESET), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDT_MATERIAL_TEXTURE_UNIT_SLOT), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDC_MATERIAL_TEXTURE_UNIT_SLOT), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDT_MATERIAL_PARAMETER_NAME), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDT_MATERIAL_PARAMETER_R), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDT_MATERIAL_PARAMETER_G), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDT_MATERIAL_PARAMETER_B), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDT_MATERIAL_PARAMETER_A), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDC_MATERIAL_DEFAULTS), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDB_MATERIAL_DEFAULTS), enable);

	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_1_N), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_2_N), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_3_N), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_4_N), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_5_N), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_6_N), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_7_N), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_8_N), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_9_N), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_10_N), enable);

	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_1_R), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_2_R), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_3_R), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_4_R), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_5_R), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_6_R), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_7_R), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_8_R), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_9_R), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_10_R), enable);

	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_1_G), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_2_G), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_3_G), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_4_G), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_5_G), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_6_G), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_7_G), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_8_G), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_9_G), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_10_G), enable);

	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_1_B), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_2_B), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_3_B), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_4_B), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_5_B), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_6_B), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_7_B), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_8_B), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_9_B), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_10_B), enable);

	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_1_A), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_2_A), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_3_A), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_4_A), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_5_A), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_6_A), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_7_A), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_8_A), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_9_A), enable);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDE_MATERIAL_PARAMETER_10_A), enable);

	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDB_MATERIAL_SAVE_MATERIAL), enable);
}


void EditorApplication::enableMaterialEditorListGUI() {
	bool material_list_flag=true;
	if ((material_editor_mode == SONICGLVL_MATERIAL_EDITOR_MODE_MODEL) && !material_editor_model) {
		material_list_flag = false;
	}
	
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDG_MATERIAL_MATERIAL_LIST), material_list_flag);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDL_MATERIAL_MATERIAL_LIST), material_list_flag);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDB_MATERIAL_NEW_MATERIAL), material_list_flag);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDB_MATERIAL_DELETE_MATERIAL), material_list_flag);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDB_MATERIAL_LOAD_SKELETON), material_list_flag);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDB_MATERIAL_LOAD_ANIMATION), material_list_flag);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDB_MATERIAL_SAVE_MODEL), material_list_flag);
}

void EditorApplication::updateMaterialEditorTextureList() {
	LibGens::Material* mat = material_editor_material;
	if (!material_editor_material) return;

	// Update Texture Units
	HWND hMaterialTextureList = GetDlgItem(hMaterialEditorDlg, IDL_MATERIAL_TEXTURE_UNIT_LIST);
	if (ListView_GetItemCount(hMaterialTextureList) != 0) {
		ListView_DeleteAllItems(hMaterialTextureList);
		while (ListView_DeleteColumn(hMaterialTextureList, 0) > 0);
		ListView_SetItemCount(hMaterialTextureList, 0);
	}

	LVCOLUMN Col;
	Col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	Col.cx = 151;
	Col.pszText = "DDS";
	Col.cchTextMax = strlen(Col.pszText);
	ListView_InsertColumn(hMaterialTextureList, 0, &Col);

	Col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	Col.cx = 70;
	Col.pszText = "Slot";
	Col.cchTextMax = strlen(Col.pszText);
	ListView_InsertColumn(hMaterialTextureList, 1, &Col);


	vector<LibGens::Texture*> texture_units = mat->getTextureUnits();
	for (size_t i = 0; i < texture_units.size(); i++) {
		char dds_str[256];
		char slot_str[256];

		LV_ITEM Item;
		Item.mask = LVIF_TEXT;
		strcpy(dds_str, texture_units[i]->getName().c_str());
		strcpy(slot_str, texture_units[i]->getUnit().c_str());

		Item.pszText = dds_str;
		Item.cchTextMax = strlen(dds_str);
		Item.iSubItem = 0;
		Item.lParam = (LPARAM)NULL;
		Item.iItem = i;
		ListView_InsertItem(hMaterialTextureList, &Item);
		ListView_SetItemText(hMaterialTextureList, i, 1, slot_str);
	}
}

void EditorApplication::updateMaterialTextureInfo() {
	LibGens::Texture* tex = material_editor_texture;
	if (!material_editor_texture) return;

	SetDlgItemText(hMaterialEditorDlg, IDE_MATERIAL_TEXTURE_FILENAME, tex->getName().c_str());
	SetDlgItemText(hMaterialEditorDlg, IDE_MATERIAL_TEXTURE_UNIT, tex->getTexset().c_str());
	SetDlgItemText(hMaterialEditorDlg, IDC_MATERIAL_TEXTURE_UNIT_SLOT, tex->getUnit().c_str());
}

void EditorApplication::updateMaterialEditorInfo() {
	LibGens::Material *mat = material_editor_material;
	if (!material_editor_material) return;

	// Update Info
	SetDlgItemText(hMaterialEditorDlg, IDE_MATERIAL_NAME, mat->getName().c_str());
	SetDlgItemText(hMaterialEditorDlg, IDC_MATERIAL_SHADER, mat->getShader().c_str());
	SendDlgItemMessage(hMaterialEditorDlg, IDC_MATERIAL_BACKFACE_CULLING, BM_SETCHECK, (WPARAM) mat->hasNoCulling(), 0);
	SendDlgItemMessage(hMaterialEditorDlg, IDC_MATERIAL_ADDITIVE, BM_SETCHECK, (WPARAM) mat->hasColorBlend(), 0);
	
	updateMaterialEditorTextureList();
	
	// Update Parameters
	vector<LibGens::Parameter *> parameters = mat->getParameters();

	for (size_t i=0; i<10; i++) {
		string parameter_name = "";
		string parameter_r = "";
		string parameter_g = "";
		string parameter_b = "";
		string parameter_a = "";

		if (i < parameters.size()) {
			LibGens::Color color = parameters[i]->getColor();
			parameter_name = parameters[i]->getName();

			parameter_r = ToString(color.r);
			parameter_g = ToString(color.g);
			parameter_b = ToString(color.b);
			parameter_a = ToString(color.a);
		}

		int n, r, g, b, a;
		if (i == 0) { n = IDE_MATERIAL_PARAMETER_1_N; r = IDE_MATERIAL_PARAMETER_1_R; g = IDE_MATERIAL_PARAMETER_1_G; b = IDE_MATERIAL_PARAMETER_1_B; a = IDE_MATERIAL_PARAMETER_1_A; }
		if (i == 1) { n = IDE_MATERIAL_PARAMETER_2_N; r = IDE_MATERIAL_PARAMETER_2_R; g = IDE_MATERIAL_PARAMETER_2_G; b = IDE_MATERIAL_PARAMETER_2_B; a = IDE_MATERIAL_PARAMETER_2_A; }
		if (i == 2) { n = IDE_MATERIAL_PARAMETER_3_N; r = IDE_MATERIAL_PARAMETER_3_R; g = IDE_MATERIAL_PARAMETER_3_G; b = IDE_MATERIAL_PARAMETER_3_B; a = IDE_MATERIAL_PARAMETER_3_A; }
		if (i == 3) { n = IDE_MATERIAL_PARAMETER_4_N; r = IDE_MATERIAL_PARAMETER_4_R; g = IDE_MATERIAL_PARAMETER_4_G; b = IDE_MATERIAL_PARAMETER_4_B; a = IDE_MATERIAL_PARAMETER_4_A; }
		if (i == 4) { n = IDE_MATERIAL_PARAMETER_5_N; r = IDE_MATERIAL_PARAMETER_5_R; g = IDE_MATERIAL_PARAMETER_5_G; b = IDE_MATERIAL_PARAMETER_5_B; a = IDE_MATERIAL_PARAMETER_5_A; }
		if (i == 5) { n = IDE_MATERIAL_PARAMETER_6_N; r = IDE_MATERIAL_PARAMETER_6_R; g = IDE_MATERIAL_PARAMETER_6_G; b = IDE_MATERIAL_PARAMETER_6_B; a = IDE_MATERIAL_PARAMETER_6_A; }
		if (i == 6) { n = IDE_MATERIAL_PARAMETER_7_N; r = IDE_MATERIAL_PARAMETER_7_R; g = IDE_MATERIAL_PARAMETER_7_G; b = IDE_MATERIAL_PARAMETER_7_B; a = IDE_MATERIAL_PARAMETER_7_A; }
		if (i == 7) { n = IDE_MATERIAL_PARAMETER_8_N; r = IDE_MATERIAL_PARAMETER_8_R; g = IDE_MATERIAL_PARAMETER_8_G; b = IDE_MATERIAL_PARAMETER_8_B; a = IDE_MATERIAL_PARAMETER_8_A; }
		if (i == 8) { n = IDE_MATERIAL_PARAMETER_9_N; r = IDE_MATERIAL_PARAMETER_9_R; g = IDE_MATERIAL_PARAMETER_9_G; b = IDE_MATERIAL_PARAMETER_9_B; a = IDE_MATERIAL_PARAMETER_9_A; }
		if (i == 9) { n = IDE_MATERIAL_PARAMETER_10_N; r = IDE_MATERIAL_PARAMETER_10_R; g = IDE_MATERIAL_PARAMETER_10_G; b = IDE_MATERIAL_PARAMETER_10_B; a = IDE_MATERIAL_PARAMETER_10_A; }

		SetDlgItemText(hMaterialEditorDlg, n, parameter_name.c_str());
		SetDlgItemText(hMaterialEditorDlg, r, parameter_r.c_str());
		SetDlgItemText(hMaterialEditorDlg, g, parameter_g.c_str());
		SetDlgItemText(hMaterialEditorDlg, b, parameter_b.c_str());
		SetDlgItemText(hMaterialEditorDlg, a, parameter_a.c_str());
	}

	SendDlgItemMessage(hMaterialEditorDlg, IDC_MATERIAL_TEXTURE_UNIT_SLOT, CB_RESETCONTENT, NULL, NULL);

	if (material_editor_shader_library) {
		string shader_name = mat->getShader();
		LibGens::Shader* vertex_shader = NULL;
		LibGens::Shader* pixel_shader = NULL;
		material_editor_shader_library->getMaterialShaders(shader_name, vertex_shader, pixel_shader, false, !mat->hasExtraGI(), false);

		if (pixel_shader) {
			vector<string> names = pixel_shader->getShaderParameterFilenames();
			for (size_t i = 0; i < names.size(); i++) {
				LibGens::ShaderParams* params = material_editor_shader_library->getPixelShaderParams(names[i]);
				if (params->getName() == "global")
					continue;
				vector<LibGens::ShaderParam*> paramList = params->getParameterList(3);
				for (size_t i2 = 0; i2 < paramList.size(); i2++) {
					SendDlgItemMessage(hMaterialEditorDlg, IDC_MATERIAL_TEXTURE_UNIT_SLOT, CB_ADDSTRING, NULL, (LPARAM)paramList[i2]->getName().c_str());
				}
			}
		}
	}
}

void EditorApplication::loadMaterialDefaultParams() {
	if (!material_editor_material || !material_editor_shader_library) {
		return;
	}

	LibGens::Material* material = material_editor_material;
	material->removeAllParameters();
	string shader_name = material->getShader();
	LibGens::Shader* vertex_shader = NULL;
	LibGens::Shader* pixel_shader = NULL;
	material_editor_shader_library->getMaterialShaders(shader_name, vertex_shader, pixel_shader, false, !material->hasExtraGI(), false);

	if (pixel_shader) {
		vector<string> names = pixel_shader->getShaderParameterFilenames();
		for (size_t i = 0; i < names.size(); i++) {
			LibGens::ShaderParams* params = material_editor_shader_library->getPixelShaderParams(names[i]);
			vector<LibGens::ShaderParam*> paramList = params->getParameterList(0);
			for (size_t i2 = 0; i2 < paramList.size(); i2++) {
				if (paramList[i2]->getName().rfind("g_", 0) == -1 && paramList[i2]->getName().rfind("mrg", 0) == -1)
				{
					material->addParameter(new LibGens::Parameter(paramList[i2]->getName(), LibGens::Color(1, 1, 1, 1)));
				}
			}
		}
		updateMaterialEditorInfo();
		updateEditShaderMaterialEditor(shader_name);
	}
}

void EditorApplication::removeMaterialEditorTexture() {
	if (!material_editor_texture)
		return;

	material_editor_material->removeTextureUnitByIndex(texture_list_selection);
	updateMaterialEditorTextureList();

	Ogre::Material* ogre_material = Ogre::MaterialManager::getSingleton().getByName(material_editor_material->getExtra(), material_editor_mesh_group).getPointer();

	if (ogre_material) {
		updateMaterialShaderParameters(ogre_material, material_editor_material, !material_editor_material->hasExtraGI(), NULL, material_editor_shader_library);
	}
}

void EditorApplication::materialEditorTerrainMode() {
	material_editor_mode = SONICGLVL_MATERIAL_EDITOR_MODE_TERRAIN;
	material_editor_mesh_group = GENERAL_MESH_GROUP;
	if (material_editor_model) {
		cleanMaterialEditorModelGUI();
	}

	clearSelectionMaterialEditorGUI();
	enableMaterialEditorListGUI();
	material_editor_materials.clear();

	material_editor_material_library = current_level->getTerrain()->getMaterialLibrary();
	material_editor_library_folder = current_level->getTerrain()->getResourcesFolder();

	for (LibGens::Material* mat : material_editor_material_library->getMaterials()) {
		material_editor_materials.push_back(mat);
	}

	std::sort(material_editor_materials.begin(), material_editor_materials.end(),
		[](LibGens::Material* lhs, LibGens::Material* rhs) {
			return lhs->getName().compare(rhs->getName()) < 0;
		});

	rebuildListMaterialEditorGUI();

	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDB_MATERIAL_LOAD_MODEL), false);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDB_MATERIAL_SAVE_MODEL), false);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDB_MATERIAL_LOAD_SKELETON), false);
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDB_MATERIAL_LOAD_ANIMATION), false);
}

void EditorApplication::materialEditorModelMode() {
	EnableWindow(GetDlgItem(hMaterialEditorDlg, IDB_MATERIAL_LOAD_MODEL), true);
	material_editor_mode = SONICGLVL_MATERIAL_EDITOR_MODE_MODEL;
	material_editor_mesh_group = PREVIEW_MESH_GROUP;
	material_editor_materials.clear();
	clearSelectionMaterialEditorGUI();
	rebuildListMaterialEditorGUI();
}

void EditorApplication::clearMaterialEditorGUI() {
	cleanMaterialEditorModelGUI();
	hMaterialEditorDlg = NULL;
}

void EditorApplication::cleanMaterialEditorModelGUI() {
	if (material_editor_model) {
		delete material_editor_model;
	}

	material_editor_model = NULL;
}

void EditorApplication::clearSelectionMaterialEditorGUI() {
	material_editor_list_selection = -1;
	last_material_editor_list_selection = -1;
	texture_list_selection = -1;
	last_texture_list_selection = -1;
	enableMaterialEditorGUI(false);
}

void EditorApplication::clearTextureInfo() {
	material_editor_texture = NULL;
	SetDlgItemText(hMaterialEditorDlg, IDE_MATERIAL_TEXTURE_FILENAME, "");
	SetDlgItemText(hMaterialEditorDlg, IDE_MATERIAL_TEXTURE_UNIT, "");
	SetDlgItemText(hMaterialEditorDlg, IDC_MATERIAL_TEXTURE_UNIT_SLOT, "");
}

void EditorApplication::createPreviewMaterialEditorGUI() {
	/*
	hMaterialEditorPreviewDlg = CreateDialog(NULL, MAKEINTRESOURCE(IDD_MATERIAL_EDITOR_PREVIEW), NULL, NULL);

	RECT window_rect;
	GetWindowRect(hMaterialEditorPreviewDlg, &window_rect);
	MoveWindow(hMaterialEditorPreviewDlg, window_rect.left, window_rect.top, 640, 480, true);
	*/

	// Create Scene for Preview
	if (!hasScene) {
		material_editor_preview_scene_manager = root->createSceneManager("DefaultSceneManager");
		material_editor_preview_scene_manager->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));

		// FIXME: Implement properly
		Ogre::SceneManager* bogus_manager = root->createSceneManager("DefaultSceneManager");

		Ogre::Light* dir_light = material_editor_preview_scene_manager->createLight("Preview Directional Light");
		dir_light->setSpecularColour(Ogre::ColourValue::White);
		dir_light->setDiffuseColour(Ogre::ColourValue(1.0, 1.0, 1.0));
		dir_light->setType(Ogre::Light::LT_DIRECTIONAL);
		dir_light->setDirection(Ogre::Vector3(1, 1, 1).normalisedCopy());

		// Create Render Window
		Ogre::NameValuePairList misc;
		//misc["externalWindowHandle"] = Ogre::StringConverter::toString((int)hMaterialEditorPreviewDlg);
		misc["FSAA"] = Ogre::StringConverter::toString((int)8);
		misc["vsync"] = Ogre::StringConverter::toString((bool)true);
		material_editor_preview_window = root->createRenderWindow("Preview Window", 640, 480, false, &misc);
		material_editor_preview_window->setDeactivateOnFocusChange(false);

		material_editor_preview_window->setAutoUpdated(true);


		// Create Viewport for Preview
		material_editor_viewport = new EditorViewport(material_editor_preview_scene_manager, bogus_manager, material_editor_preview_window, SONICGLVL_CAMERA_PREVIEW_NAME);
		material_editor_viewport->setPanningMultiplier(0.005);
		material_editor_viewport->setZoomingMultiplier(0.04);
		material_editor_viewport->setNearClipDistance(0.001f);
		material_editor_viewport->setFarClipDistance(100.0f);

		// Create Listener for Window
		material_editor_preview_listener = new MaterialEditorPreviewListener();
		material_editor_preview_listener->setEditorViewport(material_editor_viewport);
		material_editor_preview_listener->setEditorWindow(material_editor_preview_window);

		// Create Input Manager for new Window and set the listener
		OIS::ParamList pl;
		size_t windowHnd = 0;
		std::ostringstream windowHndStr;
		material_editor_preview_window->getCustomAttribute("WINDOW", &windowHnd);
		windowHndStr << windowHnd;
		pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
		pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND")));
		pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
		pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
		pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")));
		material_editor_input_manager = OIS::InputManager::createInputSystem(pl);
		material_editor_keyboard = static_cast<OIS::Keyboard*>(material_editor_input_manager->createInputObject(OIS::OISKeyboard, true));
		material_editor_mouse = static_cast<OIS::Mouse*>(material_editor_input_manager->createInputObject(OIS::OISMouse, true));

		material_editor_mouse->setEventCallback(material_editor_preview_listener);
		material_editor_keyboard->setEventCallback(material_editor_preview_listener);
		Ogre::WindowEventUtilities::addWindowEventListener(material_editor_preview_window, material_editor_preview_listener);
		root->addFrameListener(material_editor_preview_listener);

		material_editor_preview_listener->setMouse(material_editor_mouse);
		material_editor_preview_listener->setKeyboard(material_editor_keyboard);

		//mouse->setEventCallback(material_editor_preview_listener);
		//keyboard->setEventCallback(material_editor_preview_listener);


		unsigned int width, height, depth;
		int left, top;
		material_editor_preview_window->getMetrics(width, height, depth, left, top);
		const OIS::MouseState& ms = material_editor_mouse->getMouseState();
		ms.width = width;
		ms.height = height;
	}

	rebuildMaterialPreviewNodes();

	// Certain models have invalid AABB values so rebuild it.
	material_editor_model->buildAABB();

	LibGens::AABB model_aabb=material_editor_model->getAABB();
	LibGens::Vector3 aabb_center = model_aabb.center();
	Ogre::Vector3 camera_center = Ogre::Vector3(aabb_center.x, aabb_center.y, aabb_center.z);
	float size_max = model_aabb.sizeMax();
	Ogre::Camera *camera = material_editor_viewport->getCamera();

	camera->setPosition(camera_center);
	camera->setDirection(Ogre::Vector3(0, 0, -1).normalisedCopy());
	camera->moveRelative(Ogre::Vector3::UNIT_Z * size_max*1.5);
	hasScene = true;
}


void EditorApplication::rebuildMaterialPreviewNodes() {
	if (material_editor_scene_node) {
		destroySceneNode(material_editor_scene_node);
	}

	if (material_editor_model) {
		cleanModelResource(material_editor_model, PREVIEW_MESH_GROUP);
	}

	// Create the nodes
	material_editor_scene_node = material_editor_preview_scene_manager->getRootSceneNode()->createChildSceneNode();


	if (material_editor_skeleton_name.size() && material_editor_animation_name.size()) {
		prepareSkeletonAndAnimation(material_editor_skeleton_name, material_editor_animation_name);
	}

	LibGens::ShaderLibrary* shader_library = NULL;

	if (material_editor_unleashed) {
		shader_library = unleashed_shader_library;
	}
	else {
		shader_library = generations_shader_library;
	}

	buildModel(material_editor_scene_node, material_editor_model, material_editor_model->getName(), material_editor_skeleton_name, material_editor_preview_scene_manager, material_editor_material_library, 0, PREVIEW_MESH_GROUP, false, shader_library);


	material_editor_animation_state = NULL;
	if (material_editor_animation_name.size()) {
		unsigned short attached_objects=material_editor_scene_node->numAttachedObjects();
		for (unsigned short i=0; i<attached_objects; i++) {
			Ogre::Entity *entity=static_cast<Ogre::Entity *>(material_editor_scene_node->getAttachedObject(i));

			if (entity->hasAnimationState(material_editor_animation_name)) {
				material_editor_animation_state = entity->getAnimationState(material_editor_animation_name);
				material_editor_animation_state->setLoop(true);
				material_editor_animation_state->setEnabled(true);
				break;
			}
		}
	}

	material_editor_preview_listener->setAnimationState(material_editor_animation_state);
}


void EditorApplication::rebuildListMaterialEditorGUI() {
	HWND hMaterialList=GetDlgItem(hMaterialEditorDlg, IDL_MATERIAL_MATERIAL_LIST);

	// Cleanup old material list
	if(ListView_GetItemCount(hMaterialList) != 0) {
		ListView_DeleteAllItems(hMaterialList);
		while (ListView_DeleteColumn(hMaterialList, 0) > 0);
		ListView_SetItemCount(hMaterialList, 0);
	}

	// Create new material list
	for (size_t i=0; i<material_editor_materials.size(); i++) {
		char temp[256];
		LV_ITEM Item;
		Item.mask = LVIF_TEXT;
		strcpy(temp, material_editor_materials[i]->getName().c_str());

		Item.pszText = temp;
		Item.cchTextMax = strlen(temp);            
		Item.iSubItem = 0;                           
		Item.lParam = (LPARAM) NULL;                   
		Item.iItem = ListView_GetItemCount(hMaterialList); 
		ListView_InsertItem(hMaterialList, &Item);
	}

	// Populate the shader list
	bool shader_library_unleashed = false;

	if (material_editor_mode == SONICGLVL_MATERIAL_EDITOR_MODE_TERRAIN && current_level != NULL) {
		shader_library_unleashed = (current_level->getGameMode() == LIBGENS_LEVEL_GAME_UNLEASHED);
	}
	else {
		shader_library_unleashed = material_editor_unleashed;
	}

	checkShaderLibrary(shader_library_unleashed ? LIBGENS_LEVEL_GAME_UNLEASHED : LIBGENS_LEVEL_GAME_GENERATIONS);

	material_editor_shader_library = NULL;

	if (shader_library_unleashed) {
		material_editor_shader_library = unleashed_shader_library;
	}
	else {
		material_editor_shader_library = generations_shader_library;
	}

	SendDlgItemMessage(hMaterialEditorDlg, IDC_MATERIAL_SHADER, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);

	if (material_editor_shader_library) {
		for (size_t i = 0; i < material_editor_shader_library->getFileCount(); i++) {
			LibGens::ArFile* ar_file = material_editor_shader_library->getFileByIndex(i);

			if (ar_file->getName().find(".shader-list") != string::npos) {
				SendDlgItemMessage(hMaterialEditorDlg, IDC_MATERIAL_SHADER, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)LibGens::File::nameFromFilenameNoExtension(ar_file->getName()).c_str());
			}
		}
	}
}


void EditorApplication::saveMaterialEditorModelGUI(){
	char* filename = (char*)malloc(1024);
	strcpy(filename, "");

	OPENFILENAME    ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize		= sizeof(ofn);
	ofn.lpstrFilter		= "Generations Model File(.model)\0*.model\0Unleashed Model File(.model)\0*.model\0";
	ofn.nFilterIndex	= material_editor_unleashed ? 2 : 1;
	ofn.lpstrFile		= filename;
	ofn.nMaxFile		= 1024;
	ofn.lpstrTitle		= "Choose where you would like save the model";
	ofn.lpstrFile		= (LPSTR)material_editor_model_filename.c_str();
	ofn.Flags			= OFN_EXPLORER | OFN_PATHMUSTEXIST | 
						  OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT |
						  OFN_ENABLESIZING;

	if (GetSaveFileName(&ofn)) {
		string folder = LibGens::File::folderFromFilename(ofn.lpstrFile);
		material_editor_model->save(ToString(ofn.lpstrFile));

		for (LibGens::Material* mat : material_editor_materials) {
			mat->save(folder + "\\" + mat->getName() + ".material", ofn.nFilterIndex == 2 ? LIBGENS_MATERIAL_ROOT_UNLEASHED : LIBGENS_MATERIAL_ROOT_GENERATIONS);
		}
	}

	free(filename);
}

void EditorApplication::saveMaterialEditorMaterial() {
	if (!material_editor_material)
		return;

	bool save_unleashed = false;

	if (material_editor_mode == SONICGLVL_MATERIAL_EDITOR_MODE_TERRAIN && current_level != NULL) {
		save_unleashed = (current_level->getGameMode() == LIBGENS_LEVEL_GAME_UNLEASHED);
	}
	else {
		save_unleashed = material_editor_unleashed;
	}

	material_editor_material->save(material_editor_library_folder + "\\" + material_editor_material->getName() + ".material", save_unleashed ? LIBGENS_MATERIAL_ROOT_UNLEASHED : LIBGENS_MATERIAL_ROOT_GENERATIONS);
}

void EditorApplication::loadMaterialEditorModelGUI() {
	char *filename = (char *) malloc(1024);
	strcpy(filename, "");

	OPENFILENAME    ofn;
    memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize     = sizeof(ofn);
	ofn.lpstrFilter     = "Model File(.model)\0*.model\0Terrain Model File(.terrain-model)\0*.terrain-model\0";
	ofn.nFilterIndex    = 1;
	ofn.lpstrFile       = filename;
    ofn.nMaxFile        = 1024;
    ofn.lpstrTitle      = "Choose the Model File you want to edit";
    ofn.Flags           = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST |
                          OFN_LONGNAMES     | OFN_EXPLORER |
                          OFN_HIDEREADONLY  | OFN_ENABLESIZING;

    if(GetOpenFileName(&ofn)) {
		chdir(exe_path.c_str());

		if (material_editor_model) {
			cleanMaterialEditorModelGUI();
		}

		// Load Model into memory and fetch the material names
		material_editor_model_filename = ToString(filename);
		material_editor_model = new LibGens::Model(material_editor_model_filename);
		list<string> material_names = material_editor_model->getMaterialNames();

		// Build material library
		material_editor_library_folder = LibGens::File::folderFromFilename(material_editor_model_filename);
		material_editor_material_library = new LibGens::MaterialLibrary(material_editor_library_folder);

		Ogre::ResourceGroupManager::getSingleton().addResourceLocation(material_editor_library_folder, "FileSystem");

		// Enable the List UI
		enableMaterialEditorListGUI();

		// Clear Material List and insert the ones from the model
		clearSelectionMaterialEditorGUI();
		material_editor_materials.clear();
		material_editor_unleashed = true;
		for (list<string>::iterator it=material_names.begin(); it!=material_names.end(); it++) {
			LibGens::Material *mat = material_editor_material_library->getMaterial(*it);

			if (mat) {
				if (mat->getRootNodeType() != LIBGENS_MATERIAL_ROOT_UNLEASHED) {
					material_editor_unleashed = false;
				}
				material_editor_materials.push_back(mat);
			}
		}

		std::sort(material_editor_materials.begin(), material_editor_materials.end(),
			[](LibGens::Material* lhs, LibGens::Material* rhs) {
				return lhs->getName().compare(rhs->getName()) < 0; 
			});

		rebuildListMaterialEditorGUI();
		createPreviewMaterialEditorGUI();
	}

	chdir(exe_path.c_str());
    free(filename);
}

void EditorApplication::copyMaterialEditorTexture(const string& file) const
{
	string destination_folder;

	if (material_editor_mode == SONICGLVL_MATERIAL_EDITOR_MODE_MODEL)
		destination_folder = LibGens::File::folderFromFilename(material_editor_model_filename);

	else if (material_editor_mode == SONICGLVL_MATERIAL_EDITOR_MODE_TERRAIN && editor_application->getCurrentLevel())
		destination_folder = editor_application->getCurrentLevel()->getResourcesFolder();

	if (!destination_folder.empty())
		CopyFile(file.c_str(), (destination_folder + "\\" + LibGens::File::nameFromFilename(file)).c_str(), false);
}

void EditorApplication::pickMaterialEditorTextureGUI() {
	if (!material_editor_texture)
		return;

	char* filename = (char*)malloc(1024);
	strcpy(filename, "");

	OPENFILENAME    ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFilter = "DirectX Texture File(.dds)\0*.dds\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = 1024;
	ofn.lpstrTitle = "Choose the Texture File you want to use";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST |
				OFN_LONGNAMES | OFN_EXPLORER |
				OFN_ENABLESIZING;

	if (GetOpenFileName(&ofn)) {
		chdir(exe_path.c_str());
		string file = ToString(ofn.lpstrFile);
		copyMaterialEditorTexture(file);
		updateEditTextureMaterialEditor(LibGens::File::nameFromFilenameNoExtension(file), true);
	}
	chdir(exe_path.c_str());
	free(filename);
}

void EditorApplication::addMaterialEditorTextureGUI() {
	if (!material_editor_material)
		return;

	char* filename = (char*)malloc(1024);
	strcpy(filename, "");

	OPENFILENAME    ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFilter = "DirectX Texture File(.dds)\0*.dds\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = 1024;
	ofn.lpstrTitle = "Choose the Texture File you want to use";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST |
		OFN_LONGNAMES | OFN_EXPLORER |
		OFN_ENABLESIZING;

	if (GetOpenFileName(&ofn)) {
		chdir(exe_path.c_str());
		string file = ToString(ofn.lpstrFile);
		string internal_name = LibGens::File::nameFromFilenameNoExtension(file);
		char buffer[8];
		sprintf(buffer, "-%04d", material_editor_material->getTextureUnitsSize());
		string unitName = material_editor_material->getName() + ToString(buffer);
		string unit = "diffuse";
		if (internal_name.size() > 3) {
			string fileUnit = internal_name.substr(internal_name.size() - 3, 3);
			if (fileUnit == "spc" || fileUnit == "pec")
				unit = "specular";
			else if (fileUnit == "pow")
				unit = "gloss";
			else if (fileUnit == "nrm")
				unit = "normal";
			else if (fileUnit == "fal")
				unit = "displacement";
			else if (fileUnit == "env" || fileUnit == "ref")
				unit = "reflection";
		}
		LibGens::Texture* tex = new LibGens::Texture(unitName, unit,internal_name);
		material_editor_material->addTextureUnit(tex);

		copyMaterialEditorTexture(file);

		Ogre::Material* ogre_material = Ogre::MaterialManager::getSingleton().getByName(material_editor_material->getExtra(), material_editor_mesh_group).getPointer();

		if (ogre_material) {
			updateMaterialShaderParameters(ogre_material, material_editor_material, !material_editor_material->hasExtraGI(), NULL, material_editor_shader_library);
		}

		updateMaterialEditorTextureList();
	}
	chdir(exe_path.c_str());
	free(filename);
}

void EditorApplication::loadMaterialEditorSkeletonGUI() {
	char *filename = (char *) malloc(1024);
	strcpy(filename, "");

	OPENFILENAME    ofn;
    memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize     = sizeof(ofn);
	ofn.lpstrFilter     = "Havok Skeleton File(.skl.hkx)\0*.skl.hkx\0";
	ofn.nFilterIndex    = 1;
	ofn.lpstrFile       = filename;
    ofn.nMaxFile        = 1024;
    ofn.lpstrTitle      = "Choose the Skeleton File you want to use";
    ofn.Flags           = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST |
                          OFN_LONGNAMES     | OFN_EXPLORER |
                          OFN_HIDEREADONLY  | OFN_ENABLESIZING;

    if(GetOpenFileName(&ofn)) {
		chdir(exe_path.c_str());

		material_editor_skeleton_name = LibGens::File::nameFromFilenameNoExtension(ToString(filename));
		string havok_library_folder = LibGens::File::folderFromFilename(ToString(filename));
		material_editor_animation_name = "";

		havok_enviroment->addFolder(havok_library_folder);
	}

	chdir(exe_path.c_str());
    free(filename);
}



void EditorApplication::loadMaterialEditorAnimationGUI() {
	char *filename = (char *) malloc(1024);
	strcpy(filename, "");

	OPENFILENAME    ofn;
    memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize     = sizeof(ofn);
	ofn.lpstrFilter     = "Havok Animation File(.anm.hkx)\0*.anm.hkx\0";
	ofn.nFilterIndex    = 1;
	ofn.lpstrFile       = filename;
    ofn.nMaxFile        = 1024;
    ofn.lpstrTitle      = "Choose the Animation File you want to use";
    ofn.Flags           = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST |
                          OFN_LONGNAMES     | OFN_EXPLORER |
                          OFN_HIDEREADONLY  | OFN_ENABLESIZING;

    if(GetOpenFileName(&ofn)) {
		chdir(exe_path.c_str());

		material_editor_animation_name = LibGens::File::nameFromFilenameNoExtension(ToString(filename));
		string havok_library_folder = LibGens::File::folderFromFilename(ToString(filename));

		havok_enviroment->addFolder(havok_library_folder);

		rebuildMaterialPreviewNodes();
	}

	chdir(exe_path.c_str());
    free(filename);
}


void EditorApplication::updateMaterialEditorIndex(int selection_index) {
	material_editor_list_selection = selection_index;

	if (material_editor_list_selection != last_material_editor_list_selection) {
		last_material_editor_list_selection = material_editor_list_selection;

		if (material_editor_list_selection != -1) {
			enableMaterialEditorGUI(true);

			if ((material_editor_list_selection < material_editor_materials.size()) && material_editor_materials.size()) {
				material_editor_material = material_editor_materials[material_editor_list_selection];
				updateMaterialEditorInfo();
			}
		}
		else {
			enableMaterialEditorGUI(false);
		}
	}
}


void EditorApplication::updateMaterialEditorTextureIndex(int selection_index) {
	texture_list_selection = selection_index;

	if (texture_list_selection != last_texture_list_selection) {
		last_texture_list_selection = texture_list_selection;

		if (texture_list_selection != -1)
		{
			if (material_editor_material && material_editor_materials.size()) {
				material_editor_texture = material_editor_material->getTextureByIndex(texture_list_selection);
				updateMaterialTextureInfo();
			}
		}
		else {
			clearTextureInfo();
		}
	}
}

void EditorApplication::updateEditParameterMaterialEditor(size_t i, LibGens::Color parameter_color) {
	if (!material_editor_material) return;

	LibGens::Parameter *parameter = material_editor_material->getParameterByIndex(i);
	if (parameter) {
		parameter->color = parameter_color;
	}
	else return;

	Ogre::Material *ogre_material = Ogre::MaterialManager::getSingleton().getByName(material_editor_material->getExtra(), material_editor_mesh_group).getPointer();

	if (ogre_material) {
		updateMaterialShaderParameters(ogre_material, material_editor_material, !material_editor_material->hasExtraGI(), NULL, material_editor_shader_library);
	}
}


void EditorApplication::updateEditShaderMaterialEditor(string shader_name) {
	if (!material_editor_material) return;

	material_editor_material->setShader(shader_name);
	Ogre::Material *ogre_material = Ogre::MaterialManager::getSingleton().getByName(material_editor_material->getExtra(), material_editor_mesh_group).getPointer();

	if (ogre_material) {
		updateMaterialShaderParameters(ogre_material, material_editor_material, !material_editor_material->hasExtraGI(), NULL, material_editor_shader_library);
	}
}

void EditorApplication::updateEditTextureMaterialEditor(string texture_name, bool update_ui) {
	if (!material_editor_texture)
		return;

	material_editor_texture->setName(texture_name);
	Ogre::Material* ogre_material = Ogre::MaterialManager::getSingleton().getByName(material_editor_material->getExtra(), material_editor_mesh_group).getPointer();

	if (ogre_material) {
		updateMaterialShaderParameters(ogre_material, material_editor_material, !material_editor_material->hasExtraGI(), NULL, material_editor_shader_library);
	}

	if (update_ui) {
		SetDlgItemText(hMaterialEditorDlg, IDE_MATERIAL_TEXTURE_FILENAME, material_editor_texture->getName().c_str());
		SetDlgItemText(hMaterialEditorDlg, IDE_MATERIAL_TEXTURE_UNIT, material_editor_texture->getTexset().c_str());
		SetDlgItemText(hMaterialEditorDlg, IDC_MATERIAL_TEXTURE_UNIT_SLOT, material_editor_texture->getUnit().c_str());
		ListView_SetItemText(GetDlgItem(hMaterialEditorDlg, IDL_MATERIAL_TEXTURE_UNIT_LIST), texture_list_selection, 0, (LPSTR)texture_name.c_str());
	}
}

void EditorApplication::updateEditTextureUnitMaterialEditor(string unit_name) {
	if (!material_editor_texture)
		return;

	ListView_SetItemText(GetDlgItem(hMaterialEditorDlg, IDL_MATERIAL_TEXTURE_UNIT_LIST), texture_list_selection, 1, (LPSTR)unit_name.c_str());
	material_editor_texture->setUnit(unit_name);

	Ogre::Material* ogre_material = Ogre::MaterialManager::getSingleton().getByName(material_editor_material->getExtra(), material_editor_mesh_group).getPointer();

	if (ogre_material) {
		updateMaterialShaderParameters(ogre_material, material_editor_material, !material_editor_material->hasExtraGI(), NULL, material_editor_shader_library);
	}
}

INT_PTR CALLBACK MaterialEditorCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	int list_view_index = ListView_GetNextItem(GetDlgItem(hDlg, IDL_MATERIAL_MATERIAL_LIST), -1, LVIS_SELECTED | LVIS_FOCUSED);
	editor_application->updateMaterialEditorIndex(list_view_index);
	int texture_index = ListView_GetNextItem(GetDlgItem(hDlg, IDL_MATERIAL_TEXTURE_UNIT_LIST), -1, LVIS_SELECTED | LVIS_FOCUSED);
	editor_application->updateMaterialEditorTextureIndex(texture_index);

	int n,r,g,b,a;

	switch(msg) {
		case WM_INITDIALOG:
			return true;

		case WM_CLOSE:
			DestroyWindow(hDlg);
			editor_application->clearMaterialEditorGUI();
			return true;

		case WM_COMMAND:
			if(HIWORD(wParam) == CBN_SELCHANGE) { 
				char value_str[1024] = "";
				int nIndex=SendDlgItemMessage(hDlg, LOWORD(wParam), (UINT) CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0);
				SendDlgItemMessage(hDlg, LOWORD(wParam), (UINT)CB_GETLBTEXT, (WPARAM)nIndex, (LPARAM)value_str);

				if (LOWORD(wParam) == IDC_MATERIAL_SHADER)  {
					editor_application->updateEditShaderMaterialEditor(ToString(value_str));
				}
				else if (LOWORD(wParam) == IDC_MATERIAL_TEXTURE_UNIT_SLOT) {
					editor_application->updateEditTextureUnitMaterialEditor(ToString(value_str));
				}
				break;
			}

			if (HIWORD(wParam) == CBN_EDITCHANGE) {
				char value_str[1024] = "";
				GetDlgItemText(hDlg, LOWORD(wParam), value_str, 1024);

				if (LOWORD(wParam) == IDC_MATERIAL_SHADER)  {
					editor_application->updateEditShaderMaterialEditor(ToString(value_str));
				}
				else if (LOWORD(wParam) == IDC_MATERIAL_TEXTURE_UNIT_SLOT) {
					editor_application->updateEditTextureUnitMaterialEditor(ToString(value_str));
				}
				break;
			}

			if (HIWORD(wParam) == EN_CHANGE) {
				if (LOWORD(wParam) == IDE_MATERIAL_TEXTURE_FILENAME) {
					char value_str[1024] = "";
					GetDlgItemText(hDlg, IDE_MATERIAL_TEXTURE_FILENAME, value_str, 1024);
					editor_application->updateEditTextureMaterialEditor(ToString(value_str));
					ListView_SetItemText(GetDlgItem(hDlg, IDL_MATERIAL_TEXTURE_UNIT_LIST), texture_index, 0, (LPSTR)value_str);
				}
				else {
					for (size_t i = 0; i < 10; i++) {
						if (i == 0) { n = IDE_MATERIAL_PARAMETER_1_N; r = IDE_MATERIAL_PARAMETER_1_R; g = IDE_MATERIAL_PARAMETER_1_G; b = IDE_MATERIAL_PARAMETER_1_B; a = IDE_MATERIAL_PARAMETER_1_A; }
						if (i == 1) { n = IDE_MATERIAL_PARAMETER_2_N; r = IDE_MATERIAL_PARAMETER_2_R; g = IDE_MATERIAL_PARAMETER_2_G; b = IDE_MATERIAL_PARAMETER_2_B; a = IDE_MATERIAL_PARAMETER_2_A; }
						if (i == 2) { n = IDE_MATERIAL_PARAMETER_3_N; r = IDE_MATERIAL_PARAMETER_3_R; g = IDE_MATERIAL_PARAMETER_3_G; b = IDE_MATERIAL_PARAMETER_3_B; a = IDE_MATERIAL_PARAMETER_3_A; }
						if (i == 3) { n = IDE_MATERIAL_PARAMETER_4_N; r = IDE_MATERIAL_PARAMETER_4_R; g = IDE_MATERIAL_PARAMETER_4_G; b = IDE_MATERIAL_PARAMETER_4_B; a = IDE_MATERIAL_PARAMETER_4_A; }
						if (i == 4) { n = IDE_MATERIAL_PARAMETER_5_N; r = IDE_MATERIAL_PARAMETER_5_R; g = IDE_MATERIAL_PARAMETER_5_G; b = IDE_MATERIAL_PARAMETER_5_B; a = IDE_MATERIAL_PARAMETER_5_A; }
						if (i == 5) { n = IDE_MATERIAL_PARAMETER_6_N; r = IDE_MATERIAL_PARAMETER_6_R; g = IDE_MATERIAL_PARAMETER_6_G; b = IDE_MATERIAL_PARAMETER_6_B; a = IDE_MATERIAL_PARAMETER_6_A; }
						if (i == 6) { n = IDE_MATERIAL_PARAMETER_7_N; r = IDE_MATERIAL_PARAMETER_7_R; g = IDE_MATERIAL_PARAMETER_7_G; b = IDE_MATERIAL_PARAMETER_7_B; a = IDE_MATERIAL_PARAMETER_7_A; }
						if (i == 7) { n = IDE_MATERIAL_PARAMETER_8_N; r = IDE_MATERIAL_PARAMETER_8_R; g = IDE_MATERIAL_PARAMETER_8_G; b = IDE_MATERIAL_PARAMETER_8_B; a = IDE_MATERIAL_PARAMETER_8_A; }
						if (i == 8) { n = IDE_MATERIAL_PARAMETER_9_N; r = IDE_MATERIAL_PARAMETER_9_R; g = IDE_MATERIAL_PARAMETER_9_G; b = IDE_MATERIAL_PARAMETER_9_B; a = IDE_MATERIAL_PARAMETER_9_A; }
						if (i == 9) { n = IDE_MATERIAL_PARAMETER_10_N; r = IDE_MATERIAL_PARAMETER_10_R; g = IDE_MATERIAL_PARAMETER_10_G; b = IDE_MATERIAL_PARAMETER_10_B; a = IDE_MATERIAL_PARAMETER_10_A; }

						if ((LOWORD(wParam) == r) || (LOWORD(wParam) == g) || (LOWORD(wParam) == b) || (LOWORD(wParam) == a)) {
							float value_r = 0.0f;
							float value_g = 0.0f;
							float value_b = 0.0f;
							float value_a = 0.0f;

							value_r = GetDlgItemFloat(hDlg, r);
							value_g = GetDlgItemFloat(hDlg, g);
							value_b = GetDlgItemFloat(hDlg, b);
							value_a = GetDlgItemFloat(hDlg, a);

							editor_application->updateEditParameterMaterialEditor(i, LibGens::Color(value_r, value_g, value_b, value_a));
							break;
						}
					}
				}
				break;
			}


			switch(LOWORD(wParam)) {
				case IDCANCEL:
					SendMessage(hDlg, WM_CLOSE, 0, 0);
					return true;

				case IDB_MATERIAL_LOAD_MODEL:
					editor_application->loadMaterialEditorModelGUI();
					return true;

				case IDB_MATERIAL_LOAD_SKELETON:
					editor_application->loadMaterialEditorSkeletonGUI();
					return true;

				case IDB_MATERIAL_LOAD_ANIMATION:
					editor_application->loadMaterialEditorAnimationGUI();
					return true;

				case IDB_MATERIAL_SAVE_MODEL:
					editor_application->saveMaterialEditorModelGUI();
					return true;

				case IDB_MATERIAL_SAVE_MATERIAL:
					editor_application->saveMaterialEditorMaterial();
					return true;

				case IDR_MATERIAL_TERRAIN_MODE:
					editor_application->materialEditorTerrainMode();
					return true;

				case IDR_MATERIAL_MODEL_MODE:
					editor_application->materialEditorModelMode();
					return true;
				case IDB_MATERIAL_OPEN_TEXTURE_FILENAME:
					editor_application->pickMaterialEditorTextureGUI();
					return true;

				case IDB_MATERIAL_NEW_TEXTURE_UNIT:
					editor_application->addMaterialEditorTextureGUI();
					return true;

				case IDB_MATERIAL_DELETE_TEXTURE_UNIT:
					editor_application->removeMaterialEditorTexture();
					return true;

				case IDB_MATERIAL_DEFAULTS:
					editor_application->loadMaterialDefaultParams();
					return true;
			}

			break;

		case WM_NOTIFY:
			/* SPIN CONTROL
			if ((LOWORD(wParam) == IDS_EDIT_VECTOR_X) || (LOWORD(wParam) == IDS_EDIT_VECTOR_Y) || (LOWORD(wParam) == IDS_EDIT_VECTOR_Z)) {
				if (((LPNMUPDOWN)lParam)->hdr.code == UDN_DELTAPOS) {
					int delta = ((LPNMUPDOWN)lParam)->iDelta;
				}
			}
			*/

			return false;
	}
	
	return false;
}



INT_PTR CALLBACK MaterialEditorPreviewCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
		case WM_INITDIALOG:
			return true;

		case WM_CLOSE:
			DestroyWindow(hDlg);
			return true;

		case WM_COMMAND:
			return true;

		case WM_NOTIFY:
			return true;
	}
	
	return false;
}