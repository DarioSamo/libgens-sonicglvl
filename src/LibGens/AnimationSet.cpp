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

#include "AnimationSet.h"
#include "KeyframeSet.h"

namespace LibGens {
	AnimationSet::AnimationSet() {
		fps = 30;
		start_time = 0;
		end_time = 0;
		current_frame = 0;
		animation_name = "";
	}

	void AnimationSet::addTime(float time_s) {
		current_frame += time_s * fps;
		while (current_frame > end_time) {
			current_frame -= (end_time - start_time);
		}
	}

	float AnimationSet::getCurrentValue(unsigned int flag) {
		for (size_t i=0; i<keyframe_sets.size(); i++) {
			if (keyframe_sets[i]->getFlag() == flag) {
				return keyframe_sets[i]->getValue(current_frame);
			}
		}

		return 0.0f;
	}

	string AnimationSet::getName() {
		return animation_name;
	}

	float AnimationSet::getFPS() {
		return fps;
	}
};