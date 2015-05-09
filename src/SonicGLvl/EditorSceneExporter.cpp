//=========================================================================
//	  Copyright (c) 2015 SonicGLvl
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

void EditorApplication::exportSceneFBX(string filename) {
	if (!current_level) return;

	LibGens::FBX *fbx_pack = new LibGens::FBX(fbx_manager->getManager());

	fbx_pack->setMaterialLibrary(current_level->getMaterialLibrary());
	
	for (list<TerrainNode *>::iterator it=terrain_nodes_list.begin(); it!=terrain_nodes_list.end(); it++) {
		FbxMesh *model_node=fbx_pack->addTerrainInstance((*it)->getTerrainInstance());
	}

	fbx_manager->exportFBX(fbx_pack, filename);
}