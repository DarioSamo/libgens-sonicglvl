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

#include "Keyframe.h"
#include "KeyframeSet.h"

namespace LibGens {
	KeyframeSet::KeyframeSet() {
		flag = 0;
	}

	unsigned int KeyframeSet::getFlag() {
		return flag;
	}

	void KeyframeSet::read(File *file, vector<Keyframe *> &keyframes_buffer) {
		unsigned int keyframes_count=0;
		unsigned int keyframes_index=0;
		file->readInt32BE(&flag);
		file->readInt32BE(&keyframes_count);
		file->readInt32BE(&keyframes_index);
		
		for (size_t i=keyframes_index; i<keyframes_index+keyframes_count; i++) {
			keyframes.push_back(keyframes_buffer[i]);

			//printf("    Keyframe #%d: %f %f\n", (i-keyframes_index), keyframes_buffer[i]->getFrame(), keyframes_buffer[i]->getValue());
		}
	}

	float KeyframeSet::getValue(float current_frame) {
		Keyframe *previous_key=NULL;
		Keyframe *next_key=NULL;

		float factor=0;

		for (size_t i=0; i<keyframes.size(); i++) {
			previous_key = next_key;
			next_key = keyframes[i];

			if (next_key->getFrame() > current_frame) {
				break;
			}
		}

		if (previous_key && next_key) {
			float frame_offset = current_frame - previous_key->getFrame();
			float frame_gap    = next_key->getFrame() - previous_key->getFrame();
			factor = frame_offset / frame_gap;

			return previous_key->getValue() + ((next_key->getValue() - previous_key->getValue()) * factor);
		}
		else if (next_key) {
			return next_key->getValue();
		}
		else if (previous_key) {
			return previous_key->getValue();
		}

		return 0.0f;
	}
}