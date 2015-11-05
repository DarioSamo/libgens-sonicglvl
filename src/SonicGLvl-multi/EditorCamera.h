#pragma once

class EditorCamera {
protected:
	Ogre::Camera *camera;
public:
	EditorCamera(Ogre::SceneManager *scene_manager, Ogre::Camera *camera);
	virtual void mousePressEvent(Qt::MouseButton button, float rx, float ry);
	virtual bool mouseMoveEvent(float rx, float ry);
	virtual void mouseReleaseEvent(Qt::MouseButton button, float rx, float ry);
	virtual bool keyPressEvent(int key);
	virtual bool keyReleaseEvent(int key);
	virtual void timerEvent(Ogre::Real time);
	virtual bool wheelEvent(int z);
	void moveCameraRelative(Ogre::Vector3 movement);
	void pitchCamera(Ogre::Radian angle);
	void yawCamera(Ogre::Radian angle);
	void rollCamera(Ogre::Radian angle);
};