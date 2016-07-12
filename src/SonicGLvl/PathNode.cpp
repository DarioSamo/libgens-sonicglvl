//=========================================================================
//	  Copyright (c) 2016 SonicGLvl
//
//    This file is part of SonicGLvl, a community-created free level editor 
//    for the PC version of Sonic Generations.
//
//    SonicGLvl is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    SonicGLvl is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//    
//
//    Read AUTHORS.txt, LICENSE.txt and COPYRIGHT.txt for more details.
//=========================================================================

#include "PathNode.h"
#include "Path.h"

PathNode::PathNode(string path_name, LibGens::Spline *spline_p, LibGens::Node *node_p, Ogre::SceneManager *scene_manager, float spline_precision) {
	type = EDITOR_NODE_PATH;

	spline = spline_p;
	node = node_p;

	scene_node = scene_manager->getRootSceneNode()->createChildSceneNode();

	// Position Scene Node based on Path Instance
	LibGens::Vector3 translate_v = node->getTranslate();
	LibGens::Vector3 scale_v = node->getScale();
	LibGens::Quaternion rotation_v = node->getRotation();

	LibGens::Matrix4 node_matrix;
	node_matrix.makeTransform(translate_v, scale_v, rotation_v);

	transform_matrix = Ogre::Matrix4(node_matrix.m[0][0], node_matrix.m[0][1], node_matrix.m[0][2], node_matrix.m[0][3],
		                             node_matrix.m[1][0], node_matrix.m[1][1], node_matrix.m[1][2], node_matrix.m[1][3],
									 node_matrix.m[2][0], node_matrix.m[2][1], node_matrix.m[2][2], node_matrix.m[2][3],
									 node_matrix.m[3][0], node_matrix.m[3][1], node_matrix.m[3][2], node_matrix.m[3][3]);

	transform_matrix.decomposition(position, scale, rotation);

	scene_node->setPosition(position);
	scene_node->setScale(scale);
	scene_node->setOrientation(rotation);

	scene_node->getUserObjectBindings().setUserAny(EDITOR_NODE_BINDING, Ogre::Any((EditorNode *)this));


	// Create Line Entities
	list<LibGens::Spline3D *> splines = spline->getSplines();

	for (list<LibGens::Spline3D *>::iterator it_s=splines.begin(); it_s!=splines.end(); it_s++) {
		vector<LibGens::Knot *> knots = (*it_s)->getKnots();
		size_t knots_size = knots.size();

		DynamicLines *lines = new DynamicLines(Ogre::RenderOperation::OT_LINE_STRIP);

		for (size_t i=0; i<knots_size; i++) {
			Ogre::Vector3 pos(0.0);
			pos.x = knots[i]->point.x;
			pos.y = knots[i]->point.y;
			pos.z = knots[i]->point.z;
			lines->addPoint(pos);

			if (i < (knots_size-1)) {
				float segment_length = (*it_s)->getSegmentLength(i);
				int interpolation_steps = segment_length / spline_precision;

				if (interpolation_steps > 1) {
					float interpolation_step = 1.0f / (float)interpolation_steps;
					float current_interpolation = interpolation_step;

					while (current_interpolation < 1.0f) {
						LibGens::Vector3 int_point = (*it_s)->interpolateSegment(i, current_interpolation);
						pos.x = int_point.x;
						pos.y = int_point.y;
						pos.z = int_point.z;
						lines->addPoint(pos);

						current_interpolation += interpolation_step;
					}
				}
			}
		}

		lines->update();

		if (path_name.find("@SV") != string::npos) lines->setMaterial("pathsv");
		if (path_name.find("@GR") != string::npos) lines->setMaterial("pathgr");
		if (path_name.find("@DP") != string::npos) lines->setMaterial("pathdp");
		if (path_name.find("@QS") != string::npos) lines->setMaterial("pathqs");

		scene_node->attachObject(lines);
	}

	selected = false;
}