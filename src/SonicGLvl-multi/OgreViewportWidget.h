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

class OgreSystem;

class OgreViewportWidget : public QWidget, public Ogre::FrameListener {
    Q_OBJECT

protected:
    Ogre::RenderWindow *window;
    Ogre::Camera *camera;
    Ogre::Viewport *viewport;
    Ogre::SceneManager *scene_manager;
public:
	static const Ogre::String LowEndTechniqueName;
	static const Ogre::ColourValue BackgroundColour;
    static const Ogre::String WindowSuffixName;
    static const Ogre::String CameraSuffixName;

    explicit OgreViewportWidget(OgreSystem *ogre_system, Ogre::SceneManager *scene_manager, Ogre::String viewport_name, QWidget *parent = NULL);
    ~OgreViewportWidget();

    void createWindow(OgreSystem *ogre_system, Ogre::String viewport_name);
    void paintEvent(QPaintEvent*);
    void resizeEvent(QResizeEvent*);
    void moveAndResize();
    void createCamera(Ogre::String viewport_name);
    void createViewport();
	Ogre::Camera *getCamera();
    QPaintEngine *paintEngine() const;
};
