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
	TrajectoryNode(Ogre::SceneManager* scene_manager, EditorNode* n, TrajectoryMode m = NONE);
	~TrajectoryNode()
	{
		delete line_data1.lines;
		delete line_data1.lines_out_of_control;
		delete line_data2.lines;
		delete line_data2.lines_out_of_control;
	}

	void addTime(Ogre::Real time)
	{
		total_time += time;
		gravity_time += time;
	}

	void restart(EditorNode* n, TrajectoryMode m);

	void setPosition(Ogre::Vector3 position)
	{
		EditorNode::setPosition(position);
	}

	void setMode(TrajectoryMode m)
	{
		mode = m;
	}

	float getTrajectoryGravity(float first_speed, float keep_distance, float y_direction);
	void getTrajectorySpring(EditorNode* n);
	void getTrajectoryJumpBoard(EditorNode* n, bool boost);
	void getTrajectoryDashRing(EditorNode* n);
	void getTrajectoryTrickJumper(EditorNode* n);
};
