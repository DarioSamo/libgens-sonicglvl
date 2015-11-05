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
#include "EditorViewportsWidget.h"
#include "EditorViewport.h"

EditorViewportsWidget::EditorViewportsWidget(QWidget *parent) : QWidget(parent) {


}

EditorViewportsWidget::~EditorViewportsWidget() {

}

void EditorViewportsWidget::setup(OgreSystem *ogre_system, Ogre::SceneManager *scene_manager) {
    grid_layout = new QGridLayout(this);

    EditorViewport *viewport_widget = new EditorViewport(ogre_system, scene_manager, "ViewportA", this);
    viewports.append(viewport_widget);
    grid_layout->addWidget(viewport_widget, 0, 0);

    setLayout(grid_layout);
}

void EditorViewportsWidget::update() {
    foreach(EditorViewport *viewport, viewports) {
        viewport->update();
    }
}

void EditorViewportsWidget::createResources() {

}

EditorViewport *EditorViewportsWidget::getMainViewport() {
	return viewports.first();
}