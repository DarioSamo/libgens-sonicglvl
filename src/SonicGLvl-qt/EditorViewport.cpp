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

#include "EditorViewport.h"

EditorViewport::EditorViewport(OgreSystem *ogre_system, Ogre::SceneManager *scene_manager, Ogre::String viewport_name, QWidget *parent)
  : OgreViewportWidget(ogre_system, scene_manager, viewport_name, parent)
{
    enable_spinning = enable_panning = false;
    last_mouse_x = last_mouse_y = 0;
    viewer_angle_x = viewer_angle_y = 0.0F;
    zoom = 1.0F;

    resetCamera();
}

EditorViewport::~EditorViewport()
{

}

bool EditorViewport::frameRenderingQueued(const Ogre::FrameEvent& evt) {
    repositionCamera();
    return true;
}

void EditorViewport::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        enable_spinning = true;
        event->accept();
    }

    if (event->button() == Qt::RightButton) {
        enable_panning = true;
        event->accept();
    }
}

void EditorViewport::mouseMoveEvent(QMouseEvent *event) {
    int mouse_x = event->x();
    int mouse_y = event->y();
    int delta_x = mouse_x - last_mouse_x;
    int delta_y = mouse_y - last_mouse_y;
    float delta_f_x = (float)delta_x / (float)width();
    float delta_f_y = (float)delta_y / (float)height();

    if (enable_spinning) {
        spinCamera(delta_f_x, delta_f_y);
    }

    if (enable_panning) {
        panCamera(delta_f_x, delta_f_y);
    }

    last_mouse_x = mouse_x;
    last_mouse_y = mouse_y;

    event->accept();

}
void EditorViewport::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        enable_spinning = false;
        event->accept();
    }

    if (event->button() == Qt::RightButton) {
        enable_panning = false;
        event->accept();
    }
}
void EditorViewport::wheelEvent(QWheelEvent *event) {
    zoomCamera(event->delta());
    event->accept();
}


void EditorViewport::spinCamera(float delta_x, float delta_y) {
    viewer_angle_x += delta_x * 4.0f;
    viewer_angle_y += delta_y * -4.0f;

    if (viewer_angle_x >= Ogre::Math::TWO_PI) viewer_angle_x -= Ogre::Math::TWO_PI;
    if (viewer_angle_x < 0) viewer_angle_x += Ogre::Math::TWO_PI;

    if (viewer_angle_y >= Ogre::Math::HALF_PI - 0.1) viewer_angle_y = Ogre::Math::HALF_PI - 0.1;
    if (viewer_angle_y < -Ogre::Math::HALF_PI + 0.1) viewer_angle_y = -Ogre::Math::HALF_PI + 0.1;
}

void EditorViewport::panCamera(float delta_x, float delta_y) {
    viewer_center += camera->getOrientation() * Ogre::Vector3(delta_x * -2.0f, -delta_y * -2.0f, 0.0);
}

void EditorViewport::zoomCamera(float delta) {
    zoom += delta * -0.001f;

    if (zoom < 0.01f) zoom = 0.01f;
}

void EditorViewport::repositionCamera() {
    Ogre::Quaternion rotation_x;
    Ogre::Quaternion rotation_y;
    rotation_x.FromAngleAxis(Ogre::Radian(viewer_angle_x), Ogre::Vector3::UNIT_Y);
    rotation_y.FromAngleAxis(Ogre::Radian(viewer_angle_y), Ogre::Vector3::UNIT_X);

    Ogre::Vector3 new_position = viewer_center + ((rotation_x * rotation_y) * Ogre::Vector3(0, 0, -3.0 * zoom));
    camera->setPosition(new_position);
    camera->lookAt(viewer_center);
}

void EditorViewport::resetCamera() {
    viewer_center = Ogre::Vector3(0, 0.5, 0);
    viewer_angle_x = Ogre::Math::PI;
    viewer_angle_y = 0.0f;
    zoom = 1.0f;
}
