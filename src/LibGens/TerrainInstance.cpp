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

		unsigned int identifiers_total=0;
		size_t identifiers_address=0;
		unsigned int faces_total=0;
		size_t faces_address=0;

		file->readInt32BE(&identifiers_total);
		file->readInt32BEA(&identifiers_address);
		file->readInt32BE(&faces_total);
		file->readInt32BEA(&faces_address);

		for (size_t i=0; i<identifiers_total; i++) {
			file->goToAddress(identifiers_address + i*4);
			unsigned int identifier=0;
			file->readInt32BE(&identifier);
			identifiers.push_back(identifier);
		}

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

		unsigned int identifiers_total=identifiers.size();
		size_t identifiers_address=0;
		unsigned int faces_total=faces.size();
		size_t faces_address=0;

		file->writeInt32BE(&identifiers_total);
		file->writeNull(4);
		file->writeInt32BE(&faces_total);
		file->writeNull(4);

		identifiers_address = file->getCurrentAddress();
		for (size_t i=0; i<identifiers_total; i++) {
			file->writeInt32BE(&identifiers[i]);
		}

		faces_address = file->getCurrentAddress();
		for (size_t i=0; i<faces_total; i++) {
			file->writeInt16BE(&faces[i]);
		}
		file->fixPadding();

		// Fix header
		file->goToAddress(header_address);
		file->moveAddress(4);
		file->writeInt32BEA(&identifiers_address);
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

		for (size_t i=0; i<element_count; i++) {
			file->goToAddress(element_table_address + i*4);

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
			file->goToAddress(header_address + slot*8);

			unsigned int submesh_count=0;
			size_t submesh_table_address=0;
			file->readInt32BE(&submesh_count);
			file->readInt32BEA(&submesh_table_address);

			for (size_t i=0; i<submesh_count; i++) {
				file->goToAddress(submesh_table_address + i*4);

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
			for (size_t i=0; i<instance_mesh_count; i++) {
				file->goToAddress(instance_mesh_address + i*4);

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

		if (!model) {
			Error::addMessage(Error::NULL_REFERENCE, ToString(LIBGENS_TERRAIN_INSTANCE_ERROR_MESSAGE_NULL_MODEL_FILE) + " Model Name: " + model_name + ", Instance Name: " + name);
			return;
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


	void TerrainInstance::createSamplePoints(list<VRMapSample *> *list, Bitmap *bitmap, float unit_size, float saturation_multiplier, float brightness_offset) {
		samples.clear();

		if (model) model->createSamplePoints(list, matrix, bitmap, unit_size, saturation_multiplier, brightness_offset);
		else return;

		// Build a lookup grid for all the samples in the instance
		// Check each sample to the ones in the same cell/around
		// if it finds one closer than unit_size, don't add this to the global list
		/*
		AABB sample_aabb;
		sample_aabb.reset();
		for (std::list<VRMapSample *>::iterator it=samples.begin(); it!=samples.end(); it++) {
			sample_aabb.addPoint((*it)->point);
		}
		sample_aabb.expand(unit_size);
		
		int w=(int)(sample_aabb.sizeX()/unit_size)+1;
		int h=(int)(sample_aabb.sizeY()/unit_size)+1;
		int d=(int)(sample_aabb.sizeZ()/unit_size)+1;

		grid.clear();
		grid.resize(w);
		for (int x=0; x<w; x++) {
			grid[x].resize(h);
			for (int y=0; y<h; y++) {
				grid[x][y].resize(d);
			}
		}

		for (std::list<VRMapSample *>::iterator it=samples.begin(); it!=samples.end(); it++) {
			int x=0;
			int y=0;
			int z=0;

			x = (int)(((*it)->point.x - sample_aabb.start.x) / unit_size);
			y = (int)(((*it)->point.y - sample_aabb.start.y) / unit_size);
			z = (int)(((*it)->point.z - sample_aabb.start.z) / unit_size);

			int sx = x - 1, sy = y - 1, sz = z - 1, fx = x + 1, fy = y + 1, fz = z + 1;

			if (sx < 0) sx = 0;
			if (sx > w-1) sx = w-1;
			if (fx < 0) fx = 0;
			if (fx > w-1) fx = w-1;

			if (sy < 0) sy = 0;
			if (sy > h-1) sy = h-1;
			if (fy < 0) fy = 0;
			if (fy > h-1) fy = h-1;

			if (sz < 0) sz = 0;
			if (sz > d-1) sz = d-1;
			if (fz < 0) fz = 0;
			if (fz > d-1) fz = d-1;

			bool can_add=true;

			for (int x=sx; x<=fx; x++) {
				for (int y=sy; y<=fy; y++) {
					for (int z=sz; z<=fz; z++) {
						for (size_t i=0; i<grid[x][y][z].size(); i++) {
							if (((*it)->point - grid[x][y][z][i]->point).length() < (unit_size)) {
								can_add = false;
								break;
							}
						}

						if (!can_add) break;
					}
					if (!can_add) break;
				}
				if (!can_add) break;
			}

			if (can_add) {
				grid[x][y][z].push_back(*it);
				list->push_back(*it);
			}
			else {
				delete *it;
			}
		}
		*/
		samples.clear();
		grid.clear();
		
	}


	TerrainInstance::~TerrainInstance() {
		for (vector<TerrainInstanceMesh *>::iterator it=meshes.begin(); it!=meshes.end(); it++) {
			delete (*it);
		}
		meshes.clear();
	}
};