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

class OgreSystem {
protected:
    Ogre::Root *root;
    Ogre::RenderSystem *render_system;
public:
    OgreSystem();
    ~OgreSystem();

    Ogre::RenderSystem *loadRenderSystem(const std::string systemName);
    Ogre::SceneManager *createSceneManager();
    Ogre::Root *getRoot();
    void setupResources();
};
