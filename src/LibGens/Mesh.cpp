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

#include "Mesh.h"
#include "Submesh.h"

namespace LibGens {
	Mesh::Mesh() {
		for (size_t slot=0; slot<LIBGENS_MODEL_SUBMESH_SLOTS; slot++) {
			submeshes[slot].clear();
		}
		aabb.reset();
		extra="";
	}

	Mesh::~Mesh() {
		for (size_t slot=0; slot<LIBGENS_MODEL_SUBMESH_SLOTS; slot++) {
			for (vector<Submesh *>::iterator it=submeshes[slot].begin(); it!=submeshes[slot].end(); it++) {
				delete (*it);
			}
			submeshes[slot].clear();
		}
	}

	Mesh::Mesh(Mesh *clone, LibGens::Matrix4 transform, float uv2_left, float uv2_right, float uv2_top, float uv2_bottom) {
		for (int slot=0; slot<LIBGENS_MODEL_SUBMESH_SLOTS; slot++) {
			for (int i = 0; i < clone->submeshes[slot].size(); i++) {
				Submesh *submesh = new Submesh(clone->submeshes[slot][i], transform, uv2_left, uv2_right, uv2_top, uv2_bottom);
				submeshes[slot].push_back(submesh);
			}
		}

		water_slot_string = clone->water_slot_string;
		buildAABB();
	}
	
	void Mesh::read(File *file) {
		size_t header_address=file->getCurrentAddress();

		for (int slot=0; slot<LIBGENS_MODEL_SUBMESH_SLOTS; slot++) {
			file->goToAddress(header_address + slot*8);

			// Submesh Table
			unsigned int submesh_count=0;
			file->readInt32BE(&submesh_count);
			size_t submesh_table_address=0;
			file->readInt32BEA(&submesh_table_address);

			if (slot == LIBGENS_MODEL_SUBMESH_SLOT_WATER) {
				if (submesh_count > 1) {
					printf("Water slot more than 1? Unhandled case.\n");
					getchar();
				}

				if (submesh_count == 0) break;

				size_t submesh_count_address=0;
				file->readInt32BEA(&submesh_count_address);

				size_t submesh_subtable_address=0;
				file->readInt32BEA(&submesh_subtable_address);

				size_t address=0;

				file->goToAddress(submesh_table_address);
				file->readInt32BEA(&address);
				file->goToAddress(address);
				file->readString(&water_slot_string);

				if ((water_slot_string != "water") && (water_slot_string != "transparentz") && (water_slot_string != "transparentz2") && (water_slot_string != "transparentafterpunch") && (water_slot_string != "opaqueblending")) {
					printf("A string different than water for the 4th slot? Report this model! %s\n", water_slot_string.c_str());
					getchar();
				}


				file->goToAddress(submesh_count_address);
				file->readInt32BEA(&address);
				file->goToAddress(address);
				file->readInt32BE(&submesh_count);

				file->goToAddress(submesh_subtable_address);
				file->readInt32BEA(&address);
				for (size_t i=0; i<submesh_count; i++) {
					size_t submesh_address=0;
					file->goToAddress(address + i*4);
					file->readInt32BEA(&submesh_address);
					file->goToAddress(submesh_address);

					Submesh *submesh = new Submesh();
					submesh->read(file);
					submesh->setMeshSlot(slot);
					submesh->buildAABB();
					submeshes[slot].push_back(submesh);
				}
			}
			else {
				for (size_t i=0; i<submesh_count; i++) {
					size_t submesh_address=0;
					file->goToAddress(submesh_table_address + i*4);
					file->readInt32BEA(&submesh_address);
					file->goToAddress(submesh_address);

					Submesh *submesh = new Submesh();
					submesh->read(file);
					submesh->setMeshSlot(slot);
					submesh->buildAABB();

					submeshes[slot].push_back(submesh);

					if (submesh->getBoneTable().size() > 25) {
						Error::addMessage(Error::WARNING, ToString(slot) + " - " + ToString(i) + " - Bone Table Size: " + ToString(submesh->getBoneTable().size()) + " Material Name: " + submesh->getMaterialName());
						//submeshes[slot].push_back(submesh);
					}

					if (submesh->getVerticesSize() >= 32768) {
						Error::addMessage(Error::WARNING, ToString(slot) + " - " + ToString(i) + " - Vertex Table Size: " + ToString(submesh->getVerticesSize()) + " Material Name: " + submesh->getMaterialName());
						//submeshes[slot].push_back(submesh);
					}
				}
			}
		}
	}

	void Mesh::write(File *file, bool unleashed2_mode) {		
		size_t header_address=file->getCurrentAddress();
		vector<unsigned int> slot_addresses;
		size_t water_slot_address=0;

		// Prepare address table
		int slots = LIBGENS_MODEL_SUBMESH_SLOTS;
		if (unleashed2_mode) {
			slots = LIBGENS_MODEL_SUBMESH_ROOT_SLOTS;
		}

		for (int slot=0; slot<slots; slot++) {
			unsigned int submesh_count=submeshes[slot].size();

			if (slot == LIBGENS_MODEL_SUBMESH_SLOT_WATER) {
				if (submesh_count==0) {
					file->writeNull(4);
					unsigned char filler=0xFF;
					for (size_t i=0; i<12; i++) {
						file->write(&filler, 1);
					}
					file->writeNull(4);
				}
				else {
					unsigned int water_count=1;
					file->writeInt32BE(&water_count);
					water_slot_address = file->getCurrentAddress();
					file->writeNull(16);
				}
			}
			else {
				file->writeInt32BE(&submesh_count);
				file->writeNull(4);
			}
		}

		if (unleashed2_mode) {
			file->writeNull(24);
		}

		// Write Submesh slots
		for (int slot=0; slot<LIBGENS_MODEL_SUBMESH_SLOTS; slot++) {
			vector<unsigned int> submesh_addresses;
			unsigned int submesh_count=submeshes[slot].size();

			if ((slot == LIBGENS_MODEL_SUBMESH_SLOT_WATER) && submesh_count) {
				size_t water_string_address=file->getCurrentAddress();
				size_t water_total_address=file->getCurrentAddress()+4;
				size_t water_table_address=file->getCurrentAddress()+8;

				file->goToAddress(water_slot_address);
				file->writeInt32BEA(&water_string_address);
				file->writeInt32BEA(&water_total_address);
				file->writeInt32BEA(&water_table_address);
				file->goToEnd();

				file->writeNull(12);

				size_t sub_water_string_address=file->getCurrentAddress();
				file->writeString(&water_slot_string);
				file->fixPadding();

				size_t sub_water_total_address=file->getCurrentAddress();
				unsigned int sub_water_total=submesh_count;
				file->writeInt32BE(&sub_water_total);

				size_t sub_water_table_address=file->getCurrentAddress();

				file->goToAddress(water_string_address);
				file->writeInt32BEA(&sub_water_string_address);
				file->goToAddress(water_total_address);
				file->writeInt32BEA(&sub_water_total_address);
				file->goToAddress(water_table_address);
				file->writeInt32BEA(&sub_water_table_address);

				file->goToEnd();
			}

			slot_addresses.push_back(file->getCurrentAddress());

			size_t submesh_table_address=file->getCurrentAddress();
			file->writeNull(submesh_count*4);

			for (size_t i=0; i<submesh_count; i++) {
				submesh_addresses.push_back(file->getCurrentAddress());
				submeshes[slot][i]->write(file);
			}

			for (size_t i=0; i<submesh_count; i++) {
				file->goToAddress(submesh_table_address + i*4);
				file->writeInt32BEA(&submesh_addresses[i]);
			}

			file->goToEnd();
		}


		// Fix address table
		for (int slot=0; slot<LIBGENS_MODEL_SUBMESH_ROOT_SLOTS; slot++) {
			file->goToAddress(header_address + 4 + slot*8);
			file->writeInt32BEA(&slot_addresses[slot]);
		}

		file->goToEnd();
	}

	void Mesh::buildAABB() {
		aabb.reset();

		for (size_t slot=0; slot<LIBGENS_MODEL_SUBMESH_SLOTS; slot++) {
			for (std::vector<Submesh *>::iterator it=submeshes[slot].begin(); it!=submeshes[slot].end(); it++) {
				aabb.merge((*it)->getAABB());
			}
		}
	}

	AABB Mesh::getAABB() {
		return aabb;
	}

	void Mesh::setExtra(string v) {
		extra=v;
	}

	string Mesh::getExtra() {
		return extra;
	}

	bool Mesh::hasExtra() {
		return (extra.size() > 0);
	}

	void Mesh::setWaterSlotString(string v) {
		water_slot_string=v;
	}

	std::vector<Submesh *> Mesh::getSubmeshes() {
		std::vector<Submesh *> ret;

		for (size_t slot=0; slot<LIBGENS_MODEL_SUBMESH_SLOTS; slot++) {
			for (std::vector<Submesh *>::iterator it=submeshes[slot].begin(); it!=submeshes[slot].end(); it++) {
				ret.push_back(*it);
			}
		}

		return ret;
	}

	std::vector<Submesh *> Mesh::getSubmeshes(size_t slot) {
		return submeshes[slot];
	}

	std::vector<Submesh *> *Mesh::getSubmeshSlots() {
		return submeshes;
	}

	
	list<Vertex *> Mesh::getVertexList() {
		list<Vertex *> vertices;
		list<Vertex *> submesh_vertices;

		for (size_t slot=0; slot<LIBGENS_MODEL_SUBMESH_SLOTS; slot++) {
			for (vector<Submesh *>::iterator it=submeshes[slot].begin(); it!=submeshes[slot].end(); it++) {
				submesh_vertices=(*it)->getVertexList();
	
				for (list<Vertex *>::iterator it_v=submesh_vertices.begin(); it_v!=submesh_vertices.end(); it_v++) {
					vertices.push_back(*it_v);
				}
			}
		}

		return vertices;
	}

	list<unsigned int> Mesh::getFaceList() {
		list<unsigned int> faces;
		list<unsigned int> submesh_faces;
		unsigned int face_offset=0;

		for (size_t slot=0; slot<LIBGENS_MODEL_SUBMESH_SLOTS; slot++) {
			for (vector<Submesh *>::iterator it=submeshes[slot].begin(); it!=submeshes[slot].end(); it++) {
				submesh_faces=(*it)->getFaceList();
	
				for (list<unsigned int>::iterator it_f=submesh_faces.begin(); it_f!=submesh_faces.end(); it_f++) {
					faces.push_back((*it_f) + face_offset);
				}

				face_offset += (*it)->getVertexList().size();
			}
		}

		return faces;
	}

	
	void Mesh::createSamplePoints(list<VRMapSample *> *list, Matrix4 &matrix, Bitmap *bitmap, float unit_size, float saturation_multiplier, float brightness_offset) {
		for (size_t slot=0; slot<LIBGENS_MODEL_SUBMESH_SLOTS; slot++) {
			for (std::vector<Submesh *>::iterator it=submeshes[slot].begin(); it!=submeshes[slot].end(); it++) {
				(*it)->createSamplePoints(list, matrix, bitmap, unit_size, saturation_multiplier, brightness_offset);
			}
		}
	}

	void Mesh::addSubmesh(Submesh *submesh, size_t slot) {
		if (slot < LIBGENS_MODEL_SUBMESH_SLOTS) {
			submeshes[slot].push_back(submesh);
		}
	}

	void Mesh::removeSubmesh(Submesh *submesh, size_t slot) {
		if (slot < LIBGENS_MODEL_SUBMESH_SLOTS) {
			for (std::vector<Submesh *>::iterator it=submeshes[slot].begin(); it!=submeshes[slot].end(); it++) {
				if (*it == submesh) {
					submeshes[slot].erase(it);
					break;
				}
			}
		}
	}
	
	list<string> Mesh::getMaterialNames() {
		list<string> material_names;

		for (size_t slot=0; slot<LIBGENS_MODEL_SUBMESH_SLOTS; slot++) {
			for (vector<Submesh *>::iterator it=submeshes[slot].begin(); it!=submeshes[slot].end(); it++) {
				string material_name = (*it)->getMaterialName();

				bool found=false;
				for (list<string>::iterator it_m=material_names.begin(); it_m!=material_names.end(); it_m++) {
					if ((*it_m) == material_name) {
						found = true;
						break;
					}
				}

				if (!found) material_names.push_back(material_name);
			}
		}

		return material_names;
	}

	vector<unsigned int> Mesh::getMaterialMappings(list<string> &material_names) {
		vector<unsigned int> material_mappings;

		for (size_t slot=0; slot<LIBGENS_MODEL_SUBMESH_SLOTS; slot++) {
			for (vector<Submesh *>::iterator it=submeshes[slot].begin(); it!=submeshes[slot].end(); it++) {
				string material_name = (*it)->getMaterialName();

				unsigned int material_map   = 0;
				unsigned int material_index = 0;
				for (list<string>::iterator it_m=material_names.begin(); it_m!=material_names.end(); it_m++) {
					if ((*it_m) == material_name) {
						material_map = material_index;
						break;
					}

					material_index++;
				}

				size_t faces_size = (*it)->getFaceList().size();
				for (size_t i=0; i<faces_size; i++) {
					material_mappings.push_back(material_map);
				}
			}
		}

		return material_mappings;
	}

	
	void Mesh::fixVertexFormatForPC() {
		for (size_t slot=0; slot<LIBGENS_MODEL_SUBMESH_SLOTS; slot++) {
			for (vector<Submesh *>::iterator it=submeshes[slot].begin(); it!=submeshes[slot].end(); it++) {
				(*it)->fixVertexFormatForPC();
			}
		}
	}
};