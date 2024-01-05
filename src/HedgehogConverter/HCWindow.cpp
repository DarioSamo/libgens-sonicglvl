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

#include "HCWindow.h"
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
#include "assimp/Importer.hpp"
#include "assimp/importerdesc.h"


const QString HCWindow::AppName = "Hedgehog Converter";
const QString HCWindow::VersionNumber = "0.1";
const QString HCWindow::DefaultSettingsPath = "HedgehogConverter.ini";
const QString HCWindow::LogPath = "HedgehogConverter.log";
const QString HCWindow::CommunityGuideURL = "http://info.sonicretro.org/SCHG:Sonic_Generations";

HCWindow::HCWindow(QWidget *parent) : QMainWindow(parent) {
	setupUi(this);
	cb_game_engine->addItem("Generations");
	cb_game_engine->addItem("Unleashed");
	cb_game_engine->addItem("Lost World");
	cb_game_engine->setCurrentIndex(0);

	connect(action_open_settings, SIGNAL(triggered()), this, SLOT(openSettingsTriggered()));
	connect(action_save_settings, SIGNAL(triggered()), this, SLOT(saveSettingsTriggered()));
	connect(action_about, SIGNAL(triggered()), this, SLOT(aboutTriggered()));
	connect(action_about_qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
	connect(action_close, SIGNAL(triggered()), this, SLOT(close()));
	connect(action_community_guide, SIGNAL(triggered()), this, SLOT(communityGuideTriggered()));
	connect(chk_gen_materials, SIGNAL(stateChanged(int)), this, SLOT(generateMaterialsChanged(int)));
	connect(chk_copy_convert_textures, SIGNAL(stateChanged(int)), this, SLOT(copyConvertTexturesChanged(int)));
	connect(pb_add_source_models, SIGNAL(released()), this, SLOT(addSourceModelsTriggered()));
	connect(pb_remove_source_model, SIGNAL(released()), this, SLOT(removeSourceModelTriggered()));
	connect(pb_clear_source_models, SIGNAL(released()), this, SLOT(clearSourceModelsTriggered()));
	connect(pb_source_textures, SIGNAL(released()), this, SLOT(browseTexturesTriggered()));
	connect(pb_terrain_output, SIGNAL(released()), this, SLOT(browseOutputTriggered()));
	connect(pb_reset_transform, SIGNAL(released()), this, SLOT(resetTransformTriggered()));
	connect(pb_tag_sheet, SIGNAL(released()), this, SLOT(tagCheatSheetTriggered()));
	connect(pb_convert, SIGNAL(released()), this, SLOT(convertTriggered()));
	te_progress->setReadOnly(true);

	// Reset log file
	QFile log_file(LogPath);
    log_file.open(QIODevice::WriteOnly);
	log_file.close();

	logProgress(ProgressNormal, "Logging progress to " + LogPath);

	if (!QFileInfo(DefaultSettingsPath).exists()) {
		// Default settings
		converter_settings.model_source_paths = QStringList();
		converter_settings.texture_source_path = "";
		converter_settings.terrain_output_path = "";
		converter_settings.game_engine = Generations;
		converter_settings.merge_existing = true;
		converter_settings.generate_materials = true;
		converter_settings.copy_and_convert_textures = true;
		converter_settings.force_tags_layers = true;
		converter_settings.remove_material_tags = true;
		converter_settings.remove_model_tags = true;
		converter_settings.use_model_groups = true;
		converter_settings.convert_lights = true;
		converter_settings.group_cell_size = 25.0;
		converter_settings.position_x = converter_settings.position_y = converter_settings.position_z = 0.0;
		converter_settings.scale_x = converter_settings.scale_y = converter_settings.scale_z = 100.0;
		converter_settings.rotation_x = converter_settings.rotation_y = converter_settings.rotation_z = 0.0;

		saveSettings(DefaultSettingsPath);
	}
	else {
		loadSettings(DefaultSettingsPath);
	}

	updateUiFromSettings();
}

HCWindow::~HCWindow() {
	updateSettingsFromUi();
	saveSettings(DefaultSettingsPath);
}

void HCWindow::close() {
	QMainWindow::close();
}

void HCWindow::closeEvent(QCloseEvent *event) {
	if (!isEnabled())
		event->ignore();
}

void HCWindow::loadSettings(QString filename) {
	QSettings settings(filename, QSettings::IniFormat);
	converter_settings.model_source_paths = settings.value("model_source_paths", QString()).toString().split(";");

	// Remove all empty paths from list
	converter_settings.model_source_paths.removeAll(QString());

	converter_settings.texture_source_path = settings.value("texture_source_path", QString()).toString();
	converter_settings.terrain_output_path = settings.value("terrain_output_path", QString()).toString();
	converter_settings.game_engine = (GameEngine) settings.value("game_engine", 0).toInt();
	converter_settings.merge_existing = settings.value("merge_existing", true).toBool();
	converter_settings.generate_materials = settings.value("generate_materials", true).toBool();
	converter_settings.copy_and_convert_textures = settings.value("copy_and_convert_textures", true).toBool();
	converter_settings.force_tags_layers = settings.value("force_tags_layers", true).toBool();
	converter_settings.remove_material_tags = settings.value("remove_material_tags", true).toBool();
	converter_settings.remove_model_tags = settings.value("remove_model_tags", true).toBool();
	converter_settings.use_model_groups = settings.value("use_model_groups", true).toBool();
	converter_settings.convert_lights = settings.value("convert_lights", true).toBool();
	converter_settings.position_x = settings.value("position_x", 0.0).toDouble();
	converter_settings.position_y = settings.value("position_y", 0.0).toDouble();
	converter_settings.position_z = settings.value("position_z", 0.0).toDouble();
	converter_settings.scale_x = settings.value("scale_x", 100.0).toDouble();
	converter_settings.scale_y = settings.value("scale_y", 100.0).toDouble();
	converter_settings.scale_z = settings.value("scale_z", 100.0).toDouble();
	converter_settings.rotation_x = settings.value("rotation_x", 0.0).toDouble();
	converter_settings.rotation_y = settings.value("rotation_y", 0.0).toDouble();
	converter_settings.rotation_z = settings.value("rotation_z", 0.0).toDouble();
	converter_settings.group_cell_size = settings.value("group_cell_size", 25.0).toDouble();
}


void HCWindow::saveSettings(QString filename) {
	QSettings settings(filename, QSettings::IniFormat);
	settings.setValue("model_source_paths", converter_settings.model_source_paths.join(";"));
	settings.setValue("texture_source_path", converter_settings.texture_source_path);
	settings.setValue("terrain_output_path", converter_settings.terrain_output_path);
	settings.setValue("game_engine", converter_settings.game_engine);
	settings.setValue("merge_existing", converter_settings.merge_existing);
	settings.setValue("generate_materials", converter_settings.generate_materials);
	settings.setValue("copy_and_convert_textures", converter_settings.copy_and_convert_textures);
	settings.setValue("force_tags_layers", converter_settings.force_tags_layers);
	settings.setValue("remove_material_tags", converter_settings.remove_material_tags);
	settings.setValue("remove_model_tags", converter_settings.remove_model_tags);
	settings.setValue("use_model_groups", converter_settings.use_model_groups);
	settings.setValue("convert_lights", converter_settings.convert_lights);
	settings.setValue("position_x", converter_settings.position_x);
	settings.setValue("position_y", converter_settings.position_y);
	settings.setValue("position_z", converter_settings.position_z);
	settings.setValue("scale_x", converter_settings.scale_x);
	settings.setValue("scale_y", converter_settings.scale_y);
	settings.setValue("scale_z", converter_settings.scale_z);
	settings.setValue("rotation_x", converter_settings.rotation_x);
	settings.setValue("rotation_y", converter_settings.rotation_y);
	settings.setValue("rotation_z", converter_settings.rotation_z);
	settings.setValue("group_cell_size", converter_settings.group_cell_size);
	settings.sync();
}

void HCWindow::updateSettingsFromUi() {
	converter_settings.model_source_paths.clear();
	for (int i = 0; i < list_source_models->count(); i++)
		converter_settings.model_source_paths.append(list_source_models->item(i)->text());

	converter_settings.texture_source_path = le_source_textures->text();
	converter_settings.terrain_output_path = le_terrain_output->text();
	converter_settings.game_engine = (GameEngine) cb_game_engine->currentIndex();
	converter_settings.merge_existing = chk_merge_existing->isChecked();
	converter_settings.generate_materials = chk_gen_materials->isChecked();
	converter_settings.copy_and_convert_textures = chk_copy_convert_textures->isChecked();
	converter_settings.force_tags_layers = chk_force_tags_layers->isChecked();
	converter_settings.remove_material_tags = chk_remove_tags_materials->isChecked();
	converter_settings.remove_model_tags = chk_remove_tags_models->isChecked();
	converter_settings.use_model_groups = chk_model_groups->isChecked();
	converter_settings.position_x = dsb_position_x->value();
	converter_settings.position_y = dsb_position_y->value();
	converter_settings.position_z = dsb_position_z->value();
	converter_settings.scale_x = dsb_scale_x->value();
	converter_settings.scale_y = dsb_scale_y->value();
	converter_settings.scale_z = dsb_scale_z->value();
	converter_settings.rotation_x = dsb_rotation_x->value();
	converter_settings.rotation_y = dsb_rotation_y->value();
	converter_settings.rotation_z = dsb_rotation_z->value();
	converter_settings.group_cell_size = dsb_group_cell_size->value();
}

void HCWindow::updateUiFromSettings() {
	list_source_models->clear();
	foreach(QString path, converter_settings.model_source_paths)
		list_source_models->addItem(path);

	le_source_textures->setText(converter_settings.texture_source_path);
	le_terrain_output->setText(converter_settings.terrain_output_path);
	cb_game_engine->setCurrentIndex(converter_settings.game_engine);
	chk_merge_existing->setChecked(converter_settings.merge_existing);
	chk_gen_materials->setChecked(converter_settings.generate_materials);
	chk_copy_convert_textures->setChecked(converter_settings.copy_and_convert_textures);
	chk_force_tags_layers->setChecked(converter_settings.force_tags_layers);
	chk_remove_tags_materials->setChecked(converter_settings.remove_material_tags);
	chk_remove_tags_models->setChecked(converter_settings.remove_model_tags);
	chk_model_groups->setChecked(converter_settings.use_model_groups);
	dsb_position_x->setValue(converter_settings.position_x);
	dsb_position_y->setValue(converter_settings.position_y);
	dsb_position_z->setValue(converter_settings.position_z);
	dsb_scale_x->setValue(converter_settings.scale_x);
	dsb_scale_y->setValue(converter_settings.scale_y);
	dsb_scale_z->setValue(converter_settings.scale_z);
	dsb_rotation_x->setValue(converter_settings.rotation_x);
	dsb_rotation_y->setValue(converter_settings.rotation_y);
	dsb_rotation_z->setValue(converter_settings.rotation_z);
	dsb_group_cell_size->setValue(converter_settings.group_cell_size);

	generateMaterialsChanged(chk_gen_materials->isChecked());
	copyConvertTexturesChanged(chk_copy_convert_textures->isChecked());
}

void HCWindow::aboutTriggered() {
		QMessageBox::about(this, "About " + AppName,
	"<p><b>" + AppName + "</b> is a free, open source, community-created terrain model converter for Hedgehog Engine games.</p>"
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
	"<li><b>Qt 5.5.0</b>: <a href=\"http://qt-project.org/\">Homepage</a></li>"
	"<li><b>Assimp 3.2</b>: <a href=\"http://www.assimp.org/\">Homepage</a></li>"
	"</ul>");
}

void HCWindow::messageBox(QString text) {
	QMessageBox::about(this, AppName, text);
}

void HCWindow::logProgress(ProgressType progress_type, QString message) {
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
    stream << log_file_message << endl << flush;

	QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

void HCWindow::addSourceModelsTriggered() {
	Assimp::Importer importer;
	std::string extensions_assimp = "";
	importer.GetExtensionList(extensions_assimp);

	QStringList extensions= QString(extensions_assimp.c_str()).split(";");
	extensions.removeOne("*");

	// Build Extensions String for Open Dialog
	QString extensions_str = "All supported formats (";
	foreach(QString extension, extensions)
		extensions_str += extension + " ";
	extensions_str += ");;";

	// Specific filters
	foreach(QString extension, extensions) {
		size_t index = importer.GetImporterIndex(extension.toStdString().c_str());
		if (index != -1) {
			const aiImporterDesc *description = importer.GetImporterInfo(index);
			extensions_str += QString("%1 (%2);;").arg(description->mName).arg(extension);
		}
	}

	// Remove the last two ;;
	extensions_str.remove(extensions_str.size() - 3, 2);

	QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter(extensions_str);
    if (dialog.exec()) {
    	QStringList files = dialog.selectedFiles();
		foreach(QString file, files) {
			QList<QListWidgetItem *> items = list_source_models->findItems(file, Qt::MatchExactly);
			if (items.isEmpty()) {
				list_source_models->addItem(file);
			}
		}
	}
}

void HCWindow::removeSourceModelTriggered() {
	QList<QListWidgetItem *> items = list_source_models->selectedItems();
	foreach(QListWidgetItem *item, items) {
		delete item;
	}
}

void HCWindow::clearSourceModelsTriggered() {
	list_source_models->clear();
}

void HCWindow::browseTexturesTriggered() {
	QString source_texture_directory = QFileDialog::getExistingDirectory(this, tr("Open Textures Directory..."), QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (!source_texture_directory.isEmpty()) {
		le_source_textures->setText(source_texture_directory);
	}
}

void HCWindow::browseOutputTriggered() {
	QString output_directory = QFileDialog::getExistingDirectory(this, tr("Open Output Directory..."), QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (!output_directory.isEmpty()) {
		le_terrain_output->setText(output_directory);
	}
}

void HCWindow::convertTriggered() {
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
	beep();
}

void HCWindow::openSettingsTriggered() {
	QString ini_filename = QFileDialog::getOpenFileName(this, "Open configuration file...", QString(), "INI Configuration File (*.ini)");
	if (!ini_filename.isEmpty()) {
		loadSettings(ini_filename);
		updateUiFromSettings();
	}
}

void HCWindow::saveSettingsTriggered() {
	QString ini_filename = QFileDialog::getSaveFileName(this, "Save configuration file...", QString(), "INI Configuration File (*.ini)");
	if (!ini_filename.isEmpty()) {
		updateSettingsFromUi();
		saveSettings(ini_filename);
	}
}

void HCWindow::resetTransformTriggered() {
	dsb_position_x->setValue(0.0);
	dsb_position_y->setValue(0.0);
	dsb_position_z->setValue(0.0);
	dsb_scale_x->setValue(100.0);
	dsb_scale_y->setValue(100.0);
	dsb_scale_z->setValue(100.0);
	dsb_rotation_x->setValue(0.0);
	dsb_rotation_y->setValue(0.0);
	dsb_rotation_z->setValue(0.0);
}

void HCWindow::generateMaterialsChanged(int v) {
	chk_copy_convert_textures->setEnabled(v);
}

void HCWindow::copyConvertTexturesChanged(int v) {
	lbl_source_textures->setEnabled(v);
	le_source_textures->setEnabled(v);
	pb_source_textures->setEnabled(v);
}

void HCWindow::tagCheatSheetTriggered() {
	QMessageBox message_box;
    QSpacerItem *spacer = new QSpacerItem(1200, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

    message_box.setText("<html><head/><body><p align=\"center\"><span style=\" font-size:12pt; font-weight:600; text-decoration: underline;\">"
		"Tag Cheat Sheet</span><br/></p><p>Tags are indicated with <span style=\" font-weight:600;\">@TAG(value0[, value1, value2, value3, etc.])</span>, with [] indicating the optional parameters."
		" You can also write <span style=\" font-weight:600;\">@TAG()</span> without any values to use whatever default value is used. You can also use as many tags as you want by simply appending "
		"another one right next to it. <span style=\" font-weight:600;\">Supported values can be integers, floats, booleans or strings. </span>The type of value itself does not need to be defined, "
		"you can just type the value directly.</p><p>Using a consistent tag system allows us to serialize between any work pipeline that supports names and the game's formats without losing information."
		" Note however it is not the intended way to work in the long run when it comes to something like materials, as those are much better off being tweaked in something like a material editor to "
		"preview changes properly.</p><p><span style=\" font-weight:600;\">Example</span>:<span style=\" font-weight:600;\"> ghz_grass_a@LYR(punch)@SHDR(Common_dn)</span> would give you a material called "
		"<span style=\" font-weight:600;\">ghz_grass_a</span> (if removing tags is checked) which uses the drawing layer <span style=\" font-weight:600;\">punch</span> and the material shader "
		"<span style=\" font-weight:600;\">Common_dn</span>.</p><p align=\"center\"><span style=\" font-weight:600; text-decoration: underline;\">Supported Tags</span></p>"
		"<p><span style=\" font-weight:600; text-decoration: underline;\">Materials:</span></p><p><span style=\" font-weight:600;\">LYR: </span>Sets the drawing layer used by any submeshes that have this "
		"material assigned. Different drawing layers have different properties, such as drawing / reading the depth buffer, using the alpha blending channel, or just custom drawing orders altogether. </p>"
		"<p>The supported drawing layers are specific to each game. However, all games using the engine support 3 global layers, which are <span style=\" font-weight:600;\">solid</span>, "
		"<span style=\" font-weight:600;\">punch</span>, and <span style=\" font-weight:600;\">trans</span>. The first is the layer used by default for most meshes. The last two are used for transparent "
		"meshes, the only difference being the former affects the depth buffer and ignores alpha pixels below a certain value (ideal for stuff like leaves), while the latter doesn't (good for anything that"
		" needs to be drawn on top of something else).</p><p>For example, ghz_tree_leaf01@LYR(punch) will make any submeshes assigned to this material use the <span style=\" font-weight:600;\">punch</span> "
		"layer, and therefore have punch-through alpha support. <span style=\" font-weight:600;\">If you're unsure of how to use drawing layers yet, you'll only need to experiment with them if you want to "
		"do special effects like textures with alpha channels.</span></p><p><span style=\" font-weight:600;\">TXTR: </span>Assigns a texture's name to a texture slot. You can write the texture filename with "
		"its extension or ignore it. Not all texture slots can get properly converted from all work pipelines, so if you don't feel like using a material editor you can also assign textures like this. It "
		"takes the texture unit slot as its first value and the texture filename as its second value.</p><p>For example, ghz_ground_01_b@TXTR(gloss, ghz_ground_pow.dds) will assign the given texture to the"
		" gloss texture unit.</p><p><span style=\" font-weight:600;\">SHDR: </span>Assigns the shader used by the material without using a material editor. e.g. ghz_sky_01@SHDR(Sky_d) assigns the Sky_d shader"
		" to this material. Notice it's up to you to properly assign any parameters the shader might need. <span style=\" font-weight:600;\">If unsure, it's recommended to use a material editor instead.</span>"
		"</p><p><span style=\" font-weight:600;\">PMTR: </span>Sets a parameter used by the material with the parameter name as the first value and the 4 corresponding floating point numbers as the next 4"
		" values. The parameter will be added if it does not exist already, or the existing values will be set if it does. e.g. ghz_water_01@PRM(g_WaterParam, 10, 1, 20, 0.1) sets the 4 values of g_WaterParam"
		" on this material. </p><p><span style=\" font-weight:600;\">CULL: </span>Sets the backface culling flag for this material. If this flag is disabled, the faces on the meshes that use this material "
		"will be visible from their back as well. <span style=\" font-weight:600;\">Normally recommended to use for transparent or punch-through meshes</span>, but keep in mind the GI will be visible the"
		" same way from both sides, which can be unrealistic depending on the mesh itself. True by default.</p><p><span style=\" font-weight:600;\">ADD: </span>Sets the add blending flag for this material."
		" If this flag is enabled, the material will use a different blending mode where the RGB color values are <span style=\" font-weight:600;\">added</span> to the screen instead of "
		"<span style=\" font-weight:600;\">blended with alpha</span>. This is really useful for meshes that use textures that do not use alpha channels, but use effects like black representing the transparent "
		"parts. Effects like a starry sky or light beams are very simple to do with this flag. False by default.</p><p><span style=\" font-weight:600; text-decoration: underline;\">Nodes:</span></p>"
		"<p><span style=\" font-weight:600;\">TGRP: </span>All children of the tagged node will be part of a custom terrain group instead of being automatically generated. An optional integer value is "
		"supported to define the Subset ID of the custom terrain group.e.g. @TGRP(27) makes all the children of this node be part of a terrain group with the Subset ID 27, which can be used by the game to"
		" identify the terrain group.</p></body></html>");

    QGridLayout *layout = (QGridLayout *) message_box.layout();
    layout->addItem(spacer, layout->rowCount(), 0, 1, layout->columnCount());
    message_box.exec();
}

void HCWindow::communityGuideTriggered() {
	QDesktopServices::openUrl(QUrl(CommunityGuideURL));
}

void HCWindow::beep() {
	QApplication::alert(this);
	QApplication::beep();
}