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

#include "Havok.h"
#include "FBX.h"
#include "FBXManager.h"
#include "Material.h"
#include "MaterialLibrary.h"
#include "Texture.h"
#include "Bone.h"
#include "Model.h"
#include "Submesh.h"
#include "Vertex.h"
#include "TerrainInstance.h"
#include "HavokSkeletonCache.h"
#include "HavokAnimationCache.h"

namespace LibGens {
	void FBXManager::exportFBX(FBX *fbx, string filename) {
		int lFileFormat = sdk_manager->GetIOPluginRegistry()->GetNativeWriterFormat();

		FbxExporter* lExporter = FbxExporter::Create(sdk_manager, "");
		bool lExportStatus=lExporter->Initialize(filename.c_str(), lFileFormat, sdk_manager->GetIOSettings());

		if(!lExportStatus) {
			printf("Call to FbxExporter::Initialize() failed.\n");
			printf("Error returned: %s\n\n", lExporter->GetStatus().GetErrorString());
			return;
		}

		FbxScene* lScene = fbx->getScene();
		lScene->GetGlobalSettings().SetAxisSystem(FbxAxisSystem::Max);
		lScene->GetGlobalSettings().SetSystemUnit(FbxSystemUnit::m);

		// Export scene
		lExporter->Export(lScene);
		lExporter->Destroy();
	}


	FbxSurfacePhong *FBX::addMaterial(Material *material) {
		FbxSurfacePhong* lMaterial = NULL;

		FbxString lMaterialName = material->getName().c_str();
		FbxString lShadingName  = "Phong";
		FbxDouble3 lBlack(0.0, 0.0, 0.0);
		FbxDouble3 lDiffuseColor(1.0, 1.0, 1.0);

		lMaterial = FbxSurfacePhong::Create(scene, lMaterialName.Buffer());
		// Generate primary and secondary colors.
		lMaterial->Emissive.Set(lBlack);
		lMaterial->Ambient.Set(lBlack);
		lMaterial->AmbientFactor.Set(1.0);
		// Add texture for diffuse channel
		lMaterial->Diffuse.Set(lDiffuseColor);
		lMaterial->DiffuseFactor.Set(1.0);
		lMaterial->ShadingModel.Set(lShadingName);
		lMaterial->Shininess.Set(0.0);
		lMaterial->Specular.Set(lBlack);
		lMaterial->SpecularFactor.Set(0.0);

		// Set texture properties.
		Texture *texture_unit = material->getTextureByUnit("diffuse");
		if (texture_unit) {
			FbxFileTexture* lTexture = FbxFileTexture::Create(scene, texture_unit->getTexset().c_str());
			lTexture->SetFileName((material->getFolder() + texture_unit->getName() + LIBGENS_TEXTURE_FILE_EXTENSION).c_str());
			lTexture->SetTextureUse(FbxTexture::eStandard);
			lTexture->SetMappingType(FbxTexture::eUV);
			lTexture->SetMaterialUse(FbxFileTexture::eModelMaterial);
			lTexture->SetSwapUV(false);
			lTexture->SetTranslation(0.0, 0.0);
			lTexture->SetScale(1.0, 1.0);
			lTexture->SetRotation(0.0, 0.0);
			lTexture->UVSet.Set("UVChannel_1");
			if (lMaterial) lMaterial->Diffuse.ConnectSrcObject(lTexture);
		}

		if (texture_unit) {
			FbxFileTexture* lTexture = FbxFileTexture::Create(scene, (texture_unit->getTexset()+"_ambient").c_str());
			lTexture->SetFileName((material->getFolder() + texture_unit->getName() + LIBGENS_TEXTURE_FILE_EXTENSION).c_str());
			lTexture->SetTextureUse(FbxTexture::eStandard);
			lTexture->SetMappingType(FbxTexture::eUV);
			lTexture->SetMaterialUse(FbxFileTexture::eModelMaterial);
			lTexture->SetSwapUV(false);
			lTexture->SetTranslation(0.0, 0.0);
			lTexture->SetScale(1.0, 1.0);
			lTexture->SetRotation(0.0, 0.0);
			lTexture->UVSet.Set("UVChannel_2");
			if (lMaterial) lMaterial->Ambient.ConnectSrcObject(lTexture);
		}

		return lMaterial;
	}


	FbxNode *FBX::addHavokBone(FbxNode *parent_node, unsigned int parent_index, vector<FbxNode *> &skeleton_bones, hkaSkeleton *skeleton) {
		FbxNode *root_bone=NULL;
		for (int b=0; b<skeleton->m_bones.getSize(); b++) {
			hkaBone &bone           = skeleton->m_bones[b];
			string bone_name        = bone.m_name;
			size_t model_bone_index = 0;
			bool found=false;

			if (skeleton->m_parentIndices[b] != parent_index) {
				continue;
			}

			FbxSkeleton* lSkeletonRootAttribute = FbxSkeleton::Create(scene, bone_name.c_str());
			if (parent_index == -1) lSkeletonRootAttribute->SetSkeletonType(FbxSkeleton::eRoot);
			else lSkeletonRootAttribute->SetSkeletonType(FbxSkeleton::eLimbNode);

			FbxNode* bone_node = FbxNode::Create(scene, bone_name.c_str());
			bone_node->SetNodeAttribute(lSkeletonRootAttribute);
			root_bone = bone_node;

			hkQsTransform ref = skeleton->m_referencePose[b];
			bone_node->LclTranslation.Set(FbxVector4(ref.getTranslation()(0), ref.getTranslation()(1), ref.getTranslation()(2)));
			//bone_node->LclScaling.Set(FbxVector4(ref.getScale()(0), ref.getScale()(1), ref.getScale()(2)));
			FbxQuaternion lcl_quat(ref.getRotation()(0), ref.getRotation()(1), ref.getRotation()(2), ref.getRotation()(3));
			FbxVector4 lcl_rotation;
			lcl_rotation.SetXYZ(lcl_quat);
			bone_node->LclRotation.Set(lcl_rotation);

			skeleton_bones[b] = bone_node;
			parent_node->AddChild(bone_node);

			addHavokBone(bone_node, b, skeleton_bones, skeleton);
		}
		return root_bone;
	}

	
	// Create a skeleton with 2 segments.
	FbxNode *FBX::addHavokSkeleton(vector<FbxNode *> &skeleton_bones, hkaSkeleton *skeleton) {
		FbxNode *lRootNode = scene->GetRootNode();
		FbxNode *skeleton_root_bone=addHavokBone(lRootNode, -1, skeleton_bones, skeleton);
		return skeleton_root_bone;
	}

	void FBX::skinModelToSkeleton(Model *model, FbxMesh *model_mesh, vector<FbxNode *> &skeleton_bones, FbxAMatrix lSkinMatrix) {
		FbxSkin* lSkin = FbxSkin::Create(scene, "");
		vector<Bone *> model_bones=model->getBones();
		list<Vertex *> vertices=model->getVertexList();

		for (size_t i=0; i<skeleton_bones.size(); i++) {
			// Search for matching bone in the model, dump skinning cluster if found
			string bone_name = skeleton_bones[i]->GetName();
			bool found=false;
			size_t model_bone_index;

			for (model_bone_index = 0; model_bone_index<model_bones.size(); model_bone_index++) {
				if (model_bones[model_bone_index]->getName() == bone_name) {
					found = true;
					break;
				}
			}

			if (found) {
				// Create skin cluster
				FbxCluster *lCluster = FbxCluster::Create(scene, "");
				lCluster->SetLink(skeleton_bones[i]);
				lCluster->SetLinkMode(FbxCluster::eTotalOne);

				unsigned int index=0;
				for (list<Vertex *>::iterator it=vertices.begin(); it!=vertices.end(); it++) {
					for (size_t j=0; j<4; j++) {
						unsigned char bone_index = (*it)->getBoneIndex(j);
						if (bone_index != 0xFF) {
							unsigned char bone_weight = (*it)->getBoneWeight(j);
							float bone_weight_f = (float)bone_weight / 255.0;

							unsigned char real_bone_index=(*it)->getParent()->getBone(bone_index);
							if (real_bone_index == model_bone_index) {
								lCluster->AddControlPointIndex(index, bone_weight_f);
							}
						}
					}
					index++;
				}

				lCluster->SetTransformMatrix(lSkinMatrix);
				FbxAMatrix lXMatrix = skeleton_bones[i]->EvaluateGlobalTransform();
				lCluster->SetTransformLinkMatrix(lXMatrix);
				lSkin->AddCluster(lCluster);
			}
		}

		model_mesh->AddDeformer(lSkin);
	}


	void FBX::addHavokAnimation(vector<FbxNode *> &skeleton_bones, hkaSkeleton *skeleton, hkaAnimationBinding *animation_binding, hkaAnimation *animation) {
		float duration       = animation->m_duration;
		float frame_duration = 1.0 / 30.0;
		float current_frame  = 0.0;
		bool first_frame = true;

		if (frame_duration <= 0.0) return;

		// Create an animation control
		hkaDefaultAnimationControl* ac = new hkaDefaultAnimationControl(animation_binding);
		ac->setLocalTime(0);

		// Create a new animated skeleton
		hkaAnimatedSkeleton* animated_skeleton = new hkaAnimatedSkeleton(skeleton);
		animated_skeleton->addAnimationControl( ac );
		ac->removeReference();

		FbxAnimStack* lAnimStack = FbxAnimStack::Create(scene, "");
        FbxAnimLayer* lAnimLayer = FbxAnimLayer::Create(scene, "");
        lAnimStack->AddMember(lAnimLayer);

		while (true) {
			if (!first_frame) current_frame += frame_duration;
			if (current_frame > duration) {
				current_frame = duration;
			}

			first_frame = false;

			// Advance the animation
			animated_skeleton->stepDeltaTime(frame_duration);

			hkaPose pose(animated_skeleton->getSkeleton());
			animated_skeleton->sampleAndCombineAnimations(pose.accessUnsyncedPoseLocalSpace().begin(), pose.getFloatSlotValues().begin());

			for (int i=0; i<skeleton->m_bones.getSize(); i++) {
				string bone_name= ToString(skeleton->m_bones[i].m_name);

				for (size_t bone_index=0; bone_index<skeleton_bones.size(); bone_index++) {
					if (bone_name == skeleton_bones[bone_index]->GetName()) {
						hkQsTransform transform_local = pose.getBoneLocalSpace(i);
						hkQsTransform transform_model = hkQsTransform::IDENTITY;

						hkInt16 parent_index = skeleton->m_parentIndices[i];
						if ((parent_index >= 0) && (parent_index < (int) skeleton_bones.size())) {
							transform_model = pose.getBoneModelSpace(parent_index);
						}

						hkVector4 ts_m = transform_model.getScale();
						hkVector4 tt = transform_local.getTranslation();
						hkQuaternion tr = transform_local.getRotation();
						hkVector4 ts = transform_local.getScale();

						tt = hkVector4(tt(0) / ts_m(0), tt(1) / ts_m(0), tt(2) / ts_m(0));

						//printfErrorMessage(LOG, "%s at %f: %f %f %f %f\n", bone_name.c_str(), current_frame, (float)tt(0), (float)tt(1), (float)tt(2), (float)tt(3));
						//printfErrorMessage(LOG, "%s at %f: %f %f %f %f\n", bone_name.c_str(), current_frame, (float)ts(0), (float)ts(1), (float)ts(2), (float)ts(3));

						FbxAnimCurve* curveTX = skeleton_bones[bone_index]->LclTranslation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
						FbxAnimCurve* curveTY = skeleton_bones[bone_index]->LclTranslation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
						FbxAnimCurve* curveTZ = skeleton_bones[bone_index]->LclTranslation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);

						FbxAnimCurve* curveRX = skeleton_bones[bone_index]->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
						FbxAnimCurve* curveRY = skeleton_bones[bone_index]->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
						FbxAnimCurve* curveRZ = skeleton_bones[bone_index]->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);

						FbxAnimCurve* curveSX = skeleton_bones[bone_index]->LclScaling.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
						FbxAnimCurve* curveSY = skeleton_bones[bone_index]->LclScaling.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
						FbxAnimCurve* curveSZ = skeleton_bones[bone_index]->LclScaling.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);

						FbxTime lTime;
						lTime.SetSecondDouble(current_frame);

						if (curveTX) {
							curveTX->KeyModifyBegin();
							int lKeyIndex = curveTX->KeyAdd(lTime);
							curveTX->KeySetValue(lKeyIndex, tt(0));
							curveTX->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear);
							curveTX->KeyModifyEnd();
						}

						if (curveTY) {
							curveTY->KeyModifyBegin();
							int lKeyIndex = curveTY->KeyAdd(lTime);
							curveTY->KeySetValue(lKeyIndex, tt(1));
							curveTY->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear);
							curveTY->KeyModifyEnd();
						}

						if (curveTZ) {
							curveTZ->KeyModifyBegin();
							int lKeyIndex = curveTZ->KeyAdd(lTime);
							curveTZ->KeySetValue(lKeyIndex, tt(2));
							curveTZ->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear);
							curveTZ->KeyModifyEnd();
						}

						
						if (curveSX) {
							curveSX->KeyModifyBegin();
							int lKeyIndex = curveSX->KeyAdd(lTime);
							curveSX->KeySetValue(lKeyIndex, ts(0));
							curveSX->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear);
							curveSX->KeyModifyEnd();
						}

						if (curveSY) {
							curveSY->KeyModifyBegin();
							int lKeyIndex = curveSY->KeyAdd(lTime);
							curveSY->KeySetValue(lKeyIndex, ts(0));
							curveSY->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear);
							curveSY->KeyModifyEnd();
						}

						if (curveSZ) {
							curveSZ->KeyModifyBegin();
							int lKeyIndex = curveSZ->KeyAdd(lTime);
							curveSZ->KeySetValue(lKeyIndex, ts(0));
							curveSZ->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear);
							curveSZ->KeyModifyEnd();
						}
						
						

						FbxQuaternion lcl_quat(tr(0), tr(1), tr(2), tr(3));
						FbxVector4 lcl_rotation;
						lcl_rotation.SetXYZ(lcl_quat);
						
						if (curveRX) {
							curveRX->KeyModifyBegin();
							int lKeyIndex = curveRX->KeyAdd(lTime);
							curveRX->KeySetValue(lKeyIndex, lcl_rotation[0]);
							curveRX->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear);
							curveRX->KeyModifyEnd();
						}

						if (curveRY) {
							curveRY->KeyModifyBegin();
							int lKeyIndex = curveRY->KeyAdd(lTime);
							curveRY->KeySetValue(lKeyIndex, lcl_rotation[1]);
							curveRY->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear);
							curveRY->KeyModifyEnd();
						}

						if (curveRZ) {
							curveRZ->KeyModifyBegin();
							int lKeyIndex = curveRZ->KeyAdd(lTime);
							curveRZ->KeySetValue(lKeyIndex, lcl_rotation[2]);
							curveRZ->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear);
							curveRZ->KeyModifyEnd();
						}
							

						break;
					}
				}
			}

			if (current_frame == duration) {
				break;
			}
		}
	}

	FbxMesh *FBX::addTerrainInstance(TerrainInstance *instance) {
		if (!instance) return NULL;

		return addNamedNode(instance->getName(), instance->getModel(), NULL, NULL, instance->getMatrix());
	}


	FbxMesh *FBX::addNode(Model *model, HavokSkeletonCache *skeleton, HavokAnimationCache *animation, Matrix4 transform_matrix) {
		string model_name = "Dummy";

		if (model) {
			model_name=model->getName();
		}

		return addNamedNode(model_name, model, skeleton, animation, transform_matrix);
	}

	FbxMesh *FBX::addNamedNode(string name, Model *model, HavokSkeletonCache *skeleton, HavokAnimationCache *animation, Matrix4 transform_matrix) {
		FbxNode* lMeshNode = NULL;
		FbxMesh* lMesh = NULL;

		if (model) {
			models.push_back(model);

			lMeshNode = FbxNode::Create(scene, name.c_str());
			lMesh = FbxMesh::Create(scene, (name + "_mesh").c_str());

			// Create Layer 0
			FbxLayer* lLayer = lMesh->GetLayer(0);
			if (lLayer == NULL)
			{
				lMesh->CreateLayer();
				lLayer = lMesh->GetLayer(0);
			}

			// Build Transform
			Vector3 position;
			Vector3 scale;
			Quaternion orientation;
			transform_matrix.decomposition(position, scale, orientation);

			lMeshNode->LclTranslation.Set(FbxVector4(position.x, position.y, position.z));
			lMeshNode->LclScaling.Set(FbxVector4(scale.x, scale.y, scale.z));

			FbxQuaternion lcl_quat(orientation.x, orientation.y, orientation.z, orientation.w);
			FbxVector4 lcl_rotation;
			lcl_rotation.SetXYZ(lcl_quat);
			lMeshNode->LclRotation.Set(lcl_rotation);
			
			// Build Submeshes
			list<Vertex *> model_vertices;
			list<unsigned int> model_faces;
			list<string> material_names;
			vector<unsigned int> material_mappings;
			model->getTotalData(model_vertices, model_faces, material_names, material_mappings);

			lMesh->InitControlPoints(model_vertices.size());
			FbxVector4* lControlPoints = lMesh->GetControlPoints();

			FbxLayerElementNormal* lLayerElementNormal = FbxLayerElementNormal::Create(lMesh, "");
			lLayerElementNormal->SetMappingMode(FbxLayerElement::eByControlPoint);
			lLayerElementNormal->SetReferenceMode(FbxLayerElement::eDirect);
			lLayer->SetNormals(lLayerElementNormal);

			FbxLayerElementBinormal* lLayerElementBinormal = FbxLayerElementBinormal::Create(lMesh, "");
			lLayerElementBinormal->SetMappingMode(FbxGeometryElement::eByControlPoint);
			lLayerElementBinormal->SetReferenceMode(FbxGeometryElement::eDirect);
			lLayer->SetBinormals(lLayerElementBinormal);

			FbxLayerElementTangent* lLayerElementTangent = FbxLayerElementTangent::Create(lMesh, "");
			lLayerElementTangent->SetMappingMode(FbxGeometryElement::eByControlPoint);
			lLayerElementTangent->SetReferenceMode(FbxGeometryElement::eDirect);
			lLayer->SetTangents(lLayerElementTangent);

			FbxLayerElementVertexColor* lLayerElementVertexColor = FbxLayerElementVertexColor::Create(lMesh, "");
			lLayerElementVertexColor->SetMappingMode(FbxGeometryElement::eByControlPoint);
			lLayerElementVertexColor->SetReferenceMode(FbxGeometryElement::eDirect);
			lLayer->SetVertexColors(lLayerElementVertexColor);

			FbxLayerElementUV* lLayerUVElement = FbxLayerElementUV::Create(lMesh, "UVChannel_1");
			lLayerUVElement->SetMappingMode(FbxGeometryElement::eByControlPoint);
			lLayerUVElement->SetReferenceMode(FbxGeometryElement::eDirect);
			lLayer->SetUVs(lLayerUVElement, FbxLayerElement::eTextureDiffuse);

			FbxLayerElementUV* lLayerUV2Element = FbxLayerElementUV::Create(lMesh, "UVChannel_2");
			lLayerUV2Element->SetMappingMode(FbxGeometryElement::eByControlPoint);
			lLayerUV2Element->SetReferenceMode(FbxGeometryElement::eDirect);
			lLayer->SetUVs(lLayerUV2Element, FbxLayerElement::eTextureAmbient);
			
			// Create Vertices
			unsigned int index=0;
			for (list<Vertex *>::iterator it=model_vertices.begin(); it!=model_vertices.end(); it++) {
				Vector3 position = (*it)->getPosition();
				Vector3 normal = (*it)->getNormal();
				Vector3 binormal = (*it)->getBinormal();
				Vector3 tangent = (*it)->getTangent();
				Vector2 uv_0 = (*it)->getUV(0);
				Vector2 uv_1 = (*it)->getUV(1);
				Color   color = (*it)->getColor();

				lControlPoints[index] = FbxVector4(position.x, position.y, position.z);
				lLayerElementNormal->GetDirectArray().Add(FbxVector4(normal.x, normal.y, normal.z));
				lLayerElementBinormal->GetDirectArray().Add(FbxVector4(binormal.x, binormal.y, binormal.z));
				lLayerElementTangent->GetDirectArray().Add(FbxVector4(tangent.x, tangent.y, tangent.z));
				lLayerUVElement->GetDirectArray().Add(FbxVector2(uv_0.x, 1.0-uv_0.y));
				lLayerUV2Element->GetDirectArray().Add(FbxVector2(uv_1.x, 1.0-uv_1.y));
				lLayerElementVertexColor->GetDirectArray().Add(FbxColor(color.r, color.g, color.b, color.a));
				index++;
			}

			// Create Faces
			unsigned int face_index=0;
			unsigned int global_index=0;
			for (list<unsigned int>::iterator it=model_faces.begin(); it!=model_faces.end(); it++) {
				if (face_index == 0) {
					lMesh->BeginPolygon(-1, -1, -1, false);
				}

				lMesh->AddPolygon((*it));

				face_index++;
				if (face_index >= 3) {
					lMesh->EndPolygon();
					face_index = 0;
				}

				global_index++;
			}

			// Set material indices
			FbxLayerElementMaterial* lMaterialLayer = FbxLayerElementMaterial::Create(lMesh, "MaterialIndices");
			lMaterialLayer->SetMappingMode(FbxLayerElement::eByPolygon);
			lMaterialLayer->SetReferenceMode(FbxLayerElement::eIndexToDirect);
			lLayer->SetMaterials(lMaterialLayer);

			for (size_t i = 0; i < model_faces.size()/3; i++) {
				lMaterialLayer->GetIndexArray().Add(material_mappings[i*3]);
			}

			// Add Materials
			for (list<string>::iterator it=material_names.begin(); it!=material_names.end(); it++) {
				if (material_library) {
					Material *material = material_library->getMaterial(*it);
					if (material) {
						FbxSurfacePhong *lMaterial=addMaterial(material);
						if (lMaterial) {
							lMeshNode->AddMaterial(lMaterial);
						}
					}
				}
			}

			lMeshNode->SetNodeAttribute(lMesh);
			lMeshNode->SetShadingMode(FbxNode::eTextureShading);

			FbxNode *lRootNode = scene->GetRootNode();
			lRootNode->AddChild(lMeshNode);
		}

		if (skeleton) {
			hkaSkeleton *havok_skeleton=skeleton->getSkeleton();
			
			if (havok_skeleton) {
				vector<FbxNode *> skeleton_bones;
				skeleton_bones.resize(havok_skeleton->m_bones.getSize(), NULL);

				addHavokSkeleton(skeleton_bones, havok_skeleton);

				if (lMeshNode) {
					skinModelToSkeleton(model, lMesh, skeleton_bones, lMeshNode->EvaluateGlobalTransform());
				}

				if (animation) {
					hkaAnimationBinding *havok_animation_binding=animation->getAnimationBinding();
					hkaAnimation *havok_animation=animation->getAnimation();
					addHavokAnimation(skeleton_bones, havok_skeleton, havok_animation_binding, havok_animation);
				}
			}
		}

		return lMesh;
	}

	/*
	FbxMesh *FBX::addSonicCollision(SonicCollision *sonic_collision) {
		FbxNode* lMeshNode = NULL;
		FbxMesh* lMesh = NULL;

		if (sonic_collision) {
			lMeshNode = FbxNode::Create(scene, "collision_mesh_node");
			lMesh = FbxMesh::Create(scene, "collision_mesh");

			lMesh->InitControlPoints(sonic_collision->vertex_pool.size());
			FbxVector4* lControlPoints = lMesh->GetControlPoints();

			unsigned int index=0;
			for (index=0; index<sonic_collision->vertex_pool.size(); index++) {
				lControlPoints[index] = FbxVector4(sonic_collision->vertex_pool[index].x, 
												   sonic_collision->vertex_pool[index].y, 
												   sonic_collision->vertex_pool[index].z);
			}

			for (index=0; index<sonic_collision->face_pool.size(); index++) {
				lMesh->BeginPolygon();
				lMesh->AddPolygon(sonic_collision->face_pool[index].face_1);
				lMesh->AddPolygon(sonic_collision->face_pool[index].face_2);
				lMesh->AddPolygon(sonic_collision->face_pool[index].face_3);
				lMesh->EndPolygon();
			}
		
			lMeshNode->SetNodeAttribute(lMesh);
			FbxNode *lRootNode = scene->GetRootNode();
			lRootNode->AddChild(lMeshNode);
		}

		return lMesh;
	}
	*/

	FbxMesh *FBX::addHavokCollision(string name, hkGeometry *geometry, Matrix4 transform) {
		FbxNode* lMeshNode = NULL;
		FbxMesh* lMesh = NULL;

		if (geometry) {
			lMeshNode = FbxNode::Create(scene, name.c_str());
			lMesh = FbxMesh::Create(scene, name.c_str());

			Vector3 position;
			Vector3 scale;
			Quaternion orientation;
			transform.decomposition(position, scale, orientation);

			lMeshNode->LclTranslation.Set(FbxVector4(position.x, position.y, position.z));
			lMeshNode->LclScaling.Set(FbxVector4(scale.x, scale.y, scale.z));

			FbxQuaternion lcl_quat(orientation.x, orientation.y, orientation.z, orientation.w);
			FbxVector4 lcl_rotation;
			lcl_rotation.SetXYZ(lcl_quat);
			lMeshNode->LclRotation.Set(lcl_rotation);

			lMesh->InitControlPoints(geometry->m_vertices.getSize());
			FbxVector4* lControlPoints = lMesh->GetControlPoints();

			int index=0;
			for (index=0; index<geometry->m_vertices.getSize(); index++) {
				lControlPoints[index] = FbxVector4(geometry->m_vertices[index](0), 
												   geometry->m_vertices[index](1), 
												   geometry->m_vertices[index](2));
			}

			for (index=0; index<geometry->m_triangles.getSize(); index++) {
				lMesh->BeginPolygon();
				lMesh->AddPolygon(geometry->m_triangles[index].m_a);
				lMesh->AddPolygon(geometry->m_triangles[index].m_b);
				lMesh->AddPolygon(geometry->m_triangles[index].m_c);
				lMesh->EndPolygon();
			}
		
			lMeshNode->SetNodeAttribute(lMesh);
			FbxNode *lRootNode = scene->GetRootNode();
			lRootNode->AddChild(lMeshNode);
		}

		return lMesh;
	}
};