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

#include "Material.h"
#include "Parameter.h"
#include "Texture.h"

namespace LibGens {
	const string Material::LayerOpaq = "opaq";
	const string Material::LayerTrans = "trans";
	const string Material::LayerPunch = "punch";

	Material::Material() {
		shader = "Common_d";
		sub_shader = "Common_d";
		no_culling = false;
		color_blend = false;
		extra="";
		gi_extra="";
		material_flag = 0x80;
		layer = LayerOpaq;
	}

	Material::Material(string filename) {
		File file(filename, LIBGENS_FILE_READ_BINARY);

		name = filename;
		folder = "";
		layer = LayerOpaq;

		size_t sep = name.find_last_of("\\/");
		if (sep != std::string::npos) {
			folder = filename.substr(0, sep+1);
			name = name.substr(sep + 1, name.size() - sep - 1);
		}
	
		size_t dot = name.find_last_of(".");
		if (dot != string::npos) {
			name = name.substr(0, dot);
		}

		extra="";
		gi_extra="";
		no_culling = false;
		color_blend = false;
		material_flag = 0x80;

		if (file.valid()) {
			file.readHeader();
			read(&file);
			file.close();
		}
	}


	void Material::save(string filename) {
		File file(filename, LIBGENS_FILE_WRITE_BINARY);

		if (file.valid()) {
			file.prepareHeader(LIBGENS_MATERIAL_ROOT_GENERATIONS);
			write(&file);
			file.writeHeader(true);
			file.close();
		}
	}

	void Material::read(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_MATERIAL_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		size_t header_address=file->getCurrentAddress();
		
		size_t shader_address=0;
		size_t sub_shader_address=0;
		size_t texset_address=0;
		size_t texture_address=0;
		size_t parameters_address=0;
		size_t address=0;

		file->readInt32BEA(&shader_address);
		file->readInt32BEA(&sub_shader_address);
		file->readInt32BEA(&texset_address);
		file->readInt32BEA(&texture_address);

		file->goToAddress(header_address+16);
		file->readUChar((unsigned char *) &material_flag);
		file->readUChar((unsigned char *) &no_culling);
		file->readUChar((unsigned char *) &color_blend);

		file->goToAddress(header_address+20);
		unsigned char parameter_count=0;
		file->readUChar(&parameter_count);

		file->goToAddress(header_address+23);
		unsigned char texture_count=0;
		file->readUChar(&texture_count);

		file->goToAddress(header_address+24);
		file->readInt32BEA(&parameters_address);

		file->goToAddress(shader_address);
		file->readString(&shader);

		file->goToAddress(sub_shader_address);
		file->readString(&sub_shader);

		// HACK: We read the layer string if it's available right after the sub-shader one.
		file->readString(&layer);

		for (size_t i=0; i<(size_t)parameter_count; i++) {
			file->goToAddress(parameters_address+i*4);
			file->readInt32BEA(&address);
			file->goToAddress(address);
			Parameter *parameter = new Parameter();
			parameter->read(file);
			parameters.push_back(parameter);
		}

		for (size_t i=0; i<(size_t)texture_count; i++) {
			string texset="";
			file->goToAddress(texset_address+i*4);
			file->readInt32BEA(&address);
			file->goToAddress(address);
			file->readString(&texset);

			file->goToAddress(texture_address+i*4);
			file->readInt32BEA(&address);
			file->goToAddress(address);
			Texture *texture=new Texture();
			texture->read(file, texset);
			textures.push_back(texture);
		}
	}


	void Material::write(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_MATERIAL_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}

		size_t header_address=file->getCurrentAddress();
		
		size_t shader_address=0;
		size_t sub_shader_address=0;
		size_t texset_address=0;
		size_t texture_address=0;
		size_t parameters_address=0;
		size_t address=0;

		file->writeNull(16);

		file->writeUChar(&material_flag);
		file->writeUChar((unsigned char *) &no_culling);
		file->writeUChar((unsigned char *) &color_blend);
		file->writeNull(1);

		unsigned char parameter_count=parameters.size();
		unsigned char texture_count=textures.size();
		file->writeUChar(&parameter_count);
		file->writeNull(2);
		file->writeUChar(&texture_count);

		file->writeNull(12);
		shader_address = file->getCurrentAddress();
		file->writeString(&shader);
		sub_shader_address = file->getCurrentAddress();
		file->writeString(&sub_shader);

		// HACK: We store our own layer information right after the sub shader string. Does not affect game at all.
		file->writeString(&layer);
		file->writeNull(1);

		file->fixPadding();

		// Parameters
		parameters_address = file->getCurrentAddress();
		file->writeNull(parameter_count * 4);
		vector<size_t> parameter_addresses;
		for (size_t i=0; i<(size_t)parameter_count; i++) {
			parameter_addresses.push_back(file->getCurrentAddress());
			parameters[i]->write(file);
		}
		file->fixPadding();
		
		for (size_t i=0; i<(size_t)parameter_count; i++) {
			file->goToAddress(parameters_address + i*4);
			file->writeInt32BEA(&parameter_addresses[i]);
		}
		file->goToEnd();

		// Texset
		texset_address = file->getCurrentAddress();
		file->writeNull(texture_count * 4);
		vector<size_t> texset_addresses;
		for (size_t i=0; i<(size_t)texture_count; i++) {
			texset_addresses.push_back(file->getCurrentAddress());
			file->writeString(&textures[i]->getTexset());
		}
		file->fixPadding();

		for (size_t i=0; i<(size_t)texture_count; i++) {
			file->goToAddress(texset_address + i*4);
			file->writeInt32BEA(&texset_addresses[i]);
		}
		file->goToEnd();

		// Textures
		texture_address = file->getCurrentAddress();
		file->writeNull(texture_count * 4);
		vector<size_t> texture_addresses;
		for (size_t i=0; i<(size_t)texture_count; i++) {
			texture_addresses.push_back(file->getCurrentAddress());
			textures[i]->write(file);
		}

		for (size_t i=0; i<(size_t)texture_count; i++) {
			file->goToAddress(texture_address + i*4);
			file->writeInt32BEA(&texture_addresses[i]);
		}

		file->goToAddress(header_address);
		file->writeInt32BEA(&shader_address);
		file->writeInt32BEA(&sub_shader_address);
		file->writeInt32BEA(&texset_address);
		file->writeInt32BEA(&texture_address);
		file->moveAddress(8);
		file->writeInt32BEA(&parameters_address);

		file->goToEnd();
	}

	void Material::setShader(string v) {
		shader = v;
		sub_shader = v;
	}

	string Material::getShader() {
		return shader;
	}

	void Material::setLayer(string v) {
		layer = v;
	}

	string Material::getLayer() {
		return layer;
	}

	Texture *Material::getTextureByUnit(string unit, size_t offset_count) {
		for (vector<Texture *>::iterator it=textures.begin(); it!=textures.end(); it++) {
			if ((*it)->getUnit() == unit) {
				if (offset_count) offset_count -= 1;
				else return (*it);
			}
		}

		return NULL;
	}

	Parameter *Material::getParameterByName(string unit) {
		for (vector<Parameter *>::iterator it=parameters.begin(); it!=parameters.end(); it++) {
			if ((*it)->getName() == unit) return (*it);
		}

		return NULL;
	}

	Parameter *Material::getParameterByIndex(size_t i) {
		if (i < parameters.size()) {
			return parameters[i];
		}

		return NULL;
	}

	vector<Parameter *> Material::getParameters() {
		return parameters;
	}

	vector<Texture *> Material::getTextureUnits() {
		return textures;
	}

	int Material::getTextureUnitsSize() {
		return textures.size();
	}

	void Material::addTextureUnit(Texture *texture) {
		textures.push_back(texture);
	}

	void Material::addParameter(Parameter *parameter) {
		parameters.push_back(parameter);
	}

	void Material::setParameter(string parameter_name, Color color) {
		Parameter *parameter = getParameterByName(parameter_name);
		if (parameter) {
			parameter->color = color;
		}
		else {
			parameter = new Parameter(parameter_name, color);
			parameters.push_back(parameter);
		}
	}

	void Material::setName(string v) {
		name = v;
	}

	string Material::getName() {
		return name;
	}

	string Material::getFolder() {
		return folder;
	}

	void Material::setExtra(string v) {
		extra=v;
	}

	string Material::getExtra() {
		return extra;
	}

	bool Material::hasExtra() {
		return (extra.size() > 0);
	}

	void Material::setExtraGI(string v) {
		gi_extra=v;
	}

	string Material::getExtraGI() {
		return gi_extra;
	}

	bool Material::hasExtraGI() {
		return (gi_extra.size() > 0);
	}

	bool Material::hasNoCulling() {
		return no_culling;
	}

	bool Material::hasColorBlend() {
		return color_blend;
	}

	void Material::setNoCulling(bool v) {
		no_culling = v;
	}

	void Material::setColorBlend(bool v) {
		color_blend = v;
	}
};