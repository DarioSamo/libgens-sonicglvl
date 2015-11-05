#include "stdafx.h"
#include "EditorCamera.h"

EditorCamera::EditorCamera(Ogre::SceneManager *scene_manager, Ogre::Camera *camera) {
	this->camera = camera;
	camera->setFixedYawAxis(false);
}

void EditorCamera::mousePressEvent(Qt::MouseButton button, float rx, float ry) {
}

bool EditorCamera::mouseMoveEvent(float rx, float ry) {
	return false;
}

void EditorCamera::mouseReleaseEvent(Qt::MouseButton button, float rx, float ry) {
}

bool EditorCamera::keyPressEvent(int key) {
	return false;
}

bool EditorCamera::keyReleaseEvent(int key) {
	return false;
}

bool EditorCamera::wheelEvent(int z) {
	return false;
}

void EditorCamera::moveCameraRelative(Ogre::Vector3 movement) {
	camera->moveRelative(movement);
}

void EditorCamera::timerEvent(Ogre::Real time) {
}

void EditorCamera::pitchCamera(Ogre::Radian angle) {
	camera->pitch(angle);
}

void EditorCamera::yawCamera(Ogre::Radian angle) {
	camera->yaw(angle);
}

void EditorCamera::rollCamera(Ogre::Radian angle) {
	camera->roll(angle);
}