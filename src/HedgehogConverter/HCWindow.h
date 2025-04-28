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

#pragma once

#include "ui_HCWindow.h"
#include <QMainWindow>
#include "assimp/scene.h"
#include "LibGens.h"
#include "Material.h"
#include "TerrainInstance.h"
#include "TerrainGroup.h"
#include "Compression.h"

struct ModelRecord {
	LibGens::AABB aabb;
	QList<int> used_meshes;
	unsigned int submesh_counts[3];
};

struct SceneData {
	QMap<QString, ModelRecord> model_map;
	list<LibGens::Material *> materials;
	QMultiMap<int, LibGens::TerrainInstance *> instances;
	QMap<string, unsigned int> model_size_map;
};

class HCWindow : public QMainWindow, public Ui_HCWindow {
	Q_OBJECT
public:
	enum ProgressType {
		ProgressNormal,
		ProgressSuccess,
		ProgressError,
		ProgressWarning,
		ProgressFatal
	};

	static const QString AppName;
	static const QString VersionNumber;
	static const QString DefaultSettingsPath;
	static const QString LogPath;
	static const QString CommunityGuideURL;
	static const int BaseUnassignedGroupIndex;

	explicit HCWindow(QWidget *parent = 0);
    ~HCWindow();
private:
	enum GameEngine {
		Generations,
		Unleashed,
		LostWorld
	};

	struct {
		GameEngine game_engine;
		QStringList model_source_paths;
		QString texture_source_path;
		QString terrain_output_path;
		bool merge_existing;
		bool generate_materials;
		bool copy_and_convert_textures;
		bool force_tags_layers;
		bool use_model_groups;
		bool remove_model_tags;
		bool remove_material_tags;
		bool convert_lights;
		double position_x, position_y, position_z;
		double scale_x, scale_y, scale_z;
		double rotation_x, rotation_y, rotation_z;
		double group_cell_size;
	} converter_settings;

	bool convert();
	bool convertSceneNode(const aiScene *scene, aiNode *node, QString path, SceneData &scene_data, LibGens::Matrix4 global_transform, int parent_group_id = -1);
	void addTextureToMaterial(LibGens::Material *material, QString material_name, QString texture_path, QString texture_unit, QStringList &textures_to_copy, QStringList &textures_to_convert);
	QList<LibGens::TerrainGroup *> convertTerrainGroups(SceneData &scene_data);
	void convertTerrainGroups(QList<LibGens::TerrainGroup *> &terrain_groups, QList<LibGens::TerrainInstance *> instances, LibGens::AABB aabb);
	bool packLostWorld(QString output_path, QString output_name, QString path);
	bool packTerrainGroups(QList<LibGens::TerrainGroup *> &terrain_groups, QString output_path, QString output_add_path, QString output_name, QString terrain_path, QString configuration_path, LibGens::CompressionType compression);
	bool packGenerations(QList<LibGens::TerrainGroup *> &terrain_groups, QString output_path, QString output_name, QString terrain_path, QString resources_path);
	bool packUnleashed(QList<LibGens::TerrainGroup *> &terrain_groups, QString output_path, QString output_name, QString terrain_path, QString configuration_path, QString resources_path);
	void logNodeTree(aiNode *node, QString prefix);
	bool findNodeTransform(aiNode *node, aiMatrix4x4 parent_transform, aiString name, aiMatrix4x4 *return_transform);
	void saveSettings(QString filename);
	void loadSettings(QString filename);
	void updateSettingsFromUi();
	void updateUiFromSettings();
	void messageBox(QString text);
	void logProgress(ProgressType progress_type, QString message);
	void beep();
	QString temporaryDirTemplate();
private slots:
	void aboutTriggered();
	void addSourceModelsTriggered();
	void removeSourceModelTriggered();
	void clearSourceModelsTriggered();
	void resetTransformTriggered();
	void browseTexturesTriggered();
	void browseOutputTriggered();
	void close();
	void convertTriggered();
	void openSettingsTriggered();
	void saveSettingsTriggered();
	void communityGuideTriggered();
	void closeEvent(QCloseEvent *event);
	void generateMaterialsChanged(int);
	void copyConvertTexturesChanged(int);
	void tagCheatSheetTriggered();
};