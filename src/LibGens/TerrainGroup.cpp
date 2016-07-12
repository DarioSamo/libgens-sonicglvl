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

#include "TerrainGroup.h"
#include "AR.h"
#include "Model.h"
#include "TerrainInstance.h"
#include "Vertex.h"

namespace LibGens {
	TerrainGroupInfo::TerrainGroupInfo(TerrainGroup *group) {
		filename = group->getName();
		subset_id = group->getSubsetID();
		folder_size = 0;

		AABB group_aabb;
		group_aabb.reset();

		list<TerrainInstance *> instances=group->getInstances();
		for (list<TerrainInstance *>::iterator it=instances.begin(); it!=instances.end(); it++) {
			AABB aabb=(*it)->getAABB();
			instance_centers.push_back(aabb.center());
			instance_radius.push_back(aabb.sizeMax() / 2.0f);
			group_aabb.merge(aabb);
		}

		center = group_aabb.center();
		radius = group_aabb.sizeMax() / 2.0f;
	}

	TerrainGroup::TerrainGroup(string group_filename, string filename_p, string terrain_folder_p) {
		filename=filename_p;
		terrain_folder=terrain_folder_p;
		name=group_filename;
		subset_id = 0;

		loaded = false;
	}

	void TerrainGroup::save(string filename_p) {
		filename = filename_p;
		File file(filename_p, LIBGENS_FILE_WRITE_BINARY);

		if (file.valid()) {
			file.prepareHeader(LIBGENS_TERRAIN_GROUP_ROOT_GENERATIONS);
			write(&file);
			file.writeHeader();
			file.close();
		}
	}

	void TerrainGroup::load() {
		File file(filename, LIBGENS_FILE_READ_BINARY);
		if (file.valid()) {
			loaded = true;
			file.readHeader();
			read(&file, terrain_folder);
			file.close();
		}
	}
	
	void TerrainGroup::read(File *file, string terrain_folder) {
		ArPack *ar_pack = NULL;
		File *ar_pack_file = NULL;

		if (terrain_folder.size()) {
			string ar_pack_filename=terrain_folder;
			ar_pack_filename.resize(ar_pack_filename.size()-1);
			ar_pack = new ArPack(ar_pack_filename, false);
			ar_pack_file = new File(ar_pack_filename, LIBGENS_FILE_READ_BINARY);
		}

		size_t header_address=file->getCurrentAddress();
		unsigned int instance_count=0;
		size_t instance_table_address=0;
		unsigned int model_count=0;
		size_t model_table_address=0;

		file->readInt32BE(&instance_count);
		file->readInt32BEA(&instance_table_address);
		file->readInt32BE(&model_count);
		file->readInt32BEA(&model_table_address);

		// Models
		for (size_t i=0; i<model_count; i++) {
			size_t model_address=0;
			file->goToAddress(model_table_address + i*4);
			file->readInt32BEA(&model_address);
			file->goToAddress(model_address);

			string model_name="";
			file->readString(&model_name);

			if (ar_pack && ar_pack_file) {
				ArFile *entry=ar_pack->getFile(model_name+LIBGENS_TERRAIN_MODEL_EXTENSION);
				if (entry) {
					ar_pack_file->setGlobalOffset(entry->getAbsoluteDataAddress());
					Model *model=new Model(ar_pack_file, true);
					model->setName(model_name);
					models.push_back(model);
				}
			}
			else {
				Model *model=new Model();
				model->setTerrainMode(true);
				model->setName(model_name);
				models.push_back(model);
			}
		}

		// Instances
		for (size_t i=0; i<instance_count; i++) {
			vector<TerrainInstance *> instances_sub;

			size_t instance_address=0;
			file->goToAddress(instance_table_address + i*4);
			file->readInt32BEA(&instance_address);
			file->goToAddress(instance_address);

			unsigned int filename_count=0;
			size_t filename_offset_table=0;
			size_t sphere_address=0;

			file->readInt32BE(&filename_count);
			file->readInt32BEA(&filename_offset_table);
			file->readInt32BEA(&sphere_address);

			for (size_t j=0; j<filename_count; j++) {
				size_t instance_name_address=0;
				string instance_name="";

				file->goToAddress(filename_offset_table + j*4);
				file->readInt32BEA(&instance_name_address);

				file->goToAddress(instance_name_address);
				file->readString(&instance_name);

				if (ar_pack && ar_pack_file) {
					ArFile *entry=ar_pack->getFile(instance_name+LIBGENS_TERRAIN_INSTANCE_EXTENSION);
					if (entry) {
						ar_pack_file->setGlobalOffset(entry->getAbsoluteDataAddress());
						TerrainInstance *instance=new TerrainInstance(ar_pack_file, &models);
						instances_sub.push_back(instance);
					}
				}
				else {
					TerrainInstance *instance=new TerrainInstance(instance_name, NULL, Matrix4());
					instances_sub.push_back(instance);
				}
			}

			file->goToAddress(sphere_address);
			Vector3 instance_center;
			float instance_rad;
			instance_center.read(file);
			file->readFloat32BE(&instance_rad);
			instance_centers.push_back(instance_center);
			instance_radius.push_back(instance_rad);

			instances.push_back(instances_sub);
		}

		if (ar_pack_file) {
			ar_pack_file->close();
			delete ar_pack_file;
		}

		if (ar_pack) {
			delete ar_pack;
		}
	}

	void TerrainGroup::addFakeModel(std::string name) {
		Model *model=new Model();
		model->setTerrainMode(true);
		model->setName(name);
		models.push_back(model);
	}
	
	void TerrainGroup::write(File *file) {
		size_t header_address=file->getCurrentAddress();
		unsigned int instance_count=instances.size();
		size_t instance_table_address=0;
		unsigned int model_count=models.size();
		size_t model_table_address=0;

		file->writeInt32BE(&instance_count);
		file->writeNull(4);
		file->writeInt32BE(&model_count);
		file->writeNull(4);

		// Instances
		instance_table_address = file->getCurrentAddress();
		vector<size_t> instance_addresses;
		file->writeNull(instance_count * 4);

		for (size_t i=0; i<instance_count; i++) {
			instance_addresses.push_back(file->getCurrentAddress());

			size_t table_address=file->getCurrentAddress();
			unsigned int filename_count=instances[i].size();
			size_t filename_offset_table=0;
			size_t sphere_offset_table=0;

			file->writeInt32BE(&filename_count);
			file->writeNull(8);

			// Filenames
			filename_offset_table = file->getCurrentAddress();
			vector<size_t> filename_addresses;
			file->writeNull(filename_count * 4);
			for (size_t j=0; j<filename_count; j++) {
				filename_addresses.push_back(file->getCurrentAddress());
				file->writeString(&instances[i][j]->getName());
				file->fixPadding();
			}

			for (size_t j=0; j<filename_count; j++) {
				file->goToAddress(filename_offset_table + j*4);
				file->writeInt32BEA(&filename_addresses[j]);
			}
			file->goToEnd();

			// Centers
			sphere_offset_table = file->getCurrentAddress();
			instance_centers[i].write(file);
			file->writeFloat32BE(&instance_radius[i]);

			// Fix Header
			file->goToAddress(table_address+4);
			file->writeInt32BEA(&filename_offset_table);
			file->writeInt32BEA(&sphere_offset_table);
			file->goToEnd();
		}


		for (size_t i=0; i<instance_count; i++) {
			file->goToAddress(instance_table_address + i*4);
			file->writeInt32BEA(&instance_addresses[i]);
		}
		file->goToEnd();


		// Models
		model_table_address = file->getCurrentAddress();
		vector<size_t> model_addresses;
		file->writeNull(model_count * 4);

		for (size_t i=0; i<model_count; i++) {
			model_addresses.push_back(file->getCurrentAddress());
			file->writeString(&models[i]->getName());
			file->fixPadding();
		}

		for (size_t i=0; i<model_count; i++) {
			file->goToAddress(model_table_address + i*4);
			file->writeInt32BEA(&model_addresses[i]);
		}

		// Header
		file->goToAddress(header_address + 4);
		file->writeInt32BEA(&instance_table_address);
		file->moveAddress(4);
		file->writeInt32BEA(&model_table_address);

		file->goToEnd();
	}

	list<TerrainInstance *> TerrainGroup::getInstances() {
		list<TerrainInstance *> sub_instances;

		for (vector< vector<TerrainInstance *> >::iterator it=instances.begin(); it!=instances.end(); it++) {
			for (size_t c=0; c<(*it).size(); c++) {
				sub_instances.push_back((*it)[c]);
			}
		}

		return sub_instances;
	}

	vector< vector<TerrainInstance *> > TerrainGroup::getInstanceVectors() {
		return instances;
	}

	vector<float> TerrainGroup::getInstanceRadiuses() {
		return instance_radius;
	}

	vector<Vector3> TerrainGroup::getInstanceCenters() {
		return instance_centers;
	}

	bool TerrainGroup::checkDistance(Vector3 position_to_check, float extra_range) {
		current_distance = position_to_check.distance(center);
		return (current_distance < (radius + extra_range));
	}

	float TerrainGroup::getDistance(Vector3 position_to_check) {
		current_distance = position_to_check.distance(center);
		return current_distance;
	}


	void TerrainGroup::buildSpheres() {
		for (vector< vector<TerrainInstance *> >::iterator it=instances.begin(); it!=instances.end(); it++) {
			AABB instance_group_aabb;
			instance_group_aabb.reset();
			for (vector<TerrainInstance *>::iterator it2=(*it).begin(); it2!=(*it).end(); it2++) {
				instance_group_aabb.merge((*it2)->getAABB());
			}
			instance_centers.push_back(instance_group_aabb.center());
			instance_radius.push_back(instance_group_aabb.sizeMax() / 2.0f);
		}
	}


	void TerrainGroup::savePack(string filename_p) {
		ArPack *ar_pack = new ArPack();
		
		for (vector< vector<TerrainInstance *> >::iterator it=instances.begin(); it!=instances.end(); it++) {
			for (vector<TerrainInstance *>::iterator it2=(*it).begin(); it2!=(*it).end(); it2++) {
				(*it2)->save("temp.bin");
				ar_pack->addFile("temp.bin", (*it2)->getName() + LIBGENS_TERRAIN_INSTANCE_EXTENSION);
			}
		}

		for (vector<Model *>::iterator it=models.begin(); it!=models.end(); it++) {
			//(*it)->changeVertexFormat(LIBGENS_VERTEX_FORMAT_PC_TERRAIN);
			(*it)->save("temp.bin");
			ar_pack->addFile("temp.bin", (*it)->getName() + LIBGENS_TERRAIN_MODEL_EXTENSION);
		}

		ar_pack->save(filename_p);
		delete ar_pack;
	}


	TerrainGroup::~TerrainGroup() {
		unload();
	}

	void TerrainGroup::setSubsetID(int v) {
		subset_id = v;
	}

	int TerrainGroup::getSubsetID() {
		return subset_id;
	}

	void TerrainGroup::unload() {
		instance_centers.clear();
		instance_radius.clear();

		for (vector< vector<TerrainInstance *> >::iterator it=instances.begin(); it!=instances.end(); it++) {
			for (vector<TerrainInstance *>::iterator it2=(*it).begin(); it2!=(*it).end(); it2++) {
				delete (*it2);
			}
			(*it).clear();
		}
		instances.clear();

		for (vector<Model *>::iterator it=models.begin(); it!=models.end(); it++) {
			delete (*it);
		}
		models.clear();
	}
};