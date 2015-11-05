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
#include "LibGens.h"
#include "EditorWindow.h"
#include "ui_EditorWindow.h"
#include "OgreSystem.h"
#include "EditorStage.h"
#include "EditorCache.h"
#include "EditorTerrain.h"
#include "EditorSky.h"
#include "EditorMaterials.h"
#include "EditorObjects.h"
#include "EditorGI.h"
#include "EditorDefaultCamera.h"
#include "EditorViewport.h"
#include "Shader.h"
#include "ObjectLibrary.h"
#include "ObjectCategory.h"
#include "Object.h"

const int EditorWindow::UpdateTimerMs = 4;
const QString EditorWindow::ObjectDatabasePath = "database/objects/";
const QString EditorWindow::ObjectDatabaseFilename = "database/ObjectsDatabase.xml";
const QString EditorWindow::ShaderDatabasePath = "database/shaders/";

EditorWindow::EditorWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::EditorWindow) {
	LibGens::initialize();
	LibGens::Error::setLogging(true);
    ui->setupUi(this);
	editor_stage = NULL;
	
    ogre_system = new OgreSystem();
    stage_scene_manager = ogre_system->createSceneManager();
    ui->viewports_widget->setup(ogre_system, stage_scene_manager);
	ui->viewports_widget->setVisible(false);

	// Set FPS Camera Controller on main viewport
	EditorViewport *viewport = ui->viewports_widget->getMainViewport();
	editor_fps_camera = new EditorDefaultCamera(stage_scene_manager, viewport->getCamera());
	viewport->setEditorCamera(editor_fps_camera);

    ogre_system->setupResources();

	shader_library = new LibGens::ShaderLibrary((programPath() + "/" + ShaderDatabasePath).toStdString());
	editor_materials = new EditorMaterials();

    timer_index = startTimer(UpdateTimerMs);
    timer_elapsed.start();

	object_library = new LibGens::ObjectLibrary((programPath() + "/" + ObjectDatabasePath).toStdString());
	object_library->loadDatabase((programPath() + "/" + ObjectDatabaseFilename).toStdString());

	editor_cache = new EditorCache(programPath());
	editor_terrain = new EditorTerrain(stage_scene_manager);
	editor_sky = new EditorSky(stage_scene_manager);
	editor_objects = new EditorObjects(stage_scene_manager, object_library);
	editor_cache->loadHashes();

	editor_gi_listener = new EditorGIListener();
	stage_scene_manager->addRenderObjectListener(editor_gi_listener);
	stage_scene_manager->setAmbientLight(Ogre::ColourValue(0.4F, 0.4F, 0.4F, 1.0F));

	connect(ui->action_close, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui->action_open_stage, SIGNAL(triggered()), this, SLOT(openStage()));
	connect(ui->open_stage_button, SIGNAL(released()), this, SLOT(openStage()));
	connect(ui->action_about, SIGNAL(triggered()), this, SLOT(about()));
	connect(ui->action_about_qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	setupWindowTitle();
	setupObjectLibrary();
}

EditorWindow::~EditorWindow() {
    delete ui;
    delete ogre_system;
	delete editor_cache;
	delete editor_stage;
	delete editor_terrain;
	delete editor_materials;
	delete editor_sky;
	delete editor_objects;
	delete shader_library;
	delete object_library;
    killTimer(timer_index);
}

void EditorWindow::timerEvent(QTimerEvent* event) {
    ui->viewports_widget->update();
}


void EditorWindow::setupWindowTitle() {
	// Set the appropiate window title depending on the compiled version
	QString window_title = "SonicGLvl v0.9.1 - ";

#ifdef SONICGLVL_LOST_WORLD
	window_title += "Lost World";
#endif

	setWindowTitle(window_title);
}

void EditorWindow::setupObjectLibrary() {
	std::vector<LibGens::ObjectCategory *> categories = object_library->getCategories();
	for (size_t i=0; i<categories.size(); i++) {
		LibGens::ObjectCategory *category = categories[i];
		QString category_name = category->getName().c_str();
		QTreeWidgetItem *category_item = new QTreeWidgetItem(ui->object_library_tree);
		category_item->setText(0, category_name);
		category_item->setIcon(0, QIcon(":/Editor/Category"));

		std::vector<LibGens::Object *> templates = category->getTemplates();
		for (size_t j=0; j<templates.size(); j++) {
			LibGens::Object *templ = templates[j];
			QString templ_name = templ->getName().c_str();
			QTreeWidgetItem *templ_item = new QTreeWidgetItem(category_item);
			templ_item->setText(0, templ_name);
			templ_item->setIcon(0, QIcon(":/Editor/Template"));
		}
	}
}

QString EditorWindow::programPath() {
	return QDir::currentPath();
}

void EditorWindow::openStage() {
	QString stage_filename = QFileDialog::getOpenFileName(this, "Open Stage...", "D:/SonicGenerationsModding/Sonic Lost World/w1a01", QString("Stage File (*%1)").arg(EditorStage::extension()));
	if (!stage_filename.isEmpty()) {
		ui->viewports_widget->setVisible(true);
		ui->welcome_frame->setVisible(false);

		QString error_message = "";
		editor_stage = new EditorStage();
		if (editor_stage->load(stage_filename, error_message)) {
			editor_cache->unpackStage(editor_stage->stageName(), editor_stage->filename(), this);
#ifdef SONICGLVL_LOST_WORLD
			// Load Terrain
			QString terrain_common_path = editor_cache->terrainCommonPath(editor_stage->stageName());
			loadMaterialsDirectory(terrain_common_path);
			editor_terrain->load(terrain_common_path, this);

			QString far_path = editor_cache->farPath(editor_stage->stageName());
			loadMaterialsDirectory(far_path);
			editor_terrain->load(far_path, this);

			// Load Sky
			QString sky_path = editor_cache->skyPath(editor_stage->stageName());
			loadMaterialsDirectory(sky_path);
			editor_sky->load(sky_path);

			// Load Objects
			//editor_objects->load(editor_stage->stageName(), editor_stage->setsDirectory(), this);
#endif
		}
		else MsgBox(error_message);
	}
}

void EditorWindow::loadMaterialsDirectory(QString directory) {
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(directory.toStdString(), "FileSystem");
	editor_materials->load(directory, shader_library, this);
}

void EditorWindow::about() {
	QMessageBox::about(this, "About SonicGLvl",
	"<p><b>SonicGLvl</b> is a free, open source, community-created level editor for Hedgehog Engine-based Sonic games.</p>"
#ifdef SONICGLVL_LOST_WORLD
	"<p><b>Lost World Version</b></p>"
#endif
	"<p>SonicGLvl is free software: you can redistribute it and/or modify "
	"it under the terms of the GNU General Public License as published by "
	"the Free Software Foundation, either version 3 of the License, or "
	"(at your option) any later version.</p>"
	"<p>SonicGLvl is distributed in the hope that it will be useful, "
	"but WITHOUT ANY WARRANTY; without even the implied warranty of "
	"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.</p>"
	"<p>Read AUTHORS.txt, LICENSE.txt and COPYRIGHT.txt for more details.</p>"
	"<ul>"
	"Dependencies used:"
	"<li><b>LibGens</b>: <a href=\"https://github.com/DarioSamo/libgens-sonicglvl\">Github Repository</a></li>"
	"<li><b>Ogre3D 1.9</b>: <a href=\"http://www.ogre3d.org/\">Homepage</a></li>"
	"<li><b>Qt 5.5.0</b>: <a href=\"http://qt-project.org/\">Homepage</a></li>"
	"</ul>");
}