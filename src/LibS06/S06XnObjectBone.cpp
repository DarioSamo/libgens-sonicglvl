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
#include <algorithm>
#include "S06XnFile.h"

namespace LibGens {
	void SonicBone::read(File *file, bool big_endian, XNFileMode file_mode) {
		file->readInt32E(&flag, big_endian);
		file->readInt16E(&matrix_index, big_endian);
		file->readInt16E(&parent_index, big_endian);
		file->readInt16E(&child_index, big_endian);
		file->readInt16E(&sibling_index, big_endian);
		translation.read(file, big_endian);
		file->readInt32E(&rotation_x, big_endian);
		file->readInt32E(&rotation_y, big_endian);
		file->readInt32E(&rotation_z, big_endian);
		scale.read(file, big_endian);
		matrix.read(file, big_endian);

		if (file_mode == MODE_GNO) {
			matrix = matrix.transpose();
			matrix[3][3] = 1.0;
		}

		center.read(file, big_endian);
		file->readFloat32E(&radius, big_endian);

		if (file_mode != MODE_GNO) {
			file->readInt32E(&user, big_endian);
			bounding_box.read(file, big_endian);
		}
		
		unsigned int rotation_flag=flag & 3840u;
		if (rotation_flag != 0u) {
			if (rotation_flag != 256u) {
				if (rotation_flag != 1024u) orientation.fromXYZInts(rotation_x, rotation_y, rotation_z);
				else orientation.fromZXYInts(rotation_x, rotation_y, rotation_z);
			}
			else orientation.fromXZYInts(rotation_x, rotation_y, rotation_z);
		}
		else orientation.fromXYZInts(rotation_x, rotation_y, rotation_z);
		
		current_matrix.makeTransform(translation, scale, orientation);
	}

	void SonicBone::write(File *file) {
		file->writeInt32(&flag);
		file->writeInt16(&matrix_index);
		file->writeInt16(&parent_index);
		file->writeInt16(&child_index);
		file->writeInt16(&sibling_index);
		translation.write(file, false);
		file->writeInt32(&rotation_x);
		file->writeInt32(&rotation_y);
		file->writeInt32(&rotation_z);
		scale.write(file, false);
		matrix.write(file, false);
		center.write(file, false);
		file->writeFloat32(&radius);
		file->writeInt32(&user);
		bounding_box.write(file, false);
	}


	void SonicBone::setScale(float sca) {
		translation = translation * sca;
		center = center * sca;
		radius = radius * sca;
		scale_animation_mod *= sca;

		Quaternion orientation;
		unsigned int rotation_flag=flag & 3840u;
		if (rotation_flag != 0u) {
			if (rotation_flag != 256u) {
				if (rotation_flag != 1024u) orientation.fromXYZInts(rotation_x, rotation_y, rotation_z);
				else orientation.fromZXYInts(rotation_x, rotation_y, rotation_z);
			}
			else orientation.fromXZYInts(rotation_x, rotation_y, rotation_z);
		}
		else orientation.fromXYZInts(rotation_x, rotation_y, rotation_z);
		current_matrix.makeTransform(translation, scale, orientation);
	}

	void SonicBone::zero() {
		flag = 0x2001C6;
		translation = Vector3();
		rotation_x = 0;
		rotation_y = 0;
		rotation_z = 0;
		scale = Vector3(1.0f, 1.0f, 1.0f);
		parent_index = 0xFF;
		matrix_index = 0;

		for (size_t x=0; x<4; x++) {
			for (size_t y=0; y<4; y++) {
				if (x==y) matrix[x][y] = 1.0f;
				else matrix[x][y] = 0.0f;
			}
		}

		child_index = 0xFFFF;
		sibling_index = 0xFFFF;
	}
};
