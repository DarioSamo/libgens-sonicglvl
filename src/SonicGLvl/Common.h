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

#include "DynamicLines.h"
#include "Resource.h"
#include "Model.h"
#include "Material.h"
#include "MaterialLibrary.h"
#include "UVAnimation.h"
#include "Havok.h"

#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#define LOG_MSG(x)                 Ogre::LogManager::getSingletonPtr()->logMessage(x)
#define SHOW_MSG(x)                MessageBox(NULL, x, "Info", MB_OK)
#define CONFIRM_MSG(x)             MessageBox(NULL, x, "Info", MB_YESNO)
#define GENERAL_MESH_GROUP         "General"
#define PREVIEW_MESH_GROUP         "Preview"
#define EDITOR_NODE_BINDING        "EditorNodePtr"
#define GENERAL_MESH_VERT_COUNT    10
#define EXTENSION_CHAR_SIZE        4

enum EditorNodeQueryFlags {
	EDITOR_NODE_QUERY_OBJECT         =  0x01,
	EDITOR_NODE_QUERY_TERRAIN        =  0x02,
	EDITOR_NODE_QUERY_PATH_NODE      =  0x04,
	EDITOR_NODE_QUERY_HAVOK          =  0x08,
	EDITOR_NODE_QUERY_GHOST          =  0x10,
	EDITOR_NODE_QUERY_VECTOR         =  0x20,
	EDITOR_NODE_QUERY_NODE           =  0x40,
	EDITOR_NODE_QUERY_PREVIEW_BOX    =  0x80,
};

void buildHavokMesh(Ogre::SceneNode *scene_node, string name, hkGeometry* geometry, Ogre::SceneManager *scene_manager, Ogre::uint32 query_flags, string resource_group);
void updateMaterialShaderParameters(Ogre::Material *ogre_material, LibGens::Material *material, bool no_gi=true, LibGens::UVAnimation *uv_animation=NULL);
void buildMaterial(LibGens::Material *material, string material_name, string resource_group, size_t mesh_slot, bool no_gi=true);
void buildModel(Ogre::SceneNode *scene_node, LibGens::Model *model, string model_name, string skeleton_name, Ogre::SceneManager *scene_manager, LibGens::MaterialLibrary *material_library, Ogre::uint32 query_flags, string resource_group, bool global_illumination=false);
Ogre::Skeleton *buildSkeleton(hkaSkeleton *havok_skeleton, string skel_name, string resource_group);
void prepareSkeletonAndAnimation(string skeleton_id, string animation_id, string resource_group=GENERAL_MESH_GROUP);
void cleanModelResource(LibGens::Model *model, string resource_group);

void quaternionToEulerShortsXYZ(Ogre::Quaternion rotation, unsigned short &rot_x_int, unsigned short &rot_y_int, unsigned short &rot_z_int);
void quaternionToEulerShortsZXY(Ogre::Quaternion rotation, unsigned short &rot_x_int, unsigned short &rot_y_int, unsigned short &rot_z_int);
void quaternionToEulerShortsXZY(Ogre::Quaternion rotation, unsigned short &rot_x_int, unsigned short &rot_y_int, unsigned short &rot_z_int);

void quaternionToEulerIntsXYZ(Ogre::Quaternion rotation, unsigned int &rot_x_int, unsigned int &rot_y_int, unsigned int &rot_z_int);
void quaternionToEulerIntsXZY(Ogre::Quaternion rotation, unsigned int &rot_x_int, unsigned int &rot_y_int, unsigned int &rot_z_int);
void quaternionToEulerIntsZXY(Ogre::Quaternion rotation, unsigned int &rot_x_int, unsigned int &rot_y_int, unsigned int &rot_z_int);

void destroyAllAttachedMovableObjects(Ogre::SceneNode* node, bool unload_resource=false);
void destroySceneNode(Ogre::SceneNode* node, bool unload_resource=false);

float GetDlgItemFloat(HWND hDlg, int idDlgItem);

#endif