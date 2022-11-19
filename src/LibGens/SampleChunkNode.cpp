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

#include "SampleChunkNode.h"
#include "Model.h"
#include "Material.h"

namespace LibGens {
	SampleChunkNode::SampleChunkNode() {
		name = "";
		value = 0;
		data = NULL;
		data_address = 0;
		nodes.clear();
	}

	SampleChunkNode::SampleChunkNode(string name_p, unsigned int value_p) {
		name = name_p;
		value = value_p;
		data = NULL;
		data_address = 0;
		nodes.clear();
	}

	SampleChunkNode::~SampleChunkNode() {
		for (vector<SampleChunkNode *>::iterator it = nodes.begin(); it != nodes.end(); it++) {
			delete (*it);
		}

		nodes.clear();
	}

	bool SampleChunkNode::read(File *file) {
		size_t header_address = file->getCurrentAddress();

		unsigned int size = 0;
		file->readInt32BE(&size);
		file->readInt32BE(&value);
		file->readString(&name, 8);

		int space_index = name.find_last_not_of(" ");
		name = name.substr(0, space_index + 1);

		unsigned int flags = size >> 29;
		size &= 0x1FFFFFFF;

		if (~flags & 1) {
			while (true) {
				SampleChunkNode *sample_chunk_node = new SampleChunkNode();
				bool last_node = sample_chunk_node->read(file);
				nodes.push_back(sample_chunk_node);
				
				if (last_node) {
					break;
				}
			}
		}

		data_address = file->getCurrentAddress();
		file->goToAddress(header_address + size);
		return flags & 2;
	}

	void SampleChunkNode::write(File *file, bool last_node) {
		size_t header_address = file->getCurrentAddress();

		file->writeNull(4);
		file->writeInt32BE(&value);

		unsigned int name_size = name.size();
		if (name_size > 8) {
			name_size = 8;
		}

		file->write((char *)name.c_str(), name_size);
		if (name_size < 8) {
			unsigned char filler = 32;
			unsigned int filler_count = 8 - name_size;
			for (unsigned int i = 0; i < filler_count; i++) {
				file->writeUChar(&filler);
			}
		}

		unsigned int flags = last_node ? 2 : 0;
		if (nodes.size()) {
			for (unsigned int i = 0; i < nodes.size(); i++) {
				nodes[i]->write(file, i == (nodes.size() - 1));
				file->fixPadding(16);
			}
		}

		else {
			flags |= 1;
		}

		if (data) {
			file->setRootNodeType(value);

			switch (value) {
				case LIBGENS_MODEL_ROOT_DYNAMIC_GENERATIONS:
					{
						Model *model = (Model *)data;
						model->write(file);
						break;
					}

				case LIBGENS_MATERIAL_ROOT_GENERATIONS:
					{
						Material *material = (Material *)data;
						material->writeRootNodeGenerations(file);
						break;
					}
			}

			file->setRootNodeType(LIBGENS_FILE_HEADER_ROOT_TYPE_LOST_WORLD);
		}

		size_t end_address = file->getCurrentAddress();
		file->goToAddress(header_address);
		unsigned int size = (end_address - header_address) | (flags << 29);
		file->writeInt32BE(&size);
		file->goToAddress(end_address);
	}

	string SampleChunkNode::getName() {
		return name;
	}

	unsigned int SampleChunkNode::getValue() {
		return value;
	}

	size_t SampleChunkNode::getDataAddress() {
		return data_address;
	}

	void SampleChunkNode::setName(string v) {
		name = v;
	}

	void SampleChunkNode::setValue(unsigned int v) {
		value = v;
	}

	void SampleChunkNode::setData(void *data_p, int data_type) {
		data = data_p;
		value = data_type;
	}

	vector<SampleChunkNode *> SampleChunkNode::getNodes() {
		return nodes;
	}

	void SampleChunkNode::addNode(SampleChunkNode *node) {
		nodes.push_back(node);
	}

	SampleChunkNode *SampleChunkNode::newNode(string name, unsigned int value) {
		SampleChunkNode *node = new SampleChunkNode(name, value);
		nodes.push_back(node);
		return node;
	}

	SampleChunkNode *SampleChunkNode::find(string name, bool search_children) {
		for (vector<SampleChunkNode *>::iterator it = nodes.begin(); it != nodes.end(); it++) {
			if ((*it)->getName() == name) {
				return (*it);
			}

			SampleChunkNode *node;
			if (search_children && (node = (*it)->find(name))) {
				return node;
			}
		}

		return NULL;
	}
}