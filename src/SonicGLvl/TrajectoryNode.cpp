#include "StdAfx.h"
#include "EditorApplication.h"

TrajectoryNode::TrajectoryNode(Ogre::SceneManager* scene_manager, EditorNode* node, TrajectoryMode mode)
{
	type = EDITOR_NODE_TRAJECTORY;

	scene_node = scene_manager->getRootSceneNode()->createChildSceneNode();
	scene_node->setPosition(position);
	scene_node->setScale(scale);
	scene_node->getUserObjectBindings().setUserAny("EditorNodePtr", Ogre::Any((EditorNode*)this));

	m_lines = new DynamicLines(Ogre::RenderOperation::OT_LINE_STRIP);
	scene_node->attachObject(m_lines);
	m_linesExtra = new DynamicLines(Ogre::RenderOperation::OT_LINE_STRIP);
	scene_node->attachObject(m_linesExtra);

	selected = false;

	m_max_time				= 10; // TODO: > OutOfControl
	keep_velocity_distance	= 0;
	m_mode					= NONE;

	restart(node, mode);
}

void TrajectoryNode::restart(EditorNode* node, TrajectoryMode mode)
{
	Ogre::Vector3 object_position = node->getPosition();
	Ogre::Quaternion object_rotation = node->getRotation();
	m_mode = mode;

	m_total_time = 0.0f;
	m_gravity_time = 0.0f;
	act_gravity = false;

	m_lines->clear();
	m_linesExtra->clear();
	while (m_total_time <= m_max_time)
	{
		switch (m_mode)
		{
		case SPRING:
		case WIDE_SPRING:
			getTrajectorySpring(node);
			break;

		case JUMP_PANEL:
			getTrajectoryJumpBoard(node, false);
			getTrajectoryJumpBoard(node, true);
			break;

		case DASH_RING:
			getTrajectoryDashRing(node);
			break;

		default:
			return;
		}

		addTime(1.0f / 30.0f);
	}

	m_lines->update();
	m_linesExtra->update();
}

float TrajectoryNode::getTrajectoryGravity(float first_speed, float ignore_distance, float y_direction)
{
	float delta_y = 0;
	float gravity = LIBGENS_MATH_GRAVITY;
	if ((first_speed * m_total_time) > ignore_distance && !act_gravity)
	{
		keep_velocity_distance = first_speed * m_total_time;
		m_gravity_time = 0.0f;
		act_gravity	= true;
	}
	else
	{
		if (!act_gravity)
		{
			keep_velocity_distance = first_speed * m_total_time;
		}
	}

	if (act_gravity)
	{
		// x = ut + 0.5(at^2)
		delta_y = (first_speed * m_gravity_time * y_direction) + (0.5f * -gravity * (m_gravity_time * m_gravity_time));
	}

	return delta_y;
}

void TrajectoryNode::getTrajectorySpring(EditorNode* node)
{
	ObjectNode* object_node = editor_application->getObjectNodeFromEditorNode(node);
	LibGens::Object* object = object_node->getObject();

	LibGens::ObjectElementFloat* first_speed_property = static_cast<LibGens::ObjectElementFloat*>(object->getElement("FirstSpeed"));
	LibGens::ObjectElementFloat* keep_distance_property = static_cast<LibGens::ObjectElementFloat*>(object->getElement("KeepVelocityDistance"));
	float first_speed	  = first_speed_property->value;
	float ignore_distance = keep_distance_property->value;

	// Springs use Y-direction
	Ogre::Vector3 spring_dir(0, 1, 0);
	Ogre::Vector3 direction = node->getRotation() * spring_dir;

	if (m_total_time >= m_max_time)
		return;

	float delta_y = getTrajectoryGravity(first_speed, ignore_distance, direction.y);
	float new_pos_x, new_pos_y, new_pos_z;

	if (m_mode == SPRING)
	{
		// Spring / AirSpring / SpringFake / SpringClassic / SpringClassicYellow
		new_pos_x = (first_speed * m_total_time) * direction.x;
		new_pos_y = (delta_y + (keep_velocity_distance * direction.y));
		new_pos_z = (first_speed * m_total_time) * direction.z;
	}
	else
	{
		// WideSpring
		new_pos_x = 0;
		new_pos_y = delta_y + keep_velocity_distance;
		new_pos_z = 0;
	}

	Ogre::Vector3 new_position = node->getPosition();
	Ogre::Vector3 position_add(new_pos_x, new_pos_y, new_pos_z);
	new_position += position_add;
	m_lines->addPoint(new_position);
}

void TrajectoryNode::getTrajectoryJumpBoard(EditorNode* node, bool boost)
{
	ObjectNode* object_node = editor_application->getObjectNodeFromEditorNode(node);
	LibGens::Object* object = object_node->getObject();
	std::string object_name = object->getName();

	LibGens::ObjectElementFloat* speed_property;
	LibGens::ObjectElementFloat* angle_property;
	LibGens::ObjectElementFloat* size_property;
	int angle_type;
	int size_type;
	float angle;

	std::string impulse_speed_variable = boost ? "ImpulseSpeedOnBoost" : "ImpulseSpeedOnNormal";
	speed_property = static_cast<LibGens::ObjectElementFloat*>(object->getElement(impulse_speed_variable));
	if (object_name == "JumpBoard")
	{
		angle_property = static_cast<LibGens::ObjectElementFloat*>(object->getElement("AngleType"));
		angle_type	= angle_property->value;
	}
	else
	{
		size_property = static_cast<LibGens::ObjectElementFloat*>(object->getElement("SizeType"));
		size_type = size_property->value;
	}

	float impulse_speed = speed_property->value;
	float y_offset = 0.0f;
	if (object_name == "JumpBoard")
	{
		switch (angle_type)
		{
		case 0:
			y_offset = 0.8;
			angle = 15;
			break;
			
		case 1:
			y_offset = 1.0;
			angle = 30;
			break;

		case 2:
			y_offset = 1.5;
			angle = 30;
			break;

		case 3:
			y_offset = 2.5;
			angle = 30;
			break;
			
		default:
			setPosition(node->getPosition());
			return;
		}
	}
	else
	{
		y_offset = size_type ? 2.5 : 1.5;
		angle = 30;
	}

	Ogre::Vector3 direction(0, 0, 1);
	Ogre::Quaternion obj_rotation = node->getRotation();
	Ogre::Radian y_rad, p_rad, r_rad;
	Ogre::Matrix3 rot_matrix;
	obj_rotation.ToRotationMatrix(rot_matrix);
	rot_matrix.ToEulerAnglesYXZ(y_rad, p_rad, r_rad);

	Ogre::Real yaw_rad	 = y_rad.valueRadians();
	Ogre::Real pitch_rad = p_rad.valueRadians();
	Ogre::Real roll_rad  = r_rad.valueRadians();
	pitch_rad += (angle * LIBGENS_MATH_PI) / 180;

	rot_matrix.FromEulerAnglesYXZ(Ogre::Radian(yaw_rad), Ogre::Radian(pitch_rad), Ogre::Radian(roll_rad));
	obj_rotation.FromRotationMatrix(rot_matrix);

	direction = obj_rotation * direction;
	direction *= -1;

	if (m_total_time >= m_max_time)
		return;

	act_gravity		= true;
	float gravity   = LIBGENS_MATH_GRAVITY;
	float delta_y   = (impulse_speed * m_total_time * direction.y) + (0.5 * -gravity * (m_total_time * m_total_time));

	float new_pos_x = (impulse_speed * m_total_time) * direction.x;
	float new_pos_y = delta_y + y_offset;
	float new_pos_z = (impulse_speed * m_total_time) * direction.z;

	Ogre::Vector3 new_position = node->getPosition();
	Ogre::Vector3 position_add(new_pos_x, new_pos_y, new_pos_z);
	new_position += position_add;
	boost ? m_linesExtra->addPoint(new_position) : m_lines->addPoint(new_position);
}

void TrajectoryNode::getTrajectoryDashRing(EditorNode* node)
{
	ObjectNode* object_node = editor_application->getObjectNodeFromEditorNode(node);
	LibGens::Object* object = object_node->getObject();

	LibGens::ObjectElementFloat* first_speed_property   = static_cast<LibGens::ObjectElementFloat*>(object->getElement("FirstSpeed"));
	LibGens::ObjectElementFloat* keep_distance_property = static_cast<LibGens::ObjectElementFloat*>(object->getElement("KeepVelocityDistance"));
	float first_speed	  = first_speed_property->value;
	float ignore_distance = keep_distance_property->value;

	Ogre::Vector3 direction(0, 0, 1);
	Ogre::Quaternion node_rotation = node->getRotation();
	direction  = node_rotation * direction;
	direction *= -1;

	if (m_total_time >= m_max_time)
		return;

	float delta_y	= getTrajectoryGravity(first_speed, ignore_distance, direction.y);

	float new_pos_x = first_speed * m_total_time * direction.x;
	float new_pos_y = delta_y + (keep_velocity_distance * direction.y);
	float new_pos_z = first_speed * m_total_time * direction.z;

	Ogre::Vector3 new_position = node->getPosition();
	Ogre::Vector3 position_add(new_pos_x, new_pos_y, new_pos_z);
	new_position += position_add;
	m_lines->addPoint(new_position);
}
