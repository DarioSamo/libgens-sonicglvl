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

#include "ui_GIWindow.h"
#include <QMainWindow>
#include "LibGens.h"
#include "GITextureGroup.h"
#include "MathGens.h"
#include "Path.h"

class GIWindow : public QMainWindow, public Ui_GIWindow {
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

	explicit GIWindow(QWidget *parent = 0);
    ~GIWindow();
private:
	enum GameEngine {
		Generations,
		Unleashed,
		LostWorld
	};

	enum Mode {
		PreRender,
		PostRender
	};

	struct {
		GameEngine game_engine;
		Mode mode;
		QString source_gi_directory;
		QString terrain_directory;
		QString reference_path_file;
		QString render_output_file;
		double octree_cell_size;
		double reference_distance_interval;
		double quality_size_multiplier;
		int reference_max_intervals;
		int max_texture_size;
		int max_atlas_texture_size;
		bool no_gi_nodes;
		bool inverted_shadowmaps;
		bool scale_to_atlasinfo;
		bool scale_to_lightmap;
		bool scale_to_shadowmap;
		bool override_lightmap;
		bool override_shadowmap;
		int override_lightmap_r, override_lightmap_g, override_lightmap_b;
		int override_shadowmap_a;
	} converter_settings;

	bool convert();
	void saveSettings(QString filename);
	void loadSettings(QString filename);
	void updateSettingsFromUi();
	void updateUiFromSettings();
	void messageBox(QString text);
	void logProgress(ProgressType progress_type, QString message);

	struct RenderItem {
		QString instance_name;
		int size;
	};

	struct GIGroup {
		int level;
		int index;
		int instance_count;
		LibGens::AABB aabb;
	};

	QList<GIWindow::GIGroup> createGroups(LibGens::AABB current_aabb, QList<string> instances, LibGens::GITextureGroupInfo *gi_group_info, LibGens::PathNodeList &reference_nodes, QList<RenderItem> &render_items, const QMap<string, LibGens::AABB> &instance_aabbs);
	bool packGenerations(QString output_path, QString output_name, QString path, QString stage_path, QString stage_add_path);
	void compressFileCAB(QString filename);
	void expandFileCAB(QString filename, QString new_filename);
	unsigned int nextPowerOfTwo(unsigned int v);
	QColor debugColor(int quality_level, int size);
private slots:
	void aboutTriggered();
	void close();
	void convertTriggered();
	void openSettingsTriggered();
	void saveSettingsTriggered();
	void browseSourceTriggered();
	void browseReferencePathTriggered();
	void browseTerrainDirectoryTriggered();
	void browseOutputListTriggered();
	void communityGuideTriggered();
	void modeChanged();
	void overrideLightmapChanged(int state);
	void overrideShadowmapChanged(int state);
	void closeEvent(QCloseEvent *event);
	void textureSizeChanged();
	void textureAtlasSizeChanged();
};