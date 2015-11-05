#pragma once

namespace LibGens {
	class LostWorldObjectSet;
	class ObjectLibrary;
}

class EditorObjects {
protected:
	Ogre::SceneNode *root_node;
	Ogre::SceneManager *scene_manager;
	QList<LibGens::LostWorldObjectSet *> object_sets;
	LibGens::ObjectLibrary *object_library;
public:
	EditorObjects(Ogre::SceneManager *scene_manager, LibGens::ObjectLibrary *object_library);
	void load(QString stage_name, QString directory, QWidget *parent);
};