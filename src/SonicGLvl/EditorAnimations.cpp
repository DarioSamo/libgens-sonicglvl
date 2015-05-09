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

#include "EditorAnimations.h"
#include "UVAnimationSet.h"


void EditorAnimationTexcoordOffset::updateParameters() {
	float offset_x=animation_set->getCurrentValue(0x00010000);
	float offset_y=animation_set->getCurrentValue(0x01010000);

	float offset_x_unl=animation_set->getCurrentValue(0x00000000);
	float offset_y_unl=animation_set->getCurrentValue(0x01000000);

	Ogre::Vector4 value(0.0, 0.0, 0.0, 0.0);

	if (offset_x != 0.0)     value.x = offset_x;
	if (offset_y != 0.0)     value.y = offset_y;
	if (offset_x_unl != 0.0) value.x = offset_x_unl;
	if (offset_y_unl != 0.0) value.y = offset_y_unl;

	program_parameters->setConstant(program_constant_index, value);
}


void EditorAnimationsList::addTime(float time_s) {
	for (list<LibGens::AnimationSet *>::iterator it=animation_sets.begin(); it!=animation_sets.end(); it++) {
		(*it)->addTime(time_s);
	}

	for (list<EditorAnimationTexcoordOffset *>::iterator it=texcoord_offset_animations.begin(); it!=texcoord_offset_animations.end(); it++) {
		(*it)->updateParameters();
	}
}



void EditorAnimationsList::addAnimationSet(LibGens::AnimationSet *animation_set) {
	bool found=false;

	for (list<LibGens::AnimationSet *>::iterator it=animation_sets.begin(); it!=animation_sets.end(); it++) {
		if ((*it) == animation_set) {
			return;
		}
	}

	if (!found) animation_sets.push_back(animation_set);
}

void EditorAnimationsList::addTexcoordAnimation(LibGens::UVAnimation *uv_animation, Ogre::GpuProgramParametersSharedPtr program_parameters, size_t program_index) {
	LibGens::AnimationSet *animation_set = uv_animation->getAnimationSet();

	if (animation_set) {
		addAnimationSet(animation_set);

		EditorAnimationTexcoordOffset *animation_texcoord = new EditorAnimationTexcoordOffset();
		animation_texcoord->setAnimationSet(animation_set);
		animation_texcoord->setParameters(program_parameters);
		animation_texcoord->setIndex(program_index);
		texcoord_offset_animations.push_back(animation_texcoord);
	}
}