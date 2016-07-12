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
	void SonicXNBones::read(File *file) {
		SonicXNSection::read(file);

		size_t table_address=0;
		file->readInt32EA(&table_address, big_endian);
		file->goToAddress(table_address+4);

		unsigned int bones_count=0;
		size_t bones_address=0;
		file->readInt32E(&bones_count, big_endian);
		file->readInt32EA(&bones_address, big_endian);

		for (size_t i=0; i<bones_count; i++) {
			file->goToAddress(bones_address + i*8);

			unsigned int index=0;
			size_t name_address=0;
			string name="";

			file->readInt32E(&index, big_endian);
			file->readInt32EA(&name_address, big_endian);

			file->goToAddress(name_address);
			file->readString(&name);

			bone_indices.push_back(index);
			bone_names.push_back(name);
		}
	}

	
	void SonicXNBones::writeBody(File *file) {
		file->fixPadding(16);

		size_t bone_address=file->getCurrentAddress();
		file->writeNull(bone_names.size() * 8);

		size_t table_address=file->getCurrentAddress();
		unsigned int bone_count = bone_names.size();

		file->writeNull(4);
		file->writeInt32(&bone_count);
		file->writeInt32A(&bone_address);

		bone_names_addresses.clear();
		for (size_t i=0; i<bone_count; i++) {
			bone_names_addresses.push_back(file->getCurrentAddress());
			file->writeString(&bone_names[i]);
		}

		size_t bookmark=file->getCurrentAddress();

		file->goToAddress(head_address + 8);
		file->writeInt32A(&table_address, false);

		for (size_t i=0; i<bone_count; i++) {
			file->goToAddress(bone_address + i*8);

			file->writeInt32(&bone_indices[i]);
			file->writeInt32A(&bone_names_addresses[i]);
		}

		file->goToAddress(bookmark);
	}
}