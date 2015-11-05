#include "stdafx.h"
#include "EditorSky.h"
#include "EditorModelConverter.h"
#include "LibGens.h"
#include "Model.h"

EditorSky::EditorSky(Ogre::SceneManager *scene_manager) {
	this->scene_manager = scene_manager;
	root_node = scene_manager->getRootSceneNode()->createChildSceneNode();
}

void EditorSky::load(QString directory) {
	QDir dir(directory);
	dir.setFilter(QDir::Files);
	dir.setNameFilters(QStringList() << "*.model");
	QStringList sky_model_names = dir.entryList();
	if (!sky_model_names.isEmpty()) {
		QString sky_model_name = sky_model_names.first();
		LibGens::Model *sky_model = new LibGens::Model((directory + "/" + sky_model_name).toStdString());
		sky_models.append(sky_model);

		QList<Ogre::String> mesh_names = EditorModelConverter::convertModel(sky_model);
		foreach(Ogre::String mesh_name, mesh_names) {
			Ogre::Entity *entity = scene_manager->createEntity(mesh_name);
			root_node->attachObject(entity);
		}
	}
}