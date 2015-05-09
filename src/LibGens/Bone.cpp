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

#include "Bone.h"

namespace LibGens {
	Bone::Bone() {
	}

	void Bone::readDescription(File *file) {
		size_t name_address=0;
		file->readInt32BE(&parent_index);
		file->readInt32BEA(&name_address);

		file->goToAddress(name_address);
		file->readString(&name);
	}

	void Bone::readMatrix(File *file) {
		matrix.read(file);
	}

	void Bone::writeDescription(File *file) {
		size_t name_address=file->getCurrentAddress()+8;
		file->writeInt32BE(&parent_index);
		file->writeInt32BEA(&name_address);
		file->writeString(&name);
		file->fixPadding();
	}

	void Bone::writeMatrix(File *file) {
		matrix.write(file);
	}

	string Bone::getName() {
		return name;
	}

	unsigned int Bone::getParentIndex() {
		return parent_index;
	}

	Matrix4 Bone::getMatrix() {
		return matrix;
	}
};