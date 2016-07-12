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

#include "ui_HCMaterialDialog.h"
#include <string>

using namespace std;

class HCMaterialDialog : public QDialog, Ui_HCMaterialDialog {
	Q_OBJECT
protected:
	QMap<string, bool> *override_map;
public:
	HCMaterialDialog(QMap<string, bool> *material_override_map, QWidget *parent = NULL);
	void setupOverrideMap();
	void updateOverrideMap();
	void setFBXName(QString v);
private slots:
	void checkSelectedTriggered();
	void uncheckSelectedTriggered();
	void checkAllTriggered();
	void uncheckAllTriggered();
	void selectRestTriggered();
};