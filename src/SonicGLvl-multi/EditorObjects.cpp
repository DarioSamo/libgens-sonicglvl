#include "stdafx.h"
#include "EditorObjects.h"
#include "LibGens.h"
#include "LostWorldObjectSet.h"
#include "Object.h"
#include "EditorNode.h"

EditorObjects::EditorObjects(Ogre::SceneManager *scene_manager, LibGens::ObjectLibrary *object_library) {
	this->scene_manager = scene_manager;
	this->object_library = object_library;
	root_node = scene_manager->getRootSceneNode()->createChildSceneNode();

#ifdef SONICGLVL_LOST_WORLD
	root_node->setScale(Ogre::Vector3::UNIT_SCALE * 0.1F);
#endif
}

void EditorObjects::load(QString stage_name, QString directory, QWidget *parent) {
#ifdef SONICGLVL_LOST_WORLD
	// This is how Lost World actually loads the orc files
	for (int i=0; i < 9; i++) {
		QString orc_filename = QString("%1/%2_obj_%3.orc").arg(directory).arg(stage_name).arg(i, 2, 10, QChar('0'));
		if (QFile::exists(orc_filename)) {
			LibGens::LostWorldObjectSet *object_set = new LibGens::LostWorldObjectSet(orc_filename.toStdString(), object_library);
			std::list<LibGens::Object *> objects = object_set->getObjects();
			for (std::list<LibGens::Object *>::iterator it=objects.begin(); it!=objects.end(); it++) {
				LibGens::Object *object = (*it);
				EditorNode *editor_node = new EditorNode(scene_manager, root_node);
				LibGens::Vector3 p = object->getPosition();
				LibGens::Quaternion o = object->getRotation();
				editor_node->setPosition(Ogre::Vector3(p.x, p.y, p.z));
				editor_node->setOrientation(Ogre::Quaternion(o.w, o.x, o.y, o.z));
				editor_node->setScale(Ogre::Vector3::UNIT_SCALE * 10.0F);
				editor_node->attachEntity("unknown.mesh");
			}

			object_sets.append(object_set);
		}
	}
#endif
}