#pragma once

#include "EditorCamera.h"

class EditorDefaultCamera : public EditorCamera {
protected:
	bool left, right, up, down;
	bool roll_left, roll_right;
	bool shift, alt;
	bool rotating, panning, rolling;
	Ogre::Real pitch_angle;
public:
	static const Ogre::Real MoveMultiplier;
	static const Ogre::Real MoveShiftMultiplier;
	static const Ogre::Real MoveRollMultiplier;
	static const Ogre::Real PanMultiplier;
	static const Ogre::Real RotateMultiplier;
	static const Ogre::Real RollMultiplier;
	static const Ogre::Real ZoomMultiplier;

	EditorDefaultCamera(Ogre::SceneManager *scene_manager, Ogre::Camera *camera);
	virtual void mousePressEvent(Qt::MouseButton button, float rx, float ry);
	virtual bool mouseMoveEvent(float rx, float ry);
	virtual void mouseReleaseEvent(Qt::MouseButton button, float rx, float ry);
	virtual bool keyPressEvent(int key);
	virtual bool keyReleaseEvent(int key);
	virtual bool wheelEvent(int z);
	virtual void timerEvent(Ogre::Real time);
	Ogre::Vector3 getPosition();
	Ogre::Quaternion getOrientation();
};