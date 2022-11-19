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
		bool lExportStatus = lExporter->Initialize(filename.c_str(), lFileFormat, sdk_manager->GetIOSettings());

		if(!lExportStatus) {
			printf("Call to FbxExporter::Initialize() failed.\n");
			printf("Error returned: %s\n\n", lExporter->GetStatus().GetErrorString());
			return;
		}

		FbxScene* lScene = fbx->getScene();
		lScene->GetGlobalSettings().SetAxisSystem(FbxAxisSystem::OpenGL); // Matches with the game despite the name
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

			bind_pose->Add(bone_node, bone_node->EvaluateGlobalTransform());

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
						unsigned short bone_index = (*it)->getBoneIndex(j);
						if (bone_index != 0xFFFF) {
							unsigned char bone_weight = (*it)->getBoneWeight(j);
							float bone_weight_f = (float)bone_weight / 255.0;

							unsigned short real_bone_index=(*it)->getParent()->getBone(bone_index);
							if (real_bone_index == model_bone_index) {
								lCluster->AddControlPointIndex(index, bone_weight_f);
							}
						}
					}
					index++;
				}

				lCluster->SetTransformMatrix(lSkinMatrix);

				const Matrix4 matrix = model_bones[model_bone_index]->getMatrix().inverse();

				FbxAMatrix lXMatrix;
				lXMatrix.mData[0][0] = matrix.m[0][0]; lXMatrix.mData[0][1] = matrix.m[1][0]; lXMatrix.mData[0][2] = matrix.m[2][0]; lXMatrix.mData[0][3] = matrix.m[3][0];
				lXMatrix.mData[1][0] = matrix.m[0][1]; lXMatrix.mData[1][1] = matrix.m[1][1]; lXMatrix.mData[1][2] = matrix.m[2][1]; lXMatrix.mData[1][3] = matrix.m[3][1];
				lXMatrix.mData[2][0] = matrix.m[0][2]; lXMatrix.mData[2][1] = matrix.m[1][2]; lXMatrix.mData[2][2] = matrix.m[2][2]; lXMatrix.mData[2][3] = matrix.m[3][2];
				lXMatrix.mData[3][0] = matrix.m[0][3]; lXMatrix.mData[3][1] = matrix.m[1][3]; lXMatrix.mData[3][2] = matrix.m[2][3]; lXMatrix.mData[3][3] = matrix.m[3][3];

				lCluster->SetTransformLinkMatrix(lXMatrix);

				lSkin->AddCluster(lCluster);
			}
		}

		model_mesh->AddDeformer(lSkin);
	}


	void FBX::addHavokAnimation(vector<FbxNode *> &skeleton_bones, hkaSkeleton *skeleton, hkaAnimationBinding *animation_binding, hkaAnimation *animation, const string& animation_name) {
		// Create an animation control
		hkaDefaultAnimationControl* ac = new hkaDefaultAnimationControl(animation_binding);

		// Create a new animated skeleton
		hkaAnimatedSkeleton* animated_skeleton = new hkaAnimatedSkeleton(skeleton);
		animated_skeleton->addAnimationControl( ac );
		ac->removeReference();

		FbxAnimStack* lAnimStack = FbxAnimStack::Create(scene, animation_name.c_str());
		FbxAnimLayer* lAnimLayer = FbxAnimLayer::Create(scene, animation_name.c_str());
		lAnimStack->AddMember(lAnimLayer);
		lAnimStack->SetLocalTimeSpan(FbxTimeSpan(FbxTimeSeconds(0), FbxTimeSeconds(animation->m_duration)));

		for (int i = 0; i < animation->getNumOriginalFrames(); i++) {
			// Set animation time
			FbxTime lTime;
			lTime.SetSecondDouble((double)i / (double)(animation->getNumOriginalFrames() - 1) * (double)animation->m_duration);
			ac->setLocalTime((float)lTime.GetSecondDouble());

			hkaPose pose(animated_skeleton->getSkeleton());
			animated_skeleton->sampleAndCombineAnimations(pose.accessUnsyncedPoseLocalSpace().begin(), pose.getFloatSlotValues().begin());

			for (int j=0; j<skeleton->m_bones.getSize(); j++) {
				string bone_name= ToString(skeleton->m_bones[j].m_name);

				for (size_t bone_index=0; bone_index<skeleton_bones.size(); bone_index++) {
					if (bone_name == skeleton_bones[bone_index]->GetName()) {
						hkQsTransform transform_local = pose.getBoneLocalSpace(j);
						hkQsTransform transform_model = hkQsTransform::IDENTITY;

						hkInt16 parent_index = skeleton->m_parentIndices[j];
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
		}

		// Apply unroll filter to fix flickering when interpolating
		FbxAnimCurveFilterUnroll lFilter;

		for (size_t i = 0; i < skeleton_bones.size(); i++)
		{
			FbxAnimCurve* lCurve[3];

			lCurve[0] = skeleton_bones[i]->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
			lCurve[1] = skeleton_bones[i]->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
			lCurve[2] = skeleton_bones[i]->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);

			if (lCurve[0] && lCurve[1] && lCurve[2] && lFilter.NeedApply(lCurve, 3))
				lFilter.Apply(lCurve, 3);
		}
	}

	FbxMesh *FBX::addTerrainInstance(TerrainInstance *instance) {
		if (!instance) return NULL;

		return addNamedNode(instance->getName(), instance->getModel(), NULL, NULL, instance->getMatrix());
	}


	FbxMesh *FBX::addNode(Model *model, HavokSkeletonCache *skeleton, HavokAnimationCache *animation, Matrix4 transform_matrix, bool skin) {
		string model_name = "Dummy";

		if (model) {
			model_name=model->getName();
		}

		return addNamedNode(model_name, model, skeleton, animation, transform_matrix, skin);
	}

	FbxMesh *FBX::addNamedNode(string name, Model *model, HavokSkeletonCache *skeleton, HavokAnimationCache *animation, Matrix4 transform_matrix, bool skin) {
		FbxNode* lMeshNode = NULL;
		FbxMesh* lMesh = NULL;

		if (model) {
			models.push_back(model);

			lMeshNode = FbxNode::Create(scene, name.c_str());
			lMesh = FbxMesh::Create(scene, (name + "_mesh").c_str());

			// Create layers
			// UV2 needs to be in the second layer
			while (lMesh->CreateLayer() != 1);
			FbxLayer* lLayer = lMesh->GetLayer(0);
			FbxLayer* lLayer1 = lMesh->GetLayer(1);

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

			// Element creation needs to be in this exact order for 3DS max to read them properly.
			FbxLayerElementNormal* lLayerElementNormal = (FbxLayerElementNormal*)lLayer->CreateLayerElementOfType(FbxLayerElement::eNormal);
			lLayerElementNormal->SetMappingMode(FbxLayerElement::eByControlPoint);
			lLayerElementNormal->SetReferenceMode(FbxLayerElement::eDirect);

			FbxLayerElementBinormal* lLayerElementBinormal = (FbxLayerElementBinormal*)lLayer->CreateLayerElementOfType(FbxLayerElement::eBiNormal);
			lLayerElementBinormal->SetName("UVChannel_1");
			lLayerElementBinormal->SetMappingMode(FbxGeometryElement::eByControlPoint);
			lLayerElementBinormal->SetReferenceMode(FbxGeometryElement::eDirect);

			FbxLayerElementTangent* lLayerElementTangent = (FbxLayerElementTangent*)lLayer->CreateLayerElementOfType(FbxLayerElement::eTangent);
			lLayerElementTangent->SetName("UVChannel_1");
			lLayerElementTangent->SetMappingMode(FbxGeometryElement::eByControlPoint);
			lLayerElementTangent->SetReferenceMode(FbxGeometryElement::eDirect);

			FbxLayerElementMaterial* lMaterialLayer = (FbxLayerElementMaterial*)lLayer->CreateLayerElementOfType(FbxLayerElement::eMaterial);
			lMaterialLayer->SetMappingMode(FbxLayerElement::eByPolygon);
			lMaterialLayer->SetReferenceMode(FbxLayerElement::eIndexToDirect);

			FbxLayerElementVertexColor* lLayerElementVertexColor = (FbxLayerElementVertexColor*)lLayer->CreateLayerElementOfType(FbxLayerElement::eVertexColor);
			lLayerElementVertexColor->SetMappingMode(FbxGeometryElement::eByPolygonVertex); // 3DS Max requires exactly these modes for vertex color.
			lLayerElementVertexColor->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

			FbxLayerElementUV* lLayerUVElement = (FbxLayerElementUV*)lLayer->CreateLayerElementOfType(FbxLayerElement::eUV);
			lLayerUVElement->SetName("UVChannel_1");
			lLayerUVElement->SetMappingMode(FbxGeometryElement::eByControlPoint);
			lLayerUVElement->SetReferenceMode(FbxGeometryElement::eDirect);

			FbxLayerElementUV* lLayerUV2Element = (FbxLayerElementUV*)lLayer1->CreateLayerElementOfType(FbxLayerElement::eUV);
			lLayerUV2Element->SetName("UVChannel_2");
			lLayerUV2Element->SetMappingMode(FbxGeometryElement::eByControlPoint);
			lLayerUV2Element->SetReferenceMode(FbxGeometryElement::eDirect);
			
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
					lMesh->BeginPolygon(material_mappings[global_index], -1, -1, false);
				}

				lMesh->AddPolygon((*it));

				face_index++;
				if (face_index >= 3) {
					lMesh->EndPolygon();
					face_index = 0;
				}

				// Vertex color element was set to eByPolygonVertex
				lLayerElementVertexColor->GetIndexArray().Add(*it);

				global_index++;
			}

			// Add Materials
			for (list<string>::iterator it = material_names.begin(); it != material_names.end(); it++) {
				if (material_library) {
					Material* material = material_library->getMaterial(*it);
					if (material) {
						FbxSurfacePhong* lMaterial = material_map[material];
						if (!lMaterial) {
							lMaterial = addMaterial(material);
							material_map[material] = lMaterial;
						}

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

			bind_pose->Add(lMeshNode, lMeshNode->EvaluateGlobalTransform());
		}

		if (skeleton && skin) {
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
					const string animation_name = File::nameFromFilenameNoExtension(animation->getName());
					addHavokAnimation(skeleton_bones, havok_skeleton, havok_animation_binding, havok_animation, animation_name);
				}
			}
		}
		
		else if (lMeshNode && skin) {
			vector<FbxNode *> skeleton_bones;
			addSkeleton(skeleton_bones, model);
			
			if (skeleton_bones.size()) {
				skinModelToSkeleton(model, lMesh, skeleton_bones, lMeshNode->EvaluateGlobalTransform());
			}
		}

		return lMesh;
	}

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
	
	void FBX::addSkeleton(vector<FbxNode *>& skeleton_bones, Model *model) {
		vector<Bone *> bones = model->getBones();
		
		FbxNode *lRootNode = scene->GetRootNode();
		skeleton_bones.reserve(bones.size());
		
		addBone(lRootNode, -1, skeleton_bones, bones);
	}
	
	void FBX::addBone(FbxNode *parent_node, unsigned int parent_index, vector<FbxNode *>& skeleton_bones, vector<Bone *>& bones) {
		for (size_t i = 0; i < bones.size(); i++) {
			if (bones[i]->getParentIndex() != parent_index) {
				continue;
			}
			
			string bone_name = bones[i]->getName();
			
			FbxNode *lBoneNode = FbxNode::Create(scene, bone_name.c_str());
			FbxSkeleton* lSkeletonRootAttribute = FbxSkeleton::Create(scene, bone_name.c_str());
			if (parent_index == -1) {
				lSkeletonRootAttribute->SetSkeletonType(FbxSkeleton::eRoot);
			}
			else {
				lSkeletonRootAttribute->SetSkeletonType(FbxSkeleton::eLimbNode);
			}
			lBoneNode->SetNodeAttribute(lSkeletonRootAttribute);
			
			Matrix4 bone_matrix = bones[i]->getMatrix().inverse();
			if (parent_index >= 0 && parent_index < bones.size()) {
				bone_matrix = bones[parent_index]->getMatrix() * bone_matrix;
			}
			
			Vector3 pos, sca;
			Quaternion ori;
			bone_matrix.decomposition(pos, sca, ori);
			
			lBoneNode->LclTranslation.Set(FbxVector4(pos.x, pos.y, pos.z));
			lBoneNode->LclScaling.Set(FbxVector4(sca.x, sca.y, sca.z));
			
			FbxQuaternion quaternion(ori.x, ori.y, ori.z, ori.w);
			FbxVector4 lcl_rotation;
			lcl_rotation.SetXYZ(quaternion);
			lBoneNode->LclRotation.Set(lcl_rotation);
			
			parent_node->AddChild(lBoneNode);
			skeleton_bones.push_back(lBoneNode);

			bind_pose->Add(lBoneNode, lBoneNode->EvaluateGlobalTransform());
			
			addBone(lBoneNode, i, skeleton_bones, bones);
		}
	}
};
