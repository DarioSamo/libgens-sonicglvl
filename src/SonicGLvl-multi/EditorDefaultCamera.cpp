#include "stdafx.h"
#include "EditorDefaultCamera.h"

// Movement Multipliers (Scale with Units)
const Ogre::Real EditorDefaultCamera::MoveMultiplier = 50.0F;
const Ogre::Real EditorDefaultCamera::MoveShiftMultiplier = 200.0F;
const Ogre::Real EditorDefaultCamera::PanMultiplier = 60.0F;
const Ogre::Real EditorDefaultCamera::ZoomMultiplier = 0.05F;

// Rotation Multipliers
const Ogre::Real EditorDefaultCamera::RollMultiplier = 3.0F;
const Ogre::Real EditorDefaultCamera::MoveRollMultiplier = 1.5F;
const Ogre::Real EditorDefaultCamera::RotateMultiplier = 3.0F;

EditorDefaultCamera::EditorDefaultCamera(Ogre::SceneManager *scene_manager, Ogre::Camera *camera) : EditorCamera(scene_manager, camera) {
	left = right = up = down = false;
	shift = alt = false;
	roll_left = roll_right = false;
	rotating = panning = rolling = false;
	pitch_angle = 0.0F;

#ifdef SONICGLVL_LOST_WORLD
	camera->setFixedYawAxis(false);
#endif
}

void EditorDefaultCamera::mousePressEvent(Qt::MouseButton button, float rx, float ry) {
	if (button == Qt::RightButton) {
		if (alt) {
			rolling = true;
		}
		else {
			rotating = true;
		}
	}
	else if (button == Qt::MiddleButton) {
		panning = true;
	}
}

bool EditorDefaultCamera::mouseMoveEvent(float rx, float ry) {
	if (panning) {
		Ogre::Vector3 move_x = (rx) * PanMultiplier * Ogre::Vector3::NEGATIVE_UNIT_X;
		Ogre::Vector3 move_y = (ry) * PanMultiplier * Ogre::Vector3::UNIT_Y;
		moveCameraRelative(move_x + move_y);
		return true;
	}
	else if (rolling) {
		rollCamera(Ogre::Radian(rx) * RollMultiplier);
		return true;
	}
	else if (rotating) {
		yawCamera(Ogre::Radian(-rx) * RotateMultiplier);
		pitchCamera(Ogre::Radian(-ry) * RotateMultiplier);
		return true;
	}

	return EditorCamera::mouseMoveEvent(rx, ry);
}

void EditorDefaultCamera::mouseReleaseEvent(Qt::MouseButton button, float rx, float ry) {
	if (button == Qt::RightButton) {
		rotating = false;
		rolling = false;
	}
	else if (button == Qt::MiddleButton) {
		panning = false;
	}
}

bool EditorDefaultCamera::wheelEvent(int z) {
	moveCameraRelative(z * ZoomMultiplier * Ogre::Vector3::NEGATIVE_UNIT_Z);
	return true;
}

bool EditorDefaultCamera::keyPressEvent(int key) {
	if (key == Qt::Key_W) up = true;
	else if (key == Qt::Key_S) down = true;
	else if (key == Qt::Key_A) left = true;
	else if (key == Qt::Key_D) right = true;
#ifdef SONICGLVL_LOST_WORLD
	else if (key == Qt::Key_Q) roll_left = true;
	else if (key == Qt::Key_E) roll_right = true;
#endif
	else if (key == Qt::Key_Shift) shift = true;
	else if (key == Qt::Key_Alt) alt = true;
	else return false;

	return true;
}

bool EditorDefaultCamera::keyReleaseEvent(int key) {
	if (key == Qt::Key_W) up = false;
	else if (key == Qt::Key_S) down = false;
	else if (key == Qt::Key_A) left = false;
	else if (key == Qt::Key_D) right = false;
	else if (key == Qt::Key_Q) roll_left = false;
	else if (key == Qt::Key_E) roll_right = false;
	else if (key == Qt::Key_Shift) shift = false;
	else if (key == Qt::Key_Alt) alt = false;
	else return false;

	return true;
}

void EditorDefaultCamera::timerEvent(Ogre::Real time_since_last) {
	Ogre::Real multiplier = time_since_last * (shift ? MoveShiftMultiplier : MoveMultiplier);
	if (up) moveCameraRelative(Ogre::Vector3::NEGATIVE_UNIT_Z * multiplier);
	if (down) moveCameraRelative(Ogre::Vector3::UNIT_Z * multiplier);
	if (left) moveCameraRelative(Ogre::Vector3::NEGATIVE_UNIT_X * multiplier);
	if (right) moveCameraRelative(Ogre::Vector3::UNIT_X * multiplier);
	if (roll_left) rollCamera(Ogre::Radian(MoveRollMultiplier * time_since_last));
	if (roll_right) rollCamera(Ogre::Radian(-MoveRollMultiplier * time_since_last));
}

Ogre::Vector3 EditorDefaultCamera::getPosition() {
	return camera->getPosition();
}

Ogre::Quaternion EditorDefaultCamera::getOrientation() {
	return camera->getOrientation();
}