#include "stdafx.h"
#include "EditorNode.h"
#include "EditorGI.h"

EditorNode::EditorNode(Ogre::SceneManager *scene_manager, Ogre::SceneNode *parent_node) {
	scene_node = parent_node->createChildSceneNode();
}

EditorNode::~EditorNode() {
	Ogre::SceneManager *scene_manager = scene_node->getCreator();
	scene_manager->destroySceneNode(scene_node);
}

void EditorNode::attachEntity(Ogre::String entity_name) {
	Ogre::SceneManager *scene_manager = scene_node->getCreator();
	Ogre::Entity *entity = scene_manager->createEntity(entity_name);
	if (entity) {
		attachEntity(entity);
	}
}

void EditorNode::attachEntity(Ogre::Entity *entity) {
	scene_node->attachObject(entity);
}

void EditorNode::setTransform(Ogre::Matrix4 transform) {
	Ogre::Vector3 position, scale;
	Ogre::Quaternion orientation;
	transform.decomposition(position, scale, orientation);
	scene_node->setPosition(position);
	scene_node->setScale(scale);
	scene_node->setOrientation(orientation);
}

void EditorNode::setPosition(Ogre::Vector3 position) {
	scene_node->setPosition(position);
}

void EditorNode::setOrientation(Ogre::Quaternion orientation) {
	scene_node->setOrientation(orientation);
}

void EditorNode::setScale(Ogre::Vector3 scale) {
	scene_node->setScale(scale);
}

void EditorNode::assignGITexture(Ogre::String texture_name) {
	Ogre::TexturePtr texture_ptr = Ogre::TextureManager::getSingletonPtr()->getByName(texture_name);
	if (texture_ptr.isNull()) {
		texture_ptr = Ogre::TextureManager::getSingletonPtr()->load(texture_name, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
	}

	if (!texture_ptr.isNull()) {
		EditorGIParameter *gi_parameter = new EditorGIParameter(texture_ptr);
		EditorGIVisitor visitor(gi_parameter);

		unsigned short attached_objects=scene_node->numAttachedObjects();
		for (unsigned short i=0; i<attached_objects; i++) {
			Ogre::Entity *entity=static_cast<Ogre::Entity *>(scene_node->getAttachedObject(i));
			entity->visitRenderables(&visitor);
		}
	}
}

