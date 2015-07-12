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
	void SonicSubmesh::read(File *file, bool big_endian, XNFileMode file_mode) {
		center.read(file, big_endian);
		file->readFloat32E(&radius, big_endian);
		file->readInt32E(&node_index, big_endian);
		file->readInt32E(&matrix_index, big_endian);
		file->readInt32E(&material_index, big_endian);
		file->readInt32E(&vertex_index, big_endian);
		file->readInt32E(&indices_index, big_endian);

		if (file_mode != MODE_GNO) {
			file->readInt32E(&indices_index_2, big_endian);

			if (indices_index != indices_index_2) {
				printf("Unhandled case! Submesh Index 1 and 2 are different! (%d vs %d)", indices_index, indices_index_2);
				getchar();
			}
		}

		printf("Found submesh with:\n Position: %f %f %f Radius: %f\n Node Index: %d Matrix Index: %d Material Index: %d Vertex Index: %d\n Indices Index: %d Indices Index 2: %d\n", center.x, center.y, center.z, radius, node_index, matrix_index, material_index, vertex_index, indices_index, indices_index_2);

		Error::addMessage(Error::WARNING, "Submesh:");
		Error::addMessage(Error::WARNING, "  Node Index:     " + ToString(node_index));
		Error::addMessage(Error::WARNING, "  Matrix Index:   " + ToString(matrix_index));
		Error::addMessage(Error::WARNING, "  Material Index: " + ToString(material_index));
		Error::addMessage(Error::WARNING, "  Vertex Index:   " + ToString(vertex_index));
		Error::addMessage(Error::WARNING, "  Indices Index:  " + ToString(indices_index));
	}

	void SonicSubmesh::write(File *file) {
		center.write(file, false);
		file->writeFloat32(&radius);
		file->writeInt32(&node_index);
		file->writeInt32(&matrix_index);
		file->writeInt32(&material_index);
		file->writeInt32(&vertex_index);
		file->writeInt32(&indices_index);
		file->writeInt32(&indices_index_2);
	}

	void SonicMesh::read(File *file, bool big_endian, XNFileMode file_mode) {
		unsigned int submesh_count=0;
		size_t submesh_offset=0;
		unsigned int extra_count=0;
		size_t extra_offset=0;

		file->readInt32E(&flag, big_endian);
		file->readInt32E(&submesh_count, big_endian);
		file->readInt32EA(&submesh_offset, big_endian);
		file->readInt32E(&extra_count, big_endian);
		file->readInt32EA(&extra_offset, big_endian);

		Error::addMessage(Error::WARNING, "Mesh (" + ToString(flag) + ") found with " + ToString(submesh_count) + " submeshes.");

		for (size_t i=0; i<submesh_count; i++) {
			if (file_mode != MODE_GNO) {
				file->goToAddress(submesh_offset + i * 40);
			}
			else {
				file->goToAddress(submesh_offset + i * 36);
			}

			SonicSubmesh *submesh = new SonicSubmesh();
			submesh->read(file, big_endian, file_mode);
			submeshes.push_back(submesh);
		}

		printf("Found %d extras: ", extra_count);

		for (size_t i=0; i<extra_count; i++) {
			file->goToAddress(extra_offset + i*4);

			unsigned int extra=0;
			file->readInt32E(&extra, big_endian);
			extras.push_back(extra);

			printf("%d ", extra);
		}
		printf("\n", flag, submesh_count);
	}

	void SonicMesh::writeSubmeshes(File *file) {
		submesh_table_address = file->getCurrentAddress();
		for (size_t i=0; i<submeshes.size(); i++) {
			submeshes[i]->write(file);
		}
	}

	void SonicMesh::writeExtras(File *file) {
		extra_table_address = file->getCurrentAddress();
		for (size_t i=0; i<extras.size(); i++) {
			file->writeInt32(&extras[i]);
		}
	}

	void SonicMesh::write(File *file) {
		file->writeInt32(&flag);
		unsigned int submeshes_count = submeshes.size();
		unsigned int extras_count = extras.size();
		file->writeInt32(&submeshes_count);
		file->writeInt32A(&submesh_table_address);
		file->writeInt32(&extras_count);
		file->writeInt32A(&extra_table_address);
	}
	
};
