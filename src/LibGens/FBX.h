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

#pragma once

#define LIBGENS_FBX_DEFAULT_SCENE_NAME_IMPORT    "FBXImport"
#define LIBGENS_FBX_DEFAULT_SCENE_NAME_EXPORT    "FBXExport"

class hkaSkeleton;
struct hkGeometry;
class hkaAnimationBinding;
class hkaAnimation;

namespace LibGens {
	class Submesh;
	class Model;
	class TerrainInstance;
	class Material;
	class MaterialLibrary;
	class HavokSkeletonCache;
	class HavokAnimationCache;

	class FBX {
		protected:
			list<Model *> models;
			list<TerrainInstance *> instances;
			map<Material *, FbxSurfacePhong *> material_map;
			MaterialLibrary *material_library;
			FbxScene *scene;
		public:
			FBX(FbxManager *sdk_manager, string scene_name=LIBGENS_FBX_DEFAULT_SCENE_NAME_EXPORT);
			FBX(FbxScene* lScene);
			FbxSurfacePhong *addMaterial(Material *material);

			FbxMesh *addNode(Model *model, HavokSkeletonCache *skeleton=NULL, HavokAnimationCache *animation=NULL, Matrix4 transform_matrix = Matrix4());
			FbxMesh *addNamedNode(string name, Model *model, HavokSkeletonCache *skeleton=NULL, HavokAnimationCache *animation=NULL, Matrix4 transform_matrix = Matrix4());
			FbxMesh *addTerrainInstance(TerrainInstance *instance);

			void skinModelToSkeleton(Model *model, FbxMesh *model_mesh, vector<FbxNode *> &skeleton_bones, FbxAMatrix lSkinMatrix);
			FbxNode *addHavokSkeleton(vector<FbxNode *> &skeleton_bones, hkaSkeleton *skeleton);
			FbxNode *addHavokBone(FbxNode *parent_node, unsigned int parent_index, vector<FbxNode *> &skeleton_bones, hkaSkeleton *skeleton);
			void addHavokAnimation(vector<FbxNode *> &skeleton_bones, hkaSkeleton *skeleton, hkaAnimationBinding *animation_binding, hkaAnimation *animation);
			FbxMesh *addHavokCollision(string name, hkGeometry *geometry, Matrix4 transform);

			list<Model *> getModels();
			list<TerrainInstance *> getInstances();
			void setMaterialLibrary(MaterialLibrary *v);
			MaterialLibrary *getMaterialLibrary();
			FbxScene *getScene();

			Material *buildMaterialFromFBX(FbxSurfaceMaterial *lMaterial);
			Submesh *buildSubmeshFromFBX(FbxMesh *lMesh, int material_index, bool single_material=false);
			Model *buildModelFromFBX(FbxMesh *lMesh, int material_count, vector<string> material_names);
			TerrainInstance *buildTerrainInstanceFromFBX(FbxNode *lNode);
	};
};
