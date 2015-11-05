#pragma once

namespace LibGens {
	class Model;
}

class EditorSky {
protected:
	QList<LibGens::Model *> sky_models;
	Ogre::SceneNode *root_node;
	Ogre::SceneManager *scene_manager;
public:
	EditorSky(Ogre::SceneManager *scene_manager);
	void load(QString directory);
};