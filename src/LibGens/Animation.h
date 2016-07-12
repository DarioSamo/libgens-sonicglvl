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

#pragma once

#define LIBGENS_ANIMATION_ERROR_MESSAGE_NULL_FILE     "Trying to read animation data from unreferenced file."
#define LIBGENS_UV_ANIMATION_EXTENSION                ".uv-anim"
#define LIBGENS_ANIMATION_KEYFRAME_BYTES              8

namespace LibGens {
	class Keyframe;

	class Animation {
		protected:
			string name;
			char *names_buffer;
			vector<Keyframe *> keyframes_buffer;
			size_t animations_address;
		public:
			Animation();
			void read(File *file);
			void readKeyframes(File *file, unsigned int size);
			void readStrings(File *file, unsigned int size);
			string getName();
	};
};
