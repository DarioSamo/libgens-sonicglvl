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

#include "Common.h"
#include "AnimationSet.h"

#ifndef EDITOR_ANIMATIONS_H_INCLUDED
#define EDITOR_ANIMATIONS_H_INCLUDED


class EditorAnimationTexcoordOffset {
	protected:
		LibGens::AnimationSet *animation_set;
		Ogre::GpuProgramParametersSharedPtr program_parameters;
		size_t program_constant_index;
	public:
		EditorAnimationTexcoordOffset() {

		}

		void setAnimationSet(LibGens::AnimationSet *v) {
			animation_set = v;
		}

		void setParameters(Ogre::GpuProgramParametersSharedPtr v) {
			program_parameters = v;
		}

		void setIndex(size_t v) {
			program_constant_index = v;
		}

		void updateParameters();
};


class EditorAnimationsList {
	protected:
		list<LibGens::AnimationSet *> animation_sets;
		list<EditorAnimationTexcoordOffset *> texcoord_offset_animations;
	public:
		EditorAnimationsList() {
		}

		void addAnimationSet(LibGens::AnimationSet *animation_set);
		void addTime(float time_s);

		void addTexcoordAnimation(LibGens::UVAnimation *uv_animation, Ogre::GpuProgramParametersSharedPtr program_parameters, size_t program_index);
};

#endif