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

#pragma once

namespace Ui {
	class EditorWindow;
}

namespace LibGens {
	class ShaderLibrary;
	class ObjectLibrary;
}

class EditorDefaultCamera;
class EditorMaterials;
class EditorCache;
class EditorStage;
class EditorTerrain;
class EditorGIListener;
class EditorSky;
class EditorObjects;
class OgreSystem;
class OgreViewportWidget;

class EditorWindow : public QMainWindow
{
    Q_OBJECT

public:
    static const int UpdateTimerMs;
	static const QString ObjectDatabasePath;
	static const QString ObjectDatabaseFilename;
	static const QString ShaderDatabasePath;

    explicit EditorWindow(QWidget *parent = 0);
    ~EditorWindow();

    void timerEvent(QTimerEvent* event);
private:
    OgreSystem *ogre_system;
    Ui::EditorWindow *ui;
    OgreViewportWidget *viewport_widget;
    Ogre::SceneManager *stage_scene_manager;
	EditorMaterials *editor_materials;
	EditorCache *editor_cache;
	EditorStage *editor_stage;
	EditorTerrain *editor_terrain;
	EditorGIListener *editor_gi_listener;
	EditorSky *editor_sky;
	EditorObjects *editor_objects;
	EditorDefaultCamera *editor_fps_camera;
	LibGens::ShaderLibrary *shader_library;
	LibGens::ObjectLibrary *object_library;
    int timer_index;
    QElapsedTimer timer_elapsed;

	void setupWindowTitle();
	void setupObjectLibrary();
	QString programPath();
	void loadMaterialsDirectory(QString directory);
private slots:
	void openStage();
	void about();
};

