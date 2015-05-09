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

#include "Model.h"
#include "Terrain.h"
#include "TerrainInstance.h"
#include "MaterialLibrary.h"
#include "TerrainGroup.h"
#include "VRMap.h"

namespace LibGens {
	TerrainAutodraw::TerrainAutodraw(string filename) {
		File file(filename, LIBGENS_FILE_READ_TEXT);

		size_t sep = filename.find_last_of("\\/");
		if (sep != std::string::npos) {
			folder = filename.substr(0, sep+1);
		}

		if (file.valid()) {
			read(&file);
			file.close();
		}
	}


	void TerrainAutodraw::read(File *file) {
		string line="";

		while (file->readLine(&line)) {
			printf("Autodraw: %s (%d)\n", line.c_str(), line.size());
			elements.push_back(line);
		}
	}

	Terrain::Terrain(string filename, string groups_folder, string resources_folder_p, string terrain_folder, string gia, bool load_groups) {
		File file(filename, LIBGENS_FILE_READ_BINARY);

		gia_folder = gia;
		resources_folder = resources_folder_p;
		stage_folder = terrain_folder;

		material_library = new MaterialLibrary(resources_folder);

		if (file.valid()) {
			file.readHeader();
			read(&file, groups_folder, terrain_folder, load_groups);
			file.close();
		}
	}

	void Terrain::save(string filename_p) {
		File file(filename_p, LIBGENS_FILE_WRITE_BINARY);

		if (file.valid()) {
			file.prepareHeader(LIBGENS_TERRAIN_ROOT_GENERATIONS);
			write(&file);
			file.writeHeader();
			file.close();
		}
	}

	void TerrainGroupInfo::read(File *file) {
		size_t header_address=file->getCurrentAddress();

		size_t position_address=0;
		size_t name_address=0;
		unsigned int instance_total=0;
		size_t instances_address=0;
		file->readInt32BEA(&position_address);
		file->readInt32BEA(&name_address);
		file->readInt32BE(&folder_size);
		file->readInt32BE(&instance_total);
		file->readInt32BEA(&instances_address);
		file->readInt32BE(&subset_id);

		file->goToAddress(position_address);
		center.read(file);
		file->readFloat32BE(&radius);

		file->goToAddress(name_address);
		file->readString(&filename);

		for (size_t i=0; i<instance_total; i++) {
			file->goToAddress(instances_address + i*4);
			size_t address=0;
			file->readInt32BEA(&address);
			file->goToAddress(address);
			
			Vector3 instance_center;
			float instance_rad;
			instance_center.read(file);
			file->readFloat32BE(&instance_rad);
			instance_centers.push_back(instance_center);
			instance_radius.push_back(instance_rad);
		}
	}

	void TerrainGroupInfo::write(File *file) {
		size_t header_address=file->getCurrentAddress();

		size_t position_address=0;
		size_t name_address=0;
		unsigned int instance_total=instance_centers.size();
		size_t instances_address=0;
		file->writeNull(8);
		file->writeInt32BE(&folder_size);
		file->writeInt32BE(&instance_total);
		file->writeNull(4);
		file->writeInt32BE(&subset_id);

		position_address = file->getCurrentAddress();
		center.write(file);
		file->writeFloat32BE(&radius);

		name_address = file->getCurrentAddress();
		file->writeString(&filename);
		file->fixPadding();

		instances_address = file->getCurrentAddress();
		vector<size_t> instance_addresses;
		file->writeNull(instance_total*4);

		for (size_t i=0; i<instance_total; i++) {
			instance_addresses.push_back(file->getCurrentAddress());
			instance_centers[i].write(file);
			file->writeFloat32BE(&instance_radius[i]);
		}

		for (size_t i=0; i<instance_total; i++) {
			file->goToAddress(instances_address + i*4);
			file->writeInt32BEA(&instance_addresses[i]);
		}

		file->goToAddress(header_address);
		file->writeInt32BEA(&position_address);
		file->writeInt32BEA(&name_address);
		file->moveAddress(8);
		file->writeInt32BEA(&instances_address);

		file->goToEnd();
	}

	
	void Terrain::read(File *file, string groups_folder, string terrain_folder, bool load_groups) {
		size_t header_address=file->getCurrentAddress();
		unsigned int group_count=0;
		size_t group_table_address=0;
		file->readInt32BE(&group_count);
		file->readInt32BEA(&group_table_address);

		for (size_t i=0; i<group_count; i++) {
			size_t group_address=0;
			file->goToAddress(group_table_address + i*4);
			file->readInt32BEA(&group_address);
			file->goToAddress(group_address);
			TerrainGroupInfo *group_info = new TerrainGroupInfo();
			group_info->read(file);
			groups_info.push_back(group_info);

			string group_filename = group_info->getName();
			TerrainGroup *group=new TerrainGroup(group_filename, groups_folder + group_filename + LIBGENS_TERRAIN_GROUP_EXTENSION, terrain_folder + group_filename + LIBGENS_TERRAIN_GROUP_FOLDER_EXTENSION + "/");
			if (load_groups) group->load();
			group->setCenter(group_info->getCenter());
			group->setRadius(group_info->getRadius());
			groups.push_back(group);
		}
	}


	void Terrain::write(File *file) {
		size_t header_address=file->getCurrentAddress();

		unsigned int group_count=groups.size();
		size_t group_table_address=32;

		file->writeInt32BE(&group_count);
		file->writeInt32BEA(&group_table_address);

		file->writeNull(group_count * 4);

		vector<size_t> group_addresses;
		for (size_t i=0; i<group_count; i++) {
			group_addresses.push_back(file->getCurrentAddress());
			groups_info[i]->write(file);
		}
		
		for (size_t i=0; i<group_count; i++) {
			file->goToAddress(group_table_address + i*4);
			file->writeInt32BEA(&group_addresses[i]);
		}

		file->goToEnd();
	}

	
	vector<TerrainGroup *> Terrain::getGroups() {
		return groups;
	}

	list<TerrainInstance *> Terrain::getInstances() {
		list<TerrainInstance *> instances;

		for (vector<TerrainGroup *>::iterator it=groups.begin(); it!=groups.end(); it++) {
			list<TerrainInstance *> group_instances=(*it)->getInstances();

			for (list<TerrainInstance *>::iterator it_i=group_instances.begin(); it_i!=group_instances.end(); it_i++) {
				instances.push_back(*it_i);
			}
		}

		return instances;
	}

	VRMap *Terrain::generateVRMap(float unit_size, float saturation_multiplier, float brightness_offset) {
		VRMap *map=new VRMap();
		list<VRMapSample *> *list=map->getSampleListPointer();

		for (std::vector<TerrainGroup *>::iterator it=groups.begin(); it!=groups.end(); it++) {
			std::list<TerrainInstance *> group_instances=(*it)->getInstances();

			for (std::list<TerrainInstance *>::iterator it_i=group_instances.begin(); it_i!=group_instances.end(); it_i++) {
				string map=(*it_i)->getName() + "_completemap";
				Bitmap *bitmap=Bitmap::getBitmap(map, gia_folder);
				(*it_i)->createSamplePoints(list, bitmap, unit_size, saturation_multiplier, brightness_offset);

				if (bitmap) Bitmap::deleteBitmap(bitmap);
			}
		}

		return map;
	}


	void Terrain::generateGroups(unsigned int cell_size) {
		if (!instances_to_organize.size()) return;

		TerrainGroup *group = new TerrainGroup();
		char tg_name[]="tg-0000";
		sprintf(tg_name, "tg-%04d", groups.size());
		group->setName(tg_name);

		AABB world_aabb;
		world_aabb.reset();
		
		// Add Instances and models
		vector<TerrainInstance *> instances;
		for (std::list<TerrainInstance *>::iterator it=instances_to_organize.begin(); it!=instances_to_organize.end(); it++) {
			world_aabb.merge((*it)->getAABB());
			instances.push_back(*it);
		}
		instances_to_organize.clear();

		group->addInstances(instances);
		for (std::list<Model *>::iterator it=models_to_organize.begin(); it!=models_to_organize.end(); it++) {
			group->addModel(*it);
		}
		models_to_organize.clear();

		group->buildSpheres();
		TerrainGroupInfo *group_info=new TerrainGroupInfo(group);

		groups_info.push_back(group_info);
		groups.push_back(group);
	}



	void Terrain::clean() {
		for (std::vector<TerrainGroup *>::iterator it=groups.begin(); it!=groups.end(); it++) {
			delete (*it);
		}
		groups.clear();

		for (std::vector<TerrainGroupInfo *>::iterator it=groups_info.begin(); it!=groups_info.end(); it++) {
			delete (*it);
		}

		for (std::list<TerrainInstance *>::iterator it=instances_to_organize.begin(); it!=instances_to_organize.end(); it++) {
			delete (*it);
		}
		instances_to_organize.clear();

		for (std::list<Model *>::iterator it=models_to_organize.begin(); it!=models_to_organize.end(); it++) {
			delete (*it);
		}
		models_to_organize.clear();


		groups_info.clear();
	}
};