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
#include "S06Common.h"
#include "S06Text.h"

namespace LibGens {
	SonicText::SonicText(string filename) {
		File file(filename, LIBGENS_FILE_READ_BINARY);

		if (file.valid()) {
			read(&file);
			file.close();
		}
	}

	void SonicTextEntry::read(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_S06_TEXT_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		size_t header_address=file->getCurrentAddress();
		size_t id_address=0;
		size_t value_address=0;

		file->readInt32BEA(&id_address);
		file->readInt32BEA(&value_address);

		file->goToAddress(id_address);
		file->readString(&id);

		value = "";
		for (size_t i=0; i<65535; i++) {
			file->goToAddress(value_address + i*2 + 1);
			unsigned char c=0;
			file->readUChar(&c);
			if (c) value += c;
			else break;
		}
	}

	
	void SonicTextEntry::write(File *file, SonicStringTable *string_table) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_S06_TEXT_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}

		size_t header_address=file->getCurrentAddress();
		file_address = header_address;

		string_table->writeString(file, id);
		file->writeNull(8);
	}

	void SonicTextEntry::writeFixed(File *file) {
		file->goToAddress(file_address + 4);
		file->writeInt32BEA(&parameter_address);
	}

	void SonicTextEntry::writeValues(File *file) {
		parameter_address = file->getCurrentAddress();

		for (size_t i=0; i<value.size(); i++) {
			unsigned char c=value[i];
			file->writeNull(1);
			file->writeUChar(&c);
		}

		file->writeNull(2);
	}


	void SonicText::read(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_S06_TEXT_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		size_t header_address=file->getCurrentAddress();
		size_t address=0;

		
		unsigned int file_size=0;
		size_t banana_table_address=0;
		file->setRootNodeAddress(32);
		file->readInt32BE(&file_size);
		file->readInt32BEA(&banana_table_address);
		file->readInt32BE(&table_size);

		file->goToAddress(36);
		size_t name_address=0;
		file->readInt32BEA(&name_address);
		file->goToAddress(name_address);
		file->readString(&name);

		file->goToAddress(40);
		unsigned int entries_total=0;
		file->readInt32BE(&entries_total);

		for (size_t i=0; i<entries_total; i++) {
			file->goToAddress(44 + i*12);

			SonicTextEntry *entry=new SonicTextEntry();
			entry->read(file);
			entries.push_back(entry);
		}
		
		file->goToAddress(banana_table_address);
		table = new char[table_size];
		file->read(table, table_size);
	}

	
	void SonicText::save(string filename) {
		File file(filename, LIBGENS_FILE_WRITE_BINARY);

		if (file.valid()) {
			write(&file);
			file.close();
		}
	}

	void SonicText::write(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_S06_TEXT_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}

		file->setRootNodeAddress(32);

		file->writeNull(8);
		file->writeInt32BE(&table_size);
		file->writeNull(10);
		string header="1BBINA";
		file->writeString(&header);
		file->fixPadding(32);

		// Data
		header="WTXT";
		file->write((void *)header.c_str(), 4);

		string_table.writeString(file, name);
		unsigned int entries_total=entries.size();
		file->writeInt32BE(&entries_total);
		
		for (size_t i=0; i<entries_total; i++) entries[i]->write(file, &string_table);
		for (size_t i=0; i<entries_total; i++) entries[i]->writeValues(file);
		for (size_t i=0; i<entries_total; i++) entries[i]->writeFixed(file);
		file->goToEnd();

		string_table.write(file);
		file->goToEnd();

		// End
		file->fixPadding(16);
		size_t table_address=file->getCurrentAddress();
		file->write(table, table_size);
		unsigned int file_size=file->getFileSize();

		file->goToAddress(0);
		file->writeInt32BE(&file_size);
		file->writeInt32BEA(&table_address);
	}

};