//=========================================================================
//	  Copyright (c) 2016 SonicGLvl
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

#include "ui_HKWindow.h"
#include <QMainWindow>
#include "HKPropertyTag.h"
#include "assimp/scene.h"
#include "LibGens.h"
#include <unordered_set>

class hkpShape;
class hkpRigidBody;
class hkpWorld;
class hkpStaticCompoundShape;
class hkQsTransform;
class hkpBvCompressedMeshShape;
class hkVector4;

namespace LibGens {
	class HavokEnviroment;
};

class HKWindow : public QMainWindow, public Ui_HKWindow {
	Q_OBJECT
public:
	enum ProgressType {
		ProgressNormal,
		ProgressSuccess,
		ProgressError,
		ProgressWarning,
		ProgressFatal
	};

	static const QString AppName;
	static const QString WindowTitle;
	static const QString VersionNumber;
	static const QString DefaultSettingsPath;
	static const QString LogPath;
	static const QString CommunityGuideURL;
	static const float TriangleAbsoluteTolerance;
	static const int HavokBufferSizeMB;

	explicit HKWindow(QWidget *parent = 0);
    ~HKWindow();
private:
	enum Mode {
		Collision,
		RigidBodies,
		Animation
	};

	LibGens::HavokEnviroment *havok_enviroment;

	struct {
		Mode mode;
		QStringList model_source_paths;
		QString output_file;
		double position_x, position_y, position_z;
		double scale_x, scale_y, scale_z;
		double rotation_x, rotation_y, rotation_z;
		QVector<HKPropertyTag> property_tags;
	} converter_settings;

	bool convert();
	void saveSettings(QString filename);
	void loadSettings(QString filename);
	void updateSettingsFromUi();
	void updateUiFromSettings();
	void messageBox(QString text);
	void logProgress(ProgressType progress_type, QString message);
	void logNodeTree(aiNode *node, QString prefix);
	void beep();
	hkpShape *convertMeshToShape(aiMesh *mesh, LibGens::Vector3 scale);
	QList<hkpRigidBody *> convertNodeToRigidBodies(const aiScene *scene, aiNode *node, LibGens::Matrix4 transform, std::unordered_set<std::string>& names);
	void convertNodeTree(const aiScene *scene, aiNode *node, LibGens::Matrix4 parent_transform, hkpWorld *world, std::unordered_set<std::string>& names);
#ifdef HAVOKCONVERTER_LOST_WORLD
	hkpShape* convertMeshToCompressedShape(aiMesh* mesh, int userdata);
	hkQsTransform createHKTransform(LibGens::Matrix4 transform);
	hkpStaticCompoundShape* convertNodeTreeCompoundShape(const aiScene *scene, aiNode *node, LibGens::Matrix4 parent_transform);
	void convertNodeToCompressedShape(const aiScene* scene, aiNode* node, LibGens::Matrix4 parent_transform, hkpStaticCompoundShape* compound);
#endif
private slots:
	void aboutTriggered();
	void addSourceModelsTriggered();
	void removeSourceModelTriggered();
	void clearSourceModelsTriggered();
	void resetTransformTriggered();
	void browseOutputTriggered();
	void close();
	void convertTriggered();
	void tagMoveUpTriggered();
	void tagMoveDownTriggered();
	void newTagTriggered();
	void deleteTagTriggered();
	void tagChangedTriggered(int, int);
	void tagDoubleClickTriggered(int, int);
	void openSettingsTriggered();
	void saveSettingsTriggered();
	void communityGuideTriggered();
	void updateTagsTableTriggered();
	void closeEvent(QCloseEvent *event);
};