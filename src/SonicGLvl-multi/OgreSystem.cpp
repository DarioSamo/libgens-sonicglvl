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
#include "OgreSystem.h"

OgreSystem::OgreSystem() {
    root = new Ogre::Root();
    render_system = NULL;

    Ogre::RenderSystem* Direct3D9RenderSystem = NULL;

    try {
        root->loadPlugin("RenderSystem_Direct3D9");
    }
    catch (...) {
        qWarning("Failed to load Direct3D9 plugin");
    }

    Ogre::RenderSystemList list = root->getAvailableRenderers();
    Ogre::RenderSystemList::iterator i = list.begin();

    while (i != list.end()) {
        if ((*i)->getName() == "Direct3D9 Rendering Subsystem") {
            Direct3D9RenderSystem = *i;
        }
        i++;
    }

    if (!Direct3D9RenderSystem) {
        qCritical("No rendering subsystems found");
        exit(0);
    }

    if (Direct3D9RenderSystem != 0) {
        render_system = Direct3D9RenderSystem;
    }

    root->setRenderSystem(render_system);
    root->initialise(false);
}

OgreSystem::~OgreSystem() {

}

Ogre::Root *OgreSystem::getRoot() {
    return root;
}

Ogre::SceneManager *OgreSystem::createSceneManager() {
    return root->createSceneManager(Ogre::ST_GENERIC);
}

void OgreSystem::setupResources() {
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}
