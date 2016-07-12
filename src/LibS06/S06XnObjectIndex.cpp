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

#include "LibGens.h"
#include <algorithm>
#include "S06XnFile.h"

namespace LibGens {
	void SonicIndexTable::read(File *file, bool big_endian) {
		unsigned int table_count=0;
		size_t table_address=0;

		file->readInt32E(&table_count, big_endian);
		file->readInt32EA(&table_address, big_endian);

		file->goToAddress(table_address);
		file->readInt32E(&flag, big_endian);

		printf("Reading Index Table with flag %d at %d\n", flag, table_address);

		// XNO and ZNO Index Table
		unsigned int index_count=0;
		unsigned int index_morph_count=0;
		size_t index_morph_address=0;
		size_t index_address=0;
		file->readInt32E(&index_count, big_endian);
		file->readInt32E(&index_morph_count, big_endian);
		file->readInt32EA(&index_morph_address, big_endian);
		file->readInt32EA(&index_address, big_endian);

		for (size_t i=0; i<index_morph_count; i++) {
			file->goToAddress(index_morph_address + i*2);
			unsigned short index=0;
			file->readInt16E(&index, big_endian);
			strip_sizes.push_back(index);
		}
		
		size_t additional_index=0;
		for (size_t m=0; m<strip_sizes.size(); m++) {
			unsigned short last_index_1=0;
			unsigned short last_index_2=0;
			unsigned short index=0;
			int count=0;

			for (size_t i=additional_index; i<additional_index+strip_sizes[m]; i++) {
				last_index_1 = last_index_2;
				last_index_2 = index;
	
				file->goToAddress(index_address + i*2);
				file->readInt16E(&index, big_endian);
				indices.push_back(index);
				count++;

				if ((index == last_index_1) || (index == last_index_2) || (last_index_1 == last_index_2)) {
					//Error::addMessage(Error::WARNING, "Invalid triangle found at " + ToString(file->getCurrentAddress()-6) + ". Strip of size " + ToString(strip_sizes[m]) + " starts at " + ToString(index_address + additional_index*2));
					continue;
				}
			
				if (count >= 3) {
					if (count%2==1) {
						indices_vector.push_back(Vector3(last_index_1, last_index_2, index));
					}
					else {
						indices_vector.push_back(Vector3(index, last_index_2, last_index_1));
					}
				}

				if (index == (unsigned short)0xFFFF) {
					printf("Unhandled case! Index with value 0xFFFF exists.\n");
					getchar();
				}
			}

			additional_index += strip_sizes[m];
		}
	}

	

	void SonicIndexTable::writeIndices(File *file) {
		strip_sizes_address_data = file->getCurrentAddress();
		for (size_t i=0; i<strip_sizes.size(); i++) {
			file->writeInt16(&strip_sizes[i]);
		}
		file->fixPadding(4);

		indices_address_data = file->getCurrentAddress();
		for (size_t i=0; i<indices.size(); i++) {
			file->writeInt16(&indices[i]);
		}

		file->fixPadding(4);
	}

	void SonicIndexTable::writeTable(File *file) {
		indices_table_address = file->getCurrentAddress();

		file->writeInt32(&flag);
		unsigned int indices_count=indices.size();
		unsigned int indices_morph_count=strip_sizes.size();
		file->writeInt32(&indices_count);
		file->writeInt32(&indices_morph_count);
		file->writeInt32A(&strip_sizes_address_data);
		file->writeInt32A(&indices_address_data);
		file->writeNull(12);
	}

	void SonicIndexTable::write(File *file) {
		unsigned int total=1;
		file->writeInt32(&total);
		file->writeInt32A(&indices_table_address);
	}
};
