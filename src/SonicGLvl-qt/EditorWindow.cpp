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

#include "EditorWindow.h"
#include "ui_EditorWindow.h"
#include "OgreSystem.h"
#include "EditorViewerGrid.h"

const int EditorWindow::UpdateTimerMs = 4;

EditorWindow::EditorWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::EditorWindow) {
    ui->setupUi(this);

    ogre_system = new OgreSystem();
    stage_scene_manager = ogre_system->createSceneManager();
    ui->viewports_widget->setup(ogre_system, stage_scene_manager);
    ogre_system->setupResources();

    EditorViewerGrid *viewer_grid = new EditorViewerGrid(stage_scene_manager);

    timer_index = startTimer(UpdateTimerMs);
    timer_elapsed.start();

    resize(1600, 900);
}

EditorWindow::~EditorWindow() {
    delete ui;
    delete ogre_system;
    killTimer(timer_index);
}

void EditorWindow::timerEvent(QTimerEvent* event) {
    ui->viewports_widget->update();
}
