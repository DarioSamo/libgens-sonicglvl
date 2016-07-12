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

#include "HCMaterialDialog.h"

HCMaterialDialog::HCMaterialDialog(QMap<string, bool> *material_override_map, QWidget *parent) : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint) {
	setupUi(this);
	override_map = material_override_map;

	connect(pb_check_selected, SIGNAL(released()), this, SLOT(checkSelectedTriggered()));
	connect(pb_uncheck_selected, SIGNAL(released()), this, SLOT(uncheckSelectedTriggered()));
	connect(pb_check_all, SIGNAL(released()), this, SLOT(checkAllTriggered()));
	connect(pb_uncheck_all, SIGNAL(released()), this, SLOT(uncheckAllTriggered()));
	connect(pb_select_rest, SIGNAL(released()), this, SLOT(selectRestTriggered()));
}

void HCMaterialDialog::checkSelectedTriggered() {
	QList<QListWidgetItem *> items = list_materials->selectedItems();
	foreach(QListWidgetItem *item, items) {
		item->setCheckState(Qt::Checked);
	}
}

void HCMaterialDialog::uncheckSelectedTriggered() {
	QList<QListWidgetItem *> items = list_materials->selectedItems();
	foreach(QListWidgetItem *item, items) {
		item->setCheckState(Qt::Unchecked);
	}
}

void HCMaterialDialog::checkAllTriggered() {
	int count = list_materials->count();
	for (int i = 0; i < count; i++) {
		QListWidgetItem *item = list_materials->item(i);
		item->setCheckState(Qt::Checked);
	}
}

void HCMaterialDialog::uncheckAllTriggered() {
	int count = list_materials->count();
	for (int i = 0; i < count; i++) {
		QListWidgetItem *item = list_materials->item(i);
		item->setCheckState(Qt::Unchecked);
	}
}

void HCMaterialDialog::selectRestTriggered() {
	int count = list_materials->count();
	for (int i = 0; i < count; i++) {
		QListWidgetItem *item = list_materials->item(i);
		item->setSelected(!item->isSelected());
	}
}

void HCMaterialDialog::setFBXName(QString v) {
	lbl_explanation->setText("Some of the materials found in " + v + " already exist in the terrain directory that is being used.\n\n"
							 "Please check which materials you want to overwrite and leave unchecked the ones you want to keep.");
}

void HCMaterialDialog::setupOverrideMap() {
	list_materials->clear();
	QList<string> keys = override_map->keys();
	foreach(string key, keys) {
		QListWidgetItem *item = new QListWidgetItem(list_materials);
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable); 
		item->setText(key.c_str());
		item->setCheckState(Qt::Checked);
	}
}

void HCMaterialDialog::updateOverrideMap() {
	int count = list_materials->count();
	for (int i = 0; i < count; i++) {
		QListWidgetItem *item = list_materials->item(i);
		string name = item->text().toStdString();
		(*override_map)[name] = item->checkState() == Qt::Checked;
	}
}
