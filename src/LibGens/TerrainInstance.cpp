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

#include "TerrainInstance.h"
#include "Model.h"
#include "Vertex.h"

namespace LibGens {
	TerrainInstance::TerrainInstance() {
		name = "";
		model = NULL;
		filename = "";
	}

	TerrainInstance::TerrainInstance(string name_p, Model *model_p, Matrix4 matrix_p) {
		name = name_p;
		model = model_p;
		matrix = matrix_p;
		if (model) model_name = model->getName();
		buildAABB();
	}

	TerrainInstance::TerrainInstance(string filename_p, string nm, vector<Model *> *models) {
		File file(filename_p, LIBGENS_FILE_READ_BINARY);
		filename=filename_p;
		model=NULL;
		name=nm;

		if (file.valid()) {
			file.readHeader();
			read(&file, models);
			buildAABB();
			file.close();
		}
	}

	TerrainInstance::TerrainInstance(File *file, vector<Model *> *models) {
		model=NULL;
		file->readHeader();
		read(file, models);
		buildAABB();
	}

	void TerrainInstance::save(string filename_p) {
		File file(filename_p, LIBGENS_FILE_WRITE_BINARY);

		if (file.valid()) {
			file.prepareHeader(LIBGENS_MODEL_ROOT_DYNAMIC_GENERATIONS);
			write(&file);
			file.writeHeader();
			file.close();
		}
	}

	
	void TerrainInstanceElement::read(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_TERRAIN_INSTANCE_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		unsigned int light_indices_total=0;
		size_t light_indices_address=0;
		unsigned int faces_total=0;
		size_t faces_address=0;

		file->readInt32BE(&light_indices_total);
		file->readInt32BEA(&light_indices_address);
		file->readInt32BE(&faces_total);
		file->readInt32BEA(&faces_address);

		light_indices.reserve(light_indices_total);
		for (size_t i=0; i<light_indices_total; i++) {
			file->goToAddress(light_indices_address + i * file->getAddressSize());
			unsigned int identifier=0;
			file->readInt32BE(&identifier);
			light_indices.push_back(identifier);
		}

		faces.reserve(faces_total);
		for (size_t i=0; i<faces_total; i++) {
			file->goToAddress(faces_address + i*2);
			unsigned short face=0;
			file->readInt16BE(&face);
			faces.push_back(face);
		}
	}

	void TerrainInstanceElement::write(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_TERRAIN_INSTANCE_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}

		size_t header_address=file->getCurrentAddress();

		unsigned int light_indices_total=light_indices.size();
		size_t light_indices_address=0;
		unsigned int faces_total=faces.size();
		size_t faces_address=0;

		file->writeInt32BE(&light_indices_total);
		file->writeNull(4);
		file->writeInt32BE(&faces_total);
		file->writeNull(4);

		light_indices_address = file->getCurrentAddress();
		for (size_t i=0; i<light_indices_total; i++) {
			file->writeInt32BE(&light_indices[i]);
		}

		faces_address = file->getCurrentAddress();
		for (size_t i=0; i<faces_total; i++) {
			file->writeInt16BE(&faces[i]);
		}
		file->fixPadding();

		// Fix header
		file->goToAddress(header_address);
		file->moveAddress(4);
		file->writeInt32BEA(&light_indices_address);
		file->moveAddress(4);
		file->writeInt32BEA(&faces_address);

		file->goToEnd();
	}
	
	void TerrainInstanceSubmesh::read(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_TERRAIN_INSTANCE_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		unsigned int element_count=0;
		size_t element_table_address=0;
		file->readInt32BE(&element_count);
		file->readInt32BEA(&element_table_address);

		elements.reserve(element_count);
		for (size_t i=0; i<element_count; i++) {
			file->goToAddress(element_table_address + i * file->getAddressSize());

			size_t element_address=0;
			file->readInt32BEA(&element_address);
			file->goToAddress(element_address);

			TerrainInstanceElement *element=new TerrainInstanceElement();
			element->read(file);
			elements.push_back(element);
		}
	}

	
	void TerrainInstanceSubmesh::write(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_TERRAIN_INSTANCE_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}

		size_t header_address=file->getCurrentAddress();

		unsigned int element_count=elements.size();
		size_t element_table_address=file->getCurrentAddress()+8;
		file->writeInt32BE(&element_count);
		file->writeInt32BEA(&element_table_address);

		vector<size_t> element_addresses;
		file->writeNull(element_count*4);

		// Elements
		for (size_t i=0; i<element_count; i++) {
			element_addresses.push_back(file->getCurrentAddress());
			elements[i]->write(file);
		}

		// Fix headers
		for (size_t i=0; i<element_count; i++) {
			file->goToAddress(element_table_address + i*4);
			file->writeInt32BEA(&element_addresses[i]);
		}


		file->goToEnd();
	}

	
	void TerrainInstanceMesh::read(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_TERRAIN_INSTANCE_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		size_t header_address=file->getCurrentAddress();
		for (size_t slot=0; slot<LIBGENS_MODEL_SUBMESH_ROOT_SLOTS; slot++) {
			file->goToAddress(header_address + slot * file->getAddressSize() * 2);

			unsigned int submesh_count=0;
			size_t submesh_table_address=0;
			file->readInt32BE(&submesh_count);
			file->readInt32BEA(&submesh_table_address);

			submeshes[slot].reserve(submesh_count);
			for (size_t i=0; i<submesh_count; i++) {
				file->goToAddress(submesh_table_address + i * file->getAddressSize());

				size_t submesh_address=0;
				file->readInt32BEA(&submesh_address);
				file->goToAddress(submesh_address);

				TerrainInstanceSubmesh *submesh=new TerrainInstanceSubmesh();
				submesh->read(file);
				submeshes[slot].push_back(submesh);
			}
		}
	}

	
	void TerrainInstanceMesh::write(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_TERRAIN_INSTANCE_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}

		size_t header_address=file->getCurrentAddress();
		vector<size_t> slot_addresses;

		// Prepare Table
		for (size_t slot=0; slot<LIBGENS_MODEL_SUBMESH_ROOT_SLOTS; slot++) {
			unsigned int submesh_count=submeshes[slot].size();
			file->writeInt32BE(&submesh_count);
			file->writeNull(4);
		}

		// Write subtables
		for (size_t slot=0; slot<LIBGENS_MODEL_SUBMESH_ROOT_SLOTS; slot++) {
			size_t slot_address=file->getCurrentAddress();
			slot_addresses.push_back(slot_address);

			vector<size_t> submesh_addresses;
			unsigned int submesh_count=submeshes[slot].size();

			file->writeNull(submesh_count*4);

			for (size_t i=0; i<submesh_count; i++) {
				submesh_addresses.push_back(file->getCurrentAddress());
				submeshes[slot][i]->write(file);
			}
			
			for (size_t i=0; i<submesh_count; i++) {
				file->goToAddress(slot_address + i*4);
				file->writeInt32BEA(&submesh_addresses[i]);
			}

			file->goToEnd();
		}

		// Fix table
		for (size_t slot=0; slot<LIBGENS_MODEL_SUBMESH_ROOT_SLOTS; slot++) {
			file->goToAddress(header_address + slot*8 + 4);
			file->writeInt32BEA(&slot_addresses[slot]);
		}
		

		file->goToEnd();
	}

	
	void TerrainInstance::read(File *file, vector<Model *> *models) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_TERRAIN_INSTANCE_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		size_t header_address=file->getCurrentAddress();

		size_t model_address=0;
		size_t matrix_address=0;
		size_t name_address=0;
		unsigned int instance_mesh_count=0;
		size_t instance_mesh_address=0;

		file->readInt32BEA(&model_address);
		file->readInt32BEA(&matrix_address);
		file->readInt32BEA(&name_address);

		if (file->getRootNodeType() == LIBGENS_MODEL_ROOT_DYNAMIC_GENERATIONS) {
			file->readInt32BE(&instance_mesh_count);
			file->readInt32BEA(&instance_mesh_address);
		}

		if (file->getRootNodeType() == LIBGENS_MODEL_ROOT_DYNAMIC_UNLEASHED) {
			TerrainInstanceMesh *instance_mesh=new TerrainInstanceMesh();
			instance_mesh->read(file);
			meshes.push_back(instance_mesh);
		}
		
		// Model Name
		file->goToAddress(model_address);
		file->readString(&model_name);

		// Matrix
		file->goToAddress(matrix_address);
		matrix.read(file);

		// Instance Name
		file->goToAddress(name_address);
		file->readString(&name);

		// Instance Mesh
		if (file->getRootNodeType() == LIBGENS_MODEL_ROOT_DYNAMIC_GENERATIONS) {
			meshes.reserve(instance_mesh_count);
			for (size_t i=0; i<instance_mesh_count; i++) {
				file->goToAddress(instance_mesh_address + i * file->getAddressSize());

				size_t mesh_address=0;
				file->readInt32BEA(&mesh_address);
				file->goToAddress(mesh_address);

				TerrainInstanceMesh *instance_mesh=new TerrainInstanceMesh();
				instance_mesh->read(file);
				meshes.push_back(instance_mesh);
			}
		}


		model=NULL;
		if (models) {
			for (vector<Model *>::iterator it=models->begin(); it!=models->end(); it++) {
				if ((*it)->getName() == model_name) {
					model=(*it);
					break;
				}
			}
		}
	}

	void TerrainInstance::write(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_TERRAIN_INSTANCE_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}

		size_t header_address=file->getCurrentAddress();

		size_t model_address=0;
		size_t matrix_address=0;
		size_t name_address=0;
		unsigned int instance_mesh_count=meshes.size();
		size_t instance_mesh_address=0;

		file->writeNull(12);
		file->writeInt32BE(&instance_mesh_count);
		file->writeNull(4);
		
		// Model Name
		model_address = file->getCurrentAddress();
		file->writeString(&model_name);
		file->fixPadding();

		// Matrix
		matrix_address = file->getCurrentAddress();
		matrix.write(file);

		// Instance Name
		name_address = file->getCurrentAddress();
		file->writeString(&name);
		file->fixPadding();

		
		// Instance Mesh
		instance_mesh_address = file->getCurrentAddress();
		vector<size_t> instance_mesh_addresses;

		file->writeNull(instance_mesh_count*4);

		for (size_t i=0; i<instance_mesh_count; i++) {
			instance_mesh_addresses.push_back(file->getCurrentAddress());
			meshes[i]->write(file);
		}

		file->goToAddress(instance_mesh_address);
		for (size_t i=0; i<instance_mesh_count; i++) {
			file->goToAddress(instance_mesh_address + i*4);
			file->writeInt32BEA(&instance_mesh_addresses[i]);
		}

		file->goToAddress(header_address);
		file->writeInt32BEA(&model_address);
		file->writeInt32BEA(&matrix_address);
		file->writeInt32BEA(&name_address);
		file->moveAddress(4);
		file->writeInt32BEA(&instance_mesh_address);
		file->goToEnd();
	}


	list<Vertex *> TerrainInstance::getVertexList() {
		list<Vertex *> vertices;
		if (!model) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_TERRAIN_INSTANCE_ERROR_MESSAGE_NULL_MODEL);
			return vertices;
		}

		return model->getVertexList();
	}

	void TerrainInstance::buildAABB() {
		list<Vertex *> vertices=getVertexList();
		aabb.reset();
		for (list<Vertex *>::iterator it=vertices.begin(); it!=vertices.end(); it++) {
			aabb.addPoint((*it)->getTPosition(matrix));
		}
		aabb.expand(LIBGENS_TERRAIN_INSTANCE_AABB_EXPANSION);
	}


	void TerrainInstance::setPosition(Vector3 v) {
		Vector3 position;
		Vector3 scale;
		Quaternion orientation;
		matrix.decomposition(position, scale, orientation);
		matrix.makeTransform(v, scale, orientation);
	}


	void TerrainInstance::setRotation(Quaternion v) {
		Vector3 position;
		Vector3 scale;
		Quaternion orientation;
		matrix.decomposition(position, scale, orientation);
		matrix.makeTransform(position, scale, v);
	}

	TerrainInstance::~TerrainInstance() {
		for (vector<TerrainInstanceMesh *>::iterator it=meshes.begin(); it!=meshes.end(); it++) {
			delete (*it);
		}
		meshes.clear();
	}
};