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

#include "AR.h"
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

void setShaderParameters(Ogre::Pass *pass, Ogre::GpuProgramParametersSharedPtr program_params, LibGens::Material *material, LibGens::ShaderParams *shader_params, LibGens::UVAnimation *uv_animation, LibGens::ArFile* shader_code) {
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

			string shader_parameter_name = shader_parameter->getName();
			unsigned char index = shader_parameter->getIndex();
			unsigned char size = shader_parameter->getSize();

			if (slot == 0) {
				LibGens::Parameter *material_parameter = material->getParameterByName(shader_parameter_name);

				if (material_parameter) {
					LibGens::Color color = material_parameter->getColor();
					
					if (shader_parameter_name == "diffuse")  color.a = 1.0;
					if (shader_parameter_name == "specular") color.a = 1.0;
					if (shader_parameter_name == "ambient")  color.a = 1.0;

					program_params->setConstant((size_t)index, Ogre::Vector4(color.r, color.g, color.b, color.a));
					continue;
				}

				const char* haystack_begin = reinterpret_cast<const char*>(shader_code->getData());
				const char* haystack_end = haystack_begin + shader_code->getSize();

				const char* needle_begin = shader_parameter_name.data();
				const char* needle_end = needle_begin + shader_parameter_name.size();

				if (search(haystack_begin, haystack_end, needle_begin, needle_end) == haystack_end) {
					continue;
				}

				if (shader_parameter_name == "g_MtxProjection") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_PROJECTION_MATRIX);
				}
				else if (shader_parameter_name == "g_MtxInvProjection") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_INVERSE_PROJECTION_MATRIX);
				}
				else if (shader_parameter_name == "g_MtxView") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_VIEW_MATRIX);
				}
				else if (shader_parameter_name == "g_MtxInvView") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_INVERSE_VIEW_MATRIX);
				}
				else if (shader_parameter_name == "g_MtxWorld") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_WORLD_MATRIX);
				}
				else if (shader_parameter_name == "g_MtxWorldIT") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLD_MATRIX);
				}
				else if (shader_parameter_name == "g_MtxPrevView") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_VIEW_MATRIX);
				}
				else if (shader_parameter_name == "g_MtxPrevWorld") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_WORLD_MATRIX);
				}
				else if (shader_parameter_name == "g_MtxLightViewProjection") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_TEXTURE_VIEWPROJ_MATRIX);
				}
				else if (shader_parameter_name == "g_MtxVerticalLightViewProjection") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_TEXTURE_VIEWPROJ_MATRIX);
				}
				else if (shader_parameter_name == "g_MtxBillboardY") {
					// Incomplete
				}
				else if (shader_parameter_name == "g_MtxPalette") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_WORLD_MATRIX_ARRAY_3x4);
				}
				else if (shader_parameter_name == "g_MtxPrevPalette") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_WORLD_MATRIX_ARRAY_3x4);
				}
				else if (shader_parameter_name == "g_EyePosition") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_CAMERA_POSITION);
				}
				else if (shader_parameter_name == "g_EyeDirection") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_VIEW_DIRECTION);
				}
				else if (shader_parameter_name == "g_ViewportSize") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_VIEWPORT_SIZE);
				}
				else if (shader_parameter_name == "g_CameraNearFarAspect") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "mrgAmbientColor") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "mrgGroundColor") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "mrgSkyColor") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "mrgPowerGlossLevel") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "mrgEmissionPower") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "mrgGlobalLight_Direction") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_POSITION, 0);
				}
				else if (shader_parameter_name == "mrgGlobalLight_Direction_View") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_POSITION_VIEW_SPACE, 0);
				}
				else if (shader_parameter_name == "mrgGlobalLight_Diffuse") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR, 0);
				}
				else if (shader_parameter_name == "mrgGlobalLight_Specular") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_SPECULAR_COLOUR, 0);
				}
				else if (shader_parameter_name == "mrgLocallightIndexArray") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "mrgLocalLight0_Position") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_POSITION, 1);
				}
				else if (shader_parameter_name == "mrgLocalLight0_Color") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR, 1);
				}
				else if (shader_parameter_name == "mrgLocalLight0_Range") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION, 1);
				}
				else if (shader_parameter_name == "mrgLocalLight0_Attribute") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION, 1);
				}
				else if (shader_parameter_name == "mrgLocalLight1_Position") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_POSITION, 2);
				}
				else if (shader_parameter_name == "mrgLocalLight1_Color") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR, 2);
				}
				else if (shader_parameter_name == "mrgLocalLight1_Range") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION, 2);
				}
				else if (shader_parameter_name == "mrgLocalLight1_Attribute") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION, 2);
				}
				else if (shader_parameter_name == "mrgLocalLight2_Position") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_POSITION, 3);
				}
				else if (shader_parameter_name == "mrgLocalLight2_Color") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR, 3);
				}
				else if (shader_parameter_name == "mrgLocalLight2_Range") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION, 3);
				}
				else if (shader_parameter_name == "mrgLocalLight2_Attribute") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION, 3);
				}
				else if (shader_parameter_name == "mrgLocalLight3_Position") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_POSITION, 4);
				}
				else if (shader_parameter_name == "mrgLocalLight3_Color") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR, 4);
				}
				else if (shader_parameter_name == "mrgLocalLight3_Range") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION, 4);
				}
				else if (shader_parameter_name == "mrgLocalLight3_Attribute") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION, 4);
				}
				else if (shader_parameter_name == "mrgLocalLight4_Position") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_POSITION, 5);
				}
				else if (shader_parameter_name == "mrgLocalLight4_Color") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR, 5);
				}
				else if (shader_parameter_name == "mrgLocalLight4_Range") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION, 5);
				}
				else if (shader_parameter_name == "mrgLocalLight4_Attribute") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION, 5);
				}
				else if (shader_parameter_name == "mrgEyeLight_Diffuse") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 1));
				}
				else if (shader_parameter_name == "mrgEyeLight_Specular") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0.1, 0.1, 0.1, 1));
				}
				else if (shader_parameter_name == "mrgEyeLight_Range") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 1, 0, 40));
				}
				else if (shader_parameter_name == "mrgEyeLight_Attribute") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 1, 1, 2));
				}
				else if (shader_parameter_name == "mrgLuminanceRange") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "mrgInShadowScale") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "g_ShadowMapParams") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "mrgVsmEpsilon") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "mrgColourCompressFactor") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "g_BackGroundScale") {
					float background_scale = 1.0f;
					if (current_level) {
						background_scale=current_level->getSceneEffect().sky_intensity_scale;
					}

					program_params->setConstant((size_t)index, Ogre::Vector4(background_scale));
				}
				else if (shader_parameter_name == "g_GIModeParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "g_OffsetParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "g_WaterParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "g_IceParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "g_GI0Scale") {
					if (editor_application->getCurrentLevel() != NULL && editor_application->getCurrentLevel()->getGameMode() == LIBGENS_LEVEL_GAME_UNLEASHED) {
						program_params->setConstant((size_t)index, Ogre::Vector4(1, 1, 1, 0));
					}
					else {
						program_params->setConstant((size_t)index, Ogre::Vector4(1, 1, 1, 1));
					}
				}
				else if (shader_parameter_name == "g_GI1Scale") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 1, 1, 1));
				}
				else if (shader_parameter_name == "g_MotionBlur_AlphaRef_VelocityLimit_VelocityCutoff_BlurMagnitude") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "mrgDebugDistortionParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "mrgEdgeEmissionParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "g_DebugValue") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "mrgGIAtlasParam") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_CUSTOM);
				}
				else if (shader_parameter_name == "mrgTexcoordIndex") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "mrgTexcoordOffset") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));

					if (uv_animation) {
						editor_application->getAnimationsList()->addTexcoordAnimation(uv_animation, program_params, (size_t)index);
					}
				}
				else if (shader_parameter_name == "mrgFresnelParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "mrgMorphWeight") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "mrgZOffsetRate") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "g_IndexCount") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "g_TransColorMask") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "g_ChaosWaveParamEx") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "g_ChaosWaveParamY") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "g_ChaosWaveParamXZ") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "g_ChaosWaveParamXY") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "g_ChaosWaveParamZX") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "g_IgnoreLightParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "g_LightScattering_Ray_Mie_Ray2_Mie2") {
					LibGens::Color lsrm(0.291f,0.96f, 0.017543f, 0.075757f);
					if (current_level) {
						LibGens::SceneEffect& scene_effect = current_level->getSceneEffect();

						lsrm.r = scene_effect.light_scattering_rayleigh;
						lsrm.g = scene_effect.light_scattering_mie;
						lsrm.b = scene_effect.light_scattering_rayleigh * 3.0f / (LIBGENS_MATH_PI * 16.0f);
						lsrm.a = scene_effect.light_scattering_mie / (LIBGENS_MATH_PI * 4.0f);
					}

					program_params->setConstant((size_t)index, Ogre::Vector4(lsrm.r, lsrm.g, lsrm.b, lsrm.a));

					// 0.291, 0.96, 0.017543, 0.075757
					// 0.1, 0.01, 0.005952, 0.0007974481
				}
				else if (shader_parameter_name == "g_LightScattering_ConstG_FogDensity") {
					LibGens::Color lsgf(0.0f, 0.0f, 0.0f, 0.0f);
					if (current_level) {
						LibGens::SceneEffect& scene_effect = current_level->getSceneEffect();

						lsgf.r = (1.0f - scene_effect.light_scattering_g) * (1.0f - scene_effect.light_scattering_g);
						lsgf.g = scene_effect.light_scattering_g * scene_effect.light_scattering_g + 1.0f;
						lsgf.b = scene_effect.light_scattering_g * -2.0f;
					}
					program_params->setConstant((size_t)index, Ogre::Vector4(lsgf.r, lsgf.g, lsgf.b, lsgf.a));
				}
				else if (shader_parameter_name == "g_LightScatteringFarNearScale") {
					LibGens::Color lsfn(3200.0f,380.0f,1.2f,114.0f);
					if (current_level) {
						LibGens::SceneEffect& scene_effect = current_level->getSceneEffect();

						lsfn.r = scene_effect.light_scattering_z_far;
						lsfn.g = scene_effect.light_scattering_z_near;
						lsfn.b = scene_effect.light_scattering_depth_scale;
						lsfn.a = scene_effect.light_scattering_in_scattering_scale;
					}
					
					if (editor_application->getCurrentLevel() != NULL && editor_application->getCurrentLevel()->getGameMode() != LIBGENS_LEVEL_GAME_UNLEASHED) {
						lsfn.r = 1.0f / (lsfn.r - lsfn.g);
					}

					program_params->setConstant((size_t)index, Ogre::Vector4(lsfn.r, lsfn.g, lsfn.b, lsfn.a));
				}
				else if (shader_parameter_name == "g_LightScatteringColor") {
					LibGens::Color lsc(0.11,0.35,0.760001,1);
					if (current_level) {
						lsc=current_level->getSceneEffect().light_scattering_color;
					}
					program_params->setConstant((size_t)index, Ogre::Vector4(lsc.r, lsc.g, lsc.b, 1));
				}
				else if (shader_parameter_name == "g_LightScatteringMode") {
					program_params->setConstant((size_t)index, Ogre::Vector4(4, 1, 1, 1));
				}
				else if (shader_parameter_name == "g_VerticalLightDirection") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_POSITION, 0);
				}
				else if (shader_parameter_name == "g_aLightField") {
					float lightfield_cube[24];
					for (size_t i=0; i<24; i++) {
						lightfield_cube[i] = 0.5f;
					}
					lightfield_cube[3] = 1.0f;

					program_params->setConstant((size_t)index, lightfield_cube, 6);
				}
				else if (shader_parameter_name == "g_TimeParam") {
					//program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
					program_params->setAutoConstantReal((size_t)index, Ogre::GpuProgramParameters::ACT_TIME, 1.0f);
				}
				else if (shader_parameter_name == "diffuse") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 1, 1, 1));
				}
				else if (shader_parameter_name == "ambient") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 1, 1, 1));
				}
				else if (shader_parameter_name == "specular") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 1, 1, 1));
				}
				else if (shader_parameter_name == "emissive") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "opacity_reflection_refraction_spectype") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 0, 1, 3));
				}
				else if (shader_parameter_name == "power_gloss_level") {
					program_params->setConstant((size_t)index, Ogre::Vector4(50, 0.3, 0.19, 0));
				}
				else if (shader_parameter_name == "g_SonicSkinFalloffParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0.15, 2, 3, 0));
				}
				else if (shader_parameter_name == "g_SkyParam") {
					float sky_follow_y_ratio = 1.0f;
					if (current_level) {
						sky_follow_y_ratio=current_level->getSceneEffect().sky_follow_up_ratio_y;
					}

					program_params->setConstant((size_t)index, Ogre::Vector4(1, sky_follow_y_ratio, 1, 1));
				}
				else if (shader_parameter_name == "g_ViewZAlphaFade") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter_name == "g_ForceAlphaColor") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 1, 1, 1));
				}
				else if (shader_parameter_name == "g_ChrPlayableMenuParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 1, 1, 1));
				}	
				else if (shader_parameter_name == "mrgPlayableParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(-1, 1, 1, 1));
				}
				else {
					printf(("Unhandled constant/variable float4 " + shader_parameter_name + " with index " + ToString((int)index) + " on the Shader " + ToString(material->getShader()) + ". Handle it!\n").c_str());
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
			}
			else if (slot == 2) {
				if (shader_parameter_name == "mrgHasBone") {
					// Unhandled, how do I set a bool
				}
				else if (shader_parameter_name == "g_IsShadowMapEnable") {
					// Unhandled, how do I set a bool
				}
				else if (shader_parameter_name == "g_IsLightScatteringEnable") {
					// Unhandled, how do I set a bool
				}
				else if (shader_parameter_name == "mrgIsEnableHemisphere") {
					// Unhandled, how do I set a bool
				}
				else if (shader_parameter_name == "g_IsAlphaDepthBlur") {
					// Unhandled, how do I set a bool
				}
				else if (shader_parameter_name == "g_IsGIEnabled") {
					// Unhandled, how do I set a bool
				}
				else if (shader_parameter_name == "g_IsSoftParticle") {
					// Unhandled, how do I set a bool
				}
				else {
					ERROR_MSG(("Unhandled constant/variable bool " + shader_parameter_name + " with index " + ToString((int)index) + " on the Shader " + ToString(material->getShader()) + ". Handle it!").c_str());
				}
			}
			else if (slot == 3) {
				index &= 0xF;
				string texture_unit = shader_parameter_name;

				size_t texture_unit_used_count=0;
				for (size_t i=0; i<texture_units_used.size(); i++) {
					if (texture_units_used[i] == texture_unit) {
						texture_unit_used_count++;
					}
				}

				// get rid of old textures to refresh units
				pass->getTextureUnitState((size_t)index)->setTextureName("");
				LibGens::Texture *material_texture = material->getTextureByUnit(texture_unit, texture_unit_used_count);
				if (material_texture) {
					pass->getTextureUnitState((size_t)index)->setTextureFiltering(Ogre::TextureFilterOptions::TFO_TRILINEAR);
					pass->getTextureUnitState((size_t)index)->setTextureName(material_texture->getName()+LIBGENS_TEXTURE_FILE_EXTENSION);
					texture_units_used.push_back(texture_unit);
					continue;
				}


				if (shader_parameter_name == "TerrainDiffusemapMask") {
				}
				else if (shader_parameter_name == "GI") {
					pass->getTextureUnitState((size_t)index)->setTextureName("white.dds");
				}
				else if (shader_parameter_name == "Framebuffer") {
					//pass->getTextureUnitState((size_t)index)->setTextureName("ColorTex");
				}
				else if (shader_parameter_name == "Depth") {
					//pass->getTextureUnitState((size_t)index)->setTextureName("DepthTex");
					pass->getTextureUnitState((size_t)index)->setTextureName("white.dds");
				}
				else if (shader_parameter_name == "ShadowMap") {
					pass->getTextureUnitState((size_t)index)->setTextureName("white.dds");
				}
				else if (shader_parameter_name == "VerticalShadowMap") {
					pass->getTextureUnitState((size_t)index)->setTextureName("white.dds");
				}
				else if (shader_parameter_name == "ShadowMapJitter") {
				}
				else if (shader_parameter_name == "ReflectionMap") {
				}
				else if (shader_parameter_name == "ReflectionMap2") {
				}
				else if (shader_parameter_name == "INDEXEDLIGHTMAP") {
				}
				else if (shader_parameter_name == "PamNpcEye") {
				}
				else if (shader_parameter_name == "diffuse") {
				}
				else if (shader_parameter_name == "specular") {
				}
				else if (shader_parameter_name == "reflection") {
				}
				else if (shader_parameter_name == "normal") {
				}
				else if (shader_parameter_name == "displacement") {
				}
				else if (shader_parameter_name == "gloss") {
				}
				else if (shader_parameter_name == "opacity") {
				}
				else {
					ERROR_MSG(("Unhandled constant/variable sampler " + shader_parameter_name + " with index " + ToString((int)index) + " on the Shader " + ToString(material->getShader()) + ". Handle it!").c_str());
				}
			}
			else {
				ERROR_MSG(("Unhandled slot " + ToString(slot) + ". Handle it!").c_str());
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
		pass->setDepthWriteEnabled(true);
	}

	if (mesh_slot == LIBGENS_MODEL_SUBMESH_SLOT_BOOLEAN) {
		pass->setAlphaRejectSettings(Ogre::CMPF_GREATER_EQUAL, material->getAlphaThreshold());
	}

	if (material->hasColorBlend()) {
		pass->setSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE);
		pass->setDepthWriteEnabled(false);
	}

	if (material->hasNoCulling()) pass->setCullingMode(Ogre::CULL_NONE);
}

namespace Ogre {
	class D3D9GpuProgram {
	public:
		void setExternalMicrocode(const void* pMicrocode, size_t size);
	};
}

void updateMaterialShaderParameters(Ogre::Material *ogre_material, LibGens::Material *material, bool no_gi, LibGens::UVAnimation *uv_animation, LibGens::ShaderLibrary *shader_library) {
	Ogre::Pass *pass=ogre_material->getTechnique(0)->getPass(0);
	if (!pass) return;

	if (!shader_library) {
		LibGens::Texture* texture = material->getTextureByIndex(0);
		if (texture) {
			pass->getTextureUnitState(0)->setTextureName(texture->getName() + LIBGENS_TEXTURE_FILE_EXTENSION);
		}
		return;
	}

	// Search for shaders based on the material's shader
	string shader_name = material->getShader();
	LibGens::Shader *vertex_shader=NULL;
	LibGens::Shader *pixel_shader=NULL;
	shader_library->getMaterialShaders(shader_name, vertex_shader, pixel_shader, false, no_gi, /*(uv_animation ? false : true)*/ false);

	if (vertex_shader && pixel_shader) {
		// Vertex Shader
		string vertex_shader_name = vertex_shader->getShaderFilename();
		string vertex_shader_key = ToString((size_t)shader_library) + "_vertex_" + vertex_shader_name;
		LibGens::ArFile* vertex_shader_code_file = shader_library->getFile(vertex_shader_name + ".wvu");
		if (!vertex_shader->hasExtra()) {
			Ogre::ResourcePtr vertex = Ogre::GpuProgramManager::getSingletonPtr()->createProgramFromString(vertex_shader_key, GENERAL_MESH_GROUP, "", Ogre::GPT_VERTEX_PROGRAM, "vs_3_0");
			((Ogre::D3D9GpuProgram*)(vertex.get()))->setExternalMicrocode(vertex_shader_code_file->getData(), vertex_shader_code_file->getSize());
			vertex_shader->setExtra(vertex_shader_key);
		}
		pass->setVertexProgram(vertex_shader_key);

		
		if (pass->hasVertexProgram()) {
			Ogre::GpuProgramParametersSharedPtr vp_parameters = pass->getVertexProgramParameters();
			vector<string> shader_parameter_filenames=vertex_shader->getShaderParameterFilenames();

			vp_parameters->setTransposeMatrices(true);

			for (size_t i=0; i<shader_parameter_filenames.size(); i++) {
				LibGens::ShaderParams *shader_params=shader_library->getVertexShaderParams(shader_parameter_filenames[i]);

				if (shader_params) {
					setShaderParameters(pass, vp_parameters, material, shader_params, uv_animation, vertex_shader_code_file);
				}
			}
		}

		// Pixel Shader
		string pixel_shader_name = pixel_shader->getShaderFilename();
		string pixel_shader_key = ToString((size_t)shader_library) + "_pixel_" + pixel_shader_name;
		LibGens::ArFile* pixel_shader_code_file = shader_library->getFile(pixel_shader_name + ".wpu");
		if (!pixel_shader->hasExtra()) {
			Ogre::ResourcePtr shader = Ogre::GpuProgramManager::getSingletonPtr()->createProgramFromString(pixel_shader_key, GENERAL_MESH_GROUP, "", Ogre::GPT_FRAGMENT_PROGRAM, "ps_3_0");
			((Ogre::D3D9GpuProgram*)(shader.get()))->setExternalMicrocode(pixel_shader_code_file->getData(), pixel_shader_code_file->getSize());
			pixel_shader->setExtra(pixel_shader_key);
		}
		pass->setFragmentProgram(pixel_shader_key);

		if (pass->hasFragmentProgram()) {
			Ogre::GpuProgramParametersSharedPtr fp_parameters = pass->getFragmentProgramParameters();
			vector<string> shader_parameter_filenames=pixel_shader->getShaderParameterFilenames();

			fp_parameters->setTransposeMatrices(true);

			for (size_t i=0; i<shader_parameter_filenames.size(); i++) {
				LibGens::ShaderParams *shader_params=shader_library->getPixelShaderParams(shader_parameter_filenames[i]);

				if (shader_params) {
					setShaderParameters(pass, fp_parameters, material, shader_params, uv_animation, pixel_shader_code_file);
				}
			}
		}
	}
	else {
		pass->setVertexProgram("SysDummyVS");
		pass->setFragmentProgram("SysDummyPS");
	}
}


void buildMaterial(LibGens::Material *material, string material_name, string resource_group, size_t mesh_slot, bool no_gi, LibGens::ShaderLibrary *shader_library) {
	material->setExtra(material_name);
	LibGens::Texture *texture=material->getTextureByUnit(LIBGENS_MATERIAL_TEXTURE_UNIT_DIFFUSE);

	Ogre::Material *compilematerial = Ogre::MaterialManager::getSingleton().create(material_name, resource_group).getPointer();
	if (!compilematerial) {
		return;
	}

	Ogre::Pass *pass=compilematerial->getTechnique(0)->getPass(0);

	fixPass(pass, material, mesh_slot);

	// Force it to use 16 total texture units and full ambient lighting
	for (size_t i=0; i<16; i++) {
		Ogre::TextureUnitState *texture_unit_state=pass->createTextureUnitState(shader_library == NULL ? "white.dds" : "black.dds");
	}
	pass->setAmbient(1.0, 1.0, 1.0);

	// Search for UV Animations for the first texset
	LibGens::UVAnimation *uv_animation = NULL;
	if (texture) {
		uv_animation = SONICGLVL_UV_ANIMATION_LIBRARY->getUVAnimation(material->getName(), texture->getTexset());
	}

	// Search for shaders based on the material's shader
	updateMaterialShaderParameters(compilematerial, material, no_gi, uv_animation, shader_library);
}


void buildMesh(Ogre::SceneNode *scene_node, LibGens::Mesh *mesh, Ogre::SceneManager *scene_manager, LibGens::MaterialLibrary *material_library, string root_name, Ogre::uint32 query_flags, string resource_group, bool global_illumination, string skeleton_name, Ogre::Entity *&shared_entity, vector<LibGens::Bone *> model_bones, LibGens::ShaderLibrary *shader_library) {
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
				vector<LibGens::Polygon> submesh_faces = submesh->getFaces();

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
					faces[i*3]   = submesh_faces[i].a;
					faces[i*3+1] = submesh_faces[i].b;
					faces[i*3+2] = submesh_faces[i].c;
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
							buildMaterial(mat, mat_name, resource_group, mesh_slot, false, shader_library);
							mat->setExtraGI(mat_name);
						}
					}
					else {
						if (!mat->hasExtra()) {
							buildMaterial(mat, mat_name, resource_group, mesh_slot, true, shader_library);
							mat->setExtra(mat_name);
						}
					}
			
					sub->setMaterialName(mat_name);
				}

				
				// Create Bone Assignments if Skeleton name exists
				if (skeleton_name.size()) {
					Ogre::SkeletonPtr ogre_skeleton = Ogre::SkeletonManager::getSingleton().getByName(skeleton_name, resource_group);
					if (!ogre_skeleton.isNull()) {
						vector<unsigned short> bone_table = submesh->getBoneTable();

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
								unsigned short bone_index  = vertex->getBoneIndex(j);
								unsigned char bone_weight = vertex->getBoneWeight(j);

								if ((bone_index == 0xFFFF) && j) {
									break;
								}
								else {
									vba.boneIndex = 0;
									vba.weight = 1.0f;

									if (bone_index != 0xFFFF) {
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
			msh->_setBoundingSphereRadius(mesh_aabb.radius());
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


void buildModel(Ogre::SceneNode *scene_node, LibGens::Model *model, string model_name, string skeleton_name, Ogre::SceneManager *scene_manager, LibGens::MaterialLibrary *material_library, Ogre::uint32 query_flags, string resource_group, bool global_illumination, LibGens::ShaderLibrary *shader_library) {
	if (!model) return;
	if (!scene_manager) return;
	if (!material_library) return;
	
	Ogre::Entity *shared_entity=NULL;
	vector<LibGens::Mesh *> meshes=model->getMeshes();
	unsigned int i=0;
	for (vector<LibGens::Mesh *>::iterator it=meshes.begin(); it!=meshes.end(); it++) {
		buildMesh(scene_node, (*it), scene_manager, material_library, model_name + "_" + ToString(i), query_flags, resource_group, global_illumination, skeleton_name, shared_entity, model->getBones(), shader_library);
		i++;
	}
}