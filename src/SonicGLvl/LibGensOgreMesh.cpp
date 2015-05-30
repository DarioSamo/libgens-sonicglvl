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

#include "Common.h"
#include "EditorApplication.h"
#include "HavokSkeletonCache.h"
#include "HavokAnimationCache.h"
#include "Material.h"
#include "Parameter.h"
#include "Texture.h"
#include "Mesh.h"
#include "Submesh.h"
#include "Vertex.h"
#include "Bone.h"
#include "DefaultShaderParameters.h"

DefaultShaderParameters default_shader_parameters("../database/DefaultShaderParameterDatabase.xml");

void buildBones(hkaSkeleton *havok_skeleton, Ogre::Skeleton *ogre_skeleton, Ogre::Bone *parent_bone, unsigned int parent_index) {
	for (int b=0; b<havok_skeleton->m_bones.getSize(); b++) {
		hkaBone &bone               = havok_skeleton->m_bones[b];
		string bone_name            = bone.m_name;
		short bone_parent_index     = havok_skeleton->m_parentIndices[b];
		
		if (parent_index != bone_parent_index) {
			continue;
		}
		
		Ogre::Bone *mBone = ogre_skeleton->createBone(bone_name);
		if (!mBone) {
			continue;
		}

		if (parent_bone) {
			parent_bone->addChild(mBone);
		}

		//LOG_MSG((ToString(b) + " - " + bone_name + " Parent: " + ToString(bone_parent_index)).c_str());

		mBone->getUserObjectBindings().setUserAny(Ogre::Any(b));
		
		// Set Reference Pose
		hkQsTransform ref = havok_skeleton->m_referencePose[b];

		Ogre::Vector3 mPos(ref.getTranslation()(0), ref.getTranslation()(1), ref.getTranslation()(2));
		Ogre::Vector3 mScale(ref.getScale()(0), ref.getScale()(1), ref.getScale()(2));
		Ogre::Quaternion mRot(ref.getRotation()(3), ref.getRotation()(0), ref.getRotation()(1), ref.getRotation()(2));

		mBone->setPosition(mPos);
		mBone->setScale(mScale);
		mBone->setOrientation(mRot);

		mBone->setInitialState();

		// Search for Children
		buildBones(havok_skeleton, ogre_skeleton, mBone, b);
	}
}

Ogre::Skeleton *buildSkeleton(hkaSkeleton *havok_skeleton, string skel_name, string resource_group) {
	Ogre::SkeletonPtr mSkel = Ogre::SkeletonManager::getSingleton().create(skel_name, resource_group, true);

	buildBones(havok_skeleton, mSkel.getPointer(), NULL, -1);
	return mSkel.getPointer();
}


void buildAnimation(string animation_name, hkaAnimation *havok_animation, Ogre::Skeleton *ogre_skeleton, float fps) {
	float duration=havok_animation->m_duration;
	int keyframes_count=duration * fps;
	float keyframe_step=1.0 / fps;
	float current_time=0.0;
	vector<Ogre::NodeAnimationTrack *> node_tracks;
	vector<Ogre::Matrix4> parent_transform_tracks;

	Ogre::Animation* mAnim = ogre_skeleton->createAnimation(animation_name, duration);

	// Retrieve Name Tracks
	int tracks_size = havok_animation->m_annotationTracks.getSize();
	node_tracks.resize(tracks_size);
	parent_transform_tracks.resize(tracks_size);
	for (int i=0; i<tracks_size; i++) {
		node_tracks[i] = NULL;

		// Search for matching bone
		hkaAnnotationTrack &atrack = havok_animation->m_annotationTracks[i];
		string bone_name = atrack.m_trackName;

		if (ogre_skeleton->hasBone(bone_name)) {
			Ogre::Bone *mBone=ogre_skeleton->getBone(bone_name);

			// Create Animation Tracks
			Ogre::NodeAnimationTrack *mTrack = mAnim->createNodeTrack(mBone->getHandle(), mBone);
			node_tracks[i] = mTrack;
			parent_transform_tracks[i].makeTransform(mBone->getPosition(), mBone->getScale(), mBone->getOrientation());
		}
	}

	current_time = 0.0f;
	while (current_time != duration) {
		bool add_time=true;

		// Force one keyframe at the end of the animation
		if (current_time > duration) {
			current_time = duration;
			add_time = false;
		}

		// Retrieve Transforms
		int transforms_size = havok_animation->m_numberOfTransformTracks;
		hkQsTransform *transforms = (hkQsTransform *) malloc(sizeof(hkQsTransform) * transforms_size);
		havok_animation->sampleTracks(current_time, transforms, NULL, NULL);
		hkaSkeletonUtils::normalizeRotations(transforms, transforms_size);

		// Create Animation Tracks
		for (int i=0; i<tracks_size; i++) {
			Ogre::NodeAnimationTrack *mTrack=node_tracks[i];

			if (mTrack) {
				// Create Keyframe
				Ogre::TransformKeyFrame* mKey = mTrack->createNodeKeyFrame(current_time);
				hkVector4 tt=transforms[i].getTranslation();
				hkQuaternion tr=transforms[i].getRotation();

				Ogre::Vector3 translate(tt(0), tt(1), tt(2));
				Ogre::Quaternion rotation(tr(3), tr(0), tr(1), tr(2));

				translate -= mTrack->getAssociatedNode()->getPosition();
				rotation = mTrack->getAssociatedNode()->getOrientation().Inverse() * rotation;

				mKey->setTranslate(translate);
				mKey->setRotation(rotation);
				mKey->setScale(Ogre::Vector3::UNIT_SCALE);
			}
		}
		
		std::free(transforms);

		if (add_time) {
			current_time += keyframe_step;
		}
	}
}


void prepareSkeletonAndAnimation(string skeleton_id, string animation_id, string resource_group) {
	if (skeleton_id.size()) {
		LibGens::HavokSkeletonCache *skeleton_cache = SONICGLVL_HAVOK_ENVIROMENT->getSkeleton(skeleton_id);
		if (skeleton_cache) {
			if (!skeleton_cache->hasExtra()) {
				Ogre::Skeleton *ogre_skeleton_resource = buildSkeleton(skeleton_cache->getSkeleton(), skeleton_id, resource_group);
				skeleton_cache->setExtra(skeleton_id);
			}

			Ogre::SkeletonPtr ogre_skeleton = Ogre::SkeletonManager::getSingleton().getByName(skeleton_id, resource_group);
			if ((!ogre_skeleton.isNull()) && animation_id.size()) {
				LibGens::HavokAnimationCache *animation_cache = SONICGLVL_HAVOK_ENVIROMENT->getAnimation(animation_id);
				if (animation_cache) {
					if (!animation_cache->hasExtra()) {
						buildAnimation(animation_id, animation_cache->getAnimation(), ogre_skeleton.getPointer(), SONICGLVL_HAVOK_PRECISION_FPS);
						animation_cache->setExtra(animation_id);
					}
				}
			}
		}
	}
}

void setShaderParameters(Ogre::Pass *pass, Ogre::GpuProgramParametersSharedPtr program_params, LibGens::Material *material, LibGens::ShaderParams *shader_params, LibGens::UVAnimation *uv_animation) {
	EditorLevel *current_editor_level = editor_application->getCurrentLevel();
	LibGens::Level *current_level = NULL;

	if (current_editor_level) {
		current_level = current_editor_level->getLevel();
	}

	vector<string> texture_units_used;
	texture_units_used.clear();

	for (size_t slot=0; slot<5; slot++) {
		vector<LibGens::ShaderParam *> shader_param_list=shader_params->getParameterList(slot);

		for (size_t param=0; param<shader_param_list.size(); param++) {
			LibGens::ShaderParam *shader_parameter = shader_param_list[param];

			unsigned char index = shader_parameter->getIndex();
			unsigned char size = shader_parameter->getSize();

			if (slot == 0) {
				LibGens::Parameter *material_parameter = material->getParameterByName(shader_parameter->getName());

				if (material_parameter) {
					LibGens::Color color = material_parameter->getColor();
					
					if (shader_parameter->getName() == "diffuse")  color.a = 1.0;
					if (shader_parameter->getName() == "specular") color.a = 1.0;
					if (shader_parameter->getName() == "ambient")  color.a = 1.0;

					program_params->setConstant((size_t)index, Ogre::Vector4(color.r, color.g, color.b, color.a));
					continue;
				}

				bool found_in_list = false;
				string shader_parameter_name = shader_parameter->getName();

				for (list<DefaultShaderParameter *>::iterator it = default_shader_parameters.parameters.begin(); it != default_shader_parameters.parameters.end(); it++) {
					if ((*it)->name == shader_parameter_name) {
						program_params->setConstant((size_t)index, Ogre::Vector4((*it)->r, (*it)->g, (*it)->b, (*it)->a));
						found_in_list = true;
						break;
					}
				}

				if (found_in_list) {
					continue;
				}
				
				if (shader_parameter->getName() == "g_MtxProjection") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_PROJECTION_MATRIX);
				}
				else if (shader_parameter->getName() == "g_MtxInvProjection") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_INVERSE_PROJECTION_MATRIX);
				}
				else if (shader_parameter->getName() == "g_MtxView") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_VIEW_MATRIX);
				}
				else if (shader_parameter->getName() == "g_MtxInvView") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_INVERSE_VIEW_MATRIX);
				}
				else if (shader_parameter->getName() == "g_MtxWorld") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_WORLD_MATRIX);
				}
				else if (shader_parameter->getName() == "g_MtxWorldIT") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLD_MATRIX);
				}
				else if (shader_parameter->getName() == "g_MtxPrevView") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_VIEW_MATRIX);
				}
				else if (shader_parameter->getName() == "g_MtxPrevWorld") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_WORLD_MATRIX);
				}
				else if (shader_parameter->getName() == "g_MtxLightViewProjection") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_TEXTURE_VIEWPROJ_MATRIX);
				}
				else if (shader_parameter->getName() == "g_MtxVerticalLightViewProjection") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_TEXTURE_VIEWPROJ_MATRIX);
				}
				else if (shader_parameter->getName() == "g_MtxBillboardY") {
					// Incomplete
				}
				else if (shader_parameter->getName() == "g_MtxPalette") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_WORLD_MATRIX_ARRAY_3x4);
				}
				else if (shader_parameter->getName() == "g_MtxPrevPalette") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_WORLD_MATRIX_ARRAY_3x4);
				}
				else if (shader_parameter->getName() == "g_EyePosition") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_CAMERA_POSITION);
				}
				else if (shader_parameter->getName() == "g_EyeDirection") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_VIEW_DIRECTION);
				}
				else if (shader_parameter->getName() == "g_ViewportSize") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_VIEWPORT_SIZE);
				}
				else if (shader_parameter->getName() == "g_CameraNearFarAspect") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgAmbientColor") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgGroundColor") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgSkyColor") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgPowerGlossLevel") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgEmissionPower") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgGlobalLight_Direction") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_POSITION, 0);
				}
				else if (shader_parameter->getName() == "mrgGlobalLight_Direction_View") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_POSITION_VIEW_SPACE, 0);
				}
				else if (shader_parameter->getName() == "mrgGlobalLight_Diffuse") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR, 0);
				}
				else if (shader_parameter->getName() == "mrgGlobalLight_Specular") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_SPECULAR_COLOUR, 0);
				}
				else if (shader_parameter->getName() == "mrgLocallightIndexArray") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgLocalLight0_Position") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_POSITION, 1);
				}
				else if (shader_parameter->getName() == "mrgLocalLight0_Color") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR, 1);
				}
				else if (shader_parameter->getName() == "mrgLocalLight0_Range") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION, 1);
				}
				else if (shader_parameter->getName() == "mrgLocalLight0_Attribute") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION, 1);
				}
				else if (shader_parameter->getName() == "mrgLocalLight1_Position") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_POSITION, 2);
				}
				else if (shader_parameter->getName() == "mrgLocalLight1_Color") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR, 2);
				}
				else if (shader_parameter->getName() == "mrgLocalLight1_Range") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION, 2);
				}
				else if (shader_parameter->getName() == "mrgLocalLight1_Attribute") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION, 2);
				}
				else if (shader_parameter->getName() == "mrgLocalLight2_Position") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_POSITION, 3);
				}
				else if (shader_parameter->getName() == "mrgLocalLight2_Color") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR, 3);
				}
				else if (shader_parameter->getName() == "mrgLocalLight2_Range") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION, 3);
				}
				else if (shader_parameter->getName() == "mrgLocalLight2_Attribute") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION, 3);
				}
				else if (shader_parameter->getName() == "mrgLocalLight3_Position") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_POSITION, 4);
				}
				else if (shader_parameter->getName() == "mrgLocalLight3_Color") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR, 4);
				}
				else if (shader_parameter->getName() == "mrgLocalLight3_Range") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION, 4);
				}
				else if (shader_parameter->getName() == "mrgLocalLight3_Attribute") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION, 4);
				}
				else if (shader_parameter->getName() == "mrgLocalLight4_Position") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_POSITION, 5);
				}
				else if (shader_parameter->getName() == "mrgLocalLight4_Color") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR, 5);
				}
				else if (shader_parameter->getName() == "mrgLocalLight4_Range") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION, 5);
				}
				else if (shader_parameter->getName() == "mrgLocalLight4_Attribute") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION, 5);
				}
				else if (shader_parameter->getName() == "mrgEyeLight_Diffuse") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 1));
				}
				else if (shader_parameter->getName() == "mrgEyeLight_Specular") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0.1, 0.1, 0.1, 1));
				}
				else if (shader_parameter->getName() == "mrgEyeLight_Range") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 1, 0, 40));
				}
				else if (shader_parameter->getName() == "mrgEyeLight_Attribute") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 1, 1, 2));
				}
				else if (shader_parameter->getName() == "mrgLuminanceRange") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgInShadowScale") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_ShadowMapParams") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgVsmEpsilon") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgColourCompressFactor") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_BackGroundScale") {
					float background_scale = 1.0f;
					if (current_level) {
						background_scale=current_level->getSceneEffect().sky_intensity_scale;
					}

					program_params->setConstant((size_t)index, Ogre::Vector4(background_scale));
				}
				else if (shader_parameter->getName() == "g_GIModeParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_OffsetParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_WaterParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_IceParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_GI0Scale") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 1, 1, 1));
				}
				else if (shader_parameter->getName() == "g_GI1Scale") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 1, 1, 1));
				}
				else if (shader_parameter->getName() == "g_MotionBlur_AlphaRef_VelocityLimit_VelocityCutoff_BlurMagnitude") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgDebugDistortionParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgEdgeEmissionParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_DebugValue") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgGIAtlasParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 1, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgTexcoordIndex") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgTexcoordOffset") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));

					if (uv_animation) {
						editor_application->getAnimationsList()->addTexcoordAnimation(uv_animation, program_params, (size_t)index);
					}
				}
				else if (shader_parameter->getName() == "mrgFresnelParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgMorphWeight") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgZOffsetRate") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_IndexCount") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_TransColorMask") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_ChaosWaveParamEx") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_ChaosWaveParamY") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_ChaosWaveParamXZ") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_ChaosWaveParamXY") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_ChaosWaveParamZX") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_IgnoreLightParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_LightScattering_Ray_Mie_Ray2_Mie2") {
					LibGens::Color lsrm(0.291,0.96,1,1);
					if (current_level) {
						lsrm=current_level->getSceneEffect().light_scattering_ray_mie_ray2_mie2;
					}

					program_params->setConstant((size_t)index, Ogre::Vector4(lsrm.r, lsrm.g, lsrm.r / 16.587812802827, lsrm.g / 12.672096307931));

					// 0.291, 0.96, 0.017543, 0.075757
					// 0.1, 0.01, 0.005952, 0.0007974481
				}
				else if (shader_parameter->getName() == "g_LightScattering_ConstG_FogDensity") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0.0, 0.0, 0.0, 0.0));
				}
				else if (shader_parameter->getName() == "g_LightScatteringFarNearScale") {
					LibGens::Color lsfn(3200,380,1.2,114);
					if (current_level) {
						lsfn=current_level->getSceneEffect().light_scattering_far_near_scale;
					}

					program_params->setConstant((size_t)index, Ogre::Vector4(1.0/lsfn.r, lsfn.g, lsfn.b, lsfn.a));
				}
				else if (shader_parameter->getName() == "g_LightScatteringColor") {
					LibGens::Color lsc(0.11,0.35,0.760001,1);
					if (current_level) {
						lsc=current_level->getSceneEffect().light_scattering_color;
					}
					program_params->setConstant((size_t)index, Ogre::Vector4(lsc.r, lsc.g, lsc.b, 1));
				}
				else if (shader_parameter->getName() == "g_LightScatteringMode") {
					program_params->setConstant((size_t)index, Ogre::Vector4(4, 1, 1, 1));
				}
				else if (shader_parameter->getName() == "g_VerticalLightDirection") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_POSITION, 0);
				}
				else if (shader_parameter->getName() == "g_aLightField") {
					float lightfield_cube[24];
					for (size_t i=0; i<24; i++) {
						lightfield_cube[i] = 0.9;
					}
					program_params->setConstant((size_t)index, lightfield_cube, 6);
				}
				else if (shader_parameter->getName() == "g_TimeParam") {
					//program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
					program_params->setAutoConstantReal((size_t)index, Ogre::GpuProgramParameters::ACT_TIME, 1.0f);
				}
				else if (shader_parameter->getName() == "diffuse") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 1, 1, 1));
				}
				else if (shader_parameter->getName() == "ambient") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 1, 1, 1));
				}
				else if (shader_parameter->getName() == "specular") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 1, 1, 1));
				}
				else if (shader_parameter->getName() == "emissive") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "opacity_reflection_refraction_spectype") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 0, 1, 3));
				}
				else if (shader_parameter->getName() == "power_gloss_level") {
					program_params->setConstant((size_t)index, Ogre::Vector4(50, 0.3, 0.19, 0));
				}
				else if (shader_parameter->getName() == "g_SonicSkinFalloffParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0.15, 2, 3, 0));
				}
				else if (shader_parameter->getName() == "g_SkyParam") {
					float sky_follow_y_ratio = 1.0f;
					if (current_level) {
						sky_follow_y_ratio=current_level->getSceneEffect().sky_follow_up_ratio_y;
					}

					program_params->setConstant((size_t)index, Ogre::Vector4(1, sky_follow_y_ratio, 1, 1));
				}
				else if (shader_parameter->getName() == "g_ViewZAlphaFade") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_ForceAlphaColor") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 1, 1, 1));
				}
				else if (shader_parameter->getName() == "g_ChrPlayableMenuParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 1, 1, 1));
				}
				else {
					SHOW_MSG(("Unhandled constant/variable float4 " + shader_parameter->getName() + " with index " + ToString((int)index) + " on the Shader " + ToString(material->getShader()) + ". Handle it!").c_str());
				}
			}
			else if (slot == 2) {
				if (shader_parameter->getName() == "mrgHasBone") {
					// Unhandled, how do I set a bool
				}
				else if (shader_parameter->getName() == "g_IsShadowMapEnable") {
					// Unhandled, how do I set a bool
				}
				else if (shader_parameter->getName() == "g_IsLightScatteringEnable") {
					// Unhandled, how do I set a bool
				}
				else if (shader_parameter->getName() == "mrgIsEnableHemisphere") {
					// Unhandled, how do I set a bool
				}
				else if (shader_parameter->getName() == "g_IsAlphaDepthBlur") {
					// Unhandled, how do I set a bool
				}
				else if (shader_parameter->getName() == "g_IsGIEnabled") {
					// Unhandled, how do I set a bool
				}
				else if (shader_parameter->getName() == "g_IsSoftParticle") {
					// Unhandled, how do I set a bool
				}
				else {
					SHOW_MSG(("Unhandled constant/variable bool " + shader_parameter->getName() + " with index " + ToString((int)index) + " on the Shader " + ToString(material->getShader()) + ". Handle it!").c_str());
				}
			}
			else if (slot == 3) {
				if ((size_t)index > 15) index = ((size_t) (index) / 17);
				string texture_unit = shader_parameter->getName();

				size_t texture_unit_used_count=0;
				for (size_t i=0; i<texture_units_used.size(); i++) {
					if (texture_units_used[i] == texture_unit) {
						texture_unit_used_count++;
					}
				}

				LibGens::Texture *material_texture = material->getTextureByUnit(texture_unit, texture_unit_used_count);
				if (material_texture) {
					pass->getTextureUnitState((size_t)index)->setTextureName(material_texture->getName()+LIBGENS_TEXTURE_FILE_EXTENSION);
					texture_units_used.push_back(texture_unit);
					continue;
				}


				if (shader_parameter->getName() == "TerrainDiffusemapMask") {
				}
				else if (shader_parameter->getName() == "GI") {
					pass->getTextureUnitState((size_t)index)->setTextureName("white.dds");
				}
				else if (shader_parameter->getName() == "Framebuffer") {
					//pass->getTextureUnitState((size_t)index)->setTextureName("ColorTex");
				}
				else if (shader_parameter->getName() == "Depth") {
					//pass->getTextureUnitState((size_t)index)->setTextureName("DepthTex");
					pass->getTextureUnitState((size_t)index)->setTextureName("white.dds");
				}
				else if (shader_parameter->getName() == "ShadowMap") {
					pass->getTextureUnitState((size_t)index)->setTextureName("white.dds");
				}
				else if (shader_parameter->getName() == "VerticalShadowMap") {
					pass->getTextureUnitState((size_t)index)->setTextureName("white.dds");
				}
				else if (shader_parameter->getName() == "ShadowMapJitter") {
				}
				else if (shader_parameter->getName() == "ReflectionMap") {
				}
				else if (shader_parameter->getName() == "ReflectionMap2") {
				}
				else if (shader_parameter->getName() == "INDEXEDLIGHTMAP") {
				}
				else if (shader_parameter->getName() == "PamNpcEye") {
				}
				else if (shader_parameter->getName() == "diffuse") {
				}
				else if (shader_parameter->getName() == "specular") {
				}
				else if (shader_parameter->getName() == "reflection") {
				}
				else if (shader_parameter->getName() == "normal") {
				}
				else if (shader_parameter->getName() == "displacement") {
				}
				else if (shader_parameter->getName() == "gloss") {
				}
				else if (shader_parameter->getName() == "opacity") {
				}
				else {
					SHOW_MSG(("Unhandled constant/variable sampler " + shader_parameter->getName() + " with index " + ToString((int)index) + " on the Shader " + ToString(material->getShader()) + ". Handle it!").c_str());
				}
			}
			else {
				SHOW_MSG(("Unhandled slot " + ToString(slot) + ". Handle it!").c_str());
			}
		}
	}
}

void fixPass(Ogre::Pass *pass, LibGens::Material *material, size_t mesh_slot) {
	if (mesh_slot == LIBGENS_MODEL_SUBMESH_SLOT_TRANSPARENT) {
		pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
		pass->setDepthWriteEnabled(false);
	}

	if (mesh_slot == LIBGENS_MODEL_SUBMESH_SLOT_WATER) {
		pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
		//pass->setDepthWriteEnabled(false);
	}

	if (mesh_slot == LIBGENS_MODEL_SUBMESH_SLOT_BOOLEAN) {
		pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
		pass->setAlphaRejectSettings(Ogre::CMPF_GREATER_EQUAL, (unsigned char)192);
	}

	if (material->hasColorBlend()) {
		pass->setSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE);
		pass->setDepthWriteEnabled(false);
	}

	if (material->hasNoCulling()) pass->setCullingMode(Ogre::CULL_NONE);
}

void updateMaterialShaderParameters(Ogre::Material *ogre_material, LibGens::Material *material, bool no_gi, LibGens::UVAnimation *uv_animation) {
	Ogre::Pass *pass=ogre_material->getTechnique(0)->getPass(0);
	if (!pass) return;

	// Search for shaders based on the material's shader
	string shader_name = material->getShader();
	LibGens::Shader *vertex_shader=NULL;
	LibGens::Shader *pixel_shader=NULL;
	SONICGLVL_SHADER_LIBRARY->getMaterialShaders(shader_name, vertex_shader, pixel_shader, false, no_gi, /*(uv_animation ? false : true)*/ false);

	if (vertex_shader && pixel_shader) {
		// Vertex Shader
		string vertex_shader_name = vertex_shader->getShaderFilename();
		if (!vertex_shader->hasExtra()) {
			Ogre::GpuProgramPtr vertex = Ogre::GpuProgramManager::getSingletonPtr()->createProgram(vertex_shader_name, GENERAL_MESH_GROUP, vertex_shader_name+".wvu.asm", Ogre::GPT_VERTEX_PROGRAM, "vs_3_0");
			vertex_shader->setExtra(vertex_shader_name);
		}
		pass->setVertexProgram(vertex_shader_name);

		
		if (pass->hasVertexProgram()) {
			Ogre::GpuProgramParametersSharedPtr vp_parameters = pass->getVertexProgramParameters();
			vector<string> shader_parameter_filenames=vertex_shader->getShaderParameterFilenames();

			vp_parameters->setTransposeMatrices(true);

			for (size_t i=0; i<shader_parameter_filenames.size(); i++) {
				LibGens::ShaderParams *shader_params=SONICGLVL_SHADER_LIBRARY->getVertexShaderParams(shader_parameter_filenames[i]);

				if (shader_params) {
					setShaderParameters(pass, vp_parameters, material, shader_params, uv_animation);
				}
			}
		}

		// Pixel Shader
		string pixel_shader_name = pixel_shader->getShaderFilename();
		if (!pixel_shader->hasExtra()) {
			Ogre::GpuProgramPtr shader = Ogre::GpuProgramManager::getSingletonPtr()->createProgram(pixel_shader_name, GENERAL_MESH_GROUP, pixel_shader_name+".wpu.asm", Ogre::GPT_FRAGMENT_PROGRAM, "ps_3_0");
			pixel_shader->setExtra(pixel_shader_name);
		}
		pass->setFragmentProgram(pixel_shader_name);

		if (pass->hasFragmentProgram()) {
			Ogre::GpuProgramParametersSharedPtr fp_parameters = pass->getFragmentProgramParameters();
			vector<string> shader_parameter_filenames=pixel_shader->getShaderParameterFilenames();

			fp_parameters->setTransposeMatrices(true);

			for (size_t i=0; i<shader_parameter_filenames.size(); i++) {
				LibGens::ShaderParams *shader_params=SONICGLVL_SHADER_LIBRARY->getPixelShaderParams(shader_parameter_filenames[i]);

				if (shader_params) {
					setShaderParameters(pass, fp_parameters, material, shader_params, uv_animation);
				}
			}
		}
	}
	else {
		pass->setVertexProgram("SysDummyVS");
		pass->setFragmentProgram("SysDummyPS");
	}
}


void buildMaterial(LibGens::Material *material, string material_name, string resource_group, size_t mesh_slot, bool no_gi) {
	LibGens::Texture *texture=material->getTextureByUnit(LIBGENS_MATERIAL_TEXTURE_UNIT_DIFFUSE);

	Ogre::Material *compilematerial = Ogre::MaterialManager::getSingleton().create(material_name, resource_group).getPointer();
	if (!compilematerial) {
		return;
	}

	Ogre::Pass *pass=compilematerial->getTechnique(0)->getPass(0);

	fixPass(pass, material, mesh_slot);

	// Force it to use 16 total texture units and full ambient lighting
	for (size_t i=0; i<16; i++) {
		Ogre::TextureUnitState *texture_unit_state=pass->createTextureUnitState("black.dds");
	}
	pass->setAmbient(1.0, 1.0, 1.0);

	// Search for UV Animations for the first texset
	LibGens::UVAnimation *uv_animation = NULL;
	if (texture) {
		uv_animation = SONICGLVL_UV_ANIMATION_LIBRARY->getUVAnimation(material->getName(), texture->getTexset());
	}

	// Search for shaders based on the material's shader
	updateMaterialShaderParameters(compilematerial, material, no_gi, uv_animation);

	// Create low-end technique
	Ogre::Technique *low_end_technique=compilematerial->createTechnique();
	low_end_technique->setSchemeName(SONICGLVL_LOW_END_TECHNIQUE);
	pass=low_end_technique->createPass();

	fixPass(pass, material, mesh_slot);

	if (texture) {
		Ogre::TextureUnitState *state=pass->createTextureUnitState(texture->getName()+LIBGENS_TEXTURE_FILE_EXTENSION);
		state->setTextureAddressingMode(Ogre::TextureUnitState::TAM_WRAP);
	}
}


void buildMesh(Ogre::SceneNode *scene_node, LibGens::Mesh *mesh, Ogre::SceneManager *scene_manager, LibGens::MaterialLibrary *material_library, string root_name, Ogre::uint32 query_flags, string resource_group, bool global_illumination, string skeleton_name, Ogre::Entity *&shared_entity, vector<LibGens::Bone *> model_bones) {
	vector<LibGens::Submesh *> *submeshes=mesh->getSubmeshSlots();
	unsigned int i=0;
	bool create_resource = true;

	if (mesh->hasExtra()) {
		create_resource = false;
	}

	for (size_t mesh_slot=0; mesh_slot<LIBGENS_MODEL_SUBMESH_SLOTS; mesh_slot++) {
		string ent_name=root_name + "_" + ToString(mesh_slot);

		if (create_resource) {
			// Create Ogre Mesh
			Ogre::MeshPtr msh = Ogre::MeshManager::getSingleton().createManual(ent_name, resource_group);
			
			if (skeleton_name.size()) {
				msh->setSkeletonName(skeleton_name);
			}

			LibGens::AABB mesh_aabb;
			mesh_aabb.reset();

			for (size_t submesh_slot=0; submesh_slot<submeshes[mesh_slot].size(); submesh_slot++) {
				LibGens::Submesh *submesh=submeshes[mesh_slot][submesh_slot];
				
				// Get LibGens Data
				vector<LibGens::Vertex *> submesh_vertices = submesh->getVertices();
				vector<LibGens::Vector3> submesh_faces = submesh->getFaces();

				// Create Ogre Submesh
				Ogre::SubMesh* sub = msh->createSubMesh();
				const size_t nVertices = submesh_vertices.size();
				const size_t nVertCount = 24;
				const size_t vbufCount = nVertCount*nVertices;
				float *vertices = (float *) malloc(sizeof(float)*vbufCount);

				for (size_t i=0; i<submesh_vertices.size(); i++) {
					vertices[i*nVertCount]   = submesh_vertices[i]->getPosition().x;
					vertices[i*nVertCount+1] = submesh_vertices[i]->getPosition().y;
					vertices[i*nVertCount+2] = submesh_vertices[i]->getPosition().z;

					mesh_aabb.addPoint(submesh_vertices[i]->getPosition());

					vertices[i*nVertCount+3] = submesh_vertices[i]->getNormal().x;
					vertices[i*nVertCount+4] = submesh_vertices[i]->getNormal().y;
					vertices[i*nVertCount+5] = submesh_vertices[i]->getNormal().z;

					vertices[i*nVertCount+6] = submesh_vertices[i]->getTangent().x;
					vertices[i*nVertCount+7] = submesh_vertices[i]->getTangent().y;
					vertices[i*nVertCount+8] = submesh_vertices[i]->getTangent().z;

					vertices[i*nVertCount+9]  = submesh_vertices[i]->getBinormal().x;
					vertices[i*nVertCount+10] = submesh_vertices[i]->getBinormal().y;
					vertices[i*nVertCount+11] = submesh_vertices[i]->getBinormal().z;

					vertices[i*nVertCount+12] = submesh_vertices[i]->getUV(0).x;
					vertices[i*nVertCount+13] = submesh_vertices[i]->getUV(0).y;

					vertices[i*nVertCount+14] = submesh_vertices[i]->getUV(1).x;
					vertices[i*nVertCount+15] = submesh_vertices[i]->getUV(1).y;

					vertices[i*nVertCount+16] = submesh_vertices[i]->getUV(2).x;
					vertices[i*nVertCount+17] = submesh_vertices[i]->getUV(2).y;

					vertices[i*nVertCount+18] = submesh_vertices[i]->getUV(3).x;
					vertices[i*nVertCount+19] = submesh_vertices[i]->getUV(3).y;

					vertices[i*nVertCount+20] = submesh_vertices[i]->getColor().r;
					vertices[i*nVertCount+21] = submesh_vertices[i]->getColor().g;
					vertices[i*nVertCount+22] = submesh_vertices[i]->getColor().b;
					vertices[i*nVertCount+23] = submesh_vertices[i]->getColor().a;
				}
 
				Ogre::RenderSystem* rs = Ogre::Root::getSingleton().getRenderSystem();
				const size_t ibufCount = submesh_faces.size()*3;
				unsigned short *faces = (unsigned short *) malloc(sizeof(unsigned short) * ibufCount);

				for (size_t i=0; i<submesh_faces.size(); i++) {
					faces[i*3]   = submesh_faces[i].x;
					faces[i*3+1] = submesh_faces[i].y;
					faces[i*3+2] = submesh_faces[i].z;
				}

				sub->vertexData = new Ogre::VertexData();
				sub->vertexData->vertexCount = nVertices;

				Ogre::VertexDeclaration* decl = sub->vertexData->vertexDeclaration;
				size_t offset = 0;

				decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
				offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

				decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);
				offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

				decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_TANGENT);
				offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

				decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_BINORMAL);
				offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

				decl->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);
				offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT2);

				decl->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES, 1);
				offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT2);

				decl->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES, 2);
				offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT2);

				decl->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES, 3);
				offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT2);

				decl->addElement(0, offset, Ogre::VET_FLOAT4, Ogre::VES_DIFFUSE);
				offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT4);

				Ogre::HardwareVertexBufferSharedPtr vbuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(offset, sub->vertexData->vertexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
				vbuf->writeData(0, vbuf->getSizeInBytes(), vertices, true);
				Ogre::VertexBufferBinding* bind = sub->vertexData->vertexBufferBinding; 
				bind->setBinding(0, vbuf);
				Ogre::HardwareIndexBufferSharedPtr ibuf = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(Ogre::HardwareIndexBuffer::IT_16BIT, ibufCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
				ibuf->writeData(0, ibuf->getSizeInBytes(), faces, true);
				sub->useSharedVertices = false;
				sub->indexData->indexBuffer = ibuf;
				sub->indexData->indexCount = ibufCount;
				sub->indexData->indexStart = 0;


				// Assign Material to Submesh
				string mat_name=submesh->getMaterialName();
				LibGens::Material *mat=material_library->getMaterial(mat_name);

				if (mat) {
					if (global_illumination) {
						mat_name += "_GI";

						if (!mat->hasExtraGI()) {
							buildMaterial(mat, mat_name, resource_group, mesh_slot, false);
							mat->setExtraGI(mat_name);
						}
					}
					else {
						if (!mat->hasExtra()) {
							buildMaterial(mat, mat_name, resource_group, mesh_slot, true);
							mat->setExtra(mat_name);
						}
					}
			
					sub->setMaterialName(mat_name);
				}

				
				// Create Bone Assignments if Skeleton name exists
				if (skeleton_name.size()) {
					Ogre::SkeletonPtr ogre_skeleton = Ogre::SkeletonManager::getSingleton().getByName(skeleton_name, resource_group);
					if (!ogre_skeleton.isNull()) {
						vector<unsigned char> bone_table = submesh->getBoneTable();

						for (size_t i=0; i<bone_table.size(); i++) {
							string bone_name=model_bones[bone_table[i]]->getName();

							if (ogre_skeleton->hasBone(bone_name)) {
								Ogre::Bone *mBone=ogre_skeleton->getBone(bone_name);
								bone_table[i] = mBone->getHandle();
							}
							else {
								LOG_MSG(("Couldn't find " + bone_name + " on the Ogre skeleton. Index is " + ToString(bone_table[i]) + " and there's only " + ToString(ogre_skeleton->getNumBones()) + " bones.").c_str());
							}
						}
					
						for (size_t i = 0; i < nVertices; i++) {
							Ogre::VertexBoneAssignment vba;
							vba.vertexIndex = static_cast<unsigned int>(i);
							LibGens::Vertex *vertex=submesh_vertices[i];

							for (size_t j = 0; j < 4; j++) {
								unsigned char bone_index  = vertex->getBoneIndex(j);
								unsigned char bone_weight = vertex->getBoneWeight(j);

								if ((bone_index == 0xFF) && j) {
									break;
								}
								else {
									vba.boneIndex = 0;
									vba.weight = 1.0f;

									if (bone_index != 0xFF) {
										vba.boneIndex = bone_table[bone_index];
										vba.weight = (((float) bone_weight) / 255.0f);
									}

									sub->addBoneAssignment(vba);
								}
							}
						}

						// Apply changes, build the buffer
						sub->_compileBoneAssignments();
						sub->vertexData->reorganiseBuffers(decl->getAutoOrganisedDeclaration(true, false, false));
					}
				}

				// Cleanup
				free(faces);
				free(vertices);
			}

			msh->_setBounds(Ogre::AxisAlignedBox(mesh_aabb.start.x, mesh_aabb.start.y, mesh_aabb.start.z, mesh_aabb.end.x, mesh_aabb.end.y, mesh_aabb.end.z));
			msh->_setBoundingSphereRadius(mesh_aabb.sizeMax()/2);
			msh->load();
		}
		
		Ogre::Entity* entity = scene_manager->createEntity(ent_name);
		entity->setQueryFlags(query_flags);

		if (!shared_entity) {
			shared_entity = entity;
		}
		else {
			if (entity->hasSkeleton() && shared_entity->hasSkeleton()) {
				entity->shareSkeletonInstanceWith(shared_entity);
			}
		}

		if (mesh_slot == LIBGENS_MODEL_SUBMESH_SLOT_WATER) {
			entity->setRenderQueueGroup(Ogre::RENDER_QUEUE_WORLD_GEOMETRY_2);
		}
		else if (mesh_slot == LIBGENS_MODEL_SUBMESH_SLOT_TRANSPARENT) {
			entity->setRenderQueueGroup(Ogre::RENDER_QUEUE_MAX);
		}
		else {
			entity->setRenderQueueGroup(Ogre::RENDER_QUEUE_WORLD_GEOMETRY_1);
		}

		if (scene_node) {
			scene_node->attachObject(entity);
		}
	}

	if (create_resource) {
		mesh->setExtra(root_name);
	}
}


void cleanMeshResource(LibGens::Mesh *mesh, string resource_group) {
	if (!mesh) return;

	vector<LibGens::Submesh *> *submeshes=mesh->getSubmeshSlots();

	if (mesh->hasExtra()) {
		string root_name = mesh->getExtra();

		for (size_t mesh_slot=0; mesh_slot<LIBGENS_MODEL_SUBMESH_SLOTS; mesh_slot++) {
			string ent_name=root_name + "_" + ToString(mesh_slot);

			if (Ogre::MeshManager::getSingleton().resourceExists(ent_name)) {
				Ogre::MeshManager::getSingleton().remove(ent_name);
			}
		}

		mesh->setExtra("");
	}
}

void cleanModelResource(LibGens::Model *model, string resource_group) {
	if (!model) return;

	unsigned int i=0;
	vector<LibGens::Mesh *> meshes=model->getMeshes();
	for (vector<LibGens::Mesh *>::iterator it=meshes.begin(); it!=meshes.end(); it++) {
		cleanMeshResource((*it), resource_group);
		i++;
	}
}


void buildModel(Ogre::SceneNode *scene_node, LibGens::Model *model, string model_name, string skeleton_name, Ogre::SceneManager *scene_manager, LibGens::MaterialLibrary *material_library, Ogre::uint32 query_flags, string resource_group, bool global_illumination) {
	if (!model) return;
	if (!scene_manager) return;
	if (!material_library) return;
	
	Ogre::Entity *shared_entity=NULL;
	vector<LibGens::Mesh *> meshes=model->getMeshes();
	unsigned int i=0;
	for (vector<LibGens::Mesh *>::iterator it=meshes.begin(); it!=meshes.end(); it++) {
		buildMesh(scene_node, (*it), scene_manager, material_library, model_name + "_" + ToString(i), query_flags, resource_group, global_illumination, skeleton_name, shared_entity, model->getBones());
		i++;
	}
}