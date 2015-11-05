#pragma once

#include "LibGens.h"
#include "Model.h"

class EditorModelConverter {
protected:
public:
	static QList<Ogre::String> convertModel(LibGens::Model *model);
};