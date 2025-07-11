#pragma once
#include "Common.h"
#include "EditorNode.h"
#include "ObjectNode.h"

enum TrajectoryMode
{
	NONE			= 0,
	SPRING			= 1 << 0,
	WIDE_SPRING		= 1 << 1,
	JUMP_POLE		= 1 << 2,
	JUMP_PANEL		= 1 << 3,
	DASH_RING		= 1 << 4,
	SELECT_CANNON	= 1 << 5,
	TRICK_JUMPER	= 1 << 6,
};

class TrajectoryNode : public EditorNode
{
private:
	Ogre::Real total_time;
	Ogre::Real gravity_time;
	Ogre::Real max_time;
	TrajectoryMode mode;
	float keep_velocity_distance;
	bool act_gravity;

	struct LineData
	{
		DynamicLines* lines;
		DynamicLines* lines_out_of_control;
		bool draw_out_of_control;
	};

	LineData line_data1;
	LineData line_data2;

public:
	TrajectoryNode(Ogre::SceneManager* scene_manager, EditorNode* node, TrajectoryMode mode_p = NONE);
	~TrajectoryNode();

	void addTime(Ogre::Real time)
	{
		total_time += time;
		gravity_time += time;
	}

	void restart(EditorNode* node, TrajectoryMode mode_p);

	void setPosition(Ogre::Vector3 position)
	{
		EditorNode::setPosition(position);
	}

	void setMode(TrajectoryMode mode_p)
	{
		mode = mode_p;
	}

	float getTrajectoryGravity(float first_speed, float keep_distance, float y_direction);
	void getTrajectorySpring(EditorNode* node);
	void getTrajectoryJumpBoard(EditorNode* node, bool boost);
	void getTrajectoryDashRing(EditorNode* node);
	void getTrajectoryTrickJumper(EditorNode* node);
};
