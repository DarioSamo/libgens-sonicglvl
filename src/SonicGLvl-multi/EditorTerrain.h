#pragma once

#include "LibGens.h"
#include "TerrainInstance.h"
#include "Model.h"

class EditorNode;

class EditorTerrain {
protected:
	Ogre::SceneNode *root_node;
	Ogre::SceneManager *scene_manager;
	QString directory;
public:
	EditorTerrain(Ogre::SceneManager *scene_manager);
	void unload();
	void load(QString directory, QWidget *parent);
	EditorNode *createEditorNode(QString node_name, const QList<Ogre::String> &mesh_names, Ogre::Matrix4 transform = Ogre::Matrix4::IDENTITY);
};