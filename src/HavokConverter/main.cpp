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
#include <QApplication>
#include <QStyleFactory>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QFileInfo>
#include <QScreen>
#include "../QDarkPalette.cpp"

// Disable console output
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	setFusionStyle();

	HKWindow w(NULL);
	w.show();
	w.setWindowState(Qt::WindowActive);
	w.setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, w.size(), QGuiApplication::primaryScreen()->availableGeometry()));
	w.setWindowIcon(QIcon(":Icon/Icon.ico"));
	return a.exec();
}
