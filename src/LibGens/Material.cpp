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
#include "SampleChunkNode.h"
#include "SampleChunkProperty.h"

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
		alpha_threshold = 0x80;
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
		alpha_threshold = 0x80;
		root_node_type = LIBGENS_MATERIAL_ROOT_GENERATIONS;

		if (file.valid()) {
			file.readHeader();
			read(&file);
			file.close();
		}
	}

	Material::~Material() {
		for (vector<Texture *>::iterator it = textures.begin(); it != textures.end(); it++) {
			delete *it;
		}

		for (vector<Parameter *>::iterator it = parameters.begin(); it != parameters.end(); it++) {
			delete *it;
		}

		for (vector<SampleChunkProperty *>::iterator it = properties.begin(); it != properties.end(); it++) {
			delete *it;
		}
	}


	void Material::save(string filename, int root_type) {
		File file(filename, LIBGENS_FILE_WRITE_BINARY);

		if (file.valid()) {
			file.prepareHeader(root_type);
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

		root_node_type = file->getRootNodeType();

		switch (root_node_type) {
			case LIBGENS_MATERIAL_ROOT_GENERATIONS:
				readRootNodeGenerations(file);
				break;

			case LIBGENS_MATERIAL_ROOT_UNLEASHED:
				readRootNodeUnleashed(file);
				break;

			case LIBGENS_FILE_HEADER_ROOT_TYPE_LOST_WORLD:
				readRootNodeLostWorld(file);
				break;
		}
	}

	void Material::readRootNodeGenerations(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_MATERIAL_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		size_t header_address = file->getCurrentAddress();

		size_t shader_address = 0;
		size_t sub_shader_address = 0;
		size_t texset_address = 0;
		size_t texture_address = 0;
		size_t parameters_address = 0;
		size_t address = 0;

		file->readInt32BEA(&shader_address);
		file->readInt32BEA(&sub_shader_address);
		file->readInt32BEA(&texset_address);
		file->readInt32BEA(&texture_address);

		file->goToAddress(header_address + 16);
		file->readUChar((unsigned char *)&alpha_threshold);
		file->readUChar((unsigned char *)&no_culling);
		file->readUChar((unsigned char *)&color_blend);

		file->goToAddress(header_address + 20);
		unsigned char parameter_count = 0;
		file->readUChar(&parameter_count);

		file->goToAddress(header_address + 23);
		unsigned char texture_count = 0;
		file->readUChar(&texture_count);

		file->goToAddress(header_address + 24);
		file->readInt32BEA(&parameters_address);

		file->goToAddress(shader_address);
		file->readString(&shader);

		file->goToAddress(sub_shader_address);
		file->readString(&sub_shader);

		// HACK: We read the layer string if it's available right after the sub-shader one.
		file->readString(&layer);

		parameters.reserve(parameter_count);
		for (size_t i = 0; i<(size_t)parameter_count; i++) {
			file->goToAddress(parameters_address + i * file->getAddressSize());
			file->readInt32BEA(&address);
			file->goToAddress(address);
			Parameter *parameter = new Parameter();
			parameter->read(file);
			parameters.push_back(parameter);
		}

		textures.reserve(texture_count);
		for (size_t i = 0; i<(size_t)texture_count; i++) {
			string internal_name = "";
			file->goToAddress(texset_address + i * file->getAddressSize());
			file->readInt32BEA(&address);
			file->goToAddress(address);
			file->readString(&internal_name);

			file->goToAddress(texture_address + i * file->getAddressSize());
			file->readInt32BEA(&address);
			file->goToAddress(address);
			Texture *texture = new Texture();
			texture->read(file, internal_name);
			textures.push_back(texture);
		}
	}

	void Material::readRootNodeUnleashed(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_MATERIAL_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		size_t header_address = file->getCurrentAddress();

		size_t shader_address = 0;
		size_t sub_shader_address = 0;
		size_t texset_name_address = 0;
		string texset_name = "";
		size_t parameters_address = 0;
		size_t texture_count = 0;
		size_t texture_address = 0;
		size_t address = 0;

		file->readInt32BEA(&shader_address);
		file->readInt32BEA(&sub_shader_address);
		file->readInt32BEA(&texset_name_address);

		file->goToAddress(header_address + 16);
		file->readUChar((unsigned char *)&alpha_threshold);
		file->readUChar((unsigned char *)&no_culling);
		file->readUChar((unsigned char *)&color_blend);

		file->goToAddress(header_address + 20);
		unsigned char parameter_count = 0;
		file->readUChar(&parameter_count);

		file->goToAddress(header_address + 24);
		file->readInt32BEA(&parameters_address);

		file->goToAddress(shader_address);
		file->readString(&shader);

		file->goToAddress(sub_shader_address);
		file->readString(&sub_shader);

		file->goToAddress(texset_name_address);
		file->readString(&texset_name);

		// HACK: We read the layer string if it's available right after the texset one.
		file->readString(&layer);

		parameters.reserve(parameter_count);
		for (size_t i = 0; i<(size_t)parameter_count; i++) {
			file->goToAddress(parameters_address + i * file->getAddressSize());
			file->readInt32BEA(&address);
			file->goToAddress(address);
			Parameter *parameter = new Parameter();
			parameter->read(file);
			parameters.push_back(parameter);
		}

		// Look for the texset file in current directory.
		string folder = File::folderFromFilename(file->getPath());
		file = new File(folder + texset_name + LIBGENS_TEXSET_EXTENSION, LIBGENS_FILE_READ_BINARY);

		if (file->valid()) {
			file->readHeader();
			file->readInt32BE(&texture_count);
			file->readInt32BEA(&texture_address);

			textures.reserve(texture_count);
			for (size_t i = 0; i < texture_count; i++) {
				string internal_name = "";
				file->goToAddress(texture_address + i * file->getAddressSize());
				file->readInt32BEA(&address);
				file->goToAddress(address);
				file->readString(&internal_name);

				Texture *texture = new Texture(folder + internal_name + LIBGENS_TEXTURE_EXTENSION, internal_name);
				textures.push_back(texture);
			}
		}

		delete file;
	}

	void Material::readRootNodeLostWorld(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_MATERIAL_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		SampleChunkNode *root_node = new SampleChunkNode();
		root_node->read(file);

		SampleChunkNode *property_node = root_node->find("SCAParam", false);
		if (property_node) {
			vector<SampleChunkNode *> property_nodes = property_node->getNodes();
			for (vector<SampleChunkNode *>::iterator it = property_nodes.begin(); it != property_nodes.end(); it++) {
				properties.push_back(new SampleChunkProperty(*it));
			}
		}

		SampleChunkNode *contexts_node = root_node->find("Contexts", false);
		if (contexts_node) {
			file->goToAddress(contexts_node->getDataAddress());
			readRootNodeGenerations(file);
		}

		delete root_node;
	}

	void Material::write(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_MATERIAL_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}

		switch (file->getRootNodeType()) {
			case LIBGENS_MATERIAL_ROOT_GENERATIONS:
				writeRootNodeGenerations(file);
				break;

			case LIBGENS_MATERIAL_ROOT_UNLEASHED:
				writeRootNodeUnleashed(file);
				break;

			case LIBGENS_FILE_HEADER_ROOT_TYPE_LOST_WORLD:
				writeRootNodeLostWorld(file);
				break;
		}
	}

	void Material::writeRootNodeGenerations(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_MATERIAL_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}

		size_t header_address = file->getCurrentAddress();

		size_t shader_address = 0;
		size_t sub_shader_address = 0;
		size_t texset_address = 0;
		size_t texture_address = 0;
		size_t parameters_address = 0;
		size_t address = 0;

		file->writeNull(16);

		file->writeUChar(&alpha_threshold);
		file->writeUChar((unsigned char *)&no_culling);
		file->writeUChar((unsigned char *)&color_blend);
		file->writeNull(1);

		unsigned char parameter_count = parameters.size();
		unsigned char texture_count = textures.size();
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
		for (size_t i = 0; i<(size_t)parameter_count; i++) {
			parameter_addresses.push_back(file->getCurrentAddress());
			parameters[i]->write(file);
		}
		file->fixPadding();

		for (size_t i = 0; i<(size_t)parameter_count; i++) {
			file->goToAddress(parameters_address + i * 4);
			file->writeInt32BEA(&parameter_addresses[i]);
		}
		file->goToEnd();

		// Texset
		texset_address = file->getCurrentAddress();
		file->writeNull(texture_count * 4);
		vector<size_t> texset_addresses;
		for (size_t i = 0; i<(size_t)texture_count; i++) {
			texset_addresses.push_back(file->getCurrentAddress());
			file->writeString(&textures[i]->getTexset());
		}
		file->fixPadding();

		for (size_t i = 0; i<(size_t)texture_count; i++) {
			file->goToAddress(texset_address + i * 4);
			file->writeInt32BEA(&texset_addresses[i]);
		}
		file->goToEnd();

		// Textures
		texture_address = file->getCurrentAddress();
		file->writeNull(texture_count * 4);
		vector<size_t> texture_addresses;
		for (size_t i = 0; i<(size_t)texture_count; i++) {
			texture_addresses.push_back(file->getCurrentAddress());
			textures[i]->write(file);
		}

		for (size_t i = 0; i<(size_t)texture_count; i++) {
			file->goToAddress(texture_address + i * 4);
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

	void Material::writeRootNodeUnleashed(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_MATERIAL_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}

		size_t header_address = file->getCurrentAddress();

		size_t shader_address = 0;
		size_t sub_shader_address = 0;
		size_t texset_name_address = 0;
		size_t texture_address = 0;
		size_t parameters_address = 0;
		size_t address = 0;

		file->writeNull(16);

		file->writeUChar(&alpha_threshold);
		file->writeUChar((unsigned char *)&no_culling);
		file->writeUChar((unsigned char *)&color_blend);
		file->writeNull(1);

		unsigned char parameter_count = parameters.size();
		file->writeUChar(&parameter_count);
		file->writeNull(15);

		string folder = File::folderFromFilename(file->getPath());
		string texset_name = File::nameFromFilenameNoExtension(file->getPath());

		shader_address = file->getCurrentAddress();
		file->writeString(&shader);
		sub_shader_address = file->getCurrentAddress();
		file->writeString(&sub_shader);
		texset_name_address = file->getCurrentAddress();
		file->writeString(&texset_name);
		file->fixPadding();

		// Parameters
		parameters_address = file->getCurrentAddress();
		file->writeNull(parameter_count * 4);
		vector<size_t> parameter_addresses;
		for (size_t i = 0; i<(size_t)parameter_count; i++) {
			parameter_addresses.push_back(file->getCurrentAddress());
			parameters[i]->write(file);
		}
		file->fixPadding();

		for (size_t i = 0; i<(size_t)parameter_count; i++) {
			file->goToAddress(parameters_address + i * 4);
			file->writeInt32BEA(&parameter_addresses[i]);
		}
		file->goToEnd();

		file->goToAddress(header_address);
		file->writeInt32BEA(&shader_address);
		file->writeInt32BEA(&sub_shader_address);
		file->writeInt32BEA(&texset_name_address);
		file->moveAddress(12);
		file->writeInt32BEA(&parameters_address);

		file->goToEnd();

		// Save texset and textures.
		file = new File(folder + texset_name + LIBGENS_TEXSET_EXTENSION, LIBGENS_FILE_WRITE_BINARY);
		if (file->valid()) {
			file->prepareHeader(LIBGENS_TEXSET_ROOT_UNLEASHED);

			size_t header_address = file->getCurrentAddress();
			size_t texture_address = header_address + 8;
			size_t texture_count = textures.size();

			file->writeInt32BE(&texture_count);
			file->writeInt32BEA(&texture_address);

			file->goToAddress(texture_address);
			file->writeNull(texture_count * 4);

			vector<size_t> internal_name_addresses;
			for (unsigned int i = 0; i < textures.size(); i++) {
				internal_name_addresses.push_back(file->getCurrentAddress());
				file->writeString(&textures[i]->getTexset());
			}
			file->fixPadding();

			file->goToAddress(texture_address);
			for (unsigned int i = 0; i < textures.size(); i++) {
				file->writeInt32BEA(&internal_name_addresses[i]);
			}

			file->goToEnd();
			file->writeHeader(true);
			file->close();

			// Save textures
			for (unsigned int i = 0; i < textures.size(); i++) {
				textures[i]->save(folder + textures[i]->getTexset() + LIBGENS_TEXTURE_EXTENSION);
			}
		}

		delete file;
	}

	void Material::writeRootNodeLostWorld(File *file) {
		SampleChunkNode *root_node = new SampleChunkNode("Material", 1);

		if (properties.size()) {
			SampleChunkNode *property_node = root_node->newNode("SCAParam", 1);
			for (vector<SampleChunkProperty *>::iterator it = properties.begin(); it != properties.end(); it++) {
				property_node->addNode((*it)->toSampleChunkNode());
			}
		}

		SampleChunkNode *contexts_node = new SampleChunkNode();
		contexts_node->setName("Contexts");
		contexts_node->setData(this, LIBGENS_MATERIAL_ROOT_GENERATIONS);
		root_node->addNode(contexts_node);

		root_node->write(file, true);
		delete root_node;
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

	Texture* Material::getTextureByIndex(size_t i) {
		if (i < textures.size()) {
			return textures[i];
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

	void Material::removeAllParameters() {
		parameters.clear();
	}

	void Material::removeParameterByIndex(size_t i) {
		if (i > parameters.size())
			return;

		parameters.erase(parameters.begin() + i);
	}

	void Material::removeParameter(string name) {
		for (vector<Parameter*>::iterator it = parameters.begin(); it != parameters.end(); it++) {
			if ((*it)->getName() == name) {
				parameters.erase(it);
			}
		}
	}

	void Material::removeTextureUnit(string unit) {
		for (vector<Texture*>::iterator it = textures.begin(); it != textures.end(); it++) {
			if ((*it)->getUnit() == unit) {
				textures.erase(it);
			}
		}
	}

	void Material::removeTextureUnitByIndex(size_t i) {
		if (i > textures.size())
			return;

		textures.erase(textures.begin() + i);
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

	void Material::setPropertyValue(string name, unsigned int value) {
		if (name.size() > 8) name = name.substr(0, 8);

		for (vector<SampleChunkProperty *>::iterator it = properties.begin(); it != properties.end(); it++) {
			if ((*it)->getName() == name) {
				(*it)->setValue(value);
				return;
			}
		}

		properties.push_back(new SampleChunkProperty(name, value));
	}

	int Material::getRootNodeType() {
		return root_node_type;
	}

	unsigned char Material::getAlphaThreshold() {
		return alpha_threshold;
	}

	void Material::setNoCulling(bool v) {
		no_culling = v;
	}

	void Material::setColorBlend(bool v) {
		color_blend = v;
	}
};