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

#include "OgreViewportWidget.h"

class EditorViewport : public OgreViewportWidget {
    Q_OBJECT
protected:
    bool enable_spinning;
    bool enable_panning;
    int last_mouse_x;
    int last_mouse_y;
    Ogre::Real viewer_angle_x;
    Ogre::Real viewer_angle_y;
    Ogre::Real zoom;
    Ogre::Vector3 viewer_center;
public:
    explicit EditorViewport(OgreSystem *ogre_system, Ogre::SceneManager *scene_manager, Ogre::String viewport_name, QWidget *parent = NULL);
    ~EditorViewport();

    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
    void wheelEvent(QWheelEvent * event);

    void spinCamera(float delta_x, float delta_y);
    void panCamera(float delta_x, float delta_y);
    void zoomCamera(float delta);
    void repositionCamera();
    void resetCamera();

    bool frameRenderingQueued(const Ogre::FrameEvent& evt);
signals:

public slots:
};
