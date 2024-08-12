#pragma once
#include "Common.h"
#include "EditorNode.h"
#include "ObjectNode.h"

enum TrajectoryMode
{
	NONE			= 0,
	SPRING			= 1,
	WIDE_SPRING		= 2,
	JUMP_POLE		= 4,
	JUMP_PANEL		= 8,
	DASH_RING		= 16,
	SELECT_CANNON	= 32,
	TRICK_JUMPER	= 64
};

class TrajectoryNode : public EditorNode
{
private:
	Ogre::Real m_total_time;
	Ogre::Real m_gravity_time;
	Ogre::Real m_max_time;
	TrajectoryMode m_mode;
	float keep_velocity_distance;
	bool act_gravity;

public:
	TrajectoryNode(Ogre::SceneManager* scene_manager, TrajectoryMode mode = NONE);

	void addTime(Ogre::Real time)
	{
		m_total_time += time;
		m_gravity_time += time;
	}

	void resetTime()
	{
		m_total_time = 0.0f;
		m_gravity_time = 0.0f;
		act_gravity = false;
	}

	void setPosition(Ogre::Vector3 position)
	{
		EditorNode::setPosition(position);
	}

	void setMode(TrajectoryMode mode)
	{
		m_mode = mode;
	}

	float getTrajectoryGravity(float first_speed, float keep_distance, float y_direction);
	void getTrajectorySpring(EditorNode* node);
	void getTrajectoryJumpBoard(EditorNode* node, bool boost);
	void getTrajectoryTrickJumper(EditorNode* node, bool second);
	void getTrajectoryDashRing(EditorNode* node);
};
