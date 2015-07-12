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
#include "S06XnFile.h"

namespace LibGens {
	void SonicXNObject::read(File *file) {
		SonicXNSection::read(file);

		size_t table_address=0;
		file->readInt32EA(&table_address, big_endian);
		file->readInt32E(&header_flag, big_endian);
		file->goToAddress(table_address);

		// Mesh Header
		center.read(file, big_endian);
		file->readFloat32E(&radius, big_endian);

		unsigned int material_parts_count=0;
		size_t material_parts_address=0;
		unsigned int vertex_parts_count=0;
		size_t vertex_parts_address=0;
		unsigned int index_parts_count=0;
		size_t index_parts_address=0;
		unsigned int bone_parts_count=0;
		size_t bone_set_address=0;
		unsigned int mesh_count=0;
		size_t mesh_address=0;

		file->readInt32E(&material_parts_count, big_endian);
		file->readInt32EA(&material_parts_address, big_endian);
		file->readInt32E(&vertex_parts_count, big_endian);
		file->readInt32EA(&vertex_parts_address, big_endian);
		file->readInt32E(&index_parts_count, big_endian);
		file->readInt32EA(&index_parts_address, big_endian);
		file->readInt32E(&bone_parts_count, big_endian);
		file->readInt32E(&bone_max_depth, big_endian);
		file->readInt32EA(&bone_set_address, big_endian);
		file->readInt32E(&bone_matrix_count, big_endian);
		file->readInt32E(&mesh_count, big_endian);
		file->readInt32EA(&mesh_address, big_endian);
		file->readInt32E(&total_texture_count, big_endian);

		if (file_mode == MODE_ZNO) {
			file->readInt32E(&type, big_endian);
			file->readInt32E(&version, big_endian);
			bounding_box.read(file, big_endian);
		}

		printf("Object Header Totals:\n Material Tables: %d\n Vertex Tables: %d\n Index Tables: %d\n Bone Tables: %d\n Meshes: %d\n\n", 
				material_parts_count, vertex_parts_count, index_parts_count, bone_parts_count, mesh_count);

		if (file_mode == MODE_GNO) {
			for (size_t i=0; i<material_parts_count; i++) {
				file->goToAddress(material_parts_address + i*8);

				SonicOldMaterialTable *old_material_table = new SonicOldMaterialTable();
				old_material_table->read(file, file_mode, big_endian);
				old_material_tables.push_back(old_material_table);
			}

			for (size_t i=0; i<vertex_parts_count; i++) {
				file->goToAddress(vertex_parts_address + i*8);
				SonicVertexResourceTable *vertex_resource_table = new SonicVertexResourceTable();
				vertex_resource_table->read(file, file_mode, big_endian);
				vertex_resource_tables.push_back(vertex_resource_table);
			}

			for (size_t i=0; i<index_parts_count; i++) {
				file->goToAddress(index_parts_address + i*8);
				SonicPolygonTable *polygon_table = new SonicPolygonTable();
				polygon_table->read(file, big_endian);
				polygon_tables.push_back(polygon_table);
			}
		}
		else {
			for (size_t i=0; i<material_parts_count; i++) {
				file->goToAddress(material_parts_address + i*8);

				printf("Material Table %d:\n", i);
				SonicMaterialTable *material_table = new SonicMaterialTable();
				material_table->read(file, file_mode, big_endian);
				material_tables.push_back(material_table);
				printf("\n", i);
			}

			for (size_t i=0; i<vertex_parts_count; i++) {
				file->goToAddress(vertex_parts_address + i*8);

				SonicVertexTable *vertex_table = new SonicVertexTable();
				vertex_table->read(file, file_mode, big_endian);
				vertex_tables.push_back(vertex_table);

				if (vertex_table->bone_table.size() == 0) {
					vertex_table->bone_table.push_back(i);
				}
			}

			for (size_t i=0; i<index_parts_count; i++) {
				file->goToAddress(index_parts_address + i*8);

				SonicIndexTable *index_table = new SonicIndexTable();
				index_table->read(file, big_endian);
				index_tables.push_back(index_table);
			}
		}

		for (size_t i=0; i<mesh_count; i++) {
			file->goToAddress(mesh_address + i*20);

			SonicMesh *mesh = new SonicMesh();
			mesh->read(file, big_endian, file_mode);
			meshes.push_back(mesh);
		}
		

		printf("%d\n", bone_parts_count);
		
		for (size_t i=0; i<bone_parts_count; i++) {
			if (file_mode == MODE_GNO) {
				file->goToAddress(bone_set_address + i*128);
			}
			else {
				file->goToAddress(bone_set_address + i*144);
			}
			

			if (bones_names) printf("Bone %s (%d):\n", bones_names->getName(i).c_str(), i);
			else printf("Bone (%d):\n", i);

			SonicBone *bone = new SonicBone();
			bone->read(file, big_endian, file_mode);
			bones.push_back(bone);

			if (bones_names) {
				Error::addMessage(Error::WARNING, bones_names->getName(i) + ": " + ToString(bone->matrix_index));
			}
			else {
				printf("   Matrix Index: %d\n", bone->matrix_index);
				printf("   Parent: %d\n", bone->parent_index);
				printf("   Child Index: %d\n", bone->child_index);
				printf("   Sibling Index: %d\n", bone->sibling_index);
			}
			printf("\n");
		}

		
	}

	void SonicXNObject::calculateMaxBoneDepth(size_t parent, size_t depth) {
		for (size_t i=0; i<bones.size(); i++) {
			if (bones[i]->parent_index == parent) {
				if ((depth+1) > bone_max_depth) bone_max_depth = depth+1;
				calculateMaxBoneDepth(i, depth+1);
			}
		}
	}

	void SonicXNObject::calculateBoneMatrixCount() {
		bone_matrix_count = 0;

		for (size_t i=0; i<bones.size(); i++) {
			if (bones[i]->matrix_index != 0xFFFF) bone_matrix_count++;
		}
	}

	void SonicXNObject::writeBody(File *file) {
		file->writeNull(24);
		
		unsigned int material_parts_count=material_tables.size();
		size_t material_parts_address=0;
		unsigned int vertex_parts_count=vertex_tables.size();
		size_t vertex_parts_address=0;
		unsigned int index_parts_count=index_tables.size();
		size_t index_parts_address=0;
		unsigned int bone_parts_count=bones.size();
		size_t bone_set_address=0;
		unsigned int mesh_count=meshes.size();
		size_t mesh_address=0;

		// Bones
		bone_set_address = file->getCurrentAddress();
		for (size_t i=0; i<bone_parts_count; i++) {
			bones[i]->write(file);
		}

		// Material Parts
		if (file_mode == MODE_ZNO) {
			for (size_t i=0; i<material_parts_count; i++) {
				bool found=false;
				for (int j=(int)i-1; j>=0; j--) {
					if (material_tables[i]->compareDataBlock1(material_tables[j], file_mode)) {
						material_tables[i]->data_block_1_address = material_tables[j]->data_block_1_address;
						found = true;
						break;
					}
				}
				if (!found) material_tables[i]->writeDataBlock1(file, file_mode);


				material_tables[i]->writeDataBlock2(file, file_mode);
				material_tables[i]->writeTextureUnits(file, file_mode);
				material_tables[i]->writeTable(file, file_mode);
			}
		}
		else {
			for (size_t i=0; i<material_parts_count; i++) {
				bool found=false;
				for (int j=(int)i-1; j>=0; j--) {
					if (material_tables[i]->compareDataBlock1(material_tables[j], file_mode)) {
						material_tables[i]->data_block_1_address = material_tables[j]->data_block_1_address;
						found = true;
						break;
					}
				}
				if (!found) material_tables[i]->writeDataBlock1(file, file_mode);
			}

			for (size_t i=0; i<material_parts_count; i++) {
				bool found=false;
				for (int j=(int)i-1; j>=0; j--) {
					if (material_tables[i]->compareDataBlock2(material_tables[j], file_mode)) {
						material_tables[i]->data_block_2_address = material_tables[j]->data_block_2_address;
						found = true;
						break;
					}
				}
				if (!found) material_tables[i]->writeDataBlock2(file, file_mode);
			}

			for (size_t i=0; i<material_parts_count; i++) {
				bool found=false;
				found = false;
				for (int j=(int)i-1; j>=0; j--) {
					if (material_tables[i]->compareTextureUnits(material_tables[j], file_mode)) {
						material_tables[i]->texture_units_address = material_tables[j]->texture_units_address;
						found = true;
						break;
					}
				}
				if (!found) material_tables[i]->writeTextureUnits(file, file_mode);
			}

			for (size_t i=0; i<material_parts_count; i++) {
				material_tables[i]->writeTable(file, file_mode);
			}
		}

		material_parts_address = file->getCurrentAddress();
		for (size_t i=0; i<material_parts_count; i++) {
			material_tables[i]->write(file, file_mode);
		}


		// Vertex Parts
		for (size_t i=0; i<vertex_tables.size(); i++) {
			vertex_tables[i]->writeTable(file);
		}

		vertex_parts_address = file->getCurrentAddress();
		for (size_t i=0; i<vertex_tables.size(); i++) {
			vertex_tables[i]->write(file);
		}


		// Index Parts
		for (size_t i=0; i<index_tables.size(); i++) {
			index_tables[i]->writeIndices(file);
		}

		for (size_t i=0; i<index_tables.size(); i++) {
			index_tables[i]->writeTable(file);
		}

		index_parts_address = file->getCurrentAddress();
		for (size_t i=0; i<index_tables.size(); i++) {
			index_tables[i]->write(file);
		}

		// Meshes
		for (size_t i=0; i<meshes.size(); i++) {
			meshes[i]->writeSubmeshes(file);
			meshes[i]->writeExtras(file);
		}

		mesh_address = file->getCurrentAddress();
		for (size_t i=0; i<meshes.size(); i++) {
			meshes[i]->write(file);
		}

		// Calculate bone parameters
		bone_max_depth = 0;
		if (bones.size()) {
			calculateMaxBoneDepth(0, 1);
		}
		calculateBoneMatrixCount();

		// Calculate texture parameters
		if (texture) total_texture_count = texture->getTextureUnitsSize();

		// Object Header
		size_t object_header_address=file->getCurrentAddress();
		center.write(file, false);
		file->writeFloat32(&radius);
		file->writeInt32(&material_parts_count);
		file->writeInt32A(&material_parts_address);
		file->writeInt32(&vertex_parts_count);
		file->writeInt32A(&vertex_parts_address);
		file->writeInt32(&index_parts_count);
		file->writeInt32A(&index_parts_address);
		file->writeInt32(&bone_parts_count);
		file->writeInt32(&bone_max_depth);
		file->writeInt32A(&bone_set_address);
		file->writeInt32(&bone_matrix_count);
		file->writeInt32(&mesh_count);
		file->writeInt32A(&mesh_address);
		file->writeInt32(&total_texture_count);

		if (file_mode == MODE_ZNO) {
			file->writeInt32(&type);
			file->writeInt32(&version);
			bounding_box.write(file, false);
		}

		// Vertex Buffer
		size_t vertex_buffer_address=file->getCurrentAddress();
		for (size_t i=0; i<vertex_tables.size(); i++) {
			vertex_tables[i]->writeVertices(file, file_mode);
		}

		unsigned int vertex_buffer_size = file->getCurrentAddress() - vertex_buffer_address;
		size_t bookmark=file->getCurrentAddress();

		file->goToAddress(head_address + 8);
		file->writeInt32A(&object_header_address, false);
		file->writeInt32(&header_flag);
		file->writeInt32(&vertex_buffer_size);
		file->writeInt32A(&vertex_buffer_address);

		for (size_t i=0; i<vertex_tables.size(); i++) {
			vertex_tables[i]->writeTableFixed(file);
		}

		file->goToAddress(bookmark);
	}

	bool SonicXNObject::getBoneIndexByName(string name_search, unsigned int &index) {
		for (size_t i=0; i<bones.size(); i++) {
			string bone_name=(bones_names ? bones_names->getName(i) : name+ToString(i));
			if (bone_name == name_search) {
				index = i;
				return true;
			}
		}

		return false;
	}

	void SonicXNObject::setScale(float scale) {
		for (size_t i=0; i<vertex_tables.size(); i++) {
			vertex_tables[i]->setScale(scale);
		}
	}

	void SonicXNObject::setBoneScale(unsigned short current_index, float scale, bool dont_scale) {
		if (current_index == 0xFFFF) return;

		SonicBone *bone=bones[current_index];
		if (!dont_scale) bone->setScale(scale);

		setBoneScale(bone->child_index, scale);
		setBoneScale(bone->sibling_index, scale);
	}

	void SonicXNObject::calculateSkinningMatrix(unsigned short current_index, LibGens::Matrix4 parent_matrix) {
		if (current_index == 0xFFFF) return;

		SonicBone *bone=bones[current_index];
		LibGens::Matrix4 result_matrix = parent_matrix * bone->current_matrix;

		bone->matrix = result_matrix;
		bone->matrix = bone->matrix.transpose();
		bone->matrix = bone->matrix.inverse();

		calculateSkinningMatrix(bone->child_index, result_matrix);
		calculateSkinningMatrix(bone->sibling_index, parent_matrix);
	}

	void SonicXNObject::calculateSkinningMatrices() {
		Matrix4 identity=Matrix4();
		calculateSkinningMatrix(0, identity);
	}

	void SonicXNObject::calculateSkinningIDs() {

	}
};
