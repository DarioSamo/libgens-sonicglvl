#include "StdAfx.h"
#include "EditorApplication.h"

TrajectoryNode::TrajectoryNode(Ogre::SceneManager* scene_manager, EditorNode* node, TrajectoryMode mode)
{
	type = EDITOR_NODE_TRAJECTORY;

	scene_node = scene_manager->getRootSceneNode()->createChildSceneNode();
	scene_node->setPosition(position);
	scene_node->setScale(scale);
	scene_node->getUserObjectBindings().setUserAny("EditorNodePtr", Ogre::Any((EditorNode*)this));

	m_lineData1.m_lines = new DynamicLines(Ogre::RenderOperation::OT_LINE_STRIP);
	m_lineData1.m_lines_out_of_control = new DynamicLines(Ogre::RenderOperation::OT_LINE_STRIP);
	m_lineData1.m_lines_out_of_control->setMaterial("outofcontrol");
	scene_node->attachObject(m_lineData1.m_lines);
	scene_node->attachObject(m_lineData1.m_lines_out_of_control);

	m_lineData2.m_lines = new DynamicLines(Ogre::RenderOperation::OT_LINE_STRIP);
	m_lineData2.m_lines_out_of_control = new DynamicLines(Ogre::RenderOperation::OT_LINE_STRIP);
	m_lineData2.m_lines_out_of_control->setMaterial("outofcontrol");
	scene_node->attachObject(m_lineData2.m_lines);
	scene_node->attachObject(m_lineData2.m_lines_out_of_control);

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

	m_lineData1.m_lines->clear();
	m_lineData1.m_lines_out_of_control->clear();
	m_lineData1.m_draw_out_of_control = true;

	m_lineData2.m_lines->clear();
	m_lineData2.m_lines_out_of_control->clear();
	m_lineData2.m_draw_out_of_control = true;

	bool update_data2 = false;
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
			update_data2 = true;
			break;

		case DASH_RING:
			getTrajectoryDashRing(node);
			break;

		case TRICK_JUMPER:
			getTrajectoryTrickJumper(node);
			update_data2 = true;
			break;

		default:
			return;
		}

		addTime(1.0f / 30.0f);
	}

	m_lineData1.m_lines->update();
	m_lineData1.m_lines_out_of_control->update();

	if (update_data2)
	{
		m_lineData2.m_lines->update();
		m_lineData2.m_lines_out_of_control->update();
	}
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
	LibGens::ObjectElementFloat* out_of_control_property = static_cast<LibGens::ObjectElementFloat*>(object->getElement("OutOfControl"));
	float first_speed	  = first_speed_property->value;
	float ignore_distance = keep_distance_property->value;
	float out_of_control = out_of_control_property->value;

	// Springs use Y-direction
	Ogre::Vector3 spring_dir(0, 1, 0);
	Ogre::Vector3 direction = node->getRotation() * spring_dir;

	bool switched_line = false;
	if (m_lineData1.m_draw_out_of_control && m_total_time >= out_of_control)
	{
		m_lineData1.m_draw_out_of_control = false;
		switched_line = true; // add point on both lines
	}

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

	if (switched_line || m_lineData1.m_draw_out_of_control)
		m_lineData1.m_lines_out_of_control->addPoint(new_position);
	if (switched_line || !m_lineData1.m_draw_out_of_control)
		m_lineData1.m_lines->addPoint(new_position);
}

void TrajectoryNode::getTrajectoryJumpBoard(EditorNode* node, bool boost)
{
	ObjectNode* object_node = editor_application->getObjectNodeFromEditorNode(node);
	LibGens::Object* object = object_node->getObject();
	std::string object_name = object->getName();

	LibGens::ObjectElementFloat* out_of_control_property = static_cast<LibGens::ObjectElementFloat*>(object->getElement("OutOfControl"));
	float out_of_control = out_of_control_property->value;

	LineData& line_data = boost ? m_lineData1 : m_lineData2;
	bool switched_line = false;
	if (line_data.m_draw_out_of_control && m_total_time >= out_of_control)
	{
		line_data.m_draw_out_of_control = false;
		switched_line = true; // add point on both lines
	}

	LibGens::ObjectElementFloat* speed_property;
	LibGens::ObjectElementFloat* angle_property;
	LibGens::ObjectElementInteger* size_property;
	int type;
	float angle;

	std::string impulse_speed_variable = boost ? "ImpulseSpeedOnBoost" : "ImpulseSpeedOnNormal";
	speed_property = static_cast<LibGens::ObjectElementFloat*>(object->getElement(impulse_speed_variable));
	if (object_name == "ClassicJumpBoard")
	{
		angle_property = static_cast<LibGens::ObjectElementFloat*>(object->getElement(boost ? "ClassicJumpAngleOnBoost" :"ClassicJumpAngleOnNormal"));
		angle = angle_property->value;
		type = 4;
	}
	else if (object_name == "JumpBoard")
	{
		size_property = static_cast<LibGens::ObjectElementInteger*>(object->getElement("AngleType"));
		switch (size_property->value)
		{
		case 0: // 15S
			type = 0;
			break;
		case 1: // 30S
			type = 1;
			break;
		default:
			return;
		}
	}
	else // JumpBoard3D, AdlibTrickJump
	{
		size_property = static_cast<LibGens::ObjectElementInteger*>(object->getElement("SizeType"));
		switch (size_property->value)
		{
		case 0: // 30M
			type = 2;
			break;
		case 1: // 30L
			type = 3;
			break;
		default:
			return;
		}
	}

	// angle -> sub_1033F60
	// offset -> sub_10341F0
	float impulse_speed = speed_property->value;
	float y_offset = 0.0f;
	switch (type)
	{
	case 0: // 15S
		y_offset = 0.25;
		angle = 15;
		break;
			
	case 1: // 30S
		y_offset = 0.45;
		angle = 30;
		break;

	case 2: // 30M
		y_offset = 1.0;
		angle = 30;
		break;

	case 3: // 30L
		y_offset = 2.0;
		angle = 30;
		break;

	case 4: // Classic
		y_offset = 0.0;
		break;
			
	default:
		return;
	}

	Ogre::Vector3 direction(0, 0, -1);
	Ogre::Quaternion obj_rotation = node->getRotation();

	Ogre::Vector3 right_axis = node->getRotation() * Ogre::Vector3(1, 0, 0);
	Ogre::Quaternion pitch_rotation(Ogre::Radian((angle * LIBGENS_MATH_PI) / 180), right_axis);
	direction = pitch_rotation * node->getRotation() * direction;

	act_gravity		= true;
	float gravity   = LIBGENS_MATH_GRAVITY;
	float delta_y   = (impulse_speed * m_total_time * direction.y) + (0.5 * -gravity * (m_total_time * m_total_time));

	float new_pos_x = (impulse_speed * m_total_time) * direction.x;
	float new_pos_y = delta_y;
	float new_pos_z = (impulse_speed * m_total_time) * direction.z;

	Ogre::Vector3 new_position = node->getPosition();
	Ogre::Vector3 position_add(new_pos_x, new_pos_y, new_pos_z);
	Ogre::Vector3 local_offset = node->getRotation() * Ogre::Vector3(0, y_offset, 0);
	new_position += position_add + local_offset;

	if (switched_line || line_data.m_draw_out_of_control)
		line_data.m_lines_out_of_control->addPoint(new_position);
	if (switched_line || !line_data.m_draw_out_of_control)
		line_data.m_lines->addPoint(new_position);
}

void TrajectoryNode::getTrajectoryDashRing(EditorNode* node)
{
	ObjectNode* object_node = editor_application->getObjectNodeFromEditorNode(node);
	LibGens::Object* object = object_node->getObject();

	LibGens::ObjectElementFloat* first_speed_property   = static_cast<LibGens::ObjectElementFloat*>(object->getElement("FirstSpeed"));
	LibGens::ObjectElementFloat* keep_distance_property = static_cast<LibGens::ObjectElementFloat*>(object->getElement("KeepVelocityDistance"));
	LibGens::ObjectElementFloat* out_of_control_property = static_cast<LibGens::ObjectElementFloat*>(object->getElement("OutOfControl"));
	float first_speed	  = first_speed_property->value;
	float ignore_distance = keep_distance_property->value;
	float out_of_control = out_of_control_property->value;

	Ogre::Vector3 direction(0, 0, 1);
	Ogre::Quaternion node_rotation = node->getRotation();
	direction  = node_rotation * direction;
	direction *= -1;

	bool switched_line = false;
	if (m_lineData1.m_draw_out_of_control && m_total_time >= out_of_control)
	{
		m_lineData1.m_draw_out_of_control = false;
		switched_line = true; // add point on both lines
	}

	float delta_y	= getTrajectoryGravity(first_speed, ignore_distance, direction.y);

	float new_pos_x = first_speed * m_total_time * direction.x;
	float new_pos_y = delta_y + (keep_velocity_distance * direction.y);
	float new_pos_z = first_speed * m_total_time * direction.z;

	Ogre::Vector3 new_position = node->getPosition();
	Ogre::Vector3 position_add(new_pos_x, new_pos_y, new_pos_z);
	new_position += position_add;

	if (switched_line || m_lineData1.m_draw_out_of_control)
		m_lineData1.m_lines_out_of_control->addPoint(new_position);

	if (switched_line || !m_lineData1.m_draw_out_of_control)
		m_lineData1.m_lines->addPoint(new_position);
}

void TrajectoryNode::getTrajectoryTrickJumper(EditorNode* node)
{
	ObjectNode* object_node = editor_application->getObjectNodeFromEditorNode(node);
	LibGens::Object* object = object_node->getObject();
	std::string object_name = object->getName();

	LibGens::ObjectElementFloat* first_out_of_control_property = static_cast<LibGens::ObjectElementFloat*>(object->getElement("FirstOutOfControl"));
	LibGens::ObjectElementFloat* first_pitch_property = static_cast<LibGens::ObjectElementFloat*>(object->getElement("FirstPitch"));
	LibGens::ObjectElementFloat* first_speed_property = static_cast<LibGens::ObjectElementFloat*>(object->getElement("FirstSpeed"));

	LibGens::ObjectElementFloat* second_out_of_control_property = static_cast<LibGens::ObjectElementFloat*>(object->getElement("SecondOutOfControl"));
	LibGens::ObjectElementFloat* second_pitch_property = static_cast<LibGens::ObjectElementFloat*>(object->getElement("SecondPitch"));
	LibGens::ObjectElementFloat* second_speed_property = static_cast<LibGens::ObjectElementFloat*>(object->getElement("SecondSpeed"));

	LibGens::ObjectElementBool* is_side_view_property = static_cast<LibGens::ObjectElementBool*>(object->getElement("IsSideView"));

	Ogre::Quaternion obj_rotation = node->getRotation();
	Ogre::Vector3 right_axis = obj_rotation * Ogre::Vector3(1, 0, 0);
	Ogre::Quaternion first_pitch_rotation(Ogre::Radian((first_pitch_property->value * LIBGENS_MATH_PI) / 180), right_axis);
	Ogre::Quaternion second_pitch_rotation(Ogre::Radian((second_pitch_property->value * LIBGENS_MATH_PI) / 180), right_axis);
	Ogre::Vector3 first_direction = first_pitch_rotation * obj_rotation * Ogre::Vector3(0, 0, -1);
	Ogre::Vector3 second_direction = second_pitch_rotation * obj_rotation * Ogre::Vector3(0, 0, -1);

	act_gravity = true;
	float gravity = LIBGENS_MATH_GRAVITY;
	float y_offset = is_side_view_property->value ? 1.0 : 2.0;
	Ogre::Vector3 local_offset = node->getRotation() * Ogre::Vector3(0, y_offset, 0);

	// fail line
	{
		bool switched_line = false;
		if (m_lineData1.m_draw_out_of_control && m_total_time >= first_out_of_control_property->value)
		{
			m_lineData1.m_draw_out_of_control = false;
			switched_line = true; // add point on both lines
		}

		Ogre::Vector3 new_position = node->getPosition() + local_offset;
		new_position.x += (first_speed_property->value * m_total_time) * first_direction.x;
		new_position.y += (first_speed_property->value * m_total_time * first_direction.y) + (0.5 * -gravity * (m_total_time * m_total_time));
		new_position.z += (first_speed_property->value * m_total_time) * first_direction.z;

		if (switched_line || m_lineData1.m_draw_out_of_control)
			m_lineData1.m_lines_out_of_control->addPoint(new_position);
		if (switched_line || !m_lineData1.m_draw_out_of_control)
			m_lineData1.m_lines->addPoint(new_position);
	}

	// success line
	{
		bool switched_line = false;
		if (m_lineData2.m_draw_out_of_control && m_total_time >= first_out_of_control_property->value)
		{
			m_lineData2.m_draw_out_of_control = false;
			switched_line = true; // add point on both lines
		}

		float peak_time = first_speed_property->value * first_direction.y / gravity;
		Ogre::Vector3 start_position = node->getPosition() + local_offset;
		if (peak_time > 0.0f)
		{
			start_position.x += (first_speed_property->value * peak_time) * first_direction.x;
			start_position.y += (first_speed_property->value * peak_time * first_direction.y) + (0.5 * -gravity * (peak_time * peak_time));
			start_position.z += (first_speed_property->value * peak_time) * first_direction.z;
		}

		Ogre::Vector3 new_position = start_position;
		new_position.x += (second_speed_property->value * m_total_time) * second_direction.x;
		new_position.y += (second_speed_property->value * m_total_time * second_direction.y) + (0.5 * -gravity * (m_total_time * m_total_time));
		new_position.z += (second_speed_property->value * m_total_time) * second_direction.z;

		if (switched_line || m_lineData2.m_draw_out_of_control)
			m_lineData2.m_lines_out_of_control->addPoint(new_position);
		if (switched_line || !m_lineData2.m_draw_out_of_control)
			m_lineData2.m_lines->addPoint(new_position);
	}
}
