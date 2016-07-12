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

#include "InstanceMTI.h"

namespace LibGens {
	InstanceBrushNode::InstanceBrushNode() {
		position = Vector3(0, 0, 0);
		index = 0;
		color = Color();
	}


	void InstanceBrushNode::read(File *file) {
		size_t header_address=file->getCurrentAddress();
		size_t address=0;

		position.read(file);
		file->readUChar(&index);
		file->goToAddress(file->getCurrentAddress()+7);
		color.readARGB8(file);
	}

	void InstanceBrushNode::write(File *file) {
		unsigned char node_header=LIBGENS_INSTANCE_MTI_NODE_HEADER;
		unsigned short unk=32767;
		position.write(file);
		file->writeUChar(&index);
		file->writeUChar(&node_header);

		file->writeNull(3);
		file->writeUChar(&node_header);
		file->writeUChar(&node_header);
		file->writeUChar(&node_header);
		color.writeARGB8(file);
	}

	InstanceBrush::InstanceBrush(string filename) {
		File file(filename, LIBGENS_FILE_READ_BINARY);

		if (file.valid()) {
			read(&file);
			file.close();
		}
	}

	void InstanceBrush::read(File *file) {
		size_t header_address=file->getCurrentAddress();
		size_t address=0;

		unsigned int instance_count=0;
		size_t instance_address=0;
		file->goToAddress(LIBGENS_INSTANCE_MTI_COUNT_ADDRESS);
		file->readInt32BE(&instance_count);

		file->goToAddress(LIBGENS_INSTANCE_MTI_TABLE_ADDRESS);
		file->readInt32BEA(&instance_address);

		for (size_t i=0; i<instance_count; i++) {
			file->goToAddress(instance_address + i*LIBGENS_INSTANCE_MTI_NODE_SIZE);
			InstanceBrushNode *node=new InstanceBrushNode();
			node->read(file);
			nodes.push_back(node);
		}
	}

	
	void InstanceBrush::save(string filename) {
		File file(filename, LIBGENS_FILE_WRITE_BINARY);

		if (file.valid()) {
			write(&file);
			file.close();
		}
	}

	void InstanceBrush::write(File *file) {
		unsigned int header=LIBGENS_INSTANCE_MTI_HEADER;
		unsigned int root=LIBGENS_INSTANCE_MTI_ROOT_TYPE;
		unsigned int total=nodes.size();
		unsigned int node_size=LIBGENS_INSTANCE_MTI_NODE_SIZE;
		unsigned int node_address=LIBGENS_INSTANCE_MTI_HEADER_NODE_ADDRESS;
		file->write(&header, 4);
		file->writeInt32BE(&root);
		file->writeInt32BE(&total);
		file->writeInt32BE(&node_size);
		file->writeNull(LIBGENS_INSTANCE_MTI_HEADER_NULL_SIZE);
		file->writeInt32BE(&node_address);

		for (list<InstanceBrushNode *>::iterator it=nodes.begin(); it!=nodes.end(); it++) {
			(*it)->write(file);
		}
	}
};