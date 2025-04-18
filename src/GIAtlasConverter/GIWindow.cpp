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

#include "GIWindow.h"
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QFileDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QTime>
#include <QSettings>
#include <QTextStream>
#include <QFile>
#include <QListWidgetItem>
#include <QScrollBar>


const QString GIWindow::AppName = "GIAtlas Converter";
const QString GIWindow::VersionNumber = "0.1";
const QString GIWindow::DefaultSettingsPath = "GIAtlasConverter.ini";
const QString GIWindow::LogPath = "GIAtlasConverter.log";
const QString GIWindow::CommunityGuideURL = "http://info.sonicretro.org/SCHG:Sonic_Generations";

GIWindow::GIWindow(QWidget *parent) : QMainWindow(parent) {
	setupUi(this);
	cb_game_engine->addItem("Generations");
	//cb_game_engine->addItem("Unleashed"); // TO VERIFY
	//cb_game_engine->addItem("Lost World"); // TO IMPLEMENT
	cb_game_engine->setCurrentIndex(0);

	cb_mode->addItem("Pre-Render");
	cb_mode->addItem("Post-Render");
	cb_mode->setCurrentIndex(0);

	connect(action_open_settings, SIGNAL(triggered()), this, SLOT(openSettingsTriggered()));
	connect(action_save_settings, SIGNAL(triggered()), this, SLOT(saveSettingsTriggered()));
	connect(action_about, SIGNAL(triggered()), this, SLOT(aboutTriggered()));
	connect(action_about_qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
	connect(action_close, SIGNAL(triggered()), this, SLOT(close()));
	connect(action_community_guide, SIGNAL(triggered()), this, SLOT(communityGuideTriggered()));
	connect(pb_convert, SIGNAL(released()), this, SLOT(convertTriggered()));
	connect(cb_mode, SIGNAL(currentIndexChanged(int)), this, SLOT(modeChanged()));
	connect(pb_source_gi_directory, SIGNAL(released()), this, SLOT(browseSourceTriggered()));
	connect(pb_reference_path_file, SIGNAL(released()), this, SLOT(browseReferencePathTriggered()));
	connect(pb_terrain_directory, SIGNAL(released()), this, SLOT(browseTerrainDirectoryTriggered()));
	connect(pb_render_output_file, SIGNAL(released()), this, SLOT(browseOutputListTriggered()));
	connect(chk_override_lightmap, SIGNAL(stateChanged(int)), this, SLOT(overrideLightmapChanged(int)));
	connect(chk_override_shadowmap, SIGNAL(stateChanged(int)), this, SLOT(overrideShadowmapChanged(int)));
	connect(sb_max_texture_size, SIGNAL(editingFinished()), this, SLOT(textureSizeChanged()));
	connect(sb_max_atlas_texture_size, SIGNAL(editingFinished()), this, SLOT(textureAtlasSizeChanged()));

	te_progress->setReadOnly(true);

	// Reset log file
	QFile log_file(LogPath);
    log_file.open(QIODevice::WriteOnly);
	log_file.close();

	logProgress(ProgressNormal, "Logging progress to " + LogPath);

	if (!QFileInfo(DefaultSettingsPath).exists()) {
		// Default settings
		converter_settings.game_engine = Generations;
		converter_settings.mode = PreRender;
		converter_settings.source_gi_directory = "";
		converter_settings.terrain_directory = "";
		converter_settings.reference_path_file = "";
		converter_settings.render_output_file = "";
		converter_settings.octree_cell_size = 75.0;
		converter_settings.reference_distance_interval = 100.0;
		converter_settings.reference_max_intervals = 3;
		converter_settings.quality_size_multiplier = 16.0;
		converter_settings.max_texture_size = 1024;
		converter_settings.max_atlas_texture_size = 2048;
		converter_settings.no_gi_nodes = true;
		converter_settings.inverted_shadowmaps = false;
		converter_settings.scale_to_atlasinfo = true;
		converter_settings.scale_to_lightmap = false;
		converter_settings.scale_to_shadowmap = false;
		converter_settings.override_lightmap = false;
		converter_settings.override_shadowmap = false;
		converter_settings.override_lightmap_r = converter_settings.override_lightmap_g = converter_settings.override_lightmap_b = 128;
		converter_settings.override_shadowmap_a = 255;
		saveSettings(DefaultSettingsPath);
	}
	else {
		loadSettings(DefaultSettingsPath);
	}

	updateUiFromSettings();
	overrideLightmapChanged(0);
	overrideShadowmapChanged(0);
}

GIWindow::~GIWindow() {
	updateSettingsFromUi();
	saveSettings(DefaultSettingsPath);
}

void GIWindow::close() {
	QMainWindow::close();
}

void GIWindow::closeEvent(QCloseEvent *event) {
	if (!isEnabled())
		event->ignore();
}

void GIWindow::loadSettings(QString filename) {
	QSettings settings(filename, QSettings::IniFormat);
	converter_settings.game_engine = (GameEngine) settings.value("game_engine", 0).toInt();
	converter_settings.mode = (Mode) settings.value("mode", 0).toInt();
	converter_settings.source_gi_directory = settings.value("source_gi_directory").toString();
	converter_settings.terrain_directory = settings.value("terrain_directory").toString();
	converter_settings.reference_path_file = settings.value("reference_path_file").toString();
	converter_settings.render_output_file = settings.value("render_output_file").toString();
	converter_settings.octree_cell_size = settings.value("octree_cell_size", 0).toDouble();
	converter_settings.reference_distance_interval = settings.value("reference_distance_interval", 60.0).toDouble();
	converter_settings.reference_max_intervals = settings.value("reference_max_intervals", 3).toInt();
	converter_settings.quality_size_multiplier = settings.value("quality_size_multiplier", 0).toDouble();
	converter_settings.max_texture_size = settings.value("max_texture_size", 0).toInt();
	converter_settings.max_atlas_texture_size = settings.value("max_atlas_texture_size", 2048).toInt();
	converter_settings.no_gi_nodes = settings.value("no_gi_nodes", false).toBool();
	converter_settings.inverted_shadowmaps = settings.value("inverted_shadowmaps", false).toBool();
	converter_settings.scale_to_atlasinfo = settings.value("scale_to_atlasinfo", false).toBool();
	converter_settings.scale_to_lightmap = settings.value("scale_to_lightmap", false).toBool();
	converter_settings.scale_to_shadowmap = settings.value("scale_to_shadowmap", false).toBool();
	converter_settings.override_lightmap = settings.value("override_lightmap", false).toBool();
	converter_settings.override_shadowmap = settings.value("override_shadowmap", false).toBool();
	converter_settings.override_lightmap_r = settings.value("override_lightmap_r", 128).toInt();
	converter_settings.override_lightmap_g = settings.value("override_lightmap_g", 128).toInt();
	converter_settings.override_lightmap_b = settings.value("override_lightmap_b", 128).toInt();
	converter_settings.override_shadowmap_a = settings.value("override_shadowmap_a", 255).toInt();
}


void GIWindow::saveSettings(QString filename) {
	QSettings settings(filename, QSettings::IniFormat);
	settings.setValue("game_engine", converter_settings.game_engine);
	settings.setValue("mode", converter_settings.mode);
	settings.setValue("source_gi_directory", converter_settings.source_gi_directory);
	settings.setValue("terrain_directory", converter_settings.terrain_directory);
	settings.setValue("reference_path_file", converter_settings.reference_path_file);
	settings.setValue("render_output_file", converter_settings.render_output_file);
	settings.setValue("octree_cell_size", converter_settings.octree_cell_size);
	settings.setValue("reference_distance_interval", converter_settings.reference_distance_interval);
	settings.setValue("reference_max_intervals", converter_settings.reference_max_intervals);
	settings.setValue("quality_size_multiplier", converter_settings.quality_size_multiplier);
	settings.setValue("max_texture_size", converter_settings.max_texture_size);
	settings.setValue("max_atlas_texture_size", converter_settings.max_atlas_texture_size);
	settings.setValue("no_gi_nodes", converter_settings.no_gi_nodes);
	settings.setValue("inverted_shadowmaps", converter_settings.inverted_shadowmaps);
	settings.setValue("scale_to_atlasinfo", converter_settings.scale_to_atlasinfo);
	settings.setValue("scale_to_lightmap", converter_settings.scale_to_lightmap);
	settings.setValue("scale_to_shadowmap", converter_settings.scale_to_shadowmap);
	settings.setValue("override_lightmap", converter_settings.override_lightmap);
	settings.setValue("override_shadowmap", converter_settings.override_shadowmap);
	settings.setValue("override_lightmap_r", converter_settings.override_lightmap_r);
	settings.setValue("override_lightmap_g", converter_settings.override_lightmap_g);
	settings.setValue("override_lightmap_b", converter_settings.override_lightmap_b);
	settings.setValue("override_shadowmap_a", converter_settings.override_shadowmap_a);
	settings.sync();
}

void GIWindow::updateSettingsFromUi() {
	converter_settings.game_engine = (GameEngine) cb_game_engine->currentIndex();
	converter_settings.mode = (Mode) cb_mode->currentIndex();
	converter_settings.source_gi_directory = le_source_gi_directory->text();
	converter_settings.terrain_directory = le_terrain_directory->text();
	converter_settings.reference_path_file = le_reference_path_file->text();
	converter_settings.render_output_file = le_render_output_file->text();
	converter_settings.octree_cell_size = dsb_octree_cell_size->value();
	converter_settings.reference_distance_interval = dsb_reference_distance_interval->value();
	converter_settings.reference_max_intervals = sb_reference_max_intervals->value();
	converter_settings.quality_size_multiplier = dsb_quality_size_multiplier->value();
	converter_settings.max_texture_size = sb_max_texture_size->value();
	converter_settings.max_atlas_texture_size = sb_max_atlas_texture_size->value();
	converter_settings.no_gi_nodes = chk_no_gi_nodes->isChecked();
	converter_settings.inverted_shadowmaps = chk_inverted_shadowmaps->isChecked();
	converter_settings.scale_to_atlasinfo = rb_scale_to_atlasinfo->isChecked();
	converter_settings.scale_to_lightmap = rb_scale_to_lightmap->isChecked();
	converter_settings.scale_to_shadowmap = rb_scale_to_shadowmap->isChecked();
	converter_settings.override_lightmap = chk_override_lightmap->isChecked();
	converter_settings.override_shadowmap = chk_override_shadowmap->isChecked();
	converter_settings.override_lightmap_r = sb_override_lightmap_r->value();
	converter_settings.override_lightmap_g = sb_override_lightmap_g->value();
	converter_settings.override_lightmap_b = sb_override_lightmap_b->value();
	converter_settings.override_shadowmap_a = sb_override_shadowmap_a->value();
}

void GIWindow::updateUiFromSettings() {
	cb_game_engine->setCurrentIndex(converter_settings.game_engine);
	cb_mode->setCurrentIndex(converter_settings.mode);
	le_source_gi_directory->setText(converter_settings.source_gi_directory);
	le_terrain_directory->setText(converter_settings.terrain_directory);
	le_reference_path_file->setText(converter_settings.reference_path_file);
	le_render_output_file->setText(converter_settings.render_output_file);
	dsb_octree_cell_size->setValue(converter_settings.octree_cell_size);
	dsb_reference_distance_interval->setValue(converter_settings.reference_distance_interval);
	sb_reference_max_intervals->setValue(converter_settings.reference_max_intervals);
	dsb_quality_size_multiplier->setValue(converter_settings.quality_size_multiplier);
	sb_max_texture_size->setValue(converter_settings.max_texture_size);
	sb_max_atlas_texture_size->setValue(converter_settings.max_atlas_texture_size);
	chk_no_gi_nodes->setChecked(converter_settings.no_gi_nodes);
	chk_inverted_shadowmaps->setChecked(converter_settings.inverted_shadowmaps);
	rb_scale_to_atlasinfo->setChecked(converter_settings.scale_to_atlasinfo);
	rb_scale_to_lightmap->setChecked(converter_settings.scale_to_lightmap);
	rb_scale_to_shadowmap->setChecked(converter_settings.scale_to_shadowmap);
	chk_override_lightmap->setChecked(converter_settings.override_lightmap);
	chk_override_shadowmap->setChecked(converter_settings.override_shadowmap);
	sb_override_lightmap_r->setValue(converter_settings.override_lightmap_r);
	sb_override_lightmap_g->setValue(converter_settings.override_lightmap_g);
	sb_override_lightmap_b->setValue(converter_settings.override_lightmap_b);
	sb_override_shadowmap_a->setValue(converter_settings.override_shadowmap_a);

	modeChanged();
}

void GIWindow::aboutTriggered() {
		QMessageBox::about(this, "About " + AppName,
	"<p><b>" + AppName + "</b> is a free, open source, community-created global illumination atlas generator and converter for Hedgehog Engine games.</p>"
	"<p>" + AppName + " is free software: you can redistribute it and/or modify "
	"it under the terms of the GNU General Public License as published by "
	"the Free Software Foundation, either version 3 of the License, or "
	"(at your option) any later version.</p>"
	"<p>" + AppName +  " is distributed in the hope that it will be useful, "
	"but WITHOUT ANY WARRANTY; without even the implied warranty of "
	"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.</p>"
	"<p>Read AUTHORS.txt, LICENSE.txt and COPYRIGHT.txt for more details.</p>"
	"<ul>"
	"Dependencies used:"
	"<li><b>LibGens</b>: <a href=\"https://github.com/DarioSamo/libgens-sonicglvl\">Github Repository</a></li>"
	"<li><b>FreeImage</b>: <a href=\"http://freeimage.sourceforge.net/\">Homepage</a></li>"
	"<li><b>Qt 5.5.0</b>: <a href=\"http://qt-project.org/\">Homepage</a></li>"
	"</ul>");
}

void GIWindow::messageBox(QString text) {
	QMessageBox::about(this, AppName, text);
}

void GIWindow::logProgress(ProgressType progress_type, QString message) {
	QString final_message = QTime::currentTime().toString() + ": ";
	QString log_file_message = final_message + message;

	switch (progress_type) {
		case ProgressNormal:
			final_message += QString("<font color=\"White\">LOG: " + message + "</font>");
			break;
		case ProgressSuccess:
			final_message += QString("<font color=\"Green\">SUCCESS: " + message + "</font>");
			break;
		case ProgressError:
			final_message += QString("<font color=\"DarkOrange\">ERROR: " + message + "</font>");
			break;
		case ProgressWarning:
			final_message += QString("<font color=\"Gold\">WARNING: " + message + "</font>");
			break;
		case ProgressFatal:
			final_message += QString("<font color=\"Red\">ERROR: " + message + "</font>");
			break;
	}

	te_progress->appendHtml(final_message);
	te_progress->verticalScrollBar()->setValue(te_progress->verticalScrollBar()->maximum());

	// Log to file as well
	QFile log_file(LogPath);
    log_file.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream stream(&log_file);
    stream << log_file_message << Qt::endl << Qt::flush;

	QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

void GIWindow::convertTriggered() {
	setEnabled(false);

	// Reset log file
	QFile log_file(LogPath);
    log_file.open(QIODevice::WriteOnly);
	log_file.close();

	te_progress->clear();
	updateSettingsFromUi();
	saveSettings(DefaultSettingsPath);

	logProgress(ProgressNormal, "Starting converter...");

	bool result = convert();

	if (result)
		logProgress(ProgressSuccess, "Conversion complete!");
	else
		logProgress(ProgressFatal, "Conversion failed.");

	setEnabled(true);

	QApplication::alert(this);
	QApplication::beep();
}

void GIWindow::openSettingsTriggered() {
	QString ini_filename = QFileDialog::getOpenFileName(this, "Open configuration file...", QString(), "INI Configuration File (*.ini)");
	if (!ini_filename.isEmpty()) {
		loadSettings(ini_filename);
		updateUiFromSettings();
	}
}

void GIWindow::saveSettingsTriggered() {
	QString ini_filename = QFileDialog::getSaveFileName(this, "Save configuration file...", QString(), "INI Configuration File (*.ini)");
	if (!ini_filename.isEmpty()) {
		updateSettingsFromUi();
		saveSettings(ini_filename);
	}
}

void GIWindow::modeChanged() {
	Mode mode = (Mode) cb_mode->currentIndex();
	gb_source->setVisible(mode == PostRender);
	gb_pre_render->setVisible(mode == PreRender);
	gb_post_render->setVisible(mode == PostRender);
	wt_reference_path_file->setVisible(mode == PreRender);
	wt_render_output_file->setVisible(mode == PreRender);
}

void GIWindow::browseSourceTriggered() {
	QString source_gi_directory = QFileDialog::getExistingDirectory(this, tr("Open Source GI Directory..."), QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (!source_gi_directory.isEmpty()) {
		le_source_gi_directory->setText(source_gi_directory);
	}
}

void GIWindow::browseReferencePathTriggered() {
	QString reference_path_filename = QFileDialog::getOpenFileName(this, "Open Reference Path...", QString(), QString("Path File (*.path.xml)"));
	if (!reference_path_filename.isEmpty()) {
		le_reference_path_file->setText(reference_path_filename);
	}
}

void GIWindow::browseTerrainDirectoryTriggered() {
	QString terrain_directory = QFileDialog::getExistingDirectory(this, tr("Open Terrain Directory..."), QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (!terrain_directory.isEmpty()) {
		le_terrain_directory->setText(terrain_directory);
	}
}

void GIWindow::browseOutputListTriggered() {
	QString output_list_filename = QFileDialog::getSaveFileName(this, "Save Output List...", QString(), QString("Render List File (*.txt)"));
	if (!output_list_filename.isEmpty()) {
		le_render_output_file->setText(output_list_filename);
	}
}

void GIWindow::overrideLightmapChanged(int state) {
	sb_override_lightmap_r->setEnabled(chk_override_lightmap->isChecked());
	sb_override_lightmap_g->setEnabled(chk_override_lightmap->isChecked());
	sb_override_lightmap_b->setEnabled(chk_override_lightmap->isChecked());
}

void GIWindow::overrideShadowmapChanged(int state) {
	sb_override_shadowmap_a->setEnabled(chk_override_shadowmap->isChecked());
}

void GIWindow::textureSizeChanged() {
	int power_value = nextPowerOfTwo(sb_max_texture_size->value());
	if (sb_max_texture_size->value() != power_value)
		sb_max_texture_size->setValue(power_value);
}

void GIWindow::textureAtlasSizeChanged() {
	int power_value = nextPowerOfTwo(sb_max_atlas_texture_size->value());
	if (sb_max_atlas_texture_size->value() != power_value)
		sb_max_atlas_texture_size->setValue(power_value);
}

void GIWindow::communityGuideTriggered() {
	QDesktopServices::openUrl(QUrl(CommunityGuideURL));
}