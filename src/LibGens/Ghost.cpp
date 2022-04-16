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

#include "Ghost.h"
#include "GhostNode.h"
#include "FBX.h"
#include "FBXManager.h"

#define FBX_SET_KEY(KEY, TIME, VALUE) { if (KEY) { KEY->KeyModifyBegin(); int lKeyIndex = KEY->KeyAdd(TIME); KEY->KeySetValue(lKeyIndex, VALUE); KEY->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear); KEY->KeyModifyEnd(); } }

namespace LibGens {
	Ghost::Ghost(string filename) {
		File file(filename, LIBGENS_FILE_READ_BINARY);

		if (file.valid()) {
			read(&file);
			file.close();
		}
	}

	void Ghost::read(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_GHOST_ERROR_MESSAGE_NULL_FILE);
			return;
		}
		size_t header_address=file->getCurrentAddress();

		unsigned int animation_total=0;
		unsigned int node_total=0;
		file->readInt32BE(&animation_total);
		file->readInt32BE(&node_total);

		for (size_t i=0; i<animation_total; i++) {
			char name[32]="";
			file->read(name, 32);
			string animation_name=ToString(name);
			animation_names.push_back(animation_name);
		}

		for (size_t i=0; i<node_total; i++) {
			GhostNode *ghost_node = new GhostNode();
			ghost_node->read(file);
			ghost_nodes.push_back(ghost_node);
		}
	}

	void Ghost::write(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_GHOST_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		int animation_count = animation_names.size();
		int node_count = ghost_nodes.size();
		file->writeInt32BE(&animation_count);
		file->writeInt32BE(&node_count);
		char name_buffer[32];

		for (int i = 0; i < animation_count; i++)
		{
			ZeroMemory(name_buffer, sizeof(name_buffer));
			strcpy(name_buffer, animation_names[i].c_str());
			file->write(name_buffer, sizeof(name_buffer));
		}

		for (int i = 0; i < node_count; i++)
		{
			ghost_nodes[i]->write(file);
		}
	}

	void Ghost::save(string filename) {
		File file(filename, LIBGENS_FILE_WRITE_BINARY);
		if (file.valid()) {
			write(&file);
			file.close();
		}
	}

	void Ghost::calculate(float time, Vector3 &position, Quaternion &rotation, string &animation_name, float &animation_frame, bool &animation_ball) const {
		GhostNode *previous_node=NULL;
		GhostNode *next_node=NULL;

		float total_time=0;
		float factor=0;

		for (size_t i=0; i<ghost_nodes.size(); i++) {
			previous_node = next_node;
			next_node = ghost_nodes[i];
			total_time += next_node->timer;

			if (total_time > time) {
				break;
			}
		}

		if (previous_node && next_node) {
			float previous_time = total_time - next_node->timer;
			factor = (time - (previous_time)) / (next_node->timer);
			position = previous_node->position + ((next_node->position - previous_node->position) * factor);
			rotation = rotation.slerp(factor, previous_node->rotation, next_node->rotation);
			animation_name = animation_names[previous_node->animation_index];
			animation_frame = previous_node->animation_frame + ((next_node->animation_frame - previous_node->animation_frame) * factor);
			animation_ball = previous_node->animation_ball;
		}
		else if (next_node) {
			position = next_node->position;
			rotation = next_node->rotation;
			animation_name = animation_names[next_node->animation_index];
			animation_frame = next_node->animation_frame;
			animation_ball = next_node->animation_ball;
		}
		else if (previous_node) {
			position = previous_node->position;
			rotation = previous_node->rotation;
			animation_name = animation_names[previous_node->animation_index];
			animation_frame = previous_node->animation_frame;
			animation_ball = previous_node->animation_ball;
		}
	}

	float Ghost::calculateDuration() const {
		float duration = 0;
		for (size_t i = 0; i < ghost_nodes.size(); i++)
		{
			duration += ghost_nodes[i]->timer;
		}
		return duration;
	}

	FBX* Ghost::buildFbx(FBXManager* manager, Model* model, MaterialLibrary* material_lib) const
	{
		FBX* fbx = new FBX(manager->getManager(), "Ghost");
		fbx->setMaterialLibrary(material_lib);

		const FbxMesh* lMesh = fbx->addNode(model, nullptr, nullptr, Matrix4(), false);
		FbxNode* lNode = lMesh->GetNode(0);

		FbxTimeSpan duration(FbxTimeSeconds(0), FbxTimeSeconds(calculateDuration()));
		FbxAnimStack* lAnimStack = FbxAnimStack::Create(fbx->getScene(), "Ghost_Move");
		FbxAnimLayer* lAnimLayer = FbxAnimLayer::Create(fbx->getScene(), "Ghost_Move");
		lAnimStack->AddMember(lAnimLayer);
		lAnimStack->SetLocalTimeSpan(duration);
		lAnimStack->SetReferenceTimeSpan(duration);
		
		FbxAnimCurve* curveTX = lNode->LclTranslation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
		FbxAnimCurve* curveTY = lNode->LclTranslation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
		FbxAnimCurve* curveTZ = lNode->LclTranslation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);

		FbxAnimCurve* curveRX = lNode->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
		FbxAnimCurve* curveRY = lNode->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
		FbxAnimCurve* curveRZ = lNode->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);

		FbxTime time;
		time.Set(0);
		if (!ghost_nodes.empty())
			time = FbxTimeSeconds(ghost_nodes[0]->timer);

		for (size_t i = 0; i < ghost_nodes.size(); ++i)
		{
			const GhostNode* lgNode = ghost_nodes[i];
			FbxQuaternion lcl_quat(lgNode->rotation.x, lgNode->rotation.y, lgNode->rotation.z, lgNode->rotation.w);
			FbxVector4 lcl_rotation;
			lcl_rotation.SetXYZ(lcl_quat);

			FBX_SET_KEY(curveTX, time, lgNode->position.x);
			FBX_SET_KEY(curveTY, time, lgNode->position.y);
			FBX_SET_KEY(curveTZ, time, lgNode->position.z);

			FBX_SET_KEY(curveRX, time, lcl_rotation[0]);
			FBX_SET_KEY(curveRY, time, lcl_rotation[1]);
			FBX_SET_KEY(curveRZ, time, lcl_rotation[2]);

			time += FbxTimeSeconds(lgNode->timer);
		}
		
		return fbx;
	}
};
