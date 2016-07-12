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

#include "LibGens.h"
#include "S06XnFile.h"

namespace LibGens {
	void SonicXNTexture::read(File *file) {
		SonicXNSection::read(file);

		size_t texture_table_info_address=0;
		file->readInt32EA(&texture_table_info_address, big_endian);
		file->goToAddress(texture_table_info_address);

		unsigned int texture_count=0;
		size_t texture_table_address=0;
		file->readInt32E(&texture_count, big_endian);
		file->readInt32EA(&texture_table_address, big_endian);

		for (size_t i=0; i<texture_count; i++) {
			if (file_mode == MODE_YNO) {
				file->goToAddress(texture_table_address + i*8);
			}
			else file->goToAddress(texture_table_address + i*20 + 4);

			size_t name_address=0;
			unsigned int name_size=0;
			file->readInt32EA(&name_address, big_endian);
			file->readInt32E(&name_size, big_endian);

			file->goToAddress(name_address);
			string texture_unit="";
			file->readString(&texture_unit);

			textures.push_back(texture_unit);
			sizes.push_back(name_size);

			Error::printfMessage(Error::WARNING, "Found texture unit %d: %s (Flags: %d)", i, texture_unit.c_str(), name_size);
		}
	}

	void SonicXNTexture::writeBody(File *file) {
		file->fixPadding(16);

		size_t texture_address=file->getCurrentAddress();
		file->writeNull(textures.size() * 20);

		size_t table_address=file->getCurrentAddress();
		unsigned int texture_count = textures.size();
		file->writeInt32(&texture_count);
		file->writeInt32A(&texture_address);

		texture_addresses.clear();
		for (size_t i=0; i<texture_count; i++) {
			texture_addresses.push_back(file->getCurrentAddress());
			file->writeString(&textures[i]);
		}

		size_t bookmark=file->getCurrentAddress();

		file->goToAddress(head_address + 8);
		file->writeInt32A(&table_address, false);

		for (size_t i=0; i<texture_count; i++) {
			file->goToAddress(texture_address + i*20 + 4);
			size_t address=texture_addresses[i];
			file->writeInt32A(&address);
			file->writeInt32(&sizes[i]);
		}

		file->goToAddress(bookmark);
	}

	unsigned int SonicXNTexture::addTexture(string name) {
		for (size_t i=0; i<textures.size(); i++) {
			if (textures[i] == name) {
				return i;
			}
		}

		textures.push_back(name);
		unsigned int size=0x010004;
		sizes.push_back(size);
		return textures.size()-1;
	}
}