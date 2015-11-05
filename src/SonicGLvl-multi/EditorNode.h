#pragma once

class EditorNode {
protected:
	Ogre::SceneNode *scene_node;
public:
	EditorNode(Ogre::SceneManager *scene_manager, Ogre::SceneNode *parent_node);
	~EditorNode();

	void attachEntity(Ogre::String entity_name);
	void attachEntity(Ogre::Entity *entity);
	void setTransform(Ogre::Matrix4 transform);
	void assignGITexture(Ogre::String texture_name);
	void setPosition(Ogre::Vector3 position);
	void setOrientation(Ogre::Quaternion orientation);
	void setScale(Ogre::Vector3 scale);
};