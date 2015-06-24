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

TerrainNode::TerrainNode(LibGens::TerrainInstance *terrain_instance_p, Ogre::SceneManager *scene_manager, LibGens::MaterialLibrary *material_library) {
	type = EDITOR_NODE_TERRAIN;

	LibGens::Model *model=terrain_instance_p->getModel();
	terrain_instance = terrain_instance_p;

	if (model) {
		//printf("Building terrain instance %s\n", terrain_instance->getName().c_str());
		LibGens::AABB instance_aabb=terrain_instance->getAABB();
		real_center = Ogre::Vector3(instance_aabb.centerX(), instance_aabb.centerY(), instance_aabb.centerZ());
		radius = instance_aabb.sizeMax()/2;
		quality_level = 3;

		scene_node = scene_manager->getRootSceneNode()->createChildSceneNode();
		buildModel(scene_node, model, model->getName(), "", scene_manager, material_library, EDITOR_NODE_QUERY_TERRAIN, GENERAL_MESH_GROUP, true);
		
		LibGens::Matrix4 instance_matrix=terrain_instance->getMatrix();

		Ogre::Matrix4 matrix = Ogre::Matrix4(instance_matrix.m[0][0], instance_matrix.m[0][1], instance_matrix.m[0][2], instance_matrix.m[0][3],
		                                     instance_matrix.m[1][0], instance_matrix.m[1][1], instance_matrix.m[1][2], instance_matrix.m[1][3],
											 instance_matrix.m[2][0], instance_matrix.m[2][1], instance_matrix.m[2][2], instance_matrix.m[2][3],
										     instance_matrix.m[3][0], instance_matrix.m[3][1], instance_matrix.m[3][2], instance_matrix.m[3][3]);


		matrix.decomposition(position, scale, rotation);
		scene_node->setPosition(position);
		scene_node->setScale(scale);
		scene_node->setOrientation(rotation);

		scene_node->getUserObjectBindings().setUserAny(EDITOR_NODE_BINDING, Ogre::Any((EditorNode *)this));
		
		
	}
	else {
		printf("Couldn't find a matching model for the terrain instance.\n");
	}

	selected = false;
}

void TerrainNode::setGIQualityLevel(LibGens::GITextureGroupInfo *gi_group_info, size_t quality_level_p) {
	/*
	if (quality_level_p != quality_level) {
		quality_level = quality_level_p;

		LibGens::GISubtexture *gi_texture=NULL;
		if (gi_group_info && (quality_level < 3)) {
			gi_texture=gi_group_info->getTextureByInstance(terrain_instance->getName(), quality_level);
		}
		
		unsigned short attached_objects=scene_node->numAttachedObjects();
		for (unsigned short i=0; i<attached_objects; i++) {
			Ogre::Entity *entity=static_cast<Ogre::Entity *>(scene_node->getAttachedObject(i));
			unsigned int attached_entities=entity->getNumSubEntities();

			for (unsigned int j=0; j<attached_entities; j++) {
				Ogre::SubEntity *sub_entity=entity->getSubEntity(j);
				Ogre::MaterialPtr material=sub_entity->getMaterial();
					
				Ogre::Pass *pass=material->getTechnique(0)->getPass(0);
				if (pass->hasVertexProgram()) {
					Ogre::GpuProgramParametersSharedPtr vp_parameters = pass->getVertexProgramParameters();

					if (gi_texture) {
						string tex_name=gi_texture->getParent()->getFolder() + "-" + gi_texture->getParent()->getName() + LIBGENS_TEXTURE_FILE_EXTENSION;
						pass->getTextureUnitState(10)->setTextureName(tex_name);

						if (!vp_parameters.isNull()) {
							vp_parameters->setConstant(186, Ogre::Vector4(gi_texture->getWidth(), gi_texture->getHeight(), gi_texture->getX(), gi_texture->getY()));
						}
					}
					else {
						pass->getTextureUnitState(10)->setTextureName("white.dds");
					}
				}
			}
		}
	}
	*/

	if (quality_level_p != quality_level) {
		quality_level = quality_level_p;

		LibGens::GISubtexture *gi_texture=NULL;
		if (gi_group_info && (quality_level < 3)) {
			gi_texture=gi_group_info->getTextureByInstance(terrain_instance->getName(), quality_level);
		}

		if (!gi_group_info) {
			string texture_name = terrain_instance->getName() + LIBGENS_TEXTURE_FILE_EXTENSION;
			string texture_path = LibGens::File::folderFromFilename(terrain_instance->getFilename()) + texture_name;

			//printf("Searching for %s\n", texture_path);

			if (LibGens::File::check(texture_path)) {
				gi_texture = new LibGens::GISubtexture();
				gi_texture->setWidth(1.0);
				gi_texture->setHeight(1.0);
				gi_texture->setX(0.0);
				gi_texture->setY(0.0);
				gi_texture->setPath(texture_name);
			}
		}

		if (gi_texture) {
			GlobalIlluminationParameter *global_illumination_parameter=new GlobalIlluminationParameter(gi_texture);
			GlobalIlluminationAssignVisitor visitor(global_illumination_parameter);

			unsigned short attached_objects=scene_node->numAttachedObjects();
			for (unsigned short i=0; i<attached_objects; i++) {
				Ogre::Entity *entity=static_cast<Ogre::Entity *>(scene_node->getAttachedObject(i));
				entity->visitRenderables(&visitor);
			}
		}
	}
}