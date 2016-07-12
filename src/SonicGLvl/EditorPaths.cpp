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

#include "EditorApplication.h"

void EditorApplication::loadLevelPaths() {
	if (!current_level) return;
	LibGens::Level *level=current_level->getLevel();
	if (!level) return;

	list<LibGens::Path *> paths = level->getPaths();
	for (list<LibGens::Path *>::iterator it=paths.begin(); it!=paths.end(); it++) {
		LibGens::Library *path_library = (*it)->getLibrary();
		LibGens::Scene *path_scene = (*it)->getScene();

		if (path_scene && path_library) {
			list<LibGens::Node *> path_scene_nodes = path_scene->getNodes();
			for (list<LibGens::Node *>::iterator it_n = path_scene_nodes.begin(); it_n != path_scene_nodes.end(); it_n++) {
				LibGens::Node *path_node = (*it_n);
				string path_instance_name = path_node->getInstanceName();

				// Delete first character #
				path_instance_name.erase(path_instance_name.begin());

				LibGens::Spline *path_spline = path_library->getSpline(path_instance_name);
				if (path_spline) {
					PathNode *editor_path_node = new PathNode(path_instance_name, path_spline, path_node, scene_manager);
				}
			}
		}
	}
}