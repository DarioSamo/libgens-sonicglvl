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

#include "stdafx.h"
#include "EditorViewport.h"
#include "EditorCamera.h"

EditorViewport::EditorViewport(OgreSystem *ogre_system, Ogre::SceneManager *scene_manager, Ogre::String viewport_name, QWidget *parent)
  : OgreViewportWidget(ogre_system, scene_manager, viewport_name, parent)
{
	editor_camera = NULL;
	setFocusPolicy(Qt::ClickFocus);
	last_mx = last_my = 0.0f;
}

EditorViewport::~EditorViewport()
{

}

bool EditorViewport::frameRenderingQueued(const Ogre::FrameEvent& evt) {
	if (editor_camera) {
		editor_camera->timerEvent(evt.timeSinceLastFrame);
	}
    return true;
}

void EditorViewport::mousePressEvent(QMouseEvent *event) {
	float fx = (float)event->x() / (float) width();
    float fy = (float)event->y() / (float) height();
	if (editor_camera) {
		editor_camera->mousePressEvent(event->button(), fx, fy);
	}
}

void EditorViewport::mouseMoveEvent(QMouseEvent *event) {
    float fx = (float)event->x() / (float) width();
    float fy = (float)event->y() / (float) height();
	float rx = fx - last_mx;
	float ry = fy - last_my;

	if (editor_camera) {
		if (editor_camera->mouseMoveEvent(rx, ry)) {
			// Cursor wrap-around widget
			QPoint global_top_left = mapToGlobal(QPoint(0, 0));
			QPoint global_bottom_right = mapToGlobal(QPoint(width(), height()));
			bool wrap_around = false;
			QPoint global_pos = mapToGlobal(event->pos());

			if (global_pos.x() < global_top_left.x()) {
				global_pos.setX(global_bottom_right.x());
				wrap_around = true;
			}

			if (global_pos.x() > global_bottom_right.x()) {
				global_pos.setX(global_top_left.x());
				wrap_around = true;
			}

			if (global_pos.y() < global_top_left.y()) {
				global_pos.setY(global_bottom_right.y());
				wrap_around = true;
			}

			if (global_pos.y() > global_bottom_right.y()) {
				global_pos.setY(global_top_left.y());
				wrap_around = true;
			}

			if (wrap_around) {
				QCursor::setPos(global_pos);
				QPoint local_pos = mapFromGlobal(global_pos);
				fx = (float) local_pos.x() / (float) width();
				fy = (float) local_pos.y() / (float) height();
			}

			// Set blank cursor
			setCursor(Qt::BlankCursor);
		}
		else setCursor(Qt::ArrowCursor);
	}

	last_mx = fx;
	last_my = fy;
}

void EditorViewport::mouseReleaseEvent(QMouseEvent *event) {
	float fx = (float)event->x() / (float) width();
    float fy = (float)event->y() / (float) height();
	if (editor_camera) {
		editor_camera->mouseReleaseEvent(event->button(), fx, fy);
	}
}

void EditorViewport::wheelEvent(QWheelEvent *event) {
	if (editor_camera) {
		editor_camera->wheelEvent(event->delta());
	}
}

void EditorViewport::keyPressEvent(QKeyEvent * event) {
	if (editor_camera) {
		if (editor_camera->keyPressEvent(event->key())) {
			return;
		}
	}

	OgreViewportWidget::keyPressEvent(event);
}

void EditorViewport::keyReleaseEvent(QKeyEvent * event) {
	if (editor_camera) {
		if (editor_camera->keyReleaseEvent(event->key())) {
			return;
		}
	}

	OgreViewportWidget::keyReleaseEvent(event);
}

void EditorViewport::setEditorCamera(EditorCamera *editor_camera) {
	this->editor_camera = editor_camera;
}