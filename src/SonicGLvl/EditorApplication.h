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

//==================================================================================================================================================
// Very Heavily Modified from the Ogre3D Wiki: http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Ogre+Wiki+Tutorial+Framework&structure=Development
//==================================================================================================================================================

#include "BaseApplication.h"
#include "EditorViewport.h"
#include "ObjectNode.h"
#include "TerrainNode.h"
#include "HavokNode.h"
#include "GhostNode.h"
#include "PathNode.h"
#include "VectorNode.h"
#include "CameraManager.h"
#include "EditorAxis.h"
#include "EditorTerrain.h"
#include "EditorLevel.h"
#include "EditorConfiguration.h"
#include "EditorAnimations.h"
#include "EditorGlobalIllumination.h"
#include "History.h"
#include "EditorMaterial.h"
#include "FBXManager.h"
#include "Shader.h"
#include "UVAnimationLibrary.h"
#include "Havok.h"
#include "HavokPropertyDatabase.h"
#include "Level.h"
#include "Object.h"
#include "ObjectCategory.h"
#include "ObjectSet.h"

#ifndef EDITOR_APPLICATION_H_INCLUDED
#define EDITOR_APPLICATION_H_INCLUDED

#define SONICGLVL_CAMERA_NAME                  "EditorCamera"
#define SONICGLVL_CAMERA_PREVIEW_NAME          "PreviewCamera"
#define SONICGLVL_CACHE_PATH                   "../cache/"
#define SONICGLVL_CACHE_DATA_PATH              "data"
#define SONICGLVL_CACHE_GI_TEMP_PATH           "gi_temp"
#define SONICGLVL_CACHE_TERRAIN_PATH           "terrain"
#define SONICGLVL_CACHE_RESOURCES_PATH         "resources"
#define SONICGLVL_LOW_END_TECHNIQUE            "LowEnd"
#define SONICGLVL_LEVEL_DATABASE_PATH          "../database/LevelDatabase.xml"
#define SONICGLVL_GHOST_DATABASE_PATH          "../database/GhostDatabase.xml"
#define SONICGLVL_OBJECTS_DATABASE_PATH        "../database/ObjectsDatabase.xml"
#define SONICGLVL_HAVOK_PROPERTY_DATABASE_PATH "../database/HavokPropertyDatabase.xml"
#define SONICGLVL_LIBRARY_PATH                 "../database/objects/"
#define SONICGLVL_RESOURCES_PATH               "../database/resources/"
#define SONICGLVL_SHADERS_PATH                 "../database/shaders/"

#define SONICGLVL_FBX_SCENE_NAME               "FBXTerrainImport"
#define SONICGLVL_UNASSIGNED_OBJECT_CATEGORY   "Unassigned"

#define SONICGLVL_CACHE_HASH_FILE              "Hashes.xml"
#define SONICGLVL_CONFIGURATION_FILE           "Configuration.xml"

#define SONICGLVL_SHADER_LIBRARY               editor_application->getShaderLibrary()
#define SONICGLVL_UV_ANIMATION_LIBRARY         editor_application->getUVAnimationLibrary()
#define SONICGLVL_HAVOK_ENVIROMENT             editor_application->getHavokEnviroment()

#define SONICGLVL_HAVOK_PRECISION_FPS          30.0f

#define SONICGLVL_GUI_LEFT_WIDTH               280
#define SONICGLVL_GUI_BOTTOM_HEIGHT            83

#define SONICGLVL_MATERIAL_EDITOR_MODE_MODEL     0
#define SONICGLVL_MATERIAL_EDITOR_MODE_MATERIAL  1
#define SONICGLVL_MATERIAL_EDITOR_MODE_TERRAIN   2

#define SONICGLVL_MULTISETPARAM_MODE_CLONE		 0
#define SONICGLVL_MULTISETPARAM_MODE_MSP		 1
#define SONICGLVL_MULTISETPARAM_MODE_MSP_ADD	 2

extern int global_cursor_state;

extern Ogre::SceneNode *camera_marker_node;
extern Ogre::SceneNode *camera_marker_tangent;

class ColorListener : public Ogre::RenderTargetListener {
	protected:
		Ogre::SceneManager *scene_manager;
	public:
		ColorListener(Ogre::SceneManager *scene_manager_param) {
			scene_manager = scene_manager_param;
		}

		void preRenderTargetUpdate(const Ogre::RenderTargetEvent& evt);
		void postRenderTargetUpdate(const Ogre::RenderTargetEvent& evt);
};

class DepthListener : public Ogre::RenderTargetListener, public Ogre::RenderQueue::RenderableListener {
	protected:
		Ogre::SceneManager *scene_manager;
		Ogre::RenderQueue* queue;
		Ogre::MaterialPtr mDepthMaterial;
	public:
		DepthListener(Ogre::SceneManager *scene_manager_param) {
			scene_manager = scene_manager_param;

			mDepthMaterial = Ogre::MaterialManager::getSingleton().getByName("DepthMap");
			mDepthMaterial->load();

			Ogre::GpuProgramParametersSharedPtr vp_parameters = mDepthMaterial->getTechnique(0)->getPass(0)->getVertexProgramParameters();
			vp_parameters->setTransposeMatrices(true);
		}

		void preRenderTargetUpdate(const Ogre::RenderTargetEvent& evt);
		void postRenderTargetUpdate(const Ogre::RenderTargetEvent& evt);
		bool renderableQueued(Ogre::Renderable* rend, Ogre::uint8 groupID, Ogre::ushort priority, Ogre::Technique** ppTech, Ogre::RenderQueue* pQueue);

		Ogre::Pass *getDepthPass() {
			return mDepthMaterial->getTechnique(0)->getPass(0);
		}
};


class EditorApplication : public BaseApplication {
	protected:
		// General Editor Variables
		Ogre::uint32 editor_mode;
		bool world_transform;
		size_t dragging_mode;
		size_t cloning_mode;
		Ogre::AnimationState *animation_state;
		EditorLevelDatabase *level_database;
		EditorLevel *current_level;
		string current_level_filename;
		map<LibGens::ObjectSet *, int> set_indices;
		map<LibGens::ObjectSet *, bool> set_visibility;
		
		// Ogre
		Ogre::Light *global_directional_light;
		Ogre::SceneManager *axis_scene_manager;

		Ogre::RenderTexture *color_texture;
		Ogre::RenderTexture *depth_texture;

		ColorListener *color_listener;
		DepthListener *depth_listener;
		
		float farPlaneChange;

		// Viewport
		EditorViewport *viewport;
		EditorAxis *axis;

		// Node
		History *history;
		list<EditorNode *> selected_nodes;
		list<EditorNode *> previous_selected_nodes;
		EditorNode *current_node;

		// FBX
		LibGens::FBXManager *fbx_manager;

		// Model
		LibGens::ModelLibrary *model_library;

		// Material
		LibGens::MaterialLibrary *material_library;
		LibGens::ShaderLibrary *shader_library;
		EditorAnimationsList *animations_list;
		LibGens::UVAnimationLibrary *uv_animation_library;

		// Terrain
		TerrainStreamer *terrain_streamer;
		list<TerrainNode *> terrain_nodes_list;
		float terrain_update_counter;

		// GI
		GlobalIlluminationListener *global_illumination_listener;

		// Object
		ObjectNodeManager *object_node_manager;
		LibGens::ObjectLibrary *library;
		LibGens::ObjectProduction *object_production;

		// Ghost
		GhostNode *ghost_node;
		unordered_map<string, string> ghost_animation_mappings;
		CameraManager *camera_manager;

		// Animation
		
		// Havok
		LibGens::HavokEnviroment *havok_enviroment;
		LibGens::HavokPropertyDatabase *havok_property_database;
		list<HavokNode *> havok_nodes_list;
		
		// Configuration
		EditorConfiguration *configuration;
		
		// WinAPI
		HMENU hMenu;
		HWND hLeftDlg;
		HWND hBottomDlg;
		HWND hEditPropertyDlg;

		RECT hEditPropertyDlg_old_rect;

		HWND hMaterialEditorDlg;
		HWND hPhysicsEditorDlg;
		HWND hMaterialEditorPreviewDlg;
		HWND hMultiSetParamDlg;

		// Object Palette
		int current_category_index;
		LibGens::Object *last_palette_selection;
		LibGens::Object *current_palette_selection;
		list<ObjectNode *> current_palette_nodes;
		LibGens::ObjectSet *current_set;
		bool palette_cloning_mode;

		// Object Properties
		list<LibGens::Object *> current_object_list_properties;
		vector<string> current_properties_names;
		vector<LibGens::ObjectElementType> current_properties_types;
		int current_property_index;
		LibGens::Object *current_single_property_object;

		HistoryActionWrapper *history_edit_property_wrapper;
		int ignore_mouse_clicks_frames;

		vector<VectorNode *> property_vector_nodes;
		History *property_vector_history;


		// Material Editor
		size_t material_editor_mode;
		string material_editor_mesh_group;
		int material_editor_list_selection;
		int last_material_editor_list_selection;
		int texture_list_selection;
		int last_texture_list_selection;
		LibGens::Model *material_editor_model;
		string material_editor_model_filename;
		string material_editor_library_folder;
		vector<LibGens::Material *> material_editor_materials;
		Ogre::RenderWindow *material_editor_preview_window;
		Ogre::Viewport *material_editor_preview_viewport;
		Ogre::Camera *material_editor_preview_camera;
		Ogre::SceneManager *material_editor_preview_scene_manager;
		LibGens::MaterialLibrary *material_editor_material_library;
		LibGens::Material *material_editor_material;
		LibGens::Texture *material_editor_texture;
		string material_editor_skeleton_name;
		string material_editor_animation_name;
		Ogre::AnimationState *material_editor_animation_state;
		Ogre::SceneNode *material_editor_scene_node;

		OIS::InputManager* material_editor_input_manager;
		OIS::Mouse*    material_editor_mouse;
		OIS::Keyboard* material_editor_keyboard;

		MaterialEditorPreviewListener *material_editor_preview_listener;
		EditorViewport *material_editor_viewport;

		// Object Movement
		float placement_grid_snap;

		// Cloning
		list<EditorNode*> cloning_nodes;
		list<EditorNode*> temporary_nodes;

	public:
		EditorApplication(void);
		virtual ~EditorApplication(void);

		void createTerrainStreamer();
		void *updateTerrainStreamer();

		bool keyPressed(const OIS::KeyEvent &arg);
		bool keyReleased(const OIS::KeyEvent &arg);
		bool mouseMoved(const OIS::MouseEvent &arg);
		bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
		bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
		bool frameRenderingQueued(const Ogre::FrameEvent& evt);
		void windowResized(Ogre::RenderWindow* rw);
		void createScene(void);

		// Editor Node Methods
		void updateSelection();
		void deleteSelection();
		void clearSelection();
		void cloneSelection();
		void showSelectionNames();
		void selectAll();
		void translateSelection(Ogre::Vector3 v);
		void rotateSelection(Ogre::Quaternion q);
		void setSelectionRotation(Ogre::Quaternion q);
		void rememberSelection(bool mode);
		void makeHistorySelection(bool mode);
		void toggleWorldTransform();
		void togglePlacementSnap();
		void updateNodeVisibility();
		void toggleNodeVisibility(unsigned int flag);
		void updateVisibilityGUI();
		void rememberCloningNodes();

		void copySelection();
		void pasteSelection();

		void undoHistory();
		void redoHistory();
		void pushHistory(HistoryAction *action);

		// Update methods
		void checkGhost(Ogre::Real timeSinceLastFrame);
		void checkTerrainStreamer();
		void checkTerrainVisibilityAndQuality(Ogre::Real timeSinceLastFrame);

		void ignoreMouseClicks(int frames) {
			ignore_mouse_clicks_frames = frames;
		}

		bool ignoringMouseClicks() {
			return (ignore_mouse_clicks_frames > 0);
		}

		// GUI Methods
		void focus();
		bool inFocus();

		void setEditorMode(Ogre::uint32 v) {
			editor_mode = v;
		}

		Ogre::uint32 getEditorMode() {
			return editor_mode;
		}

		void openLevelGUI();
		void openLostWorldLevelGUI();

		void saveLevelDataGUI();
		void saveLevelTerrainGUI();
		void importLevelTerrainFBXGUI();
		void exportSceneFBXGUI();
		void exportSceneFBX(string filename);

		void saveXNAnimation();

		void updateBottomSelectionGUI();
		void updateSetsGUI();
		void updateSelectedSetGUI();
		void newCurrentSet();
		void deleteCurrentSet();
		void updateCurrentSetVisible(bool v);
		void changeCurrentSet(string change_set);
		void renameCurrentSet(string rename_set);


		void openPhysicsEditorGUI();
		void clearPhysicsEditorGUI();
		void addPhysicsEditorEntryGUI(LibGens::LevelCollisionEntry *entry);
		void importPhysicsEditorGUI();
		void detectAndTagHavokPhysics(LibGens::HavokPhysicsCache *physics_cache);

		void openMaterialEditorGUI();
		void clearMaterialEditorGUI();
		void enableMaterialEditorGUI(bool enable);
		void enableMaterialEditorListGUI();
		void loadMaterialEditorModelGUI();
		void loadMaterialEditorSkeletonGUI();
		void loadMaterialEditorAnimationGUI();
		void rebuildMaterialPreviewNodes();
		void materialEditorTerrainMode();
		void materialEditorModelMode();
		void saveMaterialEditorModelGUI();
		void saveMaterialEditorMaterial();
		void pickMaterialEditorTextureGUI();
		void addMaterialEditorTextureGUI();

		void cleanMaterialEditorModelGUI();
		void clearSelectionMaterialEditorGUI();
		void clearTextureInfo();
		void rebuildListMaterialEditorGUI();
		void createPreviewMaterialEditorGUI();
		void updateMaterialEditorIndex(int selection_index);
		void updateMaterialEditorTextureIndex(int selection_index);
		void updateMaterialEditorInfo();
		void updateMaterialEditorTextureList();
		void updateMaterialTextureInfo();
		void updateEditParameterMaterialEditor(size_t i, LibGens::Color parameter_color);
		void updateEditShaderMaterialEditor(string shader_name);
		void updateEditTextureUnitMaterialEditor(string unit_name);
		void updateEditTextureMaterialEditor(string texture_name, bool update_ui = false);
		void removeMaterialEditorTexture();
		void loadMaterialDefaultParams();

		void createObjectsPropertiesGUI();
		void updateObjectsPropertiesGUI();
		void updateObjectsPropertiesValuesGUI(LibGens::Object *object);
		void updateObjectPropertyIndex(int selection_index);
		void editObjectPropertyIndex(int selection_index);
		void updateHelpWithObjectGUI(LibGens::Object *object);
		void updateHelpWithPropertyGUI(LibGens::ObjectElement *element);

		void openMultiSetParamDlg();
		void closeMultiSetParamDlg();
		void clearMultiSetParamDlg();
		void createMultiSetParamObjects();
		void getVectorFromObject();
		void setCloningMode(size_t mode);
		void setVectorAndSpacing();
		void deleteTemporaryNodes();

		void clearEditPropertyGUI();
		void closeEditPropertyGUI();
		HWND getEditPropertyDlg();

		void updateEditPropertyBool(bool v);
		void updateEditPropertyInteger(unsigned int v);
		void updateEditPropertyFloat(float v);
		void updateEditPropertyString(string v);
		void updateEditPropertyID(size_t v);
		void updateEditPropertyIDList(vector<size_t> v);
		void updateEditPropertyVector(LibGens::Vector3 v);
		void updateEditPropertyVectorFocus();
		void updateEditPropertyVectorGUI();
		void updateEditPropertyVectorMode(bool mode_state);
		void updateEditPropertyVectorList(vector<LibGens::Vector3> v);
		void updateEditPropertyVectorListFocus();


		void closeVectorQueryMode();

		void verifySonicSpawnChange();
		void confirmEditProperty();
		void revertEditProperty();
		

		void updateObjectCategoriesGUI();
		void updateObjectsPaletteGUI(int index=0);
		void updateObjectsPaletteSelection(int index);
		void updateObjectsPalettePreview();
		void overrideObjectsPalettePreview(list<LibGens::Object *> override_objects);
		void mouseMovedObjectsPalettePreview(const OIS::MouseEvent &arg);
		void mousePressedObjectsPalettePreview(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
		void clearObjectsPalettePreview();
		void clearObjectsPalettePreviewGUI();
		bool isPalettePreviewActive();
		bool isRegularMode();

		// LibGens Methods
		void openLevel(string filename);
		void openLostWorldLevel(string filename);

		void createDirectionalLight(LibGens::Light *direct_light);
		void createSkybox(string skybox_name);

		void saveLevelData(string filename);
		void saveLevelTerrain();
		void cleanLevelTerrain();
		void importLevelTerrainFBX(string filename);
		void generateTerrainGroups();
		void reloadTemplatesDatabase();
		void saveTemplatesDatabase();

		void loadLevelPaths();

		// Havok method
		void loadGhostAnimations();

		void createLevel(string name);
		void createLibrary();
		void createCategory(LibGens::ObjectCategory *category, string folder);
		void createNodesFromSet(LibGens::ObjectSet *set);
		void createNodesFromTerrain(LibGens::Terrain *terrain, LibGens::GITextureGroupInfo *gi_group_info);
		void createNodesFromTerrainGroup(LibGens::TerrainGroup *terrain_group);
		void createNodesFromHavokEnviroment(LibGens::HavokEnviroment *havok_enviroment);

		//bool renderOneFrame();

		EditorAxis *getEditorAxis() {
			return axis;
		}

		EditorLevel *getCurrentLevel() {
			return current_level;
		}

		LibGens::ObjectSet *getCurrentSet() {
			return current_set;
		}

		EditorAnimationsList *getAnimationsList() {
			return animations_list;
		}

		LibGens::ShaderLibrary *getShaderLibrary() {
			return shader_library;
		}

		LibGens::UVAnimationLibrary *getUVAnimationLibrary() {
			return uv_animation_library;
		}

		Ogre::SceneManager *getSceneManager() {
			return scene_manager;
		}

		LibGens::HavokEnviroment *getHavokEnviroment() {
			return havok_enviroment;
		}

		void updateBottomSelectionPosition(float value_x, float value_y, float value_z);
		void updateBottomSelectionRotation(float value_x, float value_y, float value_z);
};

extern EditorApplication *editor_application;

#endif