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

#pragma once

#define LIBGENS_AR_H_ERROR_READ_FILE_BEFORE            "Can't open the AR File: "
#define LIBGENS_AR_H_ERROR_READ_FILE_AFTER             ". Unknown header format."
#define LIBGENS_AR_PFI_ERROR_MESSAGE_WRITE_NULL_FILE   "Trying to write PFI data to an unreferenced file."
#define LIBGENS_AR_PFI_ROOT_GENERATIONS                0
#define LIBGENS_AR_MULTIPLE_START                      ".ar.00"
#define LIBGENS_AR_MAX_SEARCH                          99
#define LIBGENS_AR_MAX_SPLIT_FILE_BYTES                10485760
#define LIBGENS_ARL_HEADER                             "ARL2"

namespace LibGens {
	class ArFile {
		protected:
			string name;
			unsigned char *data;
			unsigned int data_size;
			unsigned int absolute_data_address;
		public:
			ArFile();
			ArFile(string filename);
			void read(File *file, bool data=true);
			void readData(File *file);
			void save(string filename);
			void write(File *file, unsigned int padding=0x40);
			string getName();
			unsigned char *getData();
			unsigned int getSize();
			unsigned int getAbsoluteDataAddress();
			~ArFile();
	};

	class ArPack {
		friend class ArPack;

		protected:
			vector<ArFile *> files;
			vector<unsigned int> split_sizes;
			unsigned int padding;
			unsigned int sha1_hash[5];
			SHA1Context sha1_context;
		public:
			ArPack(unsigned int padding_p=0x40);
			ArPack(string filename, bool data=true);
			void save(string filename, unsigned int padding_p=0x40);
			void saveARL(string filename);
			void savePFI(string filename);
			void writePFI(File *file);
			void generateArl(string filename);
			void extract(string folder, string add_extension="", string add_prefix="", vector<string> *output_filenames=NULL);
			ArFile *getFile(string filename);
			ArFile *getFileByIndex(size_t index);
			void merge(ArPack *pack);
			void addFile(string filename, string override_name="");
			unsigned int getFileCount();
			unsigned int *getHash();
			~ArPack();
	};
};
