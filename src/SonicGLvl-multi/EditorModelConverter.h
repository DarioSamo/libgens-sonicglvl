#pragma once

#include "LibGens.h"
#include "Model.h"

class EditorModelConverter {
protected:
public:
	static QList<Ogre::String> convertModel(LibGens::Model *model, Ogre::String resource_group_name = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
};