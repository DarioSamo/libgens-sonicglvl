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

#include "OgreViewportWidget.h"

class EditorCamera;

class EditorViewport : public OgreViewportWidget {
    Q_OBJECT
protected:
	EditorCamera *editor_camera;
	float last_mx, last_my;
public:
    explicit EditorViewport(OgreSystem *ogre_system, Ogre::SceneManager *scene_manager, Ogre::String viewport_name, QWidget *parent = NULL);
    ~EditorViewport();

    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
    void wheelEvent(QWheelEvent * event);
    bool frameRenderingQueued(const Ogre::FrameEvent& evt);
	void keyPressEvent(QKeyEvent * event);
	void keyReleaseEvent(QKeyEvent * event);
	void setEditorCamera(EditorCamera *editor_camera);
signals:

public slots:
};
