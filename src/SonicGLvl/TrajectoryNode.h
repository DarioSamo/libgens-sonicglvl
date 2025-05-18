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
	SELECT_CANNON	= 32
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

	struct LineData
	{
		DynamicLines* m_lines;
		DynamicLines* m_lines_out_of_control;
		bool m_draw_out_of_control;
	};

	LineData m_lineData1;
	LineData m_lineData2;

public:
	TrajectoryNode(Ogre::SceneManager* scene_manager, EditorNode* node, TrajectoryMode mode = NONE);
	~TrajectoryNode()
	{
		delete m_lineData1.m_lines;
		delete m_lineData1.m_lines_out_of_control;
		delete m_lineData2.m_lines;
		delete m_lineData2.m_lines_out_of_control;
	}

	void addTime(Ogre::Real time)
	{
		m_total_time += time;
		m_gravity_time += time;
	}

	void restart(EditorNode* node, TrajectoryMode mode);

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
	void getTrajectoryDashRing(EditorNode* node);
};
