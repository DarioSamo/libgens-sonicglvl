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

#include "TerrainBlock.h"

namespace LibGens {
	TerrainBlock::TerrainBlock(string filename) {
		File file(filename, LIBGENS_FILE_READ_BINARY);

		if (file.valid()) {
			file.readHeader();
			read(&file);
			file.close();
		}
	}


	void TerrainBlock::save(string filename) {
		File file(filename, LIBGENS_FILE_WRITE_BINARY);

		if (file.valid()) {
			file.prepareHeader(LIBGENS_TERRAIN_BLOCK_ROOT_GENERATIONS);
			write(&file);
			file.writeHeader();
			file.close();
		}
	}


	void TerrainBlockInstance::read(File *file) {
		size_t header_address=file->getCurrentAddress();

		file->readInt32BE(&type);
		file->readInt32BE(&indentifier_a);
		file->readInt32BE(&indentifier_b);

		size_t sphere_address=0;
		file->readInt32BEA(&sphere_address);
		file->goToAddress(sphere_address);
		center.read(file);
		file->readFloat32BE(&radius);
	}


	void TerrainBlockInstance::write(File *file) {
		size_t header_address=file->getCurrentAddress();

		file->writeInt32BE(&type);
		file->writeInt32BE(&indentifier_a);
		file->writeInt32BE(&indentifier_b);

		size_t sphere_address=file->getCurrentAddress()+4;
		file->writeInt32BEA(&sphere_address);
		center.write(file);
		file->writeFloat32BE(&radius);

		file->goToEnd();
	}

	
	void TerrainBlock::read(File *file) {
		size_t header_address=file->getCurrentAddress();

		unsigned int instance_count=0;
		size_t instance_table_address=0;
		file->readInt32BE(&instance_count);
		file->readInt32BEA(&instance_table_address);

		for (size_t i=0; i<instance_count; i++) {
			file->goToAddress(instance_table_address + i*4);
			size_t address=0;
			file->readInt32BEA(&address);
			file->goToAddress(address);

			TerrainBlockInstance *block=new TerrainBlockInstance();
			block->read(file);
			blocks.push_back(block);
		}
	}


	void TerrainBlock::write(File *file) {
		size_t header_address=file->getCurrentAddress();

		unsigned int instance_count=blocks.size();
		size_t instance_table_address=36;
		unsigned int instance_count_2=blocks.size()-1;
		file->writeInt32BE(&instance_count);
		file->writeInt32BEA(&instance_table_address);
		file->writeInt32BE(&instance_count_2);

		vector<size_t> instance_addresses;
		file->writeNull(instance_count*4);
		for (size_t i=0; i<instance_count; i++) {
			instance_addresses.push_back(file->getCurrentAddress());
			blocks[i]->write(file);
		}

		for (size_t i=0; i<instance_count; i++) {
			file->goToAddress(instance_table_address+i*4);
			file->writeInt32BEA(&instance_addresses[i]);
		}

		file->goToEnd();
	}
};