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

#include "LibGens.h"
#include <algorithm>
#include "S06XnFile.h"

namespace LibGens {
	void SonicVertexResourceTable::read(File *file, XNFileMode file_mode, bool big_endian) {
		unsigned int table_count=0;
		size_t table_address=0;

		file->readInt32E(&table_count, big_endian);
		file->readInt32EA(&table_address, big_endian);

		if ((table_count != 1) && (table_count != 16)) {
			printf("Unhandled Case Vertex Resource Table %d.\n", table_count);
			getchar();
		}

		Error::addMessage(Error::WARNING, "Vertex Resource Table of type " + ToString(table_count) + " at " + ToString(table_address));

		file->goToAddress(table_address);

		unsigned short position_type_flag=0;
		unsigned short position_total=0;
		size_t position_address=0;
		file->readInt16E(&position_type_flag, big_endian);
		file->readInt16E(&position_total, big_endian);
		file->readInt32EA(&position_address, big_endian);

		unsigned short normal_type_flag=0;
		unsigned short normal_total=0;
		size_t normal_address=0;
		file->readInt16E(&normal_type_flag, big_endian);
		file->readInt16E(&normal_total, big_endian);
		file->readInt32EA(&normal_address, big_endian);


		unsigned short color_type_flag=0;
		unsigned short color_total=0;
		size_t color_address=0;
		file->readInt16E(&color_type_flag, big_endian);
		file->readInt16E(&color_total, big_endian);
		file->readInt32EA(&color_address, big_endian);


		unsigned short uv_type_flag=0;
		unsigned short uv_total=0;
		size_t uv_address=0;
		file->readInt16E(&uv_type_flag, big_endian);
		file->readInt16E(&uv_total, big_endian);
		file->readInt32EA(&uv_address, big_endian);

		unsigned short uv2_type_flag=0;
		unsigned short uv2_total=0;
		size_t uv2_address=0;
		file->readInt16E(&uv2_type_flag, big_endian);
		file->readInt16E(&uv2_total, big_endian);
		file->readInt32EA(&uv2_address, big_endian);

		unsigned short bones_type_flag=0;
		unsigned short bones_total=0;
		size_t bones_address=0;
		file->readInt16E(&bones_type_flag, big_endian);
		file->readInt16E(&bones_total, big_endian);
		file->readInt32EA(&bones_address, big_endian);

		unsigned short unknown_type_flag=0;
		unsigned short unknown_total=0;
		size_t unknown_address=0;
		file->readInt16E(&unknown_type_flag, big_endian);
		file->readInt16E(&unknown_total, big_endian);
		file->readInt32EA(&unknown_address, big_endian);

		Error::addMessage(Error::WARNING, "  Positions: " + ToString(position_total) + "(" + ToString(position_type_flag) + ")");
		Error::addMessage(Error::WARNING, "  Normals  : " + ToString(normal_total)   + "(" + ToString(normal_type_flag) + ")");
		Error::addMessage(Error::WARNING, "  Colors   : " + ToString(color_total) + "(" + ToString(color_type_flag) + ")");
		Error::addMessage(Error::WARNING, "  UVs      : " + ToString(uv_total) + "(" + ToString(uv_type_flag) + ")");
		Error::addMessage(Error::WARNING, "  UV2s     : " + ToString(uv2_total) + "(" + ToString(uv2_type_flag) + ")");
		Error::addMessage(Error::WARNING, "  Bones    : " + ToString(bones_total) + "(" + ToString(bones_type_flag) + ")");
		Error::addMessage(Error::WARNING, "  Unknown  : " + ToString(unknown_total) + "(" + ToString(unknown_type_flag) + ")");

		if (unknown_total > 0) {
			printf("The unknown total in the vertex resource table is used, not cracked yet.\n");
			getchar();
		}

		for (size_t i=0; i<position_total; i++) {
			float fx = 0;
			float fy = 0;
			float fz = 0;

			if (position_type_flag == 1) {
				file->goToAddress(position_address + i*12);
				file->readFloat32E(&fx, big_endian);
				file->readFloat32E(&fy, big_endian);
				file->readFloat32E(&fz, big_endian);
			}
			else if ((position_type_flag >= 3) && (position_type_flag <= 8)) {
				file->goToAddress(position_address + i*6);

				unsigned short pow_factor=position_type_flag-2;
				float div_factor=pow(4.0, (double)pow_factor);

				short f=0;
				file->readInt16E((unsigned short *)(&f), big_endian);
				fx = f/div_factor;
				file->readInt16E((unsigned short *)(&f), big_endian);
				fy = f/div_factor;
				file->readInt16E((unsigned short *)(&f), big_endian);
				fz = f/div_factor;
			}
			else {
				printf("Unhandled position type case %d at address %d\n", position_type_flag, position_address);
				getchar();
			}

			Vector3 position(fx, fy, fz);
			positions.push_back(position);
		}


		
		for (size_t i=0; i<normal_total; i++) {
			float fx = 0.0;
			float fy = 0.0;
			float fz = 0.0;

			if (normal_type_flag == 3) {
				file->goToAddress(normal_address + i*3);

				char f=0;
				file->readUChar((unsigned char *) (&f));
				fx = ((int)f);
				file->readUChar((unsigned char *) (&f));
				fy = ((int)f);
				file->readUChar((unsigned char *) (&f));
				fz = ((int)f);
			}
			else {
				printf("Unhandled normal type case %d at address %d\n", normal_type_flag, normal_address);
				getchar();
			}

			Vector3 normal(fx, fy, fz);
			normal.normalise();
			normals.push_back(normal);
		}


		
		for (size_t i=0; i<color_total; i++) {
			Color color;

			if (color_type_flag == 1) {
				file->goToAddress(color_address + i*4);
				color.readRGBA8(file);
			}
			else {
				printf("Unhandled color type case %d at address %d\n", color_type_flag, color_address);
				getchar();
			}

			colors.push_back(color);
		}


		for (size_t i=0; i<uv_total; i++) {
			float fx = 0.0;
			float fy = 0.0;

			if (uv_type_flag == 2) {
				file->goToAddress(uv_address + i*4);

				short f=0;
				file->readInt16E((unsigned short *)(&f), big_endian);
				fx = f/256.0;
				file->readInt16E((unsigned short *)(&f), big_endian);
				fy = f/256.0;
			}
			else if (uv_type_flag == 3) {
				file->goToAddress(uv_address + i*4);

				short f=0;
				file->readInt16E((unsigned short *)(&f), big_endian);
				fx = f/1024.0;
				file->readInt16E((unsigned short *)(&f), big_endian);
				fy = f/1024.0;
			}
			else {
				printf("Unhandled UV type case %d at address %d\n", uv_type_flag, uv_address);
				getchar();
			}

			Vector2 uv(fx, fy);
			uvs.push_back(uv);
		}

		
		for (size_t i=0; i<uv2_total; i++) {
			float fx = 0.0;
			float fy = 0.0;

			if (uv2_type_flag == 2) {
				file->goToAddress(uv2_address + i*4);

				short f=0;
				file->readInt16E((unsigned short *)(&f), big_endian);
				fx = f/256.0;
				file->readInt16E((unsigned short *)(&f), big_endian);
				fy = f/256.0;
			}
			else if (uv2_type_flag == 3) {
				file->goToAddress(uv2_address + i*4);

				short f=0;
				file->readInt16E((unsigned short *)(&f), big_endian);
				fx = f/1024.0;
				file->readInt16E((unsigned short *)(&f), big_endian);
				fy = f/1024.0;
			}
			else {
				printf("Unhandled UV2 type case %d at address %d\n", uv2_type_flag, uv2_address);
				getchar();
			}

			Vector2 uv(fx, fy);
			uvs_2.push_back(uv);
		}

		
		for (size_t i=0; i<bones_total; i++) {
			SonicVertexBoneData bone_data;
			bone_data.weight = 0;

			if (bones_type_flag == 1) {
				file->goToAddress(bones_address + i*4);
				file->readUChar(&(bone_data.bone_1));
				file->readUChar(&(bone_data.bone_2));
				file->readInt16BE(&(bone_data.weight));
				bones.push_back(bone_data);
			}
			else {
				printf("Unhandled Bones type case %d at address %d\n", bones_type_flag, bones_address);
				getchar();
			}
		}
	}
};
