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


#include "Animation.h"
#include "Keyframe.h"

namespace LibGens {
	Animation::Animation() {
		name = "";
	}
	
	void Animation::read(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_ANIMATION_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		unsigned int animations_size=0;
		size_t keyframes_address=0;
		unsigned int keyframes_size=0;
		size_t strings_address=0;
		unsigned int strings_size=0;

		file->readInt32BEA(&animations_address);
		file->readInt32BE(&animations_size);
		file->readInt32BEA(&keyframes_address);
		file->readInt32BE(&keyframes_size);
		file->readInt32BEA(&strings_address);
		file->readInt32BE(&strings_size);

		file->goToAddress(keyframes_address);
		readKeyframes(file, keyframes_size);

		file->goToAddress(strings_address);
		readStrings(file, strings_size);
	}

	void Animation::readKeyframes(File *file, unsigned int size) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_ANIMATION_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		size_t keyframe_count = size / LIBGENS_ANIMATION_KEYFRAME_BYTES;
		for (size_t i=0; i<keyframe_count; i++) {
			Keyframe *keyframe = new Keyframe();
			keyframe->read(file);
			keyframes_buffer.push_back(keyframe);
		}
	}

	
	void Animation::readStrings(File *file, unsigned int size) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_ANIMATION_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		names_buffer = new char[size];
		file->read(names_buffer, size);
	}

	string Animation::getName() {
		return name;
	}
};