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

#include "UVAnimation.h"
#include "UVAnimationSet.h"

namespace LibGens {
	UVAnimation::UVAnimation(string filename) {
		File file(filename, LIBGENS_FILE_READ_BINARY);

		name = filename;
		size_t sep = name.find_last_of("\\/");
		if (sep != std::string::npos) {
			name = name.substr(sep + 1, name.size() - sep - 1);
		}

		if (file.valid()) {
			file.readHeader();
			read(&file);
			readAnimations(&file);
			file.close();
		}
	}

	void UVAnimation::readAnimations(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_ANIMATION_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		file->goToAddress(animations_address);

		unsigned int string_material_name_offset=0;
		unsigned int string_texture_unit_name_offset=0;
		unsigned int animations_count=0;
		size_t animation_table_address=0;

		file->readInt32BE(&string_material_name_offset);
		file->readInt32BE(&string_texture_unit_name_offset);
		file->readInt32BE(&animations_count);
		file->readInt32BEA(&animation_table_address);

		material_name = string(names_buffer + string_material_name_offset);
		texset_name = string(names_buffer + string_texture_unit_name_offset);

		//printf("UV Animation with material %s texture unit %s. Found %d animations.\n", material_name.c_str(), texset_name.c_str(), animations_count);

		file->goToAddress(animation_table_address);

		for (size_t animation=0; animation<animations_count; animation++) {
			UVAnimationSet *animation_set = new UVAnimationSet();
			animation_set->read(file, names_buffer, keyframes_buffer);
			animation_sets.push_back(animation_set);
		}
	}

	UVAnimationSet *UVAnimation::getAnimationSet(string animation_name) {
		for (size_t i=0; i<animation_sets.size(); i++) {
			if (animation_name.size()) {
				if (animation_name == animation_sets[i]->getName()) {
					return animation_sets[i];
				}
			}
			else {
				return animation_sets[i];
			}
		}

		return NULL;
	}

	string UVAnimation::getMaterialName() {
		return material_name;
	}

	string UVAnimation::getTexsetName() {
		return texset_name;
	}
};