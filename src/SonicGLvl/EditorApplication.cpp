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
#include "ObjectSet.h"
#include "ObjectLibrary.h"
#include "MessageTypes.h"

Ogre::Rectangle2D* mMiniScreen=NULL;

INT_PTR CALLBACK LeftBarCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK RightBarCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK BottomBarCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
void Game_ProcessMessage(PipeClient* client, PipeMessage* msg);

EditorApplication::EditorApplication(void)
{
	hLeftDlg = NULL;
	hBottomDlg = NULL;
	game_client = new PipeClient();
	game_client->AddMessageProcessor(Game_ProcessMessage);
	ghost_data = nullptr;
	isGhostRecording = false;
	checked_shader_library = false;
	current_level = NULL;
}

EditorApplication::~EditorApplication(void) {
	delete game_client;
}

ObjectNodeManager* EditorApplication::getObjectNodeManager()
{
	return object_node_manager;
}

void EditorApplication::selectNode(EditorNode* node)
{
	if (node)
	{
		node->setSelect(true);
		selected_nodes.push_back(node);
		viewport->focusOnPoint(node->getPosition());
	}
}

void EditorApplication::updateSelection() {
	Ogre::Vector3 center=Ogre::Vector3::ZERO;
	Ogre::Quaternion rotation=Ogre::Quaternion::IDENTITY;

	for (list<EditorNode *>::iterator it=selected_nodes.begin(); it!=selected_nodes.end(); it++) {
		center += (*it)->getPosition();
	}
	center /= selected_nodes.size();

	if ((selected_nodes.size() == 1) && !world_transform) {
		rotation = (*selected_nodes.begin())->getRotation();
	}

	axis->setVisible(selected_nodes.size() > 0);
	axis->setRotationFrozen((selected_nodes.size() > 1) || world_transform);
	axis->setPosition(center);
	axis->setRotation(rotation);

	updateNodeVisibility();

	if (editor_mode != EDITOR_NODE_QUERY_VECTOR) {
		updateObjectsPropertiesGUI();
	}

	updateTransformGUI();
}

void EditorApplication::deleteSelection() {
	if (!selected_nodes.size()) return;

	bool msp_deleted = false;

	if (editor_mode == EDITOR_NODE_QUERY_OBJECT) {
		HistoryActionWrapper *wrapper = new HistoryActionWrapper();

		for (list<EditorNode *>::iterator it=selected_nodes.begin(); it!=selected_nodes.end(); it++) {
			// Cast to appropiate types depending on the type of editor node
			if ((*it)->getType() == EDITOR_NODE_OBJECT) {
				ObjectNode *object_node=static_cast<ObjectNode *>(*it);

				LibGens::Object *object=object_node->getObject();
				if (object) {
					LibGens::ObjectSet *object_set=object->getParentSet();
					if (object_set) {
						object_set->eraseObject(object);
						updateLayerControlGUI();
					}

					object_node_manager->hideObjectNode(object, true);
					HistoryActionDeleteObjectNode *action = new HistoryActionDeleteObjectNode(object, object_node_manager);
					wrapper->push(action);

					HistoryActionSelectNode *action_select = new HistoryActionSelectNode((*it), true, false, &selected_nodes);
					(*it)->setSelect(false);
					wrapper->push(action_select);
				}
			}
			else if ((*it)->getType() == EDITOR_NODE_OBJECT_MSP) {
				ObjectMultiSetNode* object_msp_node = static_cast<ObjectMultiSetNode*>(*it);
				HistoryActionSelectNode* action_select = new HistoryActionSelectNode((*it), true, false, &selected_nodes);
				object_msp_node->setSelect(false);
				wrapper->push(action_select);
			}
		}

		removeAllTrajectoryNodes();
		selected_nodes.clear();
		axis->setVisible(false);

		pushHistory(wrapper);
	}
}

void EditorApplication::clearSelection() {
	if (!selected_nodes.size()) return;

	bool stuff_deselected = false;
	HistoryActionWrapper *wrapper = new HistoryActionWrapper();

	for (list<EditorNode *>::iterator it=selected_nodes.begin(); it!=selected_nodes.end(); it++) {
		if ((*it)->isSelected()) {
			stuff_deselected = true;

			HistoryActionSelectNode *action_select = new HistoryActionSelectNode((*it), true, false, &selected_nodes);
			(*it)->setSelect(false);
			wrapper->push(action_select);
		}
	}

	if (stuff_deselected) {
		pushHistory(wrapper);
	}
	else {
		delete wrapper;
	}

	removeAllTrajectoryNodes();
	selected_nodes.clear();
	axis->setVisible(false);
}

void EditorApplication::showSelectionNames() {
	string message = "";
	for (list<EditorNode *>::iterator it=selected_nodes.begin(); it!=selected_nodes.end(); it++) {
		if ((*it)->isSelected()) {
			if ((*it)->getType() == EDITOR_NODE_TERRAIN) {
				TerrainNode *terrain_node = (TerrainNode *) (*it);
				message += terrain_node->getTerrainInstance()->getName() + "\n";
			}
		}
	}

	SHOW_MSG(message.c_str());
}

void EditorApplication::selectAll(int layer_index) {
	bool stuff_selected = false;
	HistoryActionWrapper *wrapper = new HistoryActionWrapper();

	if (editor_mode == EDITOR_NODE_QUERY_OBJECT) {
		list<ObjectNode *> object_nodes = object_node_manager->getObjectNodes();
		for (list<ObjectNode *>::iterator it=object_nodes.begin(); it!=object_nodes.end(); it++) {
			bool isObjectInLayer = true;
			if (layer_index >= 0)
			{
				if (set_mapping.count(layer_index))
				{
					LibGens::ObjectSet* set = set_mapping[layer_index];
					isObjectInLayer = set->hasObject((*it)->getObject());
				}
				else
				{
					isObjectInLayer = false;
				}
			}
			
			if (!(*it)->isSelected() && !(*it)->isForceHidden() && isObjectInLayer) {
				stuff_selected = true;
				HistoryActionSelectNode *action_select = new HistoryActionSelectNode((*it), false, true, &selected_nodes);
				(*it)->setSelect(true);
				wrapper->push(action_select);
				selected_nodes.push_back(*it);
			}
			else if ((*it)->isSelected() && ((*it)->isForceHidden() || !isObjectInLayer))
			{
				stuff_selected = true;
				HistoryActionSelectNode* action_select = new HistoryActionSelectNode((*it), true, false, &selected_nodes);
				(*it)->setSelect(false);
				wrapper->push(action_select);
				selected_nodes.remove(*it);
			}
		}
	}

	if (stuff_selected) {
		pushHistory(wrapper);
	}
	else {
		delete wrapper;
	}

	updateSelection();
}

void EditorApplication::rememberCloningNodes()
{
	for (list<EditorNode*>::iterator it = selected_nodes.begin(); it != selected_nodes.end(); ++it)
	{
		if ((*it)->getType() == EDITOR_NODE_OBJECT)
			cloning_nodes.push_back(*it);
	}
}

list<EditorNode*> EditorApplication::getSelectedNodes()
{
	return selected_nodes;
}

void EditorApplication::cloneSelection() {
	if (!selected_nodes.size()) return;

	list<EditorNode *> nodes_to_clone = selected_nodes;
	clearSelection();
	
	HistoryActionWrapper *wrapper = new HistoryActionWrapper();
	for (list<EditorNode *>::iterator it=nodes_to_clone.begin(); it!=nodes_to_clone.end(); it++) {
		// Cast to appropiate types depending on the type of editor node
		if ((*it)->getType() == EDITOR_NODE_OBJECT) {
			ObjectNode *object_node=static_cast<ObjectNode *>(*it);

			LibGens::Object *object=object_node->getObject();
			if (object) {
				LibGens::Object *new_object = new LibGens::Object(object);

				if (current_level) {
					if (current_level->getLevel()) {
						new_object->setID(current_level->getLevel()->newObjectID());
					}
				}
			
				LibGens::ObjectSet *parent_set = object->getParentSet();
				if (parent_set) {
					parent_set->addObject(new_object);
					updateLayerControlGUI();
				}

				// Create
				ObjectNode *new_object_node = object_node_manager->createObjectNode(new_object);

				// Push to History
				HistoryActionCreateObjectNode *action = new HistoryActionCreateObjectNode(new_object, object_node_manager);
				wrapper->push(action);

				// Add to current selection
				HistoryActionSelectNode *action_select = new HistoryActionSelectNode(new_object_node, false, true, &selected_nodes);
				new_object_node->setSelect(true);
				selected_nodes.push_back(new_object_node);
				wrapper->push(action_select);
			}
		}
	}
	pushHistory(wrapper);

	updateSelection();
}

void EditorApplication::temporaryCloneSelection() {
	if (!selected_nodes.size()) return;

	list<EditorNode*> nodes_to_clone = selected_nodes;
	clearSelection();

	for (list<EditorNode*>::iterator it = nodes_to_clone.begin(); it != nodes_to_clone.end(); it++) {
		// Cast to appropiate types depending on the type of editor node
		if ((*it)->getType() == EDITOR_NODE_OBJECT) {
			ObjectNode* object_node = static_cast<ObjectNode*>(*it);

			LibGens::Object* object = object_node->getObject();
			if (object) {
				LibGens::Object* new_object = new LibGens::Object(object);

				if (current_level) {
					if (current_level->getLevel()) {
						new_object->setID(current_level->getLevel()->newObjectID());
					}
				}

				LibGens::ObjectSet* parent_set = object->getParentSet();
				if (parent_set) {
					parent_set->addObject(new_object);
					updateLayerControlGUI();
				}

				// Create
				ObjectNode* new_object_node = object_node_manager->createObjectNode(new_object);

				// Add to current selection
				new_object_node->setSelect(true);
				selected_nodes.push_back(new_object_node);
			}
		}
	}

	updateSelection();
}

void EditorApplication::translateSelection(Ogre::Vector3 v) {
	for (list<EditorNode *>::iterator it=selected_nodes.begin(); it!=selected_nodes.end(); it++) {
		(*it)->translate(v);
	}
}


void EditorApplication::rotateSelection(Ogre::Quaternion q) {
	if (selected_nodes.size() == 1 || local_rotation) {
		for (list<EditorNode*>::iterator it = selected_nodes.begin(); it != selected_nodes.end(); ++it) {
			(*it)->rotate(q);
		}
		//node->setRotation(q);
	}
	else {
		Ogre::Matrix4 matrix;
		matrix.makeTransform(axis->getPosition(), Ogre::Vector3::UNIT_SCALE, q);

		for (list<EditorNode *>::iterator it=selected_nodes.begin(); it!=selected_nodes.end(); it++) {
			Ogre::Vector3 new_pos = matrix * ((*it)->getPosition() - axis->getPosition());
			(*it)->setPosition(new_pos);
			//(*it)->rotate(q);
		}
	}
}


void EditorApplication::setSelectionRotation(Ogre::Quaternion q) {
	if (selected_nodes.size() == 1) {
		EditorNode *node = *selected_nodes.begin();
		node->setRotation(q);
	}
}


void EditorApplication::rememberSelection(bool mode) {
	for (list<EditorNode *>::iterator it=selected_nodes.begin(); it!=selected_nodes.end(); it++) {
		if (!mode) (*it)->rememberPosition();
		else {
			if (selected_nodes.size() > 1) (*it)->rememberPosition();
			(*it)->rememberRotation();
		}
	}
}

void EditorApplication::makeHistorySelection(bool mode) {
	if (current_property_index < (int)current_properties_types.size()) return;

	HistoryActionWrapper *wrapper = new HistoryActionWrapper();
	int index = 0;
	bool is_list = current_properties_types[current_property_index] == LibGens::OBJECT_ELEMENT_VECTOR_LIST;
	for (list<EditorNode *>::iterator it=selected_nodes.begin(); it!=selected_nodes.end(); it++) {
		if (!mode) {
			HistoryActionMoveNode *action = new HistoryActionMoveNode((*it), (*it)->getLastPosition(), (*it)->getPosition());
			wrapper->push(action);
			if (editor_mode == EDITOR_NODE_QUERY_VECTOR) {
				VectorNode* vector_node = static_cast<VectorNode*>(*it);
				if (!hLookAtPointDlg)
				{
					while (property_vector_nodes[index] != vector_node)
						++index;

					updateEditPropertyVectorGUI(index, is_list);
				}
				else
				{
					updateLookAtVectorGUI();
				}
			}
		}
		else {
			// Push only a rotation history if it's only one node. 
			// If it's more, push a Rotation/Move wrapper
			if (selected_nodes.size() == 1) {
				HistoryActionRotateNode *action = new HistoryActionRotateNode((*it), (*it)->getLastRotation(), (*it)->getRotation());
				wrapper->push(action);
			}
			else {
				HistoryActionWrapper *sub_wrapper = new HistoryActionWrapper();

				HistoryActionMoveNode *action_mov   = new HistoryActionMoveNode((*it), (*it)->getLastPosition(), (*it)->getPosition());
				sub_wrapper->push(action_mov);

				HistoryActionRotateNode *action_rot = new HistoryActionRotateNode((*it), (*it)->getLastRotation(), (*it)->getRotation());
				sub_wrapper->push(action_rot);

				wrapper->push(sub_wrapper);
			}
		}
		index = 0;
	}

	if (is_list && editor_mode == EDITOR_NODE_QUERY_VECTOR)
	{
		updateEditPropertyVectorList(temp_property_vector_list);
		if (hEditPropertyDlg && isVectorListSelectionValid())
		{
			Ogre::Vector3 v = property_vector_nodes[current_vector_list_selection]->getPosition();
			SetDlgItemText(hEditPropertyDlg, IDE_EDIT_VECTOR_LIST_X, ToString<float>(v.x).c_str());
			SetDlgItemText(hEditPropertyDlg, IDE_EDIT_VECTOR_LIST_Y, ToString<float>(v.y).c_str());
			SetDlgItemText(hEditPropertyDlg, IDE_EDIT_VECTOR_LIST_Z, ToString<float>(v.z).c_str());
		}
	}
	pushHistory(wrapper);
}


void EditorApplication::undoHistory() {
	if (editor_mode == EDITOR_NODE_QUERY_VECTOR) {
		if (hLookAtPointDlg)
		{
			look_at_vector_history->undo();
			updateLookAtVectorGUI();
		}
		else
		{
			property_vector_history->undo();
			bool is_list = current_properties_types[current_property_index] == LibGens::OBJECT_ELEMENT_VECTOR_LIST;

			for (int index = 0; index < property_vector_nodes.size(); ++index)
				updateEditPropertyVectorGUI(index, is_list);
			if (is_list)
			{
				updateEditPropertyVectorList(temp_property_vector_list);
				if (hEditPropertyDlg && isVectorListSelectionValid())
				{
					Ogre::Vector3 v = property_vector_nodes[current_vector_list_selection]->getPosition();
					SetDlgItemText(hEditPropertyDlg, IDE_EDIT_VECTOR_LIST_X, ToString<float>(v.x).c_str());
					SetDlgItemText(hEditPropertyDlg, IDE_EDIT_VECTOR_LIST_Y, ToString<float>(v.y).c_str());
					SetDlgItemText(hEditPropertyDlg, IDE_EDIT_VECTOR_LIST_Z, ToString<float>(v.z).c_str());
				}
			}
		}
	}
	else history->undo();
}


void EditorApplication::redoHistory() {
	if (editor_mode == EDITOR_NODE_QUERY_VECTOR) {
		if (hLookAtPointDlg)
		{
			look_at_vector_history->redo();
			updateLookAtVectorGUI();
		}
		else
		{
			property_vector_history->redo();
			bool is_list = current_properties_types[current_property_index] == LibGens::OBJECT_ELEMENT_VECTOR_LIST;

			for (int index = 0; index < property_vector_nodes.size(); ++index)
				updateEditPropertyVectorGUI(index, is_list);
			if (is_list)
			{
				updateEditPropertyVectorList(temp_property_vector_list);
				if (hEditPropertyDlg && isVectorListSelectionValid())
				{
					Ogre::Vector3 v = property_vector_nodes[current_vector_list_selection]->getPosition();
					SetDlgItemText(hEditPropertyDlg, IDE_EDIT_VECTOR_LIST_X, ToString<float>(v.x).c_str());
					SetDlgItemText(hEditPropertyDlg, IDE_EDIT_VECTOR_LIST_Y, ToString<float>(v.y).c_str());
					SetDlgItemText(hEditPropertyDlg, IDE_EDIT_VECTOR_LIST_Z, ToString<float>(v.z).c_str());
				}
			}
		}
	}
	else history->redo();
}


void EditorApplication::pushHistory(HistoryAction *action) {
	if (editor_mode == EDITOR_NODE_QUERY_VECTOR)
	{
		property_vector_history->push(action);
		look_at_vector_history->push(action);
	}
	else history->push(action);
}


void EditorApplication::toggleWorldTransform() {
	world_transform = !world_transform;

	// Update WinAPI Menu Check
	const int viewMenuPos=2;
	HMENU hViewSubMenu=GetSubMenu(hMenu, viewMenuPos);

	if (hViewSubMenu) {
		CheckMenuItem(hViewSubMenu, IMD_WORLD_TRANSFORM, (world_transform ? MF_CHECKED : MF_UNCHECKED));
	}
}


void EditorApplication::togglePlacementSnap() {
	if (placement_grid_snap > 0.0f) {
		placement_grid_snap = 0.0f;
	}
	else {
		placement_grid_snap = 0.5f;
	}

	// Update WinAPI Menu Check
	const int viewMenuPos=2;
	HMENU hViewSubMenu=GetSubMenu(hMenu, viewMenuPos);

	if (hViewSubMenu) {
		CheckMenuItem(hViewSubMenu, IMD_PLACEMENT_SNAP, ((placement_grid_snap > 0.0f) ? MF_CHECKED : MF_UNCHECKED));
	}
}

void EditorApplication::toggleLocalRotation() {
	local_rotation = !local_rotation;

	const int viewMenuPos = 2;
	HMENU hViewSubMenu = GetSubMenu(hMenu, viewMenuPos);

	if (hViewSubMenu) {
		CheckMenuItem(hViewSubMenu, IMD_LOCAL_ROTATION, (local_rotation ? MF_CHECKED : MF_UNCHECKED));
	}
}

void EditorApplication::toggleRotationSnap() {
	axis->setRotationSnap(!axis->isRotationSnap());

	const int viewMenuPos = 2;
	HMENU hViewSubMenu = GetSubMenu(hMenu, viewMenuPos);

	if (hViewSubMenu) {
		CheckMenuItem(hViewSubMenu, IMD_ROTATION_SNAP, (axis->isRotationSnap() ? MF_CHECKED : MF_UNCHECKED));
	}
}

void EditorApplication::snapToClosestPath() {
	if (!current_level || selected_nodes.empty()) {
		return;
	}

	vector<float> closest_distances(selected_nodes.size(), FLT_MAX);
	vector<LibGens::Vector3> closest_positions(selected_nodes.size());

	for (LibGens::Path *path : current_level->getLevel()->getPaths()) {
		LibGens::PathNodeList path_node_list = path->getNodes();

		for (auto& pair : path_node_list) {
			size_t editor_node_index = 0;

			for (EditorNode *editor_node : selected_nodes) {
				if ((editor_node->getType() == EDITOR_NODE_OBJECT) || (editor_node->getType() == EDITOR_NODE_OBJECT_MSP)) {
					Ogre::Vector3 position = editor_node->getPosition();
					float closest_distance = FLT_MAX;
					LibGens::Vector3 closest_position = pair.first->findClosestPoint(pair.second, LibGens::Vector3(position.x, position.y, position.z), &closest_distance);

					if (closest_distance < closest_distances[editor_node_index]) {
						closest_distances[editor_node_index] = closest_distance;
						closest_positions[editor_node_index] = closest_position;
					}
				}

				++editor_node_index;
			}
		}
	}

	HistoryActionWrapper *wrapper = new HistoryActionWrapper();
	size_t editor_node_index = 0;

	for (EditorNode *editor_node : selected_nodes) {
		if (closest_distances[editor_node_index] != FLT_MAX) {
			LibGens::Vector3 closest_position = closest_positions[editor_node_index];

			Ogre::Vector3 previous_position = editor_node->getPosition();
			Ogre::Vector3 new_position(closest_position.x, closest_position.y, closest_position.z);

			editor_node->setPosition(new_position);

			HistoryActionMoveNode *action_move = new HistoryActionMoveNode(editor_node, previous_position, new_position);
			wrapper->push(action_move);
		}

		++editor_node_index;
	}

	pushHistory(wrapper);
	updateSelection();
}

void EditorApplication::createScene(void) {
	// Initialize LibGens Managers
	havok_enviroment = new LibGens::HavokEnviroment(100 * 1024 * 1024);
	fbx_manager      = new LibGens::FBXManager();

	// Initialize Editor Managers
	havok_property_database    = new LibGens::HavokPropertyDatabase(SONICGLVL_HAVOK_PROPERTY_DATABASE_PATH);
	history                    = new History();
	property_vector_history    = new History();
	look_at_vector_history     = new History();
	level_database             = new EditorLevelDatabase(SONICGLVL_LEVEL_DATABASE_PATH);
	material_library           = new LibGens::MaterialLibrary(SONICGLVL_RESOURCES_PATH);
	model_library              = new LibGens::ModelLibrary(SONICGLVL_RESOURCES_PATH);
	generations_shader_library = new LibGens::ShaderLibrary(SONICGLVL_SHADERS_PATH);
	unleashed_shader_library   = new LibGens::ShaderLibrary(SONICGLVL_SHADERS_PATH);
	uv_animation_library       = new LibGens::UVAnimationLibrary(SONICGLVL_RESOURCES_PATH);
	generations_library        = new LibGens::ObjectLibrary(SONICGLVL_LIBRARY_PATH);
	unleashed_library          = new LibGens::ObjectLibrary(SONICGLVL_LIBRARY_PATH);
	library                    = generations_library;
	animations_list            = new EditorAnimationsList();

	bool loaded_generations_shader_library = 
		generations_shader_library->loadShaderArchive("shader_r.ar.00") &&
		generations_shader_library->loadShaderArchive("shader_r_add.ar.00");

	if (!loaded_generations_shader_library) {
		delete generations_shader_library;
		generations_shader_library = NULL;
	}

	bool loaded_unleashed_shader_library = 
		unleashed_shader_library->loadShaderArchive("shader.ar") &&
		unleashed_shader_library->loadShaderArchive("shader_d3d9.ar");

	if (!loaded_unleashed_shader_library) {
		delete unleashed_shader_library;
		unleashed_shader_library = NULL;
	}

	generations_library->loadDatabase(SONICGLVL_GENERATIONS_OBJECTS_DATABASE_PATH);
	unleashed_library->loadDatabase(SONICGLVL_UNLEASHED_OBJECTS_DATABASE_PATH);

	configuration    = new EditorConfiguration();
	configuration->load(SONICGLVL_CONFIGURATION_FILE);

	object_production = new LibGens::ObjectProduction();
	object_production->load(configuration->getObjectProductionPath());

	havok_enviroment->addFolder(SONICGLVL_RESOURCES_PATH);

	// Initialize WinAPI Interface
	hMenu = LoadMenu(NULL, MAKEINTRESOURCE(IDR_TOOLMENU));
	SetMenu(hwnd, hMenu);

	hLeftDlg=CreateDialog(NULL, MAKEINTRESOURCE(IDD_LEFT_DIALOG), hwnd, LeftBarCallback);
	SetParent(hLeftDlg, hwnd);

	hRightDlg =CreateDialog(NULL, MAKEINTRESOURCE(IDD_RIGHT_DIALOG), hwnd, RightBarCallback);
	SetParent(hRightDlg, hwnd);

	hBottomDlg=CreateDialog(NULL, MAKEINTRESOURCE(IDD_BOTTOM_DIALOG), hwnd, BottomBarCallback);
	SetParent(hBottomDlg, hwnd);

	hEditPropertyDlg = NULL;
	hMaterialEditorDlg = NULL;
	hPhysicsEditorDlg = NULL;
	hMultiSetParamDlg = NULL;
	hFindObjectDlg = NULL;
	hLookAtPointDlg = NULL;
	
	updateVisibilityGUI();
	updateObjectCategoriesGUI();
	updateObjectsPaletteGUI();
	createObjectsPropertiesGUI();
	createLayerControlGUI();

	current_category_index     = 0;
	palette_cloning_mode       = false;
	ignore_mouse_clicks_frames = 0;
	last_palette_selection     = NULL;
	current_palette_selection  = NULL;
	current_single_property_object = NULL;
	history_edit_property_wrapper = NULL;
	cloning_mode = SONICGLVL_MULTISETPARAM_MODE_CLONE;
	is_pick_target = false;
	is_pick_target_position = false;
	is_update_look_at_vector = 

	// Set up Scene Managers
	scene_manager = root->createSceneManager("OctreeSceneManager");
	axis_scene_manager = root->createSceneManager(Ogre::ST_GENERIC);
	
	// Set up Node Managers
	object_node_manager = new ObjectNodeManager(scene_manager, model_library, material_library, object_production);

	scene_manager->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));
	axis_scene_manager->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));

	viewport = new EditorViewport(scene_manager, axis_scene_manager, window, SONICGLVL_CAMERA_NAME);
	axis = new EditorAxis(axis_scene_manager);

	color_listener = new ColorListener(scene_manager);
	depth_listener = new DepthListener(scene_manager);

	/*
	Ogre::TexturePtr rtt_texture = Ogre::TextureManager::getSingleton().createManual("ColorTex", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, window->getWidth()/2.0, window->getHeight()/2.0, 0, Ogre::PF_R8G8B8, Ogre::TU_RENDERTARGET);
	color_texture = rtt_texture->getBuffer()->getRenderTarget();
	color_texture->addViewport(viewport->getCamera());
	color_texture->getViewport(0)->setClearEveryFrame(true);
	color_texture->getViewport(0)->setBackgroundColour(Ogre::ColourValue::Black);
	color_texture->getViewport(0)->setOverlaysEnabled(false);
	color_texture->addListener(color_listener);

	rtt_texture = Ogre::TextureManager::getSingleton().createManual("DepthTex", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, window->getWidth()/2.0, window->getHeight()/2.0, 0, Ogre::PF_FLOAT32_R, Ogre::TU_RENDERTARGET);
	depth_texture = rtt_texture->getBuffer()->getRenderTarget();
	Ogre::Viewport *depthViewport=depth_texture->addViewport(viewport->getCamera());
	depth_texture->getViewport(0)->setClearEveryFrame(true);
	depth_texture->getViewport(0)->setBackgroundColour(Ogre::ColourValue::White);
	depth_texture->getViewport(0)->setOverlaysEnabled(false);
	depth_texture->addListener(depth_listener);
	*/

	global_illumination_listener = new GlobalIlluminationListener();
	//global_illumination_listener->setPassToIgnore(depth_listener->getDepthPass());
	scene_manager->addRenderObjectListener(global_illumination_listener); 

	current_node = NULL;
	editor_mode = EDITOR_NODE_QUERY_OBJECT;
	world_transform = false;
	
	terrain_streamer = NULL;
	terrain_update_counter = 0;
	current_level = NULL;
	ghost_node = NULL;
	camera_manager = NULL;
	global_directional_light = NULL;
	placement_grid_snap = 0.0f;
	dragging_mode = 0;

	farPlaneChange = 0.0f;

	// Initialize Material Editor Variables
	material_editor_mode = SONICGLVL_MATERIAL_EDITOR_MODE_MODEL;
	material_editor_material_library = NULL;
}

void EditorApplication::windowResized(Ogre::RenderWindow* rw) {
	BaseApplication::windowResized(rw);

	int left_window_height=screen_height-SONICGLVL_GUI_BOTTOM_HEIGHT+1;

	// Move Windows
	if (hLeftDlg)   MoveWindow(hLeftDlg,   0, 0, SONICGLVL_GUI_LEFT_WIDTH, left_window_height, true);
	if (hRightDlg)	MoveWindow(hRightDlg, screen_width - SONICGLVL_GUI_RIGHT_WIDTH, 0, SONICGLVL_GUI_RIGHT_WIDTH, screen_height, true);
	if (hBottomDlg) MoveWindow(hBottomDlg, 0, screen_height-SONICGLVL_GUI_BOTTOM_HEIGHT, SONICGLVL_GUI_LEFT_WIDTH, SONICGLVL_GUI_BOTTOM_HEIGHT+1, true);
	
	// Move Left Bar Elements
	RECT temp_rect;

	auto fnResize = [this, &temp_rect](HWND hWND, float top_y, float x, float y, float width, float height)
	{
		temp_rect.left = x;
		temp_rect.top = y;
		temp_rect.right = width + temp_rect.left;
		temp_rect.bottom = height + temp_rect.top;
		MapDialogRect(hLeftDlg, &temp_rect);
		MoveWindow(hWND, temp_rect.left, temp_rect.top + top_y, temp_rect.right - temp_rect.left, temp_rect.bottom - temp_rect.top, true);
		temp_rect.top += top_y;
		temp_rect.bottom += top_y;
		InvalidateRect(hLeftDlg, &temp_rect, true);
	};

	int layer_control_y_coordinate = left_window_height - 245;
	fnResize(GetDlgItem(hLeftDlg, IDG_LAYER_GROUP), layer_control_y_coordinate, 2, 0, 181, 148);
	fnResize(GetDlgItem(hLeftDlg, IDT_LAYER_CURRENT), layer_control_y_coordinate, 7, 12, 64, 14);
	fnResize(GetDlgItem(hLeftDlg, IDC_LAYER_CURRENT), layer_control_y_coordinate, 60, 11, 119, 14);
	fnResize(GetDlgItem(hLeftDlg, IDL_LAYER_LIST), layer_control_y_coordinate, 7, 27, 173, 100);
	fnResize(GetDlgItem(hLeftDlg, IDB_LAYER_NEW), layer_control_y_coordinate, 7, 131, 85, 14);
	fnResize(GetDlgItem(hLeftDlg, IDB_LAYER_DELETE), layer_control_y_coordinate, 95, 131, 85, 14);

	HWND hHelpGroup = GetDlgItem(hLeftDlg, IDG_HELP_GROUP);
	HWND hHelpText  = GetDlgItem(hLeftDlg, IDT_HELP_DESCRIPTION);
	int help_y_coordinate = layer_control_y_coordinate - 90;

	// Help Group
	temp_rect.left = 2;
	temp_rect.top = 0;
	temp_rect.right = 181 + temp_rect.left;
	temp_rect.bottom = 52 + temp_rect.top;
	MapDialogRect(hLeftDlg, &temp_rect);
	MoveWindow(hHelpGroup, temp_rect.left, temp_rect.top + help_y_coordinate, temp_rect.right - temp_rect.left, temp_rect.bottom - temp_rect.top, true);
	temp_rect.top += help_y_coordinate;
	temp_rect.bottom += help_y_coordinate;
	InvalidateRect(hLeftDlg, &temp_rect, true);

	// Help Text
	temp_rect.left = 7;
	temp_rect.top = 11;
	temp_rect.right = 173 + temp_rect.left;
	temp_rect.bottom = 37 + temp_rect.top;
	MapDialogRect(hLeftDlg, &temp_rect);
	MoveWindow(hHelpText, temp_rect.left, temp_rect.top + help_y_coordinate, temp_rect.right - temp_rect.left, temp_rect.bottom - temp_rect.top, true);
	temp_rect.top += help_y_coordinate;
	temp_rect.bottom += help_y_coordinate;
	InvalidateRect(hLeftDlg, &temp_rect, true);

	HWND hPaletteGroup      = GetDlgItem(hLeftDlg, IDG_PALETTE_GROUP);
	HWND hPaletteList       = GetDlgItem(hLeftDlg, IDL_PALETTE_LIST);
	int left_window_palette_properties_height = help_y_coordinate - 5;

	// Palette Group
	temp_rect.left = 2;
	temp_rect.top = 0;
	temp_rect.right = 181 + temp_rect.left;
	temp_rect.bottom = 0 + temp_rect.top;
	MapDialogRect(hLeftDlg, &temp_rect);
	MoveWindow(hPaletteGroup, temp_rect.left, temp_rect.top, temp_rect.right - temp_rect.left, temp_rect.bottom - temp_rect.top + left_window_palette_properties_height, true);
	temp_rect.bottom += left_window_palette_properties_height;
	InvalidateRect(hLeftDlg, &temp_rect, true);

	// Palette List
	temp_rect.left = 6;
	temp_rect.top = 41;
	temp_rect.right = 174 + temp_rect.left;
	temp_rect.bottom = 0 + temp_rect.top;
	MapDialogRect(hLeftDlg, &temp_rect);
	MoveWindow(hPaletteList, temp_rect.left, temp_rect.top, temp_rect.right - temp_rect.left, temp_rect.bottom - temp_rect.top + left_window_palette_properties_height - 74, true);
	temp_rect.bottom += left_window_palette_properties_height - 74;
	InvalidateRect(hLeftDlg, &temp_rect, true);

	/*HWND hPropertiesGroup = GetDlgItem(hLeftDlg, IDG_PROPERTIES_GROUP);
	HWND hPropertiesList  = GetDlgItem(hLeftDlg, IDL_PROPERTIES_LIST);

	int properties_y_coordinate= 93 + left_window_palette_properties_height;
	// Properties Group
	temp_rect.left = 2;
	temp_rect.top = 0;
	temp_rect.right = 181 + temp_rect.left;
	temp_rect.bottom = 0 + temp_rect.top;
	MapDialogRect(hLeftDlg, &temp_rect);
	MoveWindow(hPropertiesGroup, temp_rect.left, temp_rect.top + properties_y_coordinate, temp_rect.right - temp_rect.left, temp_rect.bottom - temp_rect.top + left_window_palette_properties_height - 8, true);
	temp_rect.top += properties_y_coordinate;
	temp_rect.bottom += properties_y_coordinate;
	temp_rect.bottom += left_window_palette_properties_height - 8;
	InvalidateRect(hLeftDlg, &temp_rect, true);

	// Properties List
	temp_rect.left = 6;
	temp_rect.top = 11;
	temp_rect.right = 174 + temp_rect.left;
	temp_rect.bottom = 0 + temp_rect.top;
	MapDialogRect(hLeftDlg, &temp_rect);
	MoveWindow(hPropertiesList, temp_rect.left, temp_rect.top + properties_y_coordinate, temp_rect.right - temp_rect.left, temp_rect.bottom - temp_rect.top + left_window_palette_properties_height - 33, true);
	temp_rect.top += properties_y_coordinate;
	temp_rect.bottom += properties_y_coordinate;
	temp_rect.bottom += left_window_palette_properties_height - 33;
	InvalidateRect(hLeftDlg, &temp_rect, true);*/
	
	// Resize Viewport
	float left  = (float)SONICGLVL_GUI_LEFT_WIDTH / (float)screen_width;
	float top   = 0.0f;
	float width = (float)(screen_width  - SONICGLVL_GUI_LEFT_WIDTH - SONICGLVL_GUI_RIGHT_WIDTH) / (float)screen_width;
	float height= (float)(screen_height) / (float)screen_height;
	/*
	float left   = 0.0f;
	float top    = 0.0f;
	float width  = 1.0f;
	float height = 1.0f;
	*/

	viewport->resize(left, top, width, height);
}


bool EditorApplication::keyPressed(const OIS::KeyEvent &arg) {
	if (axis->isHolding()) return true;
	viewport->keyPressed(arg);

	bool regular_mode = isRegularMode();

	if(arg.key == OIS::KC_NUMPAD4) {
		farPlaneChange = -1.0f;
	}

	if(arg.key == OIS::KC_NUMPAD6) {
		farPlaneChange = 1.0f;
	}

	if(arg.key == OIS::KC_NUMPAD8) {
		farPlaneChange = 10.0f;
	}

	if(arg.key == OIS::KC_NUMPAD2) {
		farPlaneChange = -10.0f;
	}

	// Quit Special Placement Modes
	if(arg.key == OIS::KC_ESCAPE) {
		if (isPalettePreviewActive()) {
			clearObjectsPalettePreviewGUI();
		}

		if (editor_mode == EDITOR_NODE_QUERY_VECTOR) {
			closeVectorQueryMode();
		}

		if (editor_mode == EDITOR_NODE_QUERY_NODE) {
			closeVectorQueryMode();
		}

		if (is_pick_target)
		{
			openQueryTargetMode(false);
		}

		if (is_pick_target_position)
		{
			queryLookAtObject(false);
		}
	}

	// Regular Mode Shorcuts
	if (regular_mode && inFocus() && !viewport->isMoving()) {
		if(arg.key == OIS::KC_DELETE) {
			deleteSelection();
			updateSelection();
		}

		if (keyboard->isModifierDown(OIS::Keyboard::Ctrl)) {
			if(arg.key == OIS::KC_C) {
				copySelection();
			}

			if(arg.key == OIS::KC_V) {
				clearSelection();
				pasteSelection();
			}

			if(arg.key == OIS::KC_P) {
				if (ghost_node) ghost_node->setPlay(true);
			}

			if(arg.key == OIS::KC_R) {
				if (ghost_node) {
					ghost_node->setPlay(false);
					ghost_node->setTime(0);
				}
			}

			if(arg.key == OIS::KC_F) {
				if (!hFindObjectDlg)
					openFindGUI();
			}

			if(arg.key == OIS::KC_D) {
				clearSelection();
				updateSelection();
			}

			if(arg.key == OIS::KC_Z) {
				undoHistory();
				updateSelection();
			}

			if(arg.key == OIS::KC_Y) {
				redoHistory();
				updateSelection();
			}

			if(arg.key == OIS::KC_E) {
				toggleWorldTransform();
				updateSelection();
			}

			if(arg.key == OIS::KC_I) {
				//SHOW_MSG(ToString(farPlane).c_str());
			}

			if(arg.key == OIS::KC_A) {
				//saveXNAnimation();
				selectAll();
			}

			if(arg.key == OIS::KC_T) {
				clearSelection();
				updateSelection();
				editor_mode = (editor_mode == EDITOR_NODE_QUERY_TERRAIN ? EDITOR_NODE_QUERY_OBJECT : EDITOR_NODE_QUERY_TERRAIN);
			}

			if(arg.key == OIS::KC_I) {
				showSelectionNames();
			}

			if(arg.key == OIS::KC_G) {
				setupGhost();
				clearSelection();
				updateSelection();
				editor_mode = (editor_mode == EDITOR_NODE_QUERY_GHOST ? EDITOR_NODE_QUERY_OBJECT : EDITOR_NODE_QUERY_GHOST);
			}

			if (arg.key == OIS::KC_O) {
				editor_application->openLevelGUI();
			}

			if (arg.key == OIS::KC_S)
			{
				editor_application->saveLevelDataGUI();
			}
			if (arg.key == OIS::KC_R)
			{
				if (editor_mode == EDITOR_NODE_OBJECT || EDITOR_NODE_QUERY_GHOST)
				{
					toggleRotationSnap();
					updateSelection();
				}
			}
		}
		else if (keyboard->isModifierDown(OIS::Keyboard::Alt))
		{
			if (arg.key == OIS::KC_F) {
				if (camera_manager) {
					camera_manager->setForceCamera(!camera_manager->getForceCamera());
				}
			}

			if (arg.key == OIS::KC_G) 
			{
				if (editor_mode == EDITOR_NODE_QUERY_GHOST)
				{
					ghost_node->setPosition(Ogre::Vector3(viewport->getCamera()->getPosition() + viewport->getCamera()->getDirection() * 10));
					updateSelection();
				}
			}
		}
		else {
			if (arg.key == OIS::KC_T) {
				axis->setMode(false);
			}
			if (arg.key == OIS::KC_R) {
				axis->setMode(true);
			}
		}
	}

	// Global Mode Shortcuts
	if (keyboard->isModifierDown(OIS::Keyboard::Ctrl)) {
		if(arg.key == OIS::KC_1) {
			editor_application->toggleNodeVisibility(EDITOR_NODE_OBJECT);
			editor_application->toggleNodeVisibility(EDITOR_NODE_OBJECT_MSP);
		}

		if(arg.key == OIS::KC_2) {
			editor_application->toggleNodeVisibility(EDITOR_NODE_TERRAIN);
		}

		if(arg.key == OIS::KC_3) {
			editor_application->toggleNodeVisibility(EDITOR_NODE_TERRAIN_AUTODRAW);
		}

		if(arg.key == OIS::KC_4) {
			editor_application->toggleNodeVisibility(EDITOR_NODE_HAVOK);
		}

		if(arg.key == OIS::KC_5) {
			editor_application->toggleNodeVisibility(EDITOR_NODE_PATH);
		}

		if(arg.key == OIS::KC_6) {
			editor_application->toggleNodeVisibility(EDITOR_NODE_GHOST);
		}
		if (arg.key == OIS::KC_O)
		{
			editor_application->openLevelGUI();
		}
	}

    return true;
}


bool EditorApplication::keyReleased(const OIS::KeyEvent &arg) {
	viewport->keyReleased(arg);

	if(arg.key == OIS::KC_NUMPAD4) {
		farPlaneChange = 0;
	}

	if(arg.key == OIS::KC_NUMPAD6) {
		farPlaneChange = 0;
	}

	if(arg.key == OIS::KC_NUMPAD8) {
		farPlaneChange = 0;
	}

	if(arg.key == OIS::KC_NUMPAD2) {
		farPlaneChange = 0;
	}

    return true;
}


bool EditorApplication::mouseMoved(const OIS::MouseEvent &arg) {
	viewport->setQueryFlags(editor_mode);

	if (!axis->isHolding()) {
		viewport->mouseMoved(arg);
	}

	if (editor_mode == EDITOR_NODE_QUERY_NODE) {
		if (viewport->isMouseInLocalScreen(arg)) {
			global_cursor_state = 3;
		}
		else {
			global_cursor_state = 0;
		}
	}
	else if (!isPalettePreviewActive()) {
		Ogre::Entity *entity=viewport->getCurrentEntity();
		if (entity) {
			Ogre::SceneNode *node=entity->getParentSceneNode();
			if (node) {
				Ogre::Any ptr_container=node->getUserObjectBindings().getUserAny(EDITOR_NODE_BINDING);
				if (!ptr_container.isEmpty()) {
					EditorNode *editor_node=Ogre::any_cast<EditorNode *>(ptr_container);
					if (current_node && (editor_node != current_node)) current_node->setHighlight(false);
					current_node = editor_node;
				}
			}
			else {
				if (current_node) current_node->setHighlight(false);
				current_node = NULL;
			}
		}
		else {
			if (current_node) current_node->setHighlight(false);
			current_node = NULL;
		}

		if (current_node) current_node->setHighlight(true);
	
		if (axis->mouseMoved(viewport, arg)) {
			if (!axis->getMode())
				translateSelection(axis->getTranslate());
			else
				rotateSelection(axis->getRotate());

			updateTransformGUI();
		}

		if (axis->isHighlighted()) {
			global_cursor_state = 2;
		}
		else if (current_node) {
			global_cursor_state = 1;
		}
		else {
			global_cursor_state = 0;
		}
	}
	else {
		mouseMovedObjectsPalettePreview(arg);

		if (viewport->isMouseInLocalScreen(arg)) {
			global_cursor_state = 2;
		}
		else {
			global_cursor_state = 0;
		}
	}

    return true;
}

bool EditorApplication::mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id) {
	// Only register mouse clicks if it's inside the viewport
	if (viewport->isMouseInLocalScreen(arg)) {
		focus();

		if (ignore_mouse_clicks_frames) {
			return true;
		}

		if (isRegularMode()) {
			if (axis->mousePressed(viewport, arg, id)) {
				dragging_mode = 0;

				if (keyboard->isModifierDown(OIS::Keyboard::Shift) && !hMultiSetParamDlg) {
					dragging_mode = 1;
					rememberCloningNodes();
					temporaryCloneSelection();
				}

				if (keyboard->isModifierDown(OIS::Keyboard::Ctrl)) {
					dragging_mode = 2;
					cloneSelection();
				}

				rememberSelection(axis->getMode());
			}
			else if (id == OIS::MB_Left) {
				if (current_node) {
					if (!is_pick_target && !is_pick_target_position) {
						if (!keyboard->isModifierDown(OIS::Keyboard::Ctrl)) {
							clearSelection();
						}

						if (!current_node->isSelected())
						{
							HistoryActionSelectNode* action_select = new HistoryActionSelectNode(current_node, false, true, &selected_nodes);
							current_node->setSelect(true);
							selected_nodes.push_back(current_node);
							addTrajectory(getTrajectoryMode(current_node));
							pushHistory(action_select);

						}

						updateSelection();
					}
					else
					{
						if (current_node->getType() == EDITOR_NODE_OBJECT)
						{
							
							ObjectNode* object_node = static_cast<ObjectNode*>(current_node);
							size_t id = object_node->getObject()->getID();

							if (is_pick_target)
							{
								bool is_list = current_properties_types[current_property_index] == LibGens::OBJECT_ELEMENT_ID_LIST;
								int combo_box = is_list ? IDE_EDIT_ID_LIST_VALUE : IDC_EDIT_ID_VALUE;
								SetDlgItemText(hEditPropertyDlg, combo_box, ToString<size_t>(id).c_str());

								setTargetName(id, is_list);
							}

							if (is_pick_target_position)
							{
								is_update_look_at_vector = false;

								Ogre::Vector3 position = object_node->getPosition();
								SetDlgItemText(hLookAtPointDlg, IDE_LOOK_AT_X, ToString<float>(position.x).c_str());
								SetDlgItemText(hLookAtPointDlg, IDE_LOOK_AT_Y, ToString<float>(position.y).c_str());
								SetDlgItemText(hLookAtPointDlg, IDE_LOOK_AT_Z, ToString<float>(position.z).c_str());

								updateLookAtPointVectorNode(position);
								is_update_look_at_vector = true;
							}
						}
					}
				}
				else if (!current_node) {
					//clearSelection();
				}
			}
		}
		else if (isPalettePreviewActive()) {
			mousePressedObjectsPalettePreview(arg, id);
		}
		else if (editor_mode == EDITOR_NODE_QUERY_NODE) {
			/*
			if (id == OIS::MB_Left) {
				float mouse_x=arg.state.X.abs/float(arg.state.width);
				float mouse_y=arg.state.Y.abs/float(arg.state.height);
				viewport->convertMouseToLocalScreen(mouse_x, mouse_y);

				// Raycast from camera to viewport
				Ogre::uint32 node_query_flags = EDITOR_NODE_QUERY_OBJECT | EDITOR_NODE_QUERY_PATH_NODE | EDITOR_NODE_QUERY_GHOST;
				Ogre::Entity *node_entity = viewport->raycastEntity(mouse_x, mouse_y, node_query_flags);

				if (node_entity) {
					Ogre::SceneNode *node=node_entity->getParentSceneNode();
					if (node) {
						Ogre::Any ptr_container=node->getUserObjectBindings().getUserAny(EDITOR_NODE_BINDING);
						if (!ptr_container.isEmpty()) {
							EditorNode *editor_node=Ogre::any_cast<EditorNode *>(ptr_container);

							Ogre::Vector3 raycast_point = editor_node->getPosition();
							closeVectorQueryMode();
							updateEditPropertyVectorGUI(LibGens::Vector3(raycast_point.x, raycast_point.y, raycast_point.z));
						}
					}
				}
			}

			if (id == OIS::MB_Right) {
				closeVectorQueryMode();
			}
			*/
		}

		viewport->mousePressed(arg, id);
	}
    return true;
}


bool EditorApplication::mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id) {
	bool last_holding = axis->mouseReleased(arg, id);

	if (id == OIS::MB_Left) {
		if (last_holding) {
			if (dragging_mode != 1)
				makeHistorySelection(axis->getMode());
		}

		if (dragging_mode == 1)
		{
			if (cloning_nodes.size() && !hMultiSetParamDlg)
			{
				openMultiSetParamDlg();
				setVectorAndSpacing();

				// save temporary clones to delete them when cloning is done
				list<EditorNode*>::iterator it;
				for (it = selected_nodes.begin(); it != selected_nodes.end(); ++it)
				{
					temporary_nodes.push_back((*it));
					(*it)->setSelect(false);
				}
				selected_nodes.clear();

				for (it = cloning_nodes.begin(); it != cloning_nodes.end(); ++it)
				{
					(*it)->setSelect(true);
					selected_nodes.push_back(*it);
				}

				cloning_nodes.clear();
				updateSelection();
			}
		}
	}

	viewport->mouseReleased(arg, id);
    return true;
}

bool EditorApplication::frameRenderingQueued(const Ogre::FrameEvent& evt) {
    BaseApplication::frameRenderingQueued(evt);

	Ogre::Real timeSinceLastFrame = evt.timeSinceLastFrame;

	if (ignore_mouse_clicks_frames) {
		ignore_mouse_clicks_frames -= 1;
	}

	if (ignore_mouse_clicks_frames < 0) {
		ignore_mouse_clicks_frames = 0;
	}

	// Update Editor
	if (!inFocus()) {
		viewport->onFocusLoss();
	}
	viewport->update(timeSinceLastFrame);
	axis->update(viewport);

	// Update Objects
	updateObjectsPalettePreview();

	// Update Ghost
	checkGhost(timeSinceLastFrame);

	// Update Trajectory previews
	updateTrajectoryNodes(timeSinceLastFrame);

	if (terrain_streamer) {
		Ogre::Vector3 v = viewport->getCamera()->getPosition();
		terrain_streamer->getMutex().lock();
		terrain_streamer->setPosition(LibGens::Vector3(v.x, v.y, v.z));
		terrain_streamer->setCheck(true);
		terrain_streamer->getMutex().unlock();
	}

	// Update Terrain
	checkTerrainStreamer();
	checkTerrainVisibilityAndQuality(timeSinceLastFrame);

	// Update Animations
	object_node_manager->addTime(timeSinceLastFrame);
	animations_list->addTime(timeSinceLastFrame);
    return true;
}

void EditorApplication::loadGhostRecording() 
{
	char filename[MAX_PATH];
	ZeroMemory(filename, sizeof(filename));
	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFilter = "Ghost Recording(.gst.bin)\0*.gst.bin\0";
	ofn.nFilterIndex = 1;
	ofn.nMaxFile = 1024;
	ofn.lpstrTitle = "Open Ghost Recording";
	ofn.lpstrFile = filename;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_EXPLORER | OFN_ENABLESIZING;

	if (!GetOpenFileName(&ofn))
		return;

	chdir(exe_path.c_str());
	LibGens::Ghost* gst = new LibGens::Ghost(std::string(filename));
	setGhost(gst);
}

void EditorApplication::saveGhostRecording()
{
	if (!ghost_data)
		return;

	char filename[MAX_PATH];
	ZeroMemory(filename, sizeof(filename));
	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFilter = "Ghost Recording(.gst.bin)\0*.gst.bin\0";
	ofn.nFilterIndex = 1;
	ofn.nMaxFile = 1024;
	ofn.lpstrTitle = "Save Ghost Recording";
	ofn.lpstrFile = filename;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_EXPLORER | OFN_ENABLESIZING;

	if (!GetSaveFileName(&ofn))
		return;

	chdir(exe_path.c_str());
	ghost_data->save(std::string(filename));
}

void EditorApplication::saveGhostRecordingFbx()
{
	if (!ghost_data)
		return;

	char filename[MAX_PATH];
	ZeroMemory(filename, sizeof(filename));
	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFilter = "FBX File(.fbx)\0*.fbx\0";
	ofn.nFilterIndex = 1;
	ofn.nMaxFile = 1024;
	ofn.lpstrTitle = "Export Ghost Recording";
	ofn.lpstrFile = filename;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_EXPLORER | OFN_ENABLESIZING;

	if (!GetSaveFileName(&ofn))
		return;

	chdir(exe_path.c_str());
	LibGens::FBX* lFbx = ghost_data->buildFbx(fbx_manager, model_library->getModel("chr_Sonic_HD"), material_library);
	fbx_manager->exportFBX(lFbx, filename);

	delete lFbx;
}

void EditorApplication::launchGame()
{
	if (GetFileAttributes(configuration->game_path.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		char filename[MAX_PATH];
		ZeroMemory(filename, sizeof(filename));
		OPENFILENAME ofn;
		memset(&ofn, 0, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.lpstrFilter = "Windows Executable(.exe)\0*.exe\0";
		ofn.nFilterIndex = 1;
		ofn.nMaxFile = 1024;
		ofn.lpstrTitle = "Select Sonic Generations";
		ofn.lpstrFile = filename;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_EXPLORER | OFN_ENABLESIZING;

		if (GetOpenFileName(&ofn))
		{
			chdir(exe_path.c_str());
			configuration->game_path = std::string(ofn.lpstrFile);
		}
	}

	string directory = configuration->game_path.substr(0, configuration->game_path.find_last_of('\\'));
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));
	si.cb = sizeof(si);

	CreateProcess(configuration->game_path.c_str(), NULL, NULL, NULL, FALSE, 0, FALSE, directory.c_str(), &si, &pi);
}

bool EditorApplication::connectGame() {
	return game_client->Connect();
}

DWORD EditorApplication::sendMessageGame(const PipeMessage& msg, size_t size) {
	return game_client->UploadMessage(msg, size);
}

void Game_ProcessMessage(PipeClient* client, PipeMessage* msg) {
	editor_application->processGameMessage(client, msg);
}

void EditorApplication::processGameMessage(PipeClient* client, PipeMessage* msg) {
	switch (msg->ID)
	{
	case SONICGLVL_MSG_SETRECORDING:
		isGhostRecording = ((MsgSetRecording*)msg)->Enable;
		break;

	case SONICGLVL_MSG_SAVERECORDING:
		isGhostRecording = false;
		MsgSaveRecording* m = (MsgSaveRecording*)msg;
		LibGens::Ghost* gst = new LibGens::Ghost(std::string(m->FilePath));
		setGhost(gst);
		break;
	}
}

void ColorListener::preRenderTargetUpdate(const Ogre::RenderTargetEvent& evt)
{
	scene_manager->setSpecialCaseRenderQueueMode(Ogre::SceneManager::SCRQM_EXCLUDE);
	scene_manager->addSpecialCaseRenderQueue(Ogre::RENDER_QUEUE_WORLD_GEOMETRY_2);
	scene_manager->addSpecialCaseRenderQueue(Ogre::RENDER_QUEUE_MAX);
}
 
void ColorListener::postRenderTargetUpdate(const Ogre::RenderTargetEvent& evt)
{
	scene_manager->clearSpecialCaseRenderQueues();
}

void DepthListener::preRenderTargetUpdate(const Ogre::RenderTargetEvent& evt)
{
	queue = scene_manager->getRenderQueue();
    queue->setRenderableListener(this); 

	scene_manager->setSpecialCaseRenderQueueMode(Ogre::SceneManager::SCRQM_EXCLUDE);
	scene_manager->addSpecialCaseRenderQueue(Ogre::RENDER_QUEUE_WORLD_GEOMETRY_2);
	scene_manager->addSpecialCaseRenderQueue(Ogre::RENDER_QUEUE_MAX);
}
 
void DepthListener::postRenderTargetUpdate(const Ogre::RenderTargetEvent& evt)
{
	scene_manager->clearSpecialCaseRenderQueues();

	queue = scene_manager->getRenderQueue();
	queue->setRenderableListener(0); 
}

bool DepthListener::renderableQueued(Ogre::Renderable* rend, Ogre::uint8 groupID, Ogre::ushort priority, Ogre::Technique** ppTech, Ogre::RenderQueue* pQueue)
{
	*ppTech = mDepthMaterial->getTechnique(0);
	return true;
};

ObjectNode* EditorApplication::getObjectNodeFromEditorNode(EditorNode* node)
{
	ObjectNode* object_node = nullptr;
	if (node->getType() == EDITOR_NODE_OBJECT)
	{
		object_node = static_cast<ObjectNode*>(node);
	}
	else if (node->getType() == EDITOR_NODE_OBJECT_MSP)
	{
		ObjectMultiSetNode* ms_node = static_cast<ObjectMultiSetNode*>(node);
		object_node = ms_node->getObjectNode();
	}

	return object_node;
}

TrajectoryMode EditorApplication::getTrajectoryMode(EditorNode* node)
{
	std::string object_name;
	ObjectNode* object_node = getObjectNodeFromEditorNode(node);
	if (object_node)
		object_name = object_node->getObject()->getName();

	TrajectoryMode mode = NONE;

	if ((object_name ==  "Spring") || (object_name == "AirSpring") || (object_name == "SpringFake") ||
		(object_name == "SpringClassic") || (object_name == "SpringClassicYellow"))
		mode = SPRING;
	else if (object_name == "WideSpring")
		mode = WIDE_SPRING;
	else if (object_name ==  "JumpPole")
		mode = JUMP_POLE;
	else if ((object_name == "JumpBoard") || (object_name == "JumpBoard3D") || (object_name == "AdlibTrickJump"))
		mode = JUMP_PANEL;
	else if ((object_name == "DashRing") || (object_name == "RainbowRing"))
		mode = DASH_RING;

	return mode;
}

void EditorApplication::addTrajectory(TrajectoryMode mode)
{
	if (mode == NONE)
		return;

	trajectory_preview_nodes.push_back(new TrajectoryNode(scene_manager, mode));
	
	// JumpBoards need two nodes. One for normal, and the other for boost
	if (mode == JUMP_PANEL)
		trajectory_preview_nodes.push_back(new TrajectoryNode(scene_manager, mode));
}

void EditorApplication::updateTrajectoryNodes(Ogre::Real timeSinceLastFrame)
{
	if (!selected_nodes.size())
		return;

	for (int count = 0; count < trajectory_preview_nodes.size(); ++count)
		trajectory_preview_nodes[count]->addTime(timeSinceLastFrame);

	int count = 0;
	list<EditorNode*>::iterator it = selected_nodes.begin();

	for (; it != selected_nodes.end(); ++it)
	{
		if (count < trajectory_preview_nodes.size())
		{
			EditorNode* node = *it;
			TrajectoryMode mode = getTrajectoryMode(node);
			switch (mode)
			{
			case SPRING:
			case WIDE_SPRING:
				trajectory_preview_nodes[count]->getTrajectorySpring(node);
				break;

			case JUMP_PANEL:
				trajectory_preview_nodes[count++]->getTrajectoryJumpBoard(node, false);
				trajectory_preview_nodes[count]->getTrajectoryJumpBoard(node, true);
				break;

			case DASH_RING:
				trajectory_preview_nodes[count]->getTrajectoryDashRing(node);
				break;

			default:
				break;
			}

			++count;
		}
		else
			break;
	}
}

void EditorApplication::removeAllTrajectoryNodes()
{
	for (vector<TrajectoryNode*>::iterator it = trajectory_preview_nodes.begin(); it != trajectory_preview_nodes.end(); ++it)
		delete* it;

	trajectory_preview_nodes.clear();
}