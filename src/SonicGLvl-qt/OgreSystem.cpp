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

#include "OgreSystem.h"
#include "QtCommon.h"

OgreSystem::OgreSystem() {
    root = new Ogre::Root();
    render_system = NULL;

    Ogre::RenderSystem* Direct3D9RenderSystem = loadRenderSystem("Direct3D9");
    Ogre::RenderSystem* OpenGLRenderSystem = NULL;

    if (!Direct3D9RenderSystem) {
        OpenGLRenderSystem = loadRenderSystem("OpenGL");
        if (!OpenGLRenderSystem) {
            qCritical("No rendering subsystems found");
            exit(0);
        }
    }

    if (Direct3D9RenderSystem != 0) {
        render_system = Direct3D9RenderSystem;
    } else if (OpenGLRenderSystem != 0) {
        render_system = OpenGLRenderSystem;
    }

    root->setRenderSystem(render_system);
    root->initialise(false);
}

OgreSystem::~OgreSystem() {

}

Ogre::RenderSystem* OgreSystem::loadRenderSystem(const std::string systemName) {
    try {
        if (systemName == "OpenGL") {
            root->loadPlugin("RenderSystem_GL");
        } else {
            root->loadPlugin("RenderSystem_" + systemName);
        }
    }
    catch (...) {
        qWarning(("Failed to load " + systemName + std::string(" plugin")).c_str());
    }

    Ogre::RenderSystemList list = root->getAvailableRenderers();
    Ogre::RenderSystemList::iterator i = list.begin();

    while (i != list.end()) {
        if ((*i)->getName() == systemName + " Rendering Subsystem") {
            return *i;
        }
        i++;
    }
    return NULL;
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
