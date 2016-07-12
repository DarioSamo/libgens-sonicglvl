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

#pragma once

#define LIBGENS_FILE_H_ERROR_READ_FILE_BEFORE   "Can't open the specified file: "
#define LIBGENS_FILE_H_ERROR_READ_FILE_AFTER    ". The file doesn't exist, or is currently open by another program."
#define LIBGENS_FILE_H_ERROR_FILE_CLOSING       "Attempted to close a file with an undefined reference."
#define LIBGENS_FILE_H_ERROR_FILE_HEADER        "Attempted to read the header of a file with an undefined reference."
#define LIBGENS_FILE_H_ERROR_FILE_BOOKMARK      "Attempted to go to address on a file with an undefined reference."
#define LIBGENS_FILE_H_ERROR_FILE_GET_ADDRESS   "Attempted to get address from a file with an undefined reference."
#define LIBGENS_FILE_H_ERROR_READ_NULL          "Attempted to read variable from file, but the destination is undefined."
#define LIBGENS_FILE_H_ERROR_READ_FILE_NULL     "Attempted to read variable from an file with an undefined reference."

#define LIBGENS_FILE_HEADER_ROOT_TYPE_ADDRESS           4
#define LIBGENS_FILE_HEADER_ROOT_NODE_ADDRESS           12

#define LIBGENS_FILE_HEADER_ROOT_ADDRESS_DEFAULT        24
#define LIBGENS_FILE_HEADER_ROOT_ADDRESS_NEXT_GEN       16

#define LIBGENS_FILE_STRING_BUFFER                      1024

#define LIBGENS_FILE_READ_TEXT                          "rt"
#define LIBGENS_FILE_WRITE_TEXT                         "wt"
#define LIBGENS_FILE_READ_BINARY                        "rb"
#define LIBGENS_FILE_WRITE_BINARY                       "wb"

namespace LibGens {
	class File {
		protected:
			FILE *file_ptr;
			string name;
			string path;
			int root_node_type;
			int root_node_address;
			int address_read_count;
			size_t global_offset;
			list<size_t> final_address_table;
			unsigned char *comparison_bytes;
			unsigned char *comparison_bytes_min;
			unsigned char *comparison_bytes_max;
			size_t comparison_size;
		public:
			File(string filename, string mode);
			void prepareHeader(int root_type, int root_offset=LIBGENS_FILE_HEADER_ROOT_ADDRESS_DEFAULT);
			void writeHeader(bool no_extra_foot=false);
			void readHeader();
			void setGlobalOffset(size_t v);
			list<size_t> getAddressTable();
			void addAddressToTable();
			void sortAddressTable();
			void goToAddress(size_t address);
			void goToEnd();
			void moveAddress(size_t address);
			size_t getCurrentAddress();
			void setRootNodeAddress(size_t v);
			size_t getRootNodeAddress();
			void setRootNodeType(size_t v);
			string getPath();
			// BE: Big Endian
			// BEA: Big Endian Address (Offset by Root Node automatically)
			bool readSafeCheck(void *dest);
			void read(void *dest, size_t sz);
			void readInt16(unsigned short *dest);
			void readInt16BE(unsigned short *dest);
			void readInt32(unsigned int *dest);
			void readInt32BE(unsigned int *dest);
			void readInt32(int *dest);
			void readInt32A(size_t *dest);
			void readInt32BE(int *dest);
			void readInt32BEA(size_t *dest);
			void readFloat8(float *dest);
			void readFloat16(float *dest);
			void readFloat16BE(float *dest);
			void readFloat32(float *dest);
			void readFloat32BE(float *dest);
			void readUChar(unsigned char *dest);
			void readString(string *dest);
			void readString(string *dest, size_t n);
			void readInt16E(unsigned short *dest, bool big_endian);
			void readInt32E(int *dest, bool big_endian);
			void readInt32E(unsigned int *dest, bool big_endian);
			void readInt32EA(size_t *dest, bool big_endian);
			void readFloat16E(float *dest, bool big_endian);
			void readFloat32E(float *dest, bool big_endian);
			bool readLine(string *dest);
			void write(void *dest, size_t sz);
			void writeString(const char *dest);
			void writeString(string *dest);
			void writeUChar(unsigned char *dest);
			void writeInt16(unsigned short *dest); 
			void writeInt16BE(unsigned short *dest);
			void writeInt32(unsigned int *dest);
			void writeInt32A(size_t *dest, bool add_to_table=true);
			void writeInt32BE(unsigned int *dest);
			void writeInt32BE(int *dest);
			void writeInt32BEA(size_t *dest);
			void writeFloat8(float *dest);
			void writeFloat32(float *dest);
			void writeFloat32BE(float *dest);
			void writeNull(size_t size);
			void writeFloat32E(float *dest, bool big_endian);
			void readAddressTableBBIN(size_t table_size);
			void writeAddressTableBBIN(size_t negative_offset=0);

			size_t fixPadding(size_t multiple=4);
			size_t fixPaddingRead(size_t multiple=4);
			size_t getFileSize();

			int getAddressReadCount();
			int getRootNodeType();
			FILE *getPointer();

			// Reverse-Engineering functions, just to track down what some values could be by printing their minimum and maximum values
			void createComparison(size_t sz);
			void readComparison();
			void printComparisonResults();
			void deleteComparison();

			int endOfFile();
			bool valid();
			bool compare(File *file);
			void close();
			void clone(string dest);

			static bool check(string filename);
			static string extensionFromFilename(string filename);
			static string nameFromFilename(string filename);
			static string nameFromFilenameNoExtension(string filename);
			static string folderFromFilename(string filename);
			static void rename(string old_filename, string new_filename);
			static void remove(string filename);
	};

	
};
