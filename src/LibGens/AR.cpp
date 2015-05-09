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

#include "AR.h"

namespace LibGens {
	ArFile::ArFile() {
		data = NULL;
	}

	ArFile::ArFile(string filename) {
		name = filename;
	}

	void ArFile::read(File *file, bool read_data) {
		unsigned int entry_size=0;
		unsigned int data_address=0;

		size_t header_address = file->getCurrentAddress();

		file->readInt32(&entry_size);
		file->readInt32(&data_size);
		file->readInt32(&data_address);
		file->moveAddress(8);
		file->readString(&name);

		absolute_data_address = header_address + data_address;

		file->goToAddress(header_address + data_address);
		data = NULL;

		if (read_data) {
			data = new unsigned char[data_size];
			file->read(data, data_size);
		}
		else {
			file->moveAddress(data_size);
		}
	}

	void ArFile::readData(File *file) {
		data_size=file->getFileSize();
		data = new unsigned char[data_size];
		file->read(data, data_size);
	}

	void ArFile::write(File *file, unsigned int padding) {
		unsigned int entry_size=0;
		unsigned int data_address=0;

		size_t header_address = file->getCurrentAddress();
		file->writeNull(4);
		file->writeInt32(&data_size);
		file->writeNull(12);
		file->writeString(&name);
		file->fixPadding(padding);

		data_address = file->getCurrentAddress() - header_address;
		absolute_data_address = file->getCurrentAddress();
		file->write(data, data_size);

		entry_size = file->getCurrentAddress() - header_address;
		file->goToAddress(header_address);
		file->writeInt32(&entry_size);
		file->moveAddress(4);
		file->writeInt32(&data_address);

		file->goToEnd();
	}

	void ArFile::save(string filename) {
		File file(filename, LIBGENS_FILE_WRITE_BINARY);
		file.write(data, data_size);
		file.close();
	}

	string ArFile::getName() {
		return name;
	}

	unsigned char *ArFile::getData() {
		return data;
	}

	unsigned int ArFile::getSize() {
		return data_size;
	}

	unsigned int ArFile::getAbsoluteDataAddress() {
		return absolute_data_address;
	}

	ArFile::~ArFile() {
		if (data) delete [] data;
	}


	ArPack::ArPack(unsigned int padding_p) {
		padding = padding_p;
	}

	ArPack::ArPack(string filename, bool data) {
		if ((filename[filename.size()-1] == '\\') || (filename[filename.size()-1] == '/')) {
			padding = 0x40;

			WIN32_FIND_DATA FindFileData;
			HANDLE hFind;
			hFind = FindFirstFile((filename+"*.*").c_str(), &FindFileData);
			if (hFind == INVALID_HANDLE_VALUE) {} 
			else {
				do {
					const char *name=FindFileData.cFileName;
					if (name[0]=='.') continue;

					string new_filename=filename+ToString(name);
					addFile(new_filename);
				} while (FindNextFile(hFind, &FindFileData) != 0);
				FindClose(hFind);
			}
			return;
		}

		
		
		SHA1Reset(&sha1_context);
		
		File file(filename, LIBGENS_FILE_READ_BINARY);
		if (file.valid()) {
			unsigned int ar_flag_0=0;
			unsigned int ar_flag_1=0;
			unsigned int ar_flag_2=0;

			file.readInt32(&ar_flag_0);
			file.readInt32(&ar_flag_1);
			file.readInt32(&ar_flag_2);

			if ((ar_flag_0 != 0) || (ar_flag_1 != 0x10) || (ar_flag_2 != 0x14)) {
				Error::addMessage(Error::EXCEPTION, LIBGENS_AR_H_ERROR_READ_FILE_BEFORE + filename + LIBGENS_AR_H_ERROR_READ_FILE_AFTER);
				return;
			}

			file.readInt32(&padding);
			files.clear();

			while (!file.endOfFile()) {
				ArFile *ar_file = new ArFile();
				ar_file->read(&file, data);
				
				if (ar_file->getData()) {
					SHA1Input(&sha1_context, ar_file->getData(), ar_file->getSize());
				}

				files.push_back(ar_file);

				if (file.getCurrentAddress() >= file.getFileSize()) break;
			}

			file.close();

			size_t pos=filename.find(LIBGENS_AR_MULTIPLE_START);
			if ((pos != string::npos) && pos==(filename.size()-6)) {
				size_t index=1;
				char extension[]="00";

				while (index < LIBGENS_AR_MAX_SEARCH) {
					sprintf_s(extension, "%02d", index);

					string extension_filename=filename;
					extension_filename.resize(extension_filename.size()-2);
					extension_filename += ToString(extension);

					if (File::check(extension_filename)) {
						LibGens::ArPack *extension_ar_pack=new LibGens::ArPack(extension_filename);
						merge(extension_ar_pack);
					}
					else break;

					index++;
				}
			}
		}

		SHA1Result(&sha1_context);
		for (size_t i=0; i<5; i++) {
			sha1_hash[i] = sha1_context.Message_Digest[i];
		}
	}


	void ArPack::merge(ArPack *pack) {
		size_t file_count=pack->files.size();
		for (size_t i=0; i<file_count; i++) {
			SHA1Input(&sha1_context, pack->files[i]->getData(), pack->files[i]->getSize());

			files.push_back(pack->files[i]);
		}
		pack->files.clear();
		delete pack;
	}


	void ArPack::save(string filename, unsigned int padding_p) {
		bool split_file_mode=false;
		string pack_name=filename;
		char extension[]="00";
		size_t ar_split_index=1;

		padding = padding_p;
		split_sizes.clear();

		size_t pos=filename.find(LIBGENS_AR_MULTIPLE_START);
		if ((pos != string::npos) && pos==(filename.size()-6)) {
			split_file_mode = true;
			pack_name.resize(pack_name.size()-2);

			size_t index=1;
			char extension[]="00";

			while (index < LIBGENS_AR_MAX_SEARCH) {
				sprintf_s(extension, "%02d", index);

				string extension_filename=pack_name+ToString(extension);

				if (File::check(extension_filename)) {
					remove(extension_filename.c_str());
				}
				else break;

				index++;
			}
		}

		SHA1Reset(&sha1_context);

		File *current_file=new LibGens::File(filename, LIBGENS_FILE_WRITE_BINARY);
		if (current_file->valid()) {
			unsigned int ar_flag_0=0;
			unsigned int ar_flag_1=0x10;
			unsigned int ar_flag_2=0x14;
			current_file->writeInt32(&ar_flag_0);
			current_file->writeInt32(&ar_flag_1);
			current_file->writeInt32(&ar_flag_2);
			current_file->writeInt32(&padding);

			
			char extension[]="00";

			for (size_t i=0; i<files.size(); i++) {
				files[i]->write(current_file, padding);

				if (files[i]->getData()) {
					SHA1Input(&sha1_context, files[i]->getData(), files[i]->getSize());
				}

				if (split_file_mode && (current_file->getCurrentAddress() > LIBGENS_AR_MAX_SPLIT_FILE_BYTES) && (i<files.size()-1)) {
					split_sizes.push_back(current_file->getFileSize());
					current_file->close();
					delete current_file;

					sprintf_s(extension, "%02d", ar_split_index);
					string new_filename=pack_name+ToString(extension);
					current_file=new LibGens::File(new_filename, LIBGENS_FILE_WRITE_BINARY);
					if (!current_file->valid()) {
						printf("Couldn't write to AR File %s\n", new_filename.c_str());
						getchar();
						break;
					}
					else {
						current_file->writeInt32(&ar_flag_0);
						current_file->writeInt32(&ar_flag_1);
						current_file->writeInt32(&ar_flag_2);
						current_file->writeInt32(&padding);
						ar_split_index++;
					}
				}
			}

			split_sizes.push_back(current_file->getFileSize());
			current_file->close();
			delete current_file;
		}

		if (split_file_mode) {
			string arl_filename=pack_name;
			arl_filename[arl_filename.size()-1]='l';
			saveARL(arl_filename);
		}

		SHA1Result(&sha1_context);
		for (size_t i=0; i<5; i++) {
			sha1_hash[i] = sha1_context.Message_Digest[i];
		}
	}

	void ArPack::saveARL(string filename) {
		string arl_header=LIBGENS_ARL_HEADER;
		unsigned int split_total=split_sizes.size();

		File file(filename, LIBGENS_FILE_WRITE_BINARY);
		if (file.valid()) {
			file.write((char *)arl_header.c_str(), 4);
			file.writeInt32(&split_total);

			for (size_t i=0; i<split_sizes.size(); i++) {
				file.writeInt32(&split_sizes[i]);
			}

			for (size_t i=0; i<files.size(); i++) {
				string file_id=files[i]->getName();
				unsigned char counter=file_id.size();
				file.writeUChar(&counter);
				file.write((char *) file_id.c_str(), file_id.size());
			}

			file.close();
		}

		
	}

	void ArPack::savePFI(string filename) {
		File file(filename, LIBGENS_FILE_WRITE_BINARY);
		if (file.valid()) {
			file.prepareHeader(LIBGENS_AR_PFI_ROOT_GENERATIONS);
			writePFI(&file);
			file.writeHeader();
			file.close();
		}
	}

	void ArPack::writePFI(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_AR_PFI_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}

		unsigned int package_total=files.size();
		size_t package_table_address=32;
		file->writeInt32BE(&package_total);
		file->writeInt32BEA(&package_table_address);

		vector<unsigned int> package_addresses;
		file->writeNull(package_total * 4);

		for (size_t i=0; i<package_total; i++) {
			size_t table_address=file->getCurrentAddress();
			package_addresses.push_back(table_address);

			size_t package_name_address=0;
			unsigned int package_absolute_address=files[i]->getAbsoluteDataAddress();
			unsigned int package_size=files[i]->getSize();
			string package_name=files[i]->getName();

			file->writeNull(4);
			file->writeInt32BE(&package_absolute_address);
			file->writeInt32BE(&package_size);
			
			package_name_address = file->getCurrentAddress();
			file->writeString(&package_name);
			file->fixPadding();

			file->goToAddress(table_address);
			file->writeInt32BEA(&package_name_address);

			file->goToEnd();
		}

		for (size_t i=0; i<package_total; i++) {
			file->goToAddress(package_table_address + i*4);
			file->writeInt32BEA(&package_addresses[i]);
		}

		file->goToEnd();
	}


	void ArPack::addFile(string filename, string override_name) {
		File file(filename, LIBGENS_FILE_READ_BINARY);

		if (file.valid()) {
			string name=filename;
			if (override_name.size()) name=override_name;

			size_t sep = name.find_last_of("\\/");
			if (sep != string::npos) {
				name = name.substr(sep + 1, name.size() - sep - 1);
			}

			ArFile *ar_file=new ArFile(name);
			ar_file->readData(&file);
			files.push_back(ar_file);

			file.close();
		}
	}

	void ArPack::extract(string folder, string add_extension, string add_prefix, vector<string> *output_filenames) {
		for (size_t i=0; i<files.size(); i++) {
			string new_filename=folder + add_prefix + files[i]->getName() + add_extension;
			files[i]->save(new_filename);
			if (output_filenames) output_filenames->push_back(new_filename);
		}
	}

	ArFile *ArPack::getFile(string filename) {
		for (size_t i=0; i<files.size(); i++) {
			if (files[i]->getName() == filename) return files[i];
		}

		return NULL;
	}

	ArFile *ArPack::getFileByIndex(size_t index) {
		return files[index];
	}

	unsigned int ArPack::getFileCount() {
		return files.size();
	}

	unsigned int *ArPack::getHash() {
		return sha1_hash;
	}

	ArPack::~ArPack() {
		for (size_t i=0; i<files.size(); i++) {
			delete files[i];
		}

		files.clear();
	}
}