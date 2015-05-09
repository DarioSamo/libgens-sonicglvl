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

#include "Light.h"

namespace LibGens {
	Light::Light(string filename) {
		File file(filename, LIBGENS_FILE_READ_BINARY);

		if (file.valid()) {
			file.readHeader();
			read(&file);
			file.close();
		}
	}

	void Light::save(string filename) {
		File file(filename, LIBGENS_FILE_WRITE_BINARY);

		if (file.valid()) {
			file.prepareHeader(LIBGENS_LIGHT_ROOT_GENERATIONS);
			write(&file);
			file.writeHeader(true);
			file.close();
		}
	}

	void Light::read(File *file) {
		size_t header_address=file->getCurrentAddress();

		file->readInt32BE(&type);
		position.read(file);
		color.read(file);

		if (type == 1) {
			file->readInt32BE(&omni_attribute);
			file->moveAddress(8);
			file->readFloat32BE(&inner_range);
			file->readFloat32BE(&outer_range);
		}
	}


	void Light::write(File *file) {
		size_t header_address=file->getCurrentAddress();

		file->writeInt32BE(&type);
		position.write(file);
		color.write(file);

		if (type == 1) {
			file->writeInt32BE(&omni_attribute);
			file->writeNull(8);
			file->writeFloat32BE(&inner_range);
			file->writeFloat32BE(&outer_range);
		}

		file->goToEnd();
	}



	LightList::LightList(string filename) {
		File file(filename, LIBGENS_FILE_READ_BINARY);

		folder = "";
		size_t sep = filename.find_last_of("\\/");
		if (sep != std::string::npos) {
			folder = filename.substr(0, sep+1);
		}

		if (file.valid()) {
			file.readHeader();
			read(&file);
			file.close();
		}
	}

	void LightList::save(string filename) {
		File file(filename, LIBGENS_FILE_WRITE_BINARY);
		if (file.valid()) {
			file.prepareHeader(LIBGENS_LIGHT_LIST_ROOT_GENERATIONS);
			write(&file);
			file.writeHeader();
			file.close();
		}
	}

	void LightList::read(File *file) {
		size_t header_address=file->getCurrentAddress();

		unsigned int names_count=0;
		size_t names_address=0;
		file->readInt32BE(&names_count);
		file->readInt32BEA(&names_address);

		for (size_t i=0; i<names_count; i++) {
			size_t address=0;
			file->goToAddress(names_address+i*4);
			file->readInt32BEA(&address);
			file->goToAddress(address);

			string name="";
			file->readString(&name);
			names.push_back(name);

			string light_filename=folder + name + LIBGENS_LIGHT_EXTENSION;
			Light *light=new Light(light_filename);
			light->setName(name);
			lights.push_back(light);
		}
	}


	void LightList::write(File *file) {
		size_t header_address=file->getCurrentAddress();

		unsigned int names_count=names.size();
		size_t names_address=32;
		file->writeInt32BE(&names_count);
		file->writeInt32BEA(&names_address);

		vector<size_t> name_addresses;
		file->writeNull(names_count * 4);
		for (size_t i=0; i<names_count; i++) {
			name_addresses.push_back(file->getCurrentAddress());
			file->writeString(&names[i]);
			file->fixPadding();
		}

		for (size_t i=0; i<names_count; i++) {
			file->goToAddress(names_address + i*4);
			file->writeInt32BEA(&name_addresses[i]);
		}

		file->goToEnd();
	}

	Light *LightList::getLight(string name) {
		for (vector<Light *>::iterator it=lights.begin(); it!=lights.end(); it++) {
			if ((*it)->getName() == name) {
				return (*it);
			}
		}

		return NULL;
	}

	vector<Light *> LightList::getOmniLights() {
		vector<Light *> omni_lights;

		for (vector<Light *>::iterator it=lights.begin(); it!=lights.end(); it++) {
			if ((*it)->getType() == 1) {
				omni_lights.push_back(*it);
			}
		}

		return omni_lights;
	}
};