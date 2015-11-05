//=========================================================================
//	  Copyright (c) 2015 SonicGLvl
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

#include "stdafx.h"
#include "EditorStage.h"

EditorStage::EditorStage() {
	stage_name = "";
}

QString EditorStage::stageName() {
	return stage_name;
}

bool EditorStage::load(QString filename, QString &error) {
	stage_filename = filename;

#ifdef SONICGLVL_LOST_WORLD
	// Point to LUA file, figure out the stage directory from that
	QFileInfo info(filename);
	QDir directory = info.dir();
	stage_name = directory.dirName();
	directory.cdUp();
	directory.cd("set");
	sets_directory = directory.absolutePath();
	
	if (!stage_name.isEmpty()) {
		return true;
	}
	else {
		error = "Couldn't detect a valid stage name from the directory where this file is stored.";
	}
#endif

	return false;
}

QString EditorStage::extension() {
#ifdef SONICGLVL_LOST_WORLD
	return ".lua";
#endif
}

QString EditorStage::filename() {
	return stage_filename;
}

#ifdef SONICGLVL_LOST_WORLD
QString EditorStage::setsDirectory() {
	return sets_directory;
}
#endif