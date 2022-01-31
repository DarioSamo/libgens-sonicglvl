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

#include "HCWindow.h"

QList<LibGens::TerrainGroup *> HCWindow::convertTerrainGroups(SceneData &scene_data) {
	QList<LibGens::TerrainGroup *> groups;

	QList<int> indices = scene_data.instances.keys();
	foreach(int index, indices) {
		if (index != -1) {
			QList<LibGens::TerrainInstance *> instances = scene_data.instances.values(index);

			if (instances.size()) {
				logProgress(ProgressNormal, QString("Found %1 instances for group with index %2.").arg(instances.size()).arg(index));

				QList<std::string> model_names;
				foreach(LibGens::TerrainInstance *instance, instances) {
					std::string model_name = instance->getModelName();
					if (!model_names.contains(model_name)) {
						model_names.append(model_name);
					}
				}

				LibGens::TerrainGroup *group = new LibGens::TerrainGroup();

				foreach(LibGens::TerrainInstance* instance, instances) {
					std::vector<LibGens::TerrainInstance*> subset;
					subset.push_back(instance);
					group->addInstances(subset);
				}

				foreach(std::string model_name, model_names) {
					group->addFakeModel(model_name);
				}

				group->buildSpheres();
				group->setSubsetID(index);
				groups.append(group);
			}

			scene_data.instances.remove(index);
		}
	}

	QList<LibGens::TerrainInstance *> instances = scene_data.instances.values(-1);
	if (instances.size()) {
		logProgress(ProgressNormal, QString("Found %1 unassigned instances. Generating automatic terrain groups with cell size %2...").arg(instances.size()).arg(converter_settings.group_cell_size));
		LibGens::AABB empty;
		empty.start = empty.end = LibGens::Vector3(0, 0, 0);
		convertTerrainGroups(groups, instances, empty);
	}
	scene_data.instances.clear();

	return groups;
}

void HCWindow::convertTerrainGroups(QList<LibGens::TerrainGroup *> &terrain_groups, QList<LibGens::TerrainInstance *> instances, LibGens::AABB aabb) {
	if (instances.empty()) {
		return;
	}

	// Calculate current AABB
	if ((aabb.start == LibGens::Vector3(0, 0, 0)) && (aabb.end == LibGens::Vector3(0, 0, 0))) {
		aabb.reset();
		foreach(LibGens::TerrainInstance *instance, instances) {
			aabb.merge(instance->getAABB());
		}
	}

	logProgress(ProgressNormal, QString("Recursive Generation Step: %1 instances. [%2, %3, %4][%5, %6, %7][%8 /%9 / %10]").arg(instances.size()).arg(aabb.start.x).arg(aabb.start.y).arg(aabb.start.z).arg(aabb.end.x).arg(aabb.end.y).arg(aabb.end.z).arg(aabb.sizeX()).arg(aabb.sizeY()).arg(aabb.sizeZ()));

	float size_x = aabb.sizeX();
	float size_y = aabb.sizeY();
	float size_z = aabb.sizeZ();

	// Create terrain group
	if ((instances.size() == 1) || ((size_x < converter_settings.group_cell_size) && (size_y < converter_settings.group_cell_size) && (size_z < converter_settings.group_cell_size))) {
		// Create new group with this instance
		QList<std::string> model_names;
		foreach(LibGens::TerrainInstance *instance, instances) {
			std::string model_name = instance->getModelName();
			if (!model_names.contains(model_name)) {
				model_names.append(model_name);
			}
		}

		logProgress(ProgressNormal, QString("Recursive Generation Step: Creating terrain group with %1 instances.").arg(instances.size()));
		LibGens::TerrainGroup *group = new LibGens::TerrainGroup();

		foreach(LibGens::TerrainInstance* instance, instances) {
			std::vector<LibGens::TerrainInstance*> subset;
			subset.push_back(instance);
			group->addInstances(subset);
		}

		foreach(std::string model_name, model_names) {
			group->addFakeModel(model_name);
		}

		group->buildSpheres();
		group->setSubsetID(-1);
		terrain_groups.append(group);
	}
	// Split AABB on biggest axis and keep creating terrain groups recursively
	else if (instances.size() > 1) {
		QList<LibGens::TerrainInstance *> left_instances;
		QList<LibGens::TerrainInstance *> right_instances;
		int axis = LIBGENS_MATH_AXIS_X;
		float size = max(max(size_x, size_y), size_z);
		if (size_y == size) axis = LIBGENS_MATH_AXIS_Y;
		if (size_z == size) axis = LIBGENS_MATH_AXIS_Z;

		LibGens::AABB left_aabb = aabb.half(axis, LIBGENS_MATH_SIDE_LEFT);
		LibGens::AABB right_aabb = aabb.half(axis, LIBGENS_MATH_SIDE_RIGHT);

		foreach(LibGens::TerrainInstance *instance, instances) {
			LibGens::AABB left_inter = left_aabb.intersection(instance->getAABB());
			LibGens::AABB right_inter = right_aabb.intersection(instance->getAABB());
			float left_size = left_inter.size();
			float right_size = right_inter.size();

			// Throw at whatever has the less amount of instances
			if (left_size == right_size) {
				if (right_instances.size() < left_instances.size()) {
					right_instances.append(instance);
				}
				else {
					left_instances.append(instance);
				}
			}
			else if (left_size > right_size) {
				left_instances.append(instance);
			}
			else {
				right_instances.append(instance);
			}
		}

		logProgress(ProgressNormal, QString("Recursive Generation Step: Splitting %1 and %2 instances.").arg(left_instances.size()).arg(right_instances.size()));

		instances.clear();
		convertTerrainGroups(terrain_groups, left_instances, left_aabb);
		convertTerrainGroups(terrain_groups, right_instances, right_aabb);	
	}
}