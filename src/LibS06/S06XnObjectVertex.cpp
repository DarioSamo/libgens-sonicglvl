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
	void SonicVertex::read(File *file, unsigned int vertex_size, bool big_endian, unsigned int vertex_flag, XNFileMode file_mode) {
		size_t address=0;
		normal = Vector3(0,0,0);
		bone_indices[0]=bone_indices[1]=bone_indices[2]=bone_indices[3]=0;
		bone_weights_f[0]=1.0f;
		bone_weights_f[1]=bone_weights_f[2]=bone_weights_f[3]=0;

		float total_weight=0.0f;
		if (file_mode != MODE_ENO) {
			// Position
			if (vertex_flag & 0x1) {
				position.read(file, big_endian);
			}

			// Bone Weights
			if (vertex_flag & 0x7000) {
				file->readFloat32E(&bone_weights_f[0], big_endian);
				file->readFloat32E(&bone_weights_f[1], big_endian);
				file->readFloat32E(&bone_weights_f[2], big_endian);;
			}

			// Bone Indices
			if (vertex_flag & 0x400) {
				file->read(bone_indices, 4);
			}
			else {
				total_weight += bone_weights_f[0] + bone_weights_f[1] + bone_weights_f[2];

				bone_indices[0] = 0;
				bone_indices[1] = ((bone_weights_f[1] > 0.0f) ? 1 : 0);
				bone_indices[2] = ((bone_weights_f[2] > 0.0f) ? 2 : 0);
				bone_indices[3] = ((total_weight < 0.999f)    ? 3 : 0);
			}

			// Normal
			if (vertex_flag & 0x2) {
				normal.read(file, big_endian);
			}

			// RGBA 1
			if (vertex_flag & 0x8) {
				file->readUChar(&rgba[2]);
				file->readUChar(&rgba[1]);
				file->readUChar(&rgba[0]);
				file->readUChar(&rgba[3]);
			}

			// RGBA 2
			if (vertex_flag & 0x10) {
				file->readUChar(&rgba_2[2]);
				file->readUChar(&rgba_2[1]);
				file->readUChar(&rgba_2[0]);
				file->readUChar(&rgba_2[3]);
			}

			// UV Channel 1
			size_t uv_channels = vertex_flag / (0x10000);
			for (size_t i=0; i<uv_channels; i++) {
				uv[i].read(file, big_endian);
			}

			// Tangent / Binormal
			if (vertex_flag & 0x140) {
				tangent.read(file, big_endian);
				binormal.read(file, big_endian);
			}
			
			bone_weights_f[3] = 1.0 - bone_weights_f[0] - bone_weights_f[1] - bone_weights_f[2];
			if (bone_weights_f[3] == 1.0) bone_weights_f[3] = 0.0;
		}
		else {
			if (vertex_flag == 0x310005) {
				position.read(file, big_endian);
				normal.readNormal360(file, big_endian);
				uv[0].readHalf(file, big_endian);
			}
			else if (vertex_flag == 0x317405) {
				position.read(file, big_endian);
				normal.read(file, big_endian);
				file->moveAddress(8);
				uv[0].readHalf(file, big_endian);
			}
			else if (vertex_flag == 0x317685) {
				position.read(file, big_endian);
				normal.read(file, big_endian);
				file->moveAddress(8);
				uv[0].readHalf(file, big_endian);
				file->moveAddress(8);
			}
			else {
				printf("Unhandled vertex flag: %d\n", vertex_flag);
				getchar();
			}
		}
	}

	void SonicVertex::write(File *file, unsigned int vertex_size, bool big_endian, unsigned int vertex_flag, XNFileMode file_mode) {
		// Position
		if (vertex_flag & 0x1) {
			position.write(file, big_endian);
		}

		// Bone Weights
		if (vertex_flag & 0x7000) {
			file->writeFloat32E(&bone_weights_f[0], big_endian);
			file->writeFloat32E(&bone_weights_f[1], big_endian);
			file->writeFloat32E(&bone_weights_f[2], big_endian);
		}

		// Bone Indices
		if (vertex_flag & 0x400) {
			file->write(bone_indices, 4);
		}

		// Normal
		if (vertex_flag & 0x2) {
			normal.write(file, big_endian);
		}

		// RGBA 1
		if (vertex_flag & 0x8) {
			file->writeUChar(&rgba[2]);
			file->writeUChar(&rgba[1]);
			file->writeUChar(&rgba[0]);
			file->writeUChar(&rgba[3]);
		}

		// RGBA 2
		if (vertex_flag & 0x10) {
			file->writeUChar(&rgba_2[2]);
			file->writeUChar(&rgba_2[1]);
			file->writeUChar(&rgba_2[0]);
			file->writeUChar(&rgba_2[3]);
		}

		// UV Channel 1
		size_t uv_channels = vertex_flag / (0x10000);
		for (size_t i=0; i<uv_channels; i++) {
			uv[i].write(file, big_endian);
		}

		// Tangent / Binormal
		if (vertex_flag & 0x140) {
			tangent.write(file, big_endian);
			binormal.write(file, big_endian);
		}
			
	}

	void SonicVertexTable::read(File *file, XNFileMode file_mode, bool big_endian) {
		unsigned int table_count=0;
		size_t table_address=0;

		file->readInt32E(&table_count, big_endian);
		file->readInt32EA(&table_address, big_endian);

		if (table_count > 1) {
			printf("Unhandled Case Vertex Table %d.\n", table_count);
			getchar();
		}

		file->goToAddress(table_address);

		unsigned int vertex_count=0;
		size_t vertex_offset=0;

		file->readInt32E(&flag_1, big_endian);
		file->readInt32E(&flag_2, big_endian);
		file->readInt32E(&vertex_size, big_endian);
		file->readInt32E(&vertex_count, big_endian);
		file->readInt32EA(&vertex_offset, big_endian);


		unsigned int bone_table_count=0;
		size_t bone_table_offset=0;

		file->readInt32E(&bone_table_count, big_endian);
		file->readInt32EA(&bone_table_offset, big_endian);

		printf("%d %d %d\n", vertex_size, vertex_offset, bone_table_count);
		printf("Flags: %d %d\n", flag_1, flag_2);

		Error::addMessage(Error::WARNING, "Vertex Size / Vertex Flag 1 / Vertex Flag 2: " + ToString(vertex_size) + " / " + ToString(flag_1) + " / " + ToString(flag_2));

		if(bone_table_count > 32) {
			printf("Bone table is bigger than 32! Size: %d\n", bone_table_count);
			getchar();
		}

		string bone_table_str="Bone Blending Table: ";
		for (size_t i=0; i<bone_table_count; i++) {
			file->goToAddress(bone_table_offset + i*4);
			unsigned int bone=0;
			file->readInt32E(&bone, big_endian);
			bone_table.push_back(bone);

			bone_table_str += ToString(bone) + " ";
		}

		Error::addMessage(Error::WARNING, "Vertex Table with a bone blending table of size " + ToString(bone_table.size()));

		for (size_t i=0; i<vertex_count; i++) {
			file->goToAddress(vertex_offset + i * vertex_size);
			SonicVertex *vertex = new SonicVertex();
			vertex->read(file, vertex_size, big_endian, flag_1, file_mode);
			vertices.push_back(vertex);
		}

		printf("Done reading vertices...\n");
	}

	void SonicVertexTable::writeVertices(File *file, XNFileMode file_mode) {
		vertex_buffer_address = file->getCurrentAddress();

		for (size_t i=0; i<vertices.size(); i++) {
			vertices[i]->write(file, vertex_size, false, flag_1, file_mode);
		}
	}

	void SonicVertexTable::writeTable(File *file) {
		size_t bone_table_address = file->getCurrentAddress();
		for (size_t i=0; i<bone_table.size(); i++) {
			file->writeInt32(&bone_table[i]);
		}

		vertex_table_address = file->getCurrentAddress();

		file->writeInt32(&flag_1);
		file->writeInt32(&flag_2);
		file->writeInt32(&vertex_size);
		unsigned int vertex_count = vertices.size();
		file->writeInt32(&vertex_count);
		file->writeNull(4);

		unsigned int bone_table_count=bone_table.size();
		file->writeInt32(&bone_table_count);
		if (bone_table_count) file->writeInt32A(&bone_table_address);
		else file->writeNull(4);

		file->writeNull(20);
	}

	void SonicVertexTable::writeTableFixed(File *file) {
		file->goToAddress(vertex_table_address + 16);
		file->writeInt32A(&vertex_buffer_address);
	}

	void SonicVertexTable::write(File *file) {
		unsigned int total=1;
		file->writeInt32(&total);
		file->writeInt32A(&vertex_table_address);
	}

	void SonicVertexTable::setScale(float scale) {
		for (size_t i=0; i<vertices.size(); i++) {
			vertices[i]->setScale(scale);
		}
	}
};
