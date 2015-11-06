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

#include "Parameter.h"

namespace LibGens {
	Parameter::Parameter() {
	}

	Parameter::Parameter(string name_p, Color color_p) {
		name = name_p;
		color = color_p;
	}

	void Parameter::read(File *file) {
		size_t header_address=file->getCurrentAddress();
		
		size_t name_address=0;
		size_t color_address=0;
		file->goToAddress(header_address+4);
		file->readInt32BEA(&name_address);
		file->readInt32BEA(&color_address);

		file->goToAddress(name_address);
		file->readString(&name);

		file->goToAddress(color_address);
		color.read(file);
	}

	
	void Parameter::write(File *file) {
		size_t header_address=file->getCurrentAddress();
		unsigned char first_flag=2;
		unsigned char second_flag=1;
		size_t name_address=0;
		size_t color_address=0;
		file->writeUChar(&first_flag);
		file->writeNull(1);
		file->writeUChar(&second_flag);
		file->writeNull(1);

		file->writeNull(8);
		name_address = file->getCurrentAddress();
		file->writeString(&name);
		file->fixPadding();

		color_address = file->getCurrentAddress();
		color.write(file);

		file->goToAddress(header_address+4);
		file->writeInt32BEA(&name_address);
		file->writeInt32BEA(&color_address);
		file->goToEnd();
	}

	string Parameter::getName() {
		return name;
	}

	Color Parameter::getColor() {
		return color;
	}
};