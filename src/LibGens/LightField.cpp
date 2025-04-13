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

#include "LightField.h"

namespace LibGens {
	void ColorPoint::read(File *file) {
	}

	void ColorPoint::write(File *file) {
		for (size_t x=0; x<8; x++) {
			for (size_t y=0; y<3; y++) {
				file->writeUChar(&rgb[x][y]);
			}
		}
		file->writeUChar(&flag);
	}


	void LightFieldCube::read(File *file, size_t head_address, AABB aabb) {
		file->readInt32BE(&type);
		file->readInt32BE(&value);

		point = aabb.center();

		if (type != LIBGENS_LIGHTFIELD_CUBE_NO_SPLIT) {
			left = new LightFieldCube();
			right = new LightFieldCube();

			file->goToAddress(head_address + LIBGENS_LIGHTFIELD_CUBE_SIZE * value);
			left->read(file, head_address, aabb.half(type, LIBGENS_MATH_SIDE_LEFT));

			file->goToAddress(head_address + LIBGENS_LIGHTFIELD_CUBE_SIZE * (value+1));
			right->read(file, head_address, aabb.half(type, LIBGENS_MATH_SIDE_RIGHT));
		}
		else {

		}
	}

	void LightFieldCube::write(File *file) {
		file->writeInt32BE(&type);
		file->writeInt32BE(&value);
	}

	void LightFieldCube::getCubeList(vector<LightFieldCube *> *list) {
		if (!list) {
			return;
		}

		if (type != LIBGENS_LIGHTFIELD_CUBE_NO_SPLIT) {
			value=list->size();
			list->push_back(left);
			list->push_back(right);

			left->getCubeList(list);
			right->getCubeList(list);
		}
		else {
		}
	}

	LightField::LightField(string filename) {
		File file(filename, LIBGENS_FILE_READ_BINARY);

		if (file.valid()) {
			file.readHeader();
			read(&file);
			file.close();
		}
	}

	void LightField::read(File *file) {
		size_t header_address=file->getCurrentAddress();
		world_aabb.read(file);

		int cube_count=0;
		size_t cube_address=0;
		int color_count=0;
		size_t color_address=0;
		int index_count=0;
		size_t index_address=0;

		file->readInt32BE(&cube_count);
		file->readInt32BEA(&cube_address);
		file->readInt32BE(&color_count);
		file->readInt32BEA(&color_address);
		file->readInt32BE(&index_count);
		file->readInt32BEA(&index_address);

		// LightFieldCubes
		// Recursive method traverses the entire Octree
		file->goToAddress(cube_address);
		cube=new LightFieldCube();
		cube->read(file, cube_address, world_aabb);
	}

	void LightField::save(string filename) {
		File file(filename, LIBGENS_FILE_WRITE_BINARY);

		if (file.valid()) {
			file.prepareHeader(LIBGENS_LIGHTFIELD_FILE_ROOT_TYPE);
			write(&file);
			file.writeHeader();
			file.close();
		}
	}

	void LightField::write(File *file) {
		world_aabb.write(file);

		vector<unsigned int> indices;

		size_t root_address=file->getCurrentAddress();
		file->writeNull(24);

		unsigned int cube_count=0;
		size_t cube_address=0;
		unsigned int color_count=0;
		size_t color_address=0;
		unsigned int index_count=0;
		size_t index_address=0;

		vector<LightFieldCube *> cubes;
		cubes.push_back(cube);
		cube->getCubeList(&cubes);
		cube_count = cubes.size();
		cube_address = file->getCurrentAddress();

		for (size_t i=0; i<cubes.size(); i++) {
			if (cubes[i]->getType() == LIBGENS_LIGHTFIELD_CUBE_NO_SPLIT) {
				cubes[i]->setValue(indices.size());
				for (size_t j=0; j<8; j++) {
					indices.push_back(cubes[i]->getCorner(j)->color->index);
				}
			}
			
			cubes[i]->write(file);
		}


		color_count = color_palette.size();
		color_address = file->getCurrentAddress();

		for (size_t i=0; i<color_palette.size(); i++) {
			color_palette[i]->write(file);
		}


		index_count = indices.size();
		index_address = file->getCurrentAddress();

		for (size_t i=0; i<indices.size(); i++) {
			file->writeInt32BE(&(indices[i]));
		}

		
		file->goToAddress(root_address);
		file->writeInt32BE(&cube_count);
		file->writeInt32BEA(&cube_address);
		file->writeInt32BE(&color_count);
		file->writeInt32BEA(&color_address);
		file->writeInt32BE(&index_count);
		file->writeInt32BEA(&index_address);

		file->goToEnd();
	}

	ColorPoint *LightField::createColorPoint(unsigned char rgb[8][3], unsigned char flag) {
		for (size_t i=0; i<color_palette.size(); i++) {
			bool result=true;

			for (size_t j=0; j<8; j++) {
				for (size_t k=0; k<3; k++) {
					if (color_palette[i]->rgb[j][k] != rgb[j][k]) {
						result=false;
						break;
					}
				}

				if (!result) break;
			}

			if (color_palette[i]->flag != flag) {
				result = false;
			}

			if (result) {
				return color_palette[i];
			}
		}

		ColorPoint *color=new ColorPoint();
		for (size_t j=0; j<8; j++) {
			for (size_t k=0; k<3; k++) {
				color->rgb[j][k] = rgb[j][k];
			}
		}
		color->flag = flag;
		color->index = color_palette.size();
		color_palette.push_back(color);
		return color;
	}
};