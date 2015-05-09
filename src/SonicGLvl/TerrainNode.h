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

#include "EditorNode.h"
#include "TerrainInstance.h"
#include "GITextureGroup.h"

#ifndef TERRAIN_NODE_H_INCLUDED
#define TERRAIN_NODE_H_INCLUDED

class TerrainNode : public EditorNode {
	protected:
		LibGens::TerrainInstance *terrain_instance;
		Ogre::Vector3 real_center;
		float radius;
		size_t quality_level;
	public:
		TerrainNode(LibGens::TerrainInstance *terrain_instance_p, Ogre::SceneManager *scene_manager, LibGens::MaterialLibrary *material_library);

		void setPosition(Ogre::Vector3 v) {
			EditorNode::setPosition(v);
			if (terrain_instance) terrain_instance->setPosition(LibGens::Vector3(v.x, v.y, v.z));
		}

		void setRotation(Ogre::Quaternion v) {
			EditorNode::setRotation(v);
			if (terrain_instance) terrain_instance->setRotation(LibGens::Quaternion(v.w, v.x, v.y, v.z));
		}

		Ogre::Vector3 getRealCenter() {
			return real_center;
		}

		float getRadius() {
			return radius;
		}

		void setGIQualityLevel(LibGens::GITextureGroupInfo *gi_group_info, size_t quality_level_p);

		LibGens::TerrainInstance *getTerrainInstance() {
			return terrain_instance;
		}
};

#endif