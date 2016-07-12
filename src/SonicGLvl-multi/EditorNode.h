#pragma once

class EditorNode {
protected:
	Ogre::SceneNode *scene_node;
	EditorNode *parent;
	QList<EditorNode *> childs;
public:
	enum Type {
		Generic,
		Terrain,
		Object,
		Light,
		Path,
		Shape
	};

	EditorNode(Ogre::SceneManager *scene_manager, EditorNode *parent_node = NULL);
	~EditorNode();

	void attachEntity(Ogre::String entity_name);
	void attachEntity(Ogre::Entity *entity);
	void setTransform(Ogre::Matrix4 transform);
	void assignGITexture(Ogre::String texture_name);
	void setPosition(Ogre::Vector3 position);
	void setOrientation(Ogre::Quaternion orientation);
	void setScale(Ogre::Vector3 scale);
	virtual Type getType();
};