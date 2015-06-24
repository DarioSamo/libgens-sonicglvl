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

#include "KeyframeSet.h"
#include "UVAnimationSet.h"

namespace LibGens {
	void UVAnimationSet::read(File *file, char *names_buffer, vector<Keyframe *> &keyframes_buffer) {
		unsigned int string_animation_name_offset=0;
		file->readInt32BE(&string_animation_name_offset);
		file->readFloat32BE(&fps);
		file->readFloat32BE(&start_time);
		file->readFloat32BE(&end_time);
		animation_name = string(names_buffer + string_animation_name_offset);
		unsigned int keyframe_sets_count=0;
		file->readInt32BE(&keyframe_sets_count);

		//printf("  Animation Set %s: %d Keyframe Sets [%f - %f] FPS: %f\n", animation_name.c_str(), keyframe_sets_count, start_time, end_time, fps);

		for (size_t i=0; i<keyframe_sets_count; i++) {
			KeyframeSet *keyframe_set = new KeyframeSet();
			keyframe_set->read(file, keyframes_buffer);
			keyframe_sets.push_back(keyframe_set);
		}
	}
};