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

#include "stdafx.h"
#include "EditorTerrain.h"
#include "EditorModelConverter.h"
#include "EditorMaterialConverter.h"
#include "EditorNode.h"
#include "Light.h"

EditorTerrain::EditorTerrain(Ogre::SceneManager *scene_manager) {
	this->scene_manager = scene_manager;
	root_node = scene_manager->getRootSceneNode()->createChildSceneNode();
	directory = QString();
}

void EditorTerrain::unload() {
}

void EditorTerrain::load(QString directory, QWidget *parent) {
	this->directory = directory;

	QDir dir(directory);
	dir.setFilter(QDir::Files);

// Lost World has no terrain files that describe what to load
// Therefore, we just have to detect what files are in the directory
// and create a visible node for each model that has no instances whatsoever
#ifdef SONICGLVL_LOST_WORLD
	vector<LibGens::Model *> terrain_models;
	vector<LibGens::TerrainInstance *> terrain_instances;

	// Load lights
	dir.setNameFilters(QStringList() << "*.light");
	QStringList light_names = dir.entryList();
	if (!light_names.isEmpty()) {
		scene_manager->destroyAllLights();

		QString light_name = light_names.first();
		LibGens::Light light((directory + "/" + light_name).toStdString());
		if (light.getType() == LIBGENS_LIGHT_TYPE_DIRECTIONAL) {
			LibGens::Vector3 dir = light.getPosition();
			LibGens::Vector3 col = light.getColor();
			Ogre::Light *directional_light = scene_manager->createLight();
			directional_light->setDiffuseColour(col.x, col.y, col.z);
			directional_light->setDirection(Ogre::Vector3(dir.x, dir.y, dir.z) * -1.0F);
			directional_light->setType(Ogre::Light::LT_DIRECTIONAL);
		}
	}

	// Get all terrain model filenames
	dir.setNameFilters(QStringList() << "*.terrain-model");
	QStringList terrain_model_names = dir.entryList();

	// Get all terrain instance names
	dir.setNameFilters(QStringList() << "*.terrain-instanceinfo");
	QStringList terrain_instance_names = dir.entryList();

	int progress_count = 0;
	int progress_max_count = terrain_model_names.size() * 2 + terrain_instance_names.size() * 2;
	QProgressDialog progress(QString(), QString(), 0, progress_max_count, parent);
	progress.setWindowTitle("Loading terrain...");
    progress.setWindowModality(Qt::WindowModal);

	// Load all Terrain Models
	terrain_models.reserve(terrain_model_names.size());
	foreach(QString filename, terrain_model_names) {
		progress.setLabelText("Loading model " + filename);
		progress.setValue(progress_count++);

		QString absolute_filename = directory + "/" + filename;
		LibGens::Model *model = new LibGens::Model(absolute_filename.toStdString());
		terrain_models.push_back(model);
	}

	// Load all Terrain Instances
	QStringList used_terrain_models;
	terrain_instances.reserve(terrain_instance_names.size());
	foreach(QString filename, terrain_instance_names) {
		progress.setLabelText("Loading instance " + filename);
		progress.setValue(progress_count++);

		QString instance_name = QFileInfo(filename).baseName();
		QString absolute_filename = directory + "/" + filename;
		LibGens::TerrainInstance *terrain_instance = new LibGens::TerrainInstance(absolute_filename.toStdString(), instance_name.toStdString(), &terrain_models);
		terrain_instances.push_back(terrain_instance);

		LibGens::Model *model = terrain_instance->getModel();
		if (model) {
			used_terrain_models.append(model->getName().c_str());
		}
	}
	used_terrain_models.removeDuplicates();

	QMap<QString, QList<Ogre::String>> ogre_mesh_map;

	for (size_t i=0; i < terrain_models.size(); i++) {
		QString model_name = terrain_models[i]->getName().c_str();
		progress.setLabelText("Converting model " + model_name);
		progress.setValue(progress_count++);

		QList<Ogre::String> mesh_names = EditorModelConverter::convertModel(terrain_models[i]);
		ogre_mesh_map[model_name] = mesh_names;

		// If the model didn't get used by an instance, create a node for this model
		if (!used_terrain_models.contains(model_name)) {
			createEditorNode(model_name, mesh_names);
		}
	}

	for (size_t i=0; i < terrain_instances.size(); i++) {
		QString instance_name = terrain_instances[i]->getName().c_str();
		progress.setLabelText("Loading instance " + instance_name);
		progress.setValue(progress_count++);

		LibGens::Model *model = terrain_instances[i]->getModel();
		if (model) {
			QString model_name = model->getName().c_str();
			LibGens::Matrix4 m = terrain_instances[i]->getMatrix();
			Ogre::Matrix4 tm = Ogre::Matrix4(m[0][0], m[0][1], m[0][2], m[0][3], m[1][0], m[1][1], m[1][2], m[1][3], m[2][0], m[2][1], m[2][2], m[2][3], m[3][0], m[3][1], m[3][2], m[3][3]);
			createEditorNode(instance_name, ogre_mesh_map[model_name], tm);
		}
	}

	progress.setValue(progress_max_count);
#endif
}

EditorNode *EditorTerrain::createEditorNode(QString node_name, const QList<Ogre::String> &mesh_names, Ogre::Matrix4 transform) {
	EditorNode *editor_node = new EditorNode(scene_manager, root_node);
	editor_node->setTransform(transform);

	foreach(Ogre::String mesh_name, mesh_names) {
		editor_node->attachEntity(mesh_name);
	}

	QString gi_texture_name = directory + "/" + node_name + ".dds";
	if (QFileInfo::exists(gi_texture_name)) {
		editor_node->assignGITexture((node_name + ".dds").toStdString());
	}
	else editor_node->assignGITexture("white.dds");

	return editor_node;
}