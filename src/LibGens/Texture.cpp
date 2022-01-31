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

#include "Texture.h"
#include "Material.h"

namespace LibGens {
	Texture::Texture() {
		flags = 0;
	}

	Texture::Texture(string filename_p, string internal_name_p) {
		File file(filename_p, LIBGENS_FILE_READ_BINARY);

		if (file.valid()) {
			file.readHeader();
			read(&file, internal_name_p);
			file.close();
		}
	}

	Texture::Texture(string internal_name_p, string unit_p, string name_p) {
		internal_name = internal_name_p;
		unit = unit_p;
		name = name_p;
		flags = 0;
	}

	void Texture::setName(string v) {
		name = v;
	}

	void Texture::setUnit(string v) {
		unit = v;
	}

	void Texture::setInternalName(string v) {
		internal_name = v;
	}

	string Texture::getName() {
		return name;
	}

	string Texture::getUnit() {
		return unit;
	}

	string Texture::getTexset() {
		return internal_name;
	}

	void Texture::read(File *file, string internal_name_p) {
		internal_name = internal_name_p;
		size_t header_address=file->getCurrentAddress();
		
		size_t file_address=0;
		size_t unit_address=0;
		file->readInt32BEA(&file_address);
		file->readInt32BE(&flags);
		file->readInt32BEA(&unit_address);

		file->goToAddress(file_address);
		file->readString(&name);

		file->goToAddress(unit_address);
		file->readString(&unit);
	}

	
	void Texture::write(File *file) {
		size_t header_address=file->getCurrentAddress();

		size_t file_address=0;
		size_t unit_address=0;
		file->writeNull(12);

		file_address = file->getCurrentAddress();
		file->writeString(&name);

		unit_address = file->getCurrentAddress();
		file->writeString(&unit);
		file->fixPadding();

		file->goToAddress(header_address);
		file->writeInt32BEA(&file_address);
		file->writeInt32BE(&flags);
		file->writeInt32BEA(&unit_address);
		file->goToEnd();
	}

	void Texture::save(string filename) {
		File file(filename, LIBGENS_FILE_WRITE_BINARY);

		if (file.valid()) {
			file.prepareHeader(LIBGENS_TEXTURE_ROOT_UNLEASHED);
			write(&file);
			file.writeHeader(true);
			file.close();
		}
	}
};