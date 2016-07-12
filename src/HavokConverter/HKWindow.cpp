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

#include "HKWindow.h"
#include "HKPropertyDialog.h"
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

const QString HKWindow::AppName = "Havok Converter";
const QString HKWindow::VersionNumber = "0.1";
const QString HKWindow::DefaultSettingsPath = "HavokConverter.ini";
const QString HKWindow::LogPath = "HavokConverter.log";
const QString HKWindow::CommunityGuideURL = "http://info.sonicretro.org/SCHG:Sonic_Generations";

HKWindow::HKWindow(QWidget *parent) : QMainWindow(parent) {
	setupUi(this);
	cb_mode->addItem("Collision");
	//cb_mode->addItem("Rigid Bodies"); // TO IMPLEMENT
	//cb_mode->addItem("Animation"); // TO IMPLEMENT
	cb_mode->setCurrentIndex(0);
	havok_enviroment = NULL;

	connect(action_open_settings, SIGNAL(triggered()), this, SLOT(openSettingsTriggered()));
	connect(action_save_settings, SIGNAL(triggered()), this, SLOT(saveSettingsTriggered()));
	connect(action_about, SIGNAL(triggered()), this, SLOT(aboutTriggered()));
	connect(action_about_qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
	connect(action_close, SIGNAL(triggered()), this, SLOT(close()));
	connect(action_community_guide, SIGNAL(triggered()), this, SLOT(communityGuideTriggered()));
	connect(pb_add_source_models, SIGNAL(released()), this, SLOT(addSourceModelsTriggered()));
	connect(pb_remove_source_model, SIGNAL(released()), this, SLOT(removeSourceModelTriggered()));
	connect(pb_clear_source_models, SIGNAL(released()), this, SLOT(clearSourceModelsTriggered()));
	connect(pb_output_file, SIGNAL(released()), this, SLOT(browseOutputTriggered()));
	connect(pb_reset_transform, SIGNAL(released()), this, SLOT(resetTransformTriggered()));
	connect(pb_up, SIGNAL(released()), this, SLOT(tagMoveUpTriggered()));
	connect(pb_down, SIGNAL(released()), this, SLOT(tagMoveDownTriggered()));
	connect(pb_new, SIGNAL(released()), this, SLOT(newTagTriggered()));
	connect(pb_delete, SIGNAL(released()), this, SLOT(deleteTagTriggered()));
	connect(tb_properties, SIGNAL(cellChanged(int, int)), this, SLOT(tagChangedTriggered(int, int)));
	connect(tb_properties, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(tagDoubleClickTriggered(int, int)));
	connect(pb_convert, SIGNAL(released()), this, SLOT(convertTriggered()));
	te_progress->setReadOnly(true);

	// Reset log file
	QFile log_file(LogPath);
    log_file.open(QIODevice::WriteOnly);
	log_file.close();

	logProgress(ProgressNormal, "Logging progress to " + LogPath);

	if (!QFileInfo(DefaultSettingsPath).exists()) {
		// Default settings
		converter_settings.mode = Collision;
		converter_settings.model_source_paths = QStringList();
		converter_settings.output_file = "";
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

HKWindow::~HKWindow() {
	updateSettingsFromUi();
	saveSettings(DefaultSettingsPath);
}

void HKWindow::close() {
	QMainWindow::close();
}

void HKWindow::closeEvent(QCloseEvent *event) {
	if (!isEnabled())
		event->ignore();
}

void HKWindow::loadSettings(QString filename) {
	QSettings settings(filename, QSettings::IniFormat);
	converter_settings.model_source_paths = settings.value("model_source_paths", QString()).toString().split(";");
	converter_settings.model_source_paths.removeAll(QString());
	converter_settings.output_file = settings.value("output_file", QString()).toString();
	converter_settings.mode = (Mode) settings.value("mode", 0).toInt();
	converter_settings.position_x = settings.value("position_x", 0.0).toDouble();
	converter_settings.position_y = settings.value("position_y", 0.0).toDouble();
	converter_settings.position_z = settings.value("position_z", 0.0).toDouble();
	converter_settings.scale_x = settings.value("scale_x", 100.0).toDouble();
	converter_settings.scale_y = settings.value("scale_y", 100.0).toDouble();
	converter_settings.scale_z = settings.value("scale_z", 100.0).toDouble();
	converter_settings.rotation_x = settings.value("rotation_x", 0.0).toDouble();
	converter_settings.rotation_y = settings.value("rotation_y", 0.0).toDouble();
	converter_settings.rotation_z = settings.value("rotation_z", 0.0).toDouble();

	converter_settings.property_tags.clear();
	QString property_tag_string = settings.value("property_tags", QString()).toString();
	QStringList tag_list = property_tag_string.split("|");
	int tag_count = tag_list.size() / 3;
	for (int t = 0; t < tag_count; t++) {
		HKPropertyTag tag;
		tag.tag = tag_list[t * 3];
		tag.description = tag_list[t * 3 + 1];
			
		QStringList key_list = tag_list[t * 3 + 2].split(" ");
		int key_count = key_list.size() / 3;
		for (int k = 0; k < key_count; k++) {
			HKPropertyValue value;
			value.key = key_list[k * 3].toInt();
			value.value = key_list[k * 3 + 1].toInt();
			value.bitwise = (HKBitwise) key_list[k * 3 + 2].toInt();
			tag.values.append(value);
		}

		converter_settings.property_tags.append(tag);
	}
}


void HKWindow::saveSettings(QString filename) {
	QSettings settings(filename, QSettings::IniFormat);
	settings.setValue("model_source_paths", converter_settings.model_source_paths.join(";"));
	settings.setValue("output_file", converter_settings.output_file);
	settings.setValue("mode", converter_settings.mode);
	settings.setValue("position_x", converter_settings.position_x);
	settings.setValue("position_y", converter_settings.position_y);
	settings.setValue("position_z", converter_settings.position_z);
	settings.setValue("scale_x", converter_settings.scale_x);
	settings.setValue("scale_y", converter_settings.scale_y);
	settings.setValue("scale_z", converter_settings.scale_z);
	settings.setValue("rotation_x", converter_settings.rotation_x);
	settings.setValue("rotation_y", converter_settings.rotation_y);
	settings.setValue("rotation_z", converter_settings.rotation_z);

	QString property_tag_string = "";
	foreach(const HKPropertyTag &tag, converter_settings.property_tags) {
		QString values_str = "";
		foreach(const HKPropertyValue &value, tag.values)
			values_str += QString("%1 %2 %3 ").arg(value.key).arg(value.value).arg((int) value.bitwise);
		
		property_tag_string += QString("%1|%2|%3|").arg(tag.tag).arg(tag.description).arg(values_str);
	}
	settings.setValue("property_tags", property_tag_string);

	settings.sync();
}

void HKWindow::updateSettingsFromUi() {
	converter_settings.model_source_paths.clear();
	for (int i = 0; i < list_source_models->count(); i++)
		converter_settings.model_source_paths.append(list_source_models->item(i)->text());

	converter_settings.output_file = le_output_file->text();
	converter_settings.mode = (Mode) cb_mode->currentIndex();
	converter_settings.position_x = dsb_position_x->value();
	converter_settings.position_y = dsb_position_y->value();
	converter_settings.position_z = dsb_position_z->value();
	converter_settings.scale_x = dsb_scale_x->value();
	converter_settings.scale_y = dsb_scale_y->value();
	converter_settings.scale_z = dsb_scale_z->value();
	converter_settings.rotation_x = dsb_rotation_x->value();
	converter_settings.rotation_y = dsb_rotation_y->value();
	converter_settings.rotation_z = dsb_rotation_z->value();
}

void HKWindow::updateUiFromSettings() {
	list_source_models->clear();
	foreach(QString path, converter_settings.model_source_paths)
		list_source_models->addItem(path);

	le_output_file->setText(converter_settings.output_file);
	cb_mode->setCurrentIndex(converter_settings.mode);
	dsb_position_x->setValue(converter_settings.position_x);
	dsb_position_y->setValue(converter_settings.position_y);
	dsb_position_z->setValue(converter_settings.position_z);
	dsb_scale_x->setValue(converter_settings.scale_x);
	dsb_scale_y->setValue(converter_settings.scale_y);
	dsb_scale_z->setValue(converter_settings.scale_z);
	dsb_rotation_x->setValue(converter_settings.rotation_x);
	dsb_rotation_y->setValue(converter_settings.rotation_y);
	dsb_rotation_z->setValue(converter_settings.rotation_z);

	updateTagsTableTriggered();
}

void HKWindow::aboutTriggered() {
		QMessageBox::about(this, "About " + AppName,
	"<p><b>" + AppName + "</b> is a free, open source, community-created collision and animation converter for Hedgehog Engine games.</p>"
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
	"<li><b>Havok 2010 2.0 SDK</b></li>"
	"</ul>");
}

void HKWindow::messageBox(QString text) {
	QMessageBox::about(this, AppName, text);
}

void HKWindow::logProgress(ProgressType progress_type, QString message) {
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

void HKWindow::addSourceModelsTriggered() {
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

void HKWindow::removeSourceModelTriggered() {
	QList<QListWidgetItem *> items = list_source_models->selectedItems();
	foreach(QListWidgetItem *item, items) {
		delete item;
	}
}

void HKWindow::clearSourceModelsTriggered() {
	list_source_models->clear();
}

void HKWindow::browseOutputTriggered() {
	QString output_file = QFileDialog::getSaveFileName(this, tr("Choose Output File..."), QString(), "*.hkx");
	if (!output_file.isEmpty()) {
		le_output_file->setText(output_file);
	}
}

void HKWindow::convertTriggered() {
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


void HKWindow::tagMoveUpTriggered() {
	QModelIndexList indexes = tb_properties->selectionModel()->selection().indexes();
	if (!indexes.isEmpty()) {
		int select_index = indexes.first().row();
		if (select_index > 0) {
			HKPropertyTag swap = converter_settings.property_tags[select_index - 1];
			converter_settings.property_tags[select_index - 1] = converter_settings.property_tags[select_index];
			converter_settings.property_tags[select_index] = swap;

			updateTagsTableTriggered();
			tb_properties->selectRow(select_index - 1);
		}
	}
}

void HKWindow::tagMoveDownTriggered() {
	QModelIndexList indexes = tb_properties->selectionModel()->selection().indexes();
	if (!indexes.isEmpty()) {
		int select_index = indexes.first().row();
		if (select_index < converter_settings.property_tags.size() - 1) {
			HKPropertyTag swap = converter_settings.property_tags[select_index + 1];
			converter_settings.property_tags[select_index + 1] = converter_settings.property_tags[select_index];
			converter_settings.property_tags[select_index] = swap;

			updateTagsTableTriggered();
			tb_properties->selectRow(select_index + 1);
		}
	}
}

void HKWindow::newTagTriggered() {
	HKPropertyTag tag;
	tag.tag = "newtag";

	QModelIndexList indexes = tb_properties->selectionModel()->selection().indexes();
	int select_index = 0;
	if (!indexes.isEmpty()) {
		select_index = indexes.first().row();
		converter_settings.property_tags.insert(select_index, tag);
	}
	else {
		select_index = converter_settings.property_tags.size();
		converter_settings.property_tags.append(tag);
	}

	updateTagsTableTriggered();
	tb_properties->selectRow(select_index);
}

void HKWindow::deleteTagTriggered() {
	QModelIndexList indexes = tb_properties->selectionModel()->selection().indexes();
	if (!indexes.isEmpty()) {
		int index = indexes.first().row();
		converter_settings.property_tags.remove(index);
		updateTagsTableTriggered();
	}
}

void HKWindow::tagChangedTriggered(int row, int column) {
	if (row < converter_settings.property_tags.size()) {
		QString text = tb_properties->item(row, column)->text();
		converter_settings.property_tags[row].tag = text;
	}
}

void HKWindow::tagDoubleClickTriggered(int row, int column) {
	if (row < converter_settings.property_tags.size() && column == 1) {
		HKPropertyDialog dialog(this, &converter_settings.property_tags[row]);
		if (dialog.exec()) {
			converter_settings.property_tags[row].description = dialog.getDescription();
			converter_settings.property_tags[row].values = dialog.getValues();
			updateTagsTableTriggered();
			tb_properties->selectRow(row);
		}
	}
}

void HKWindow::updateTagsTableTriggered() {
	disconnect(tb_properties, SIGNAL(cellChanged(int, int)), this, SLOT(tagChangedTriggered(int, int)));

	tb_properties->clearContents();
	tb_properties->setRowCount(converter_settings.property_tags.size());

	int row = 0;
	foreach(const HKPropertyTag &tag, converter_settings.property_tags) {
		QString values_str = tag.values.isEmpty() ? "{ Empty }" : QString();
		foreach(const HKPropertyValue &value, tag.values) {
			values_str += QString("%1: %2 (%3); ").arg(value.key).arg(value.value).arg(value.bitwise == HKBitwise_SET ? "SET" : "OR");
		}
		
		QTableWidgetItem *tag_item = new QTableWidgetItem(tag.tag);
		QTableWidgetItem *value_item = new QTableWidgetItem(values_str);
		tag_item->setTextAlignment(Qt::AlignHCenter);
		tag_item->setToolTip(tag.description);
		value_item->setTextAlignment(Qt::AlignHCenter);
		value_item->setToolTip(tag.description);
		value_item->setFlags(value_item->flags() ^ Qt::ItemIsEditable);
		tb_properties->setItem(row, 0, tag_item);
		tb_properties->setItem(row, 1, value_item);
		tb_properties->setRowHeight(row, 20);
		row++;
	}
	
	connect(tb_properties, SIGNAL(cellChanged(int, int)), this, SLOT(tagChangedTriggered(int, int)));
}

void HKWindow::openSettingsTriggered() {
	QString ini_filename = QFileDialog::getOpenFileName(this, "Open configuration file...", QString(), "INI Configuration File (*.ini)");
	if (!ini_filename.isEmpty()) {
		loadSettings(ini_filename);
		updateUiFromSettings();
	}
}

void HKWindow::saveSettingsTriggered() {
	QString ini_filename = QFileDialog::getSaveFileName(this, "Save configuration file...", QString(), "INI Configuration File (*.ini)");
	if (!ini_filename.isEmpty()) {
		updateSettingsFromUi();
		saveSettings(ini_filename);
	}
}

void HKWindow::resetTransformTriggered() {
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

void HKWindow::communityGuideTriggered() {
	QDesktopServices::openUrl(QUrl(CommunityGuideURL));
}

void HKWindow::beep() {
	QApplication::alert(this);
	QApplication::beep();
}
