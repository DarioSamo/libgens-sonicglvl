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

#pragma once

#include "QtCommon.h"
#include "OgreCommon.h"

namespace Ui {
class EditorWindow;
}

class OgreSystem;
class OgreViewportWidget;

class EditorWindow : public QMainWindow
{
    Q_OBJECT

public:
    static const int UpdateTimerMs;

    explicit EditorWindow(QWidget *parent = 0);
    ~EditorWindow();

    void timerEvent(QTimerEvent* event);
private:
    OgreSystem *ogre_system;
    Ui::EditorWindow *ui;
    OgreViewportWidget *viewport_widget;
    Ogre::SceneManager *stage_scene_manager;
    int timer_index;
    QElapsedTimer timer_elapsed;
};

