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
	class FileImpl {
	public:
		virtual ~FileImpl() {}
		virtual size_t read(void* buffer, size_t buffer_size) = 0;
		virtual char* gets(char* buffer, int max_count) = 0;
		virtual size_t write(const void* buffer, size_t size) = 0;
		virtual long tell() = 0;
		virtual void seek(long offset, int origin) = 0;
		virtual bool eof() = 0;
		virtual void close() = 0;
	};

	class DiskFile : public FileImpl {
	public:
		FILE* file;

		DiskFile(FILE* file_p) : file(file_p) {
		}

		size_t read(void* buffer, size_t size) override {
			return fread(buffer, 1, size, file);
		}

		char* gets(char* buffer, int max_count) override {
			return fgets(buffer, max_count, file);
		}

		size_t write(const void* buffer, size_t size) override {
			return fwrite(buffer, 1, size, file);
		}

		long tell() override {
			return ftell(file);
		}

		void seek(long offset, int origin) override {
			fseek(file, offset, origin);
		}

		bool eof() override {
			return feof(file) != 0;
		}

		void close() override {
			fclose(file);
		}
	};

	struct ReadOnlyMemoryFile : public FileImpl {
	public:
		const void* data;
		size_t data_size;
		size_t data_offset;

		ReadOnlyMemoryFile(const void* data_p, size_t data_size_p)
			: data(data_p), data_size(data_size_p), data_offset(0) {
		}

		~ReadOnlyMemoryFile() override {
		}

		size_t read(void* buffer, size_t size) override {
			size_t available = data_size - data_offset;
			if (size > available) {
				size = available;
			}

			memcpy(buffer, reinterpret_cast<const unsigned char*>(data) + data_offset, size);
			data_offset += size;

			return size;
		}

		char* gets(char* buffer, int max_count) override {
			// Unimplemented!
			__debugbreak();
			return nullptr;
		}

		size_t write(const void* buffer, size_t size) override {
			// Unsupported!
			__debugbreak();
			return 0xFFFFFFFF;
		}

		long tell() override {
			return static_cast<long>(data_offset);
		}

		void seek(long offset, int origin) override {
			switch (origin) {
			case SEEK_SET:
				if (offset < data_size) {
					data_offset = offset;
				}
				else {
					data_offset = data_size;
				}
				break;

			case SEEK_CUR:
				if ((data_offset + offset) < data_size) {
					data_offset += offset;
				}
				else {
					data_offset = data_size;
				}
				break;

			case SEEK_END:
				if ((-offset) < data_size) {
					data_offset = data_size + offset;
				}
				else {
					data_offset = 0;
				}
				break;
			}
		}

		bool eof() override {
			return data_offset >= data_size;
		}

		void close() override {
		}
	};

	struct MemoryFile : public FileImpl {
	public:
		vector<unsigned char> data;
		size_t data_offset;

		MemoryFile()
			: data_offset(0) {
		}

		~MemoryFile() override {
		}

		size_t read(void* buffer, size_t size) override {
			size_t available = data.size() - data_offset;
			if (size > available) {
				size = available;
			}

			memcpy(buffer, &data[data_offset], size);
			data_offset += size;

			return size;
		}

		char* gets(char* buffer, int max_count) override {
			// Unimplemented!
			__debugbreak();
			return nullptr;
		}

		size_t write(const void* buffer, size_t size) override {
			if (data.size() < (data_offset + size)) {
				data.resize(data_offset + size);
			}

			memcpy(&data[data_offset], buffer, size);
			data_offset += size;

			return size;
		}

		long tell() override {
			return static_cast<long>(data_offset);
		}

		void seek(long offset, int origin) override {
			switch (origin) {
			case SEEK_SET:
				if (offset < data.size()) {
					data_offset = offset;
				}
				else {
					data_offset = data.size();
				}
				break;

			case SEEK_CUR:
				if ((data_offset + offset) < data.size()) {
					data_offset += offset;
				}
				else {
					data_offset = data.size();
				}
				break;

			case SEEK_END:
				if ((-offset) < data.size()) {
					data_offset = data.size() + offset;
				}
				else {
					data_offset = 0;
				}
				break;
			}
		}

		bool eof() override {
			return data_offset >= data.size();
		}

		void close() override {
		}
	};

	void File::init() {
		file_impl = NULL;
		root_node_address = 0;
		address_read_count = 0;
		global_offset = 0;
		relative_address_mode = false;
		address_64_bit_mode = false;
	}

	File::File(string filename, string mode) {
		init();

		FILE* file=fopen(filename.c_str(), mode.c_str());
		if (!file) {
			Error::addMessage(Error::FILE_NOT_FOUND, LIBGENS_FILE_H_ERROR_READ_FILE_BEFORE + filename + LIBGENS_FILE_H_ERROR_READ_FILE_AFTER);
			return;
		}

		file_impl = new DiskFile(file);
		path = filename;
	}

	File::File(const void* data, size_t data_size) {
		init();
		file_impl = new ReadOnlyMemoryFile(data, data_size);
	}

	File::File() {
		init();
		file_impl = new MemoryFile();
	}

	File::~File() {
		delete file_impl;
	}

	int File::endOfFile() {
		if (file_impl) {
			return file_impl->eof();
		}
		else {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_FILE_H_ERROR_READ_FILE_NULL);
			return true;
		}
	}

	bool File::valid() {
		if (file_impl) return true;
		else return false;
	}

	void File::close() {
		if (file_impl) {
			file_impl->close();
		}
		else {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_FILE_H_ERROR_FILE_CLOSING);
		}
	}

	bool File::readSafeCheck(void *dest) {
		if (!file_impl) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_FILE_H_ERROR_READ_FILE_NULL);
			return false;
		}
		if (!dest) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_FILE_H_ERROR_READ_NULL);
			return false;
		}

		return true;
	}

	size_t File::read(void *dest, size_t sz) {
		if (!readSafeCheck(dest)) return 0;
		return file_impl->read(dest, sz);
	}

	void File::readInt16(unsigned short *dest) {
		if (!readSafeCheck(dest)) return;
		file_impl->read(dest, sizeof(short));
	}
	
	void File::readInt16BE(unsigned short *dest) {
		if (!readSafeCheck(dest)) return;
		file_impl->read(dest, sizeof(short));
		Endian::swap(*dest);
	}

	void File::readInt32(unsigned int *dest) {
		if (!readSafeCheck(dest)) return;
		file_impl->read(dest, sizeof(int));
	}

	void File::readInt32(int *dest) {
		if (!readSafeCheck(dest)) return;
		file_impl->read(dest, sizeof(int));
	}

	void File::readInt32A(size_t *dest) {
		if (!readSafeCheck(dest)) return;

		if (address_64_bit_mode) {
			fixPaddingRead(8);
		}

		file_impl->read(dest, sizeof(size_t));

		if (address_64_bit_mode) {
			file_impl->seek(4, SEEK_CUR);
		}

		if (relative_address_mode) {
			*dest += getCurrentAddress() - 4;
		}
		else {
			*dest += root_node_address;
		}

		address_read_count++;
	}

	void File::readInt32BE(unsigned int *dest) {
		if (!readSafeCheck(dest)) return;
		file_impl->read(dest, sizeof(int));
		Endian::swap(*dest);
	}

	void File::readInt32BE(int *dest) {
		if (!readSafeCheck(dest)) return;
		file_impl->read(dest, sizeof(int));
		Endian::swap(*dest);
	}
	
	void File::readInt32BEA(size_t *dest) {
		if (!readSafeCheck(dest)) return;

		if (address_64_bit_mode) {
			fixPaddingRead(8);
			file_impl->seek(4, SEEK_CUR);
		}

		file_impl->read(dest, sizeof(size_t));

		if (relative_address_mode) {
			*dest += getCurrentAddress() - 4;
		}
		else {
			Endian::swap(*dest);
			*dest += root_node_address;
		}

		address_read_count++;
	}

	void File::readFloat8(float *dest) {
		if (!readSafeCheck(dest)) return;
		unsigned char v=0;
		file_impl->read(&v, sizeof(char));
		*dest = v / 256.0f;
	}

	// from meshoptimizer
	union FloatBits {
		float f;
		unsigned int ui;
	};

	float dequantizeHalf(unsigned short h) {
		unsigned int s = unsigned(h & 0x8000) << 16;
		int em = h & 0x7fff;

		int r = (em + (112 << 10)) << 13;
		r = (em < (1 << 10)) ? 0 : r;
		r += (em >= (31 << 10)) ? (112 << 23) : 0;

		FloatBits u;
		u.ui = s | r;
		return u.f;
	}

	void File::readFloat16(float *dest) {
		if (!readSafeCheck(dest)) return;
		
		unsigned short v=0;
		readInt16(&v);
		*dest = dequantizeHalf(v);
	}

	void File::readFloat16BE(float *dest) {
		if (!readSafeCheck(dest)) return;
		
		unsigned short v=0;
		readInt16BE(&v);
		*dest = dequantizeHalf(v);
	}

	void File::readFloat32(float *dest) {
		if (!readSafeCheck(dest)) return;
		file_impl->read(dest, sizeof(int));
	}

	void File::readFloat32BE(float *dest) {
		if (!readSafeCheck(dest)) return;
		file_impl->read(dest, sizeof(int));
		Endian::swap(*((unsigned int *) dest));
	}

	void File::readUChar(unsigned char *dest) {
		if (!readSafeCheck(dest)) return;
		file_impl->read(dest, sizeof(char));
	}

	void File::readString(string *dest) {
		if (!readSafeCheck(dest)) return;
		char c=1;
		*dest = "";
		while (c && !endOfFile()) {
			file_impl->read(&c, sizeof(char));
			if (c) *dest += c;
		}
	}

	bool File::readLine(string *dest) {
		char string_buffer[LIBGENS_FILE_STRING_BUFFER];
		if (file_impl->gets(string_buffer, LIBGENS_FILE_STRING_BUFFER)) {
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
			file_impl->read(&c, sizeof(char));
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

	size_t File::write(void *dest, size_t sz) {
		if (!readSafeCheck(dest)) return 0;
		return file_impl->write(dest, sz);
	}
	
	void File::writeString(const char *dest) {
		if (!readSafeCheck((void *) dest)) return;

		file_impl->write(dest, strlen(dest));
	}

	void File::writeString(string *dest) {
		if (!readSafeCheck(dest)) return;

		if (!dest->size()) writeNull(1);
		else file_impl->write(dest->c_str(), dest->size()+1);
	}

	void File::writeUChar(unsigned char *dest) {
		if (!readSafeCheck(dest)) return;

		file_impl->write(dest, sizeof(char));
	}

	void File::writeInt16(unsigned short *dest) {
		if (!readSafeCheck(dest)) return;

		unsigned short target=*dest;
		file_impl->write(&target, sizeof(short));
	}

	void File::writeInt16BE(unsigned short *dest) {
		if (!readSafeCheck(dest)) return;

		unsigned short target=*dest;
		Endian::swap(target);
		file_impl->write(&target, sizeof(short));
	}

	void File::writeInt32(unsigned int *dest) {
		if (!readSafeCheck(dest)) return;

		file_impl->write(dest, sizeof(int));
	}

	void File::writeInt32A(size_t *dest, bool add_to_table) {
		if (!readSafeCheck(dest)) return;

		if (add_to_table) final_address_table.push_back(getCurrentAddress()-root_node_address);

		unsigned int target=(*dest) - root_node_address;
		file_impl->write(&target, sizeof(int));
	}

	void File::writeInt32BE(int *dest) {
		if (!readSafeCheck(dest)) return;

		int target=*dest;
		Endian::swap(target);
		file_impl->write(&target, sizeof(int));
	}

	void File::writeInt32BE(unsigned int *dest) {
		if (!readSafeCheck(dest)) return;

		unsigned int target=*dest;
		Endian::swap(target);
		file_impl->write(&target, sizeof(int));
	}

	void File::writeInt32BEA(size_t *dest) {
		if (!readSafeCheck(dest)) return;

		final_address_table.push_back(getCurrentAddress()-root_node_address);

		unsigned int target=(*dest) - root_node_address;
		Endian::swap(target);
		file_impl->write(&target, sizeof(int));
	}

	// from meshoptimizer
	unsigned short quantizeHalf(float v) {
		FloatBits u = { v };
		unsigned int ui = u.ui;

		int s = (ui >> 16) & 0x8000;
		int em = ui & 0x7fffffff;

		int h = (em - (112 << 23) + (1 << 12)) >> 13;
		h = (em < (113 << 23)) ? 0 : h;
		h = (em >= (143 << 23)) ? 0x7c00 : h;
		h = (em > (255 << 23)) ? 0x7e00 : h;

		return (unsigned short)(s | h);
	}

	void File::writeFloat8(float *dest) {
		if (!readSafeCheck(dest)) return;
		unsigned char v = (int)((*dest) * 256.0f);
		writeUChar(&v);
	}

	void File::writeFloat16(float* dest) {
		if (!readSafeCheck(dest)) return;
		unsigned short v = quantizeHalf(*dest);
		writeInt16(&v);
	}

	void File::writeFloat16BE(float* dest) {
		if (!readSafeCheck(dest)) return;
		unsigned short v = quantizeHalf(*dest);
		writeInt16BE(&v);
	}

	void File::writeFloat32(float *dest) {
		if (!readSafeCheck(dest)) return;
		file_impl->write(dest, sizeof(float));
	}

	void File::writeFloat32BE(float *dest) {
		if (!readSafeCheck(dest)) return;

		float target=*dest;
		Endian::swap(*((unsigned int *) &target));
		file_impl->write(&target, sizeof(float));
	}

	void File::writeFloat32E(float *dest, bool big_endian) {
		if (big_endian) writeFloat32BE(dest);
		else writeFloat32(dest);
	}

	void File::writeNull(size_t size) {
		unsigned char zero=0;
		for (size_t i=0; i<size; i++) {
			file_impl->write(&zero, sizeof(char));
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
		for (size_t c=0; c<extra; c++) file_impl->write(&zero, sizeof(char));
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
		if (!file_impl) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_FILE_H_ERROR_FILE_HEADER);
			return 0;
		}

		size_t address=getCurrentAddress();
		file_impl->seek(0, SEEK_END);
		size_t sz= file_impl->tell();
		goToAddress(address);
		return sz;
	}

	void File::readHeader() {
		if (!file_impl) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_FILE_H_ERROR_FILE_HEADER);
			return;
		}

		file_impl->seek(LIBGENS_FILE_HEADER_ROOT_TYPE_ADDRESS + global_offset, SEEK_SET);
		readInt32BE(&root_node_type);

		relative_address_mode = false;

		if (root_node_type == LIBGENS_FILE_HEADER_ROOT_TYPE_LOST_WORLD) {
			root_node_address = LIBGENS_FILE_HEADER_ROOT_ADDRESS_LOST_WORLD;
		}

		else if (root_node_type == LIBGENS_FILE_HEADER_ROOT_TYPE_NEEDLE_ARCHIVE) {
			root_node_type = LIBGENS_FILE_HEADER_ROOT_TYPE_LOST_WORLD;
			root_node_address = LIBGENS_FILE_HEADER_ROOT_ADDRESS_NEEDLE_ARCHIVE;
			relative_address_mode = true;
		}

		else {
			file_impl->seek(LIBGENS_FILE_HEADER_ROOT_NODE_ADDRESS + global_offset, SEEK_SET);
			readInt32BE(&root_node_address);

			file_impl->seek(LIBGENS_FILE_HEADER_OFFSET_TABLE_ADDRESS + global_offset, SEEK_SET);
			unsigned int offset_table_address = 0;
			readInt32BE(&offset_table_address);

			file_impl->seek(offset_table_address + global_offset, SEEK_SET);
			unsigned int offset_count = 0;
			readInt32BE(&offset_count);
			
			if (offset_count != 0) {
				unsigned int first_offset = 0;
				readInt32BE(&first_offset);

				file_impl->seek(root_node_address + first_offset + global_offset, SEEK_SET);
				readInt32BE(&first_offset);

				address_64_bit_mode = (first_offset == 0);
			}
		}

		file_impl->seek(root_node_address + global_offset, SEEK_SET);
	}

	void File::prepareHeader(int root_type) {
		root_node_type=root_type;
		
		if (root_node_type == LIBGENS_FILE_HEADER_ROOT_TYPE_LOST_WORLD) {
			root_node_address = LIBGENS_FILE_HEADER_ROOT_ADDRESS_LOST_WORLD;
		}
		else {
			root_node_address = LIBGENS_FILE_HEADER_ROOT_ADDRESS_DEFAULT;
		}

		writeNull(root_node_address);
	}

	void File::writeHeader(bool no_extra_foot) {
		if (!file_impl) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_FILE_H_ERROR_FILE_HEADER);
			return;
		}

		sortAddressTable();

		unsigned int root_node_size=getFileSize()-root_node_address;
		unsigned int final_table_address=getFileSize();
		unsigned int final_table_size=final_address_table.size();

		if (root_node_type != LIBGENS_FILE_HEADER_ROOT_TYPE_LOST_WORLD) writeInt32BE(&final_table_size);
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
		if (root_node_type == LIBGENS_FILE_HEADER_ROOT_TYPE_LOST_WORLD) {
			size = 0x80000000 | size;
			writeInt32BE(&size);
			writeInt32BE(&root_node_type);
			writeInt32BE(&final_table_address);
			writeInt32BE(&final_table_size);
		}
		else {
			writeInt32BE(&size);
			writeInt32BE(&root_node_type);
			writeInt32BE(&root_node_size);
			writeInt32BE(&root_node_address);
			writeInt32BE(&final_table_address);
			writeInt32BE(&size_foot);
		}
	}

	size_t File::getCurrentAddress() {
		if (file_impl) {
			return file_impl->tell()-global_offset;
		}
		else {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_FILE_H_ERROR_FILE_GET_ADDRESS);
			return 0;
		}
	}

	void File::goToAddress(size_t address) {
		if (file_impl) {
			file_impl->seek(address+global_offset, SEEK_SET);
		}
		else {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_FILE_H_ERROR_FILE_BOOKMARK);
		}
	}

	void File::moveAddress(size_t address) {
		if (file_impl) {
			file_impl->seek(address, SEEK_CUR);
		}
		else {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_FILE_H_ERROR_FILE_BOOKMARK);
		}
	}

	void File::seek(long offset, int origin) {
		if (file_impl) {
			file_impl->seek(offset, origin);
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

			goToAddress(0);
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
		file_impl->seek(0L, SEEK_END);
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

	bool File::get64BitAddressMode() const {
		return address_64_bit_mode;
	}

	int File::getAddressSize() const {
		return address_64_bit_mode ? 8 : 4;
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