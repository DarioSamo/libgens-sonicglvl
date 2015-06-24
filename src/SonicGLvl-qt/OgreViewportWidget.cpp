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

#include "OgreViewportWidget.h"
#include "OgreSystem.h"

const Ogre::String OgreViewportWidget::WindowSuffixName = "_Window";
const Ogre::String OgreViewportWidget::CameraSuffixName = "_Camera";

OgreViewportWidget::OgreViewportWidget(OgreSystem *ogre_system, Ogre::SceneManager *scene_manager, Ogre::String viewport_name, QWidget *parent) : QWidget(parent, Qt::MSWindowsOwnDC) {
    this->scene_manager = scene_manager;

    setMouseTracking(true);
    setAttribute(Qt::WA_PaintOnScreen);

    createWindow(ogre_system, viewport_name);
    createCamera(viewport_name);
    createViewport();
    moveAndResize();

    ogre_system->getRoot()->addFrameListener(this);
}

OgreViewportWidget::~OgreViewportWidget() {

}

void OgreViewportWidget::createWindow(OgreSystem *ogre_system, Ogre::String viewport_name) {
    Ogre::NameValuePairList ogreWindowParams;
    ogreWindowParams["FSAA"] = "8 (Quality)";
    ogreWindowParams["vsync"] = "true";
    ogreWindowParams["parentWindowHandle"] = Ogre::StringConverter::toString((unsigned long)this->parentWidget()->winId());

    window = ogre_system->getRoot()->createRenderWindow(viewport_name+WindowSuffixName, width(), height(), false, &ogreWindowParams);

    window->setActive(true);
    window->setVisible(true);
    window->setAutoUpdated(false);

    WId window_id;
    window->getCustomAttribute("HWND", &window_id);

    QWidget::create(window_id);
}

void OgreViewportWidget::paintEvent(QPaintEvent*) {
    Ogre::Root::getSingleton()._fireFrameStarted();
    window->update();
    Ogre::Root::getSingleton()._fireFrameRenderingQueued();
    Ogre::Root::getSingleton()._fireFrameEnded();
}

void OgreViewportWidget::resizeEvent(QResizeEvent*) {
    moveAndResize();
}

void OgreViewportWidget::moveAndResize() {
    window->reposition(x(), y());
    window->resize(width(), height());
    window->windowMovedOrResized();
    camera->setAspectRatio(static_cast<Ogre::Real>(viewport->getActualWidth()) / static_cast<Ogre::Real>(viewport->getActualHeight()));
}

void OgreViewportWidget::createCamera(Ogre::String viewport_name) {
    camera = scene_manager->createCamera(viewport_name+CameraSuffixName);
    camera->setPosition(Ogre::Vector3(3.0F, 3.0F, 3.0F));
    camera->lookAt(Ogre::Vector3(0.0F, 0.0F, 0.F));
    camera->setNearClipDistance(0.1F);
}

void OgreViewportWidget::createViewport() {
    viewport = window->addViewport(camera);
    viewport->setBackgroundColour(Ogre::ColourValue(0.5, 0.5, 0.5));
    camera->setAspectRatio(Ogre::Real(viewport->getActualWidth()) / Ogre::Real(viewport->getActualHeight()));
}

QPaintEngine *OgreViewportWidget::paintEngine() const {
    return 0;
}
