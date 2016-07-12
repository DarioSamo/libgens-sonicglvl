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

#include "File.h"
#include "Model.h"

namespace LibGens {
	File::File(string filename, string mode) {
		file_ptr=fopen(filename.c_str(), mode.c_str());

		path=filename;
		root_node_address=0;
		address_read_count = 0;
		global_offset = 0;

		if (!file_ptr) {
			Error::addMessage(Error::FILE_NOT_FOUND, LIBGENS_FILE_H_ERROR_READ_FILE_BEFORE + filename + LIBGENS_FILE_H_ERROR_READ_FILE_AFTER);
			return;
		}
	}

	int File::endOfFile() {
		if (file_ptr) {
			return feof(file_ptr);
		}
		else {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_FILE_H_ERROR_READ_FILE_NULL);
			return true;
		}
	}

	bool File::valid() {
		if (file_ptr) return true;
		else return false;
	}

	void File::close() {
		if (file_ptr) {
			fclose(file_ptr);
		}
		else {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_FILE_H_ERROR_FILE_CLOSING);
		}
	}

	bool File::readSafeCheck(void *dest) {
		if (!file_ptr) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_FILE_H_ERROR_READ_FILE_NULL);
			return false;
		}
		if (!dest) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_FILE_H_ERROR_READ_NULL);
			return false;
		}

		return true;
	}

	void File::read(void *dest, size_t sz) {
		if (!readSafeCheck(dest)) return;
		fread(dest, sz, 1, file_ptr);
	}

	void File::readInt16(unsigned short *dest) {
		if (!readSafeCheck(dest)) return;
		fread(dest, sizeof(short), 1, file_ptr);
	}
	
	void File::readInt16BE(unsigned short *dest) {
		if (!readSafeCheck(dest)) return;
		fread(dest, sizeof(short), 1, file_ptr);
		Endian::swap(*dest);
	}

	void File::readInt32(unsigned int *dest) {
		if (!readSafeCheck(dest)) return;
		fread(dest, sizeof(int), 1, file_ptr);
	}

	void File::readInt32(int *dest) {
		if (!readSafeCheck(dest)) return;
		fread(dest, sizeof(int), 1, file_ptr);
	}

	void File::readInt32A(size_t *dest) {
		if (!readSafeCheck(dest)) return;
		fread(dest, sizeof(size_t), 1, file_ptr);
		*dest += root_node_address;

		address_read_count++;
	}

	void File::readInt32BE(unsigned int *dest) {
		if (!readSafeCheck(dest)) return;
		fread(dest, sizeof(int), 1, file_ptr);
		Endian::swap(*dest);
	}

	void File::readInt32BE(int *dest) {
		if (!readSafeCheck(dest)) return;
		fread(dest, sizeof(int), 1, file_ptr);
		Endian::swap(*dest);
	}
	
	void File::readInt32BEA(size_t *dest) {
		if (!readSafeCheck(dest)) return;
		fread(dest, sizeof(size_t), 1, file_ptr);
		Endian::swap(*dest);
		*dest += root_node_address;

		address_read_count++;
	}

	void File::readFloat8(float *dest) {
		if (!readSafeCheck(dest)) return;
		unsigned char v=0;
		fread(&v, sizeof(char), 1, file_ptr);
		*dest = v / 256.0f;
	}

	void File::readFloat16(float *dest) {
		if (!readSafeCheck(dest)) return;
		
		unsigned short v=0;
		readInt16(&v);
		unsigned int f=half_to_float(v);
		*dest = *(float*)&(f);
	}

	void File::readFloat16BE(float *dest) {
		if (!readSafeCheck(dest)) return;
		
		unsigned short v=0;
		readInt16BE(&v);
		unsigned int f=half_to_float(v);
		*dest = *(float*)&(f);
	}

	void File::readFloat32(float *dest) {
		if (!readSafeCheck(dest)) return;
		fread(dest, sizeof(int), 1, file_ptr);
	}

	void File::readFloat32BE(float *dest) {
		if (!readSafeCheck(dest)) return;
		fread(dest, sizeof(int), 1, file_ptr);
		Endian::swap(*((unsigned int *) dest));
	}

	void File::readUChar(unsigned char *dest) {
		if (!readSafeCheck(dest)) return;
		fread(dest, sizeof(char), 1, file_ptr);
	}

	void File::readString(string *dest) {
		if (!readSafeCheck(dest)) return;
		char c=1;
		*dest = "";
		while (c && !endOfFile()) {
			fread(&c, sizeof(char), 1, file_ptr);
			if (c) *dest += c;
		}
	}

	bool File::readLine(string *dest) {
		char string_buffer[LIBGENS_FILE_STRING_BUFFER];
		if (fgets(string_buffer, LIBGENS_FILE_STRING_BUFFER, file_ptr)) {
			size_t ln = strlen(string_buffer) - 1;
			if (string_buffer[ln] == '\n') {
				string_buffer[ln] = '\0';
			}

			*dest = ToString(string_buffer);
			return true;
		}

		return false;
	}

	void File::readString(string *dest, size_t n) {
		if (!readSafeCheck(dest)) return;
		char c=1;
		*dest = "";
		for (size_t i=0; i<n; i++) {
			fread(&c, sizeof(char), 1, file_ptr);
			if (c) *dest += c;
		}
	}


	void File::readInt16E(unsigned short *dest, bool big_endian) {
		if (big_endian) readInt16BE(dest);
		else readInt16(dest);
	}

	void File::readInt32E(int *dest, bool big_endian) {
		if (big_endian) readInt32BE(dest);
		else readInt32(dest);
	}

	void File::readInt32E(unsigned int *dest, bool big_endian) {
		if (big_endian) readInt32BE(dest);
		else readInt32(dest);
	}

	void File::readInt32EA(size_t *dest, bool big_endian) {
		if (big_endian) readInt32BEA(dest);
		else readInt32A(dest);
	}

	void File::readFloat16E(float *dest, bool big_endian) {
		if (big_endian) readFloat16BE(dest);
		else readFloat16(dest);
	}

	void File::readFloat32E(float *dest, bool big_endian) {
		if (big_endian) readFloat32BE(dest);
		else readFloat32(dest);
	}

	void File::write(void *dest, size_t sz) {
		if (!readSafeCheck(dest)) return;
		fwrite(dest, sz, 1, file_ptr);
	}
	
	void File::writeString(const char *dest) {
		if (!readSafeCheck((void *) dest)) return;

		fwrite(dest, sizeof(char), strlen(dest), file_ptr);
	}

	void File::writeString(string *dest) {
		if (!readSafeCheck(dest)) return;

		if (!dest->size()) writeNull(1);
		else fwrite(dest->c_str(), sizeof(char), dest->size()+1, file_ptr);
	}

	void File::writeUChar(unsigned char *dest) {
		if (!readSafeCheck(dest)) return;

		fwrite(dest, sizeof(char), 1, file_ptr);
	}

	void File::writeInt16(unsigned short *dest) {
		if (!readSafeCheck(dest)) return;

		unsigned short target=*dest;
		fwrite(&target, sizeof(short), 1, file_ptr);
	}

	void File::writeInt16BE(unsigned short *dest) {
		if (!readSafeCheck(dest)) return;

		unsigned short target=*dest;
		Endian::swap(target);
		fwrite(&target, sizeof(short), 1, file_ptr);
	}

	void File::writeInt32(unsigned int *dest) {
		if (!readSafeCheck(dest)) return;

		fwrite(dest, sizeof(int), 1, file_ptr);
	}

	void File::writeInt32A(size_t *dest, bool add_to_table) {
		if (!readSafeCheck(dest)) return;

		if (add_to_table) final_address_table.push_back(getCurrentAddress()-root_node_address);

		unsigned int target=(*dest) - root_node_address;
		fwrite(&target, sizeof(int), 1, file_ptr);
	}

	void File::writeInt32BE(int *dest) {
		if (!readSafeCheck(dest)) return;

		int target=*dest;
		Endian::swap(target);
		fwrite(&target, sizeof(int), 1, file_ptr);
	}

	void File::writeInt32BE(unsigned int *dest) {
		if (!readSafeCheck(dest)) return;

		unsigned int target=*dest;
		Endian::swap(target);
		fwrite(&target, sizeof(int), 1, file_ptr);
	}

	void File::writeInt32BEA(size_t *dest) {
		if (!readSafeCheck(dest)) return;

		final_address_table.push_back(getCurrentAddress()-root_node_address);

		unsigned int target=(*dest) - root_node_address;
		Endian::swap(target);
		fwrite(&target, sizeof(int), 1, file_ptr);
	}

	void File::writeFloat8(float *dest) {
		if (!readSafeCheck(dest)) return;
		unsigned char v = (int)((*dest) * 256.0f);
		writeUChar(&v);
	}

	void File::writeFloat32(float *dest) {
		if (!readSafeCheck(dest)) return;
		fwrite(dest, sizeof(float), 1, file_ptr);
	}

	void File::writeFloat32BE(float *dest) {
		if (!readSafeCheck(dest)) return;

		float target=*dest;
		Endian::swap(*((unsigned int *) &target));
		fwrite(&target, sizeof(float), 1, file_ptr);
	}

	void File::writeFloat32E(float *dest, bool big_endian) {
		if (big_endian) writeFloat32BE(dest);
		else writeFloat32(dest);
	}

	void File::writeNull(size_t size) {
		unsigned char zero=0;
		for (size_t i=0; i<size; i++) {
			fwrite(&zero, sizeof(char), 1, file_ptr);
		}
	}

	size_t File::fixPadding(size_t multiple) {
		size_t address=getCurrentAddress();
		size_t extra=multiple - (address%multiple);

		if (extra == multiple) {
			extra = 0;
			return 0;
		}

		size_t zero=0;
		for (size_t c=0; c<extra; c++) fwrite(&zero, sizeof(char), 1, file_ptr);
		return extra;
	}

	size_t File::fixPaddingRead(size_t multiple) {
		size_t address=getCurrentAddress();
		size_t extra=multiple - (address%multiple);

		if (extra == multiple) {
			extra = 0;
			return 0;
		}
		goToAddress(address+extra);
		return extra;
	}

	size_t File::getFileSize() {
		if (!file_ptr) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_FILE_H_ERROR_FILE_HEADER);
			return 0;
		}

		size_t address=getCurrentAddress();
		fseek(file_ptr, 0, SEEK_END);
		size_t sz=ftell(file_ptr);
		goToAddress(address);
		return sz;
	}

	void File::readHeader() {
		if (!file_ptr) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_FILE_H_ERROR_FILE_HEADER);
			return;
		}

		unsigned char next_gen_check=0;
		readUChar(&next_gen_check);

		goToAddress(0);

		if (next_gen_check == 0x80) {
			// Read new header
			root_node_type = LIBGENS_MODEL_ROOT_DYNAMIC_LOST_WORLD;
			root_node_address = LIBGENS_FILE_HEADER_ROOT_ADDRESS_NEXT_GEN;

			unsigned short file_size = 0;
			goToAddress(2);
			readInt16BE(&file_size);

			unsigned int signature = 0;
			readInt32BE(&signature);

			unsigned int offset_table_size=0;
			size_t offset_table_address_absolute=0;
			readInt32BE(&offset_table_address_absolute);
			readInt32BE(&offset_table_size);

			// FIXME: Awful way to seek through the new type of Lost World header, needs to be done
			// in a better way.
			size_t max_section_scan = 32;
			for (size_t i=0; i<max_section_scan; i++) {
				unsigned short section_flag = 0;
				unsigned short section_address = 0;
				unsigned int section_value = 0;
				string section_name = "";
				readInt16BE(&section_flag);
				readInt16BE(&section_address);
				readInt32BE(&section_value);
				readString(&section_name, 8);

				if (section_name == "Contexts") {
					break;
				}
			}
		}
		else {
			fseek(file_ptr, LIBGENS_FILE_HEADER_ROOT_TYPE_ADDRESS+global_offset, SEEK_SET);
			readInt32BE(&root_node_type);

			fseek(file_ptr, LIBGENS_FILE_HEADER_ROOT_NODE_ADDRESS+global_offset, SEEK_SET);
			readInt32BE(&root_node_address);

			fseek(file_ptr, root_node_address+global_offset, SEEK_SET);
		}
	}

	void File::prepareHeader(int root_type, int root_offset) {
		root_node_type=root_type;
		root_node_address=root_offset;

		writeNull(LIBGENS_FILE_HEADER_ROOT_ADDRESS_DEFAULT);
	}

	void File::writeHeader(bool no_extra_foot) {
		if (!file_ptr) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_FILE_H_ERROR_FILE_HEADER);
			return;
		}

		sortAddressTable();

		unsigned int final_table_address=getFileSize()-root_node_address;
		unsigned int final_table_address_abs=getFileSize();
		unsigned int final_table_size=final_address_table.size();

		writeInt32BE(&final_table_size);
		for (list<size_t>::iterator it=final_address_table.begin(); it!=final_address_table.end(); it++) {
			writeInt32BE(&(*it));
		}

		if (!no_extra_foot) writeNull(4);

		unsigned int size=getFileSize();
		unsigned int size_foot=size-4;

		if (no_extra_foot) {
			size_foot = 0;
		}

		goToAddress(0);
		writeInt32BE(&size);
		writeInt32BE(&root_node_type);
		writeInt32BE(&final_table_address);
		writeInt32BE(&root_node_address);
		writeInt32BE(&final_table_address_abs);
		writeInt32BE(&size_foot);
	}

	size_t File::getCurrentAddress() {
		if (file_ptr) {
			return ftell(file_ptr)-global_offset;
		}
		else {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_FILE_H_ERROR_FILE_GET_ADDRESS);
			return 0;
		}
	}

	void File::goToAddress(size_t address) {
		if (file_ptr) {
			fseek(file_ptr, address+global_offset, SEEK_SET);
		}
		else {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_FILE_H_ERROR_FILE_BOOKMARK);
		}
	}

	void File::moveAddress(size_t address) {
		if (file_ptr) {
			fseek(file_ptr, address, SEEK_CUR);
		}
		else {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_FILE_H_ERROR_FILE_BOOKMARK);
		}
	}

	void File::clone(string dest) {
		File file(dest, LIBGENS_FILE_WRITE_BINARY);
		if (file.valid()) {
			size_t sz=getFileSize();
			char *data=new char[sz];

			read(data, sz);
			file.write(data, sz);

			delete [] data;
			file.close();
		}
	}

	bool File::compare(File *file) {
		if (!file) return false;
		if (getFileSize() != file->getFileSize()) return false;

		unsigned char source=0;
		unsigned char dest=0;

		goToAddress(0);
		file->goToAddress(0);

		while (!endOfFile()) {
			readUChar(&source);
			file->readUChar(&dest);

			if (source != dest) {
				return false;
			}
		}

		return true;
	}

	void File::sortAddressTable() {
		list<size_t> new_address_table;

		for (list<size_t>::iterator it=final_address_table.begin(); it!=final_address_table.end(); it++) {
			bool found=false;

			for (list<size_t>::iterator it2=new_address_table.begin(); it2!=new_address_table.end(); it2++) {
				if ((*it) <= (*it2)) {
					new_address_table.insert(it2, (*it));
					found = true;
					break;
				}
			}

			if (!found) new_address_table.push_back(*it);
		}


		final_address_table = new_address_table;
	}


	void File::createComparison(size_t sz) {
		comparison_bytes = new unsigned char[sz];
		comparison_bytes_min = new unsigned char[sz];
		comparison_bytes_max = new unsigned char[sz];
		comparison_size = sz;

		for (size_t i=0; i<comparison_size; i++) {
			comparison_bytes[i] = 0;
			comparison_bytes_min[i] = 255;
			comparison_bytes_max[i] = 0;
		}
	}

	void File::readComparison() {
		read(comparison_bytes, comparison_size);

		for (size_t i=0; i<comparison_size; i++) {
			if (comparison_bytes[i] < comparison_bytes_min[i]) comparison_bytes_min[i] = comparison_bytes[i];
			if (comparison_bytes[i] > comparison_bytes_max[i]) comparison_bytes_max[i] = comparison_bytes[i];
		}
	}

	void File::printComparisonResults() {
		for (size_t i=0; i<comparison_size; i++) {
			printf("Byte #%d Range: [%d, %d]\n", i, (int)comparison_bytes_min[i], (int)comparison_bytes_max[i]);
		}
		getchar();
	}

	void File::deleteComparison() {
		delete [] comparison_bytes;
		delete [] comparison_bytes_min;
		delete [] comparison_bytes_max;
	}

	bool File::check(string filename) {
		FILE *fp=fopen(filename.c_str(), "rb");

		if (fp) {
			fclose(fp);
			return true;
		}

		return false;
	}

	string File::extensionFromFilename(string filename) {
		string name = filename;
		size_t sep = name.find_last_of("\\/");
		if (sep != std::string::npos) {
			name = name.substr(sep + 1, name.size() - sep - 1);
		}
	
		size_t dot = name.find_first_of(".");
		if (dot != string::npos) {
			name = name.substr(dot+1, name.size()-dot-1);
		}
		else {
			name = "";
		}

		return name;
	}
	
	string File::nameFromFilename(string filename) {
		string name = filename;
		size_t sep = name.find_last_of("\\/");
		if (sep != std::string::npos) {
			name = name.substr(sep + 1, name.size() - sep - 1);
		}
		return name;
	}

	string File::nameFromFilenameNoExtension(string filename) {
		string name = filename;
		size_t sep = name.find_last_of("\\/");
		if (sep != std::string::npos) {
			name = name.substr(sep + 1, name.size() - sep - 1);
		}
	
		size_t dot = name.find_first_of(".");
		if (dot != string::npos) {
			name = name.substr(0, dot);
		}

		return name;
	}

	string File::folderFromFilename(string filename) {
		size_t sep = filename.find_last_of("\\/");
		if (sep != std::string::npos) {
			return filename.substr(0, sep+1);
		}

		return "";
	}

	void File::rename(string old_filename, string new_filename) {
		std::rename(old_filename.c_str(), new_filename.c_str());
	}

	void File::remove(string filename) {
		std::remove(filename.c_str());
	}

	void File::setGlobalOffset(size_t v) {
		global_offset = v;
		goToAddress(0);
	}

	list<size_t> File::getAddressTable() {
		return final_address_table;
	}

	void File::goToEnd() {
		fseek(file_ptr, 0L, SEEK_END);
	}

	void File::setRootNodeAddress(size_t v) {
		root_node_address=v;
	}

	size_t File::getRootNodeAddress() {
		return root_node_address;
	}

	void File::setRootNodeType(size_t v) {
		root_node_type=v;
	}

	string File::getPath() {
		return path;
	}

	int File::getAddressReadCount() {
		return address_read_count;
	}

	int File::getRootNodeType() {
		return root_node_type;
	}

	FILE *File::getPointer() {
		return file_ptr;
	}

	
	void File::readAddressTableBBIN(size_t table_size) {
		size_t current_address = root_node_address;
		unsigned char *offset_table = new unsigned char[table_size];
		read(offset_table, table_size);

		final_address_table.clear();

		for (size_t i=0; i<table_size; i++) {
			size_t low = offset_table[i] & 0x3F;

			if ((offset_table[i] & 0x80) && (offset_table[i] & 0x40)) {
				i += 3;
				current_address += (low * 0x4000000) + (offset_table[i-2] * 0x40000) + (offset_table[i-1] * 0x400) + (offset_table[i] * 0x4);
			}
			else if (offset_table[i] & 0x80) {
				i++;
				current_address += (low * 0x400) + (offset_table[i] * 4);
			}
			else if (offset_table[i] & 0x40) {
				current_address += 4 * low;
			}

			final_address_table.push_back(current_address - root_node_address);
		}
	}
	
	void File::writeAddressTableBBIN(size_t negative_offset) {
		size_t current_address = negative_offset;
		for (list<size_t>::iterator it=final_address_table.begin(); it!=final_address_table.end(); it++) {
			size_t difference = (*it) - current_address;

			if (difference > 0xFFFC) {
				unsigned int offset_int = 0xC0000000 | (difference >> 2);
				writeInt32BE(&offset_int);
			}
			else if (difference > 0xFC) {
				unsigned short offset_short = 0x8000 | (difference >> 2);
				writeInt16BE(&offset_short);
			}
			else {
				char offset_byte = 0x40 | (difference >> 2);
				write(&offset_byte, 1);
			}

			current_address += difference;
		}
	}
}