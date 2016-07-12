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
	void SonicXNEffect::read(File *file) {
		SonicXNSection::read(file);

		size_t table_address=0;
		file->readInt32EA(&table_address, big_endian);
		file->goToAddress(table_address+4);

		unsigned int shader_count=0;
		size_t shader_address=0;
		unsigned int name_count=0;
		size_t name_address=0;
		unsigned int extras_count=0;
		size_t extras_address=0;

		file->readInt32E(&shader_count, big_endian);
		file->readInt32EA(&shader_address, big_endian);

		file->readInt32E(&name_count, big_endian);
		file->readInt32EA(&name_address, big_endian);

		file->readInt32E(&extras_count, big_endian);
		file->readInt32EA(&extras_address, big_endian);

		for (size_t i=0; i<shader_count; i++) {
			file->goToAddress(shader_address + i*8 + 4);
			size_t string_address=0;
			string shader="";
			file->readInt32EA(&string_address, big_endian);
			file->goToAddress(string_address);
			file->readString(&shader);
			material_shaders.push_back(shader);
		}


		for (size_t i=0; i<name_count; i++) {
			file->goToAddress(name_address + i*12 + 4);
			unsigned int index=0;
			size_t string_address=0;
			string name="";
			file->readInt32E(&index, big_endian);
			file->readInt32EA(&string_address, big_endian);
			file->goToAddress(string_address);
			file->readString(&name);
			material_indices.push_back(index);
			material_names.push_back(name);
		}

		for (size_t i=0; i<extras_count; i++) {
			file->goToAddress(extras_address + i*2);
			unsigned short extra=0;
			file->readInt16E(&extra, big_endian);
			extras.push_back(extra);
		}
	}

	
	void SonicXNEffect::writeBody(File *file) {
		file->fixPadding(16);

		size_t shader_address=file->getCurrentAddress();
		file->writeNull(material_shaders.size() * 8);

		size_t name_address=file->getCurrentAddress();
		file->writeNull(material_names.size() * 12);

		size_t extras_address=file->getCurrentAddress();
		file->writeNull(extras.size() * 2);
		file->fixPadding(4);

		size_t table_address=file->getCurrentAddress();
		unsigned int shader_count = material_shaders.size();
		unsigned int name_count = material_names.size();
		unsigned int extras_count = extras.size();

		file->writeNull(4);
		file->writeInt32(&shader_count);
		file->writeInt32A(&shader_address);

		file->writeInt32(&name_count);
		file->writeInt32A(&name_address);

		file->writeInt32(&extras_count);
		file->writeInt32A(&extras_address);


		material_shaders_addresses.clear();
		for (size_t i=0; i<shader_count; i++) {
			material_shaders_addresses.push_back(file->getCurrentAddress());
			file->writeString(&material_shaders[i]);
		}

		material_names_addresses.clear();
		for (size_t i=0; i<name_count; i++) {
			material_names_addresses.push_back(file->getCurrentAddress());
			file->writeString(&material_names[i]);
		}

		size_t bookmark=file->getCurrentAddress();
		
		file->goToAddress(head_address + 8);
		file->writeInt32A(&table_address, false);

		for (size_t i=0; i<shader_count; i++) {
			file->goToAddress(shader_address + i*8 + 4);
			file->writeInt32A(&material_shaders_addresses[i]);
		}

		for (size_t i=0; i<name_count; i++) {
			file->goToAddress(name_address + i*12 + 4);
			file->writeInt32(&material_indices[i]);
			file->writeInt32A(&material_names_addresses[i]);
		}

		for (size_t i=0; i<extras_count; i++) {
			file->goToAddress(extras_address + i*2);
			file->writeInt16(&extras[i]);
		}


		file->goToAddress(bookmark);
	}
}
