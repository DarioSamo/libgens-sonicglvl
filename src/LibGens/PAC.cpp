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

#include "PAC.h"
#include "StringTable.h"

namespace LibGens {
	PacPack::PacPack(string filename) {
		name = File::nameFromFilename(filename);

		readFile(filename);
	}

	void PacPack::readFile(string filename) {
		size_t offset_count = 0;

		File file(filename, LIBGENS_FILE_READ_BINARY);
		if (file.valid()) {
			string signature="";
			size_t file_size=0;
			size_t unknown_1=0;
			

			file.readString(&signature, 8);
			file.readInt32(&file_size);
			file.readInt32(&unknown_1);
			

			string data_header="";
			size_t data_size = 0;
			file.readString(&data_header, 4);
			file.readInt32(&data_size);

			unsigned int file_data_size=0;
			unsigned int extension_table_size=0;
			unsigned int string_offset_table_size=0;
			unsigned int string_table_data_size=0;
			unsigned int offset_table_size=0;
			unsigned int data_flag=0;

			file.readInt32(&file_data_size);
			file.readInt32(&extension_table_size);
			file.readInt32(&string_offset_table_size);
			file.readInt32(&string_table_data_size);
			file.readInt32(&offset_table_size);
			file.readInt32(&data_flag);

			size_t file_extensions = 0;
			size_t file_extensions_table_address = 0;
			file.readInt32(&file_extensions);
			file.readInt32(&file_extensions_table_address);

			for (size_t i=0; i<file_extensions; i++) {
				file.goToAddress(file_extensions_table_address + i*8);

				PacExtension *extension = new PacExtension();
				extension->read(&file);
				extensions.push_back(extension);
			}

			// Read string data table section
			if (string_offset_table_size) {
				unsigned int string_data_table_count=0;
				unsigned int string_data_table_offset=0;
				file.readInt32(&string_data_table_count);
				file.readInt32(&string_data_table_offset);

				for (size_t i=0; i<string_data_table_count; i++) {
					file.goToAddress(string_data_table_offset + i*12);

					PacProxyEntry *proxy_entry = new PacProxyEntry();
					proxy_entry->read(&file);
					proxy_entries.push_back(proxy_entry);
				}
			}

			file.goToAddress(48+file_data_size+extension_table_size+string_offset_table_size+string_table_data_size);
			if (offset_table_size) {
				file.readAddressTableBBIN(offset_table_size);
			}

			// Fix Current Files based on the offset table locations
			scanForAddressesInsideFiles(&file);

			file.close();
		}
	}


	bool PacFile::scanForAddress(size_t address, File *file) {
		size_t lower_limit = file_data_address + 16;
		size_t upper_limit = lower_limit + data_size;

		if (data && data_size && (address >= lower_limit) && (address < upper_limit)) {
			file->goToAddress(address);

			PacMetaString meta_string;
			file->readInt32(&meta_string.address);
			file->goToAddress(meta_string.address);
			meta_string.relative_address = address - lower_limit;
			meta_string.name = "";

			if ((meta_string.address >= lower_limit) && (meta_string.address < upper_limit)) {
				// Points inside the file, just fix the internal address
				meta_string.address -= lower_limit;
			}
			else {
				// Points to String Table, read and store the string
				file->readString(&meta_string.name);
			}

			meta_pac_extra_strings.push_back(meta_string);
			return true;
		}

		return false;
	}

	bool PacExtension::scanForAddress(size_t address, File *file) {
		for (size_t i=0; i<files.size(); i++) {
			bool result=files[i]->scanForAddress(address, file);
			if (result) return true;
		}

		return false;
	}


	void PacPack::scanForAddressesInsideFiles(File *file) {
		list<size_t> address_table = file->getAddressTable();
		for (list<size_t>::iterator it=address_table.begin(); it!=address_table.end(); it++) {
			for (size_t i=0; i<extensions.size(); i++) {
				bool result=extensions[i]->scanForAddress(*it, file);
				if (result) break;
			}
		}
	}


	void PacProxyEntry::read(File *file) {
		unsigned int string_data_extension_offset=0;
		unsigned int string_data_filename_offset=0;

		file->readInt32(&string_data_extension_offset);
		file->readInt32(&string_data_filename_offset);
		file->readInt32(&index);
		file->goToAddress(string_data_extension_offset);
		file->readString(&extension);
		file->goToAddress(string_data_filename_offset);
		file->readString(&name);
	}

	void PacExtension::read(File *file) {
		size_t file_extension_string_address=0;
		size_t file_table_address=0;
		file->readInt32(&file_extension_string_address);
		file->readInt32(&file_table_address);
		file->goToAddress(file_extension_string_address);
		file->readString(&name);

		file->goToAddress(file_table_address);

		size_t file_count = 0;
		size_t file_sub_table_address = 0;
		file->readInt32(&file_count);
		file->readInt32(&file_sub_table_address);

		//LibGens::Error::printfMessage(LibGens::Error::LOG, "-----------------------------------", name.c_str());
		//LibGens::Error::printfMessage(LibGens::Error::LOG, "FILE EXTENSION: %s", name.c_str());
		//LibGens::Error::printfMessage(LibGens::Error::LOG, "-----------------------------------", name.c_str());

		for (size_t i=0; i<file_count; i++) {
			file->goToAddress(file_sub_table_address + i*8);

			PacFile *pac_file = new PacFile();
			pac_file->read(file);
			files.push_back(pac_file);
		}
	}

	bool PacExtension::isSpecialExtension() {
		if (name == LIBGENS_PAC_EXTENSION_ANM_FULL) return false;
		if (name == LIBGENS_PAC_EXTENSION_LIGHT_FULL) return false;
		if (name == LIBGENS_PAC_EXTENSION_LFT_FULL) return false;
		if (name == LIBGENS_PAC_EXTENSION_GISM_FULL) return false;
		if (name == LIBGENS_PAC_EXTENSION_HHD_FULL) return false;
		if (name == LIBGENS_PAC_EXTENSION_PAC_PHY_HKX_FULL) return false;
		if (name == LIBGENS_PAC_EXTENSION_PAC_LUA_FULL) return true;
		if (name == LIBGENS_PAC_EXTENSION_MATERIAL_FULL) return true;
		if (name == LIBGENS_PAC_EXTENSION_TERAIN_INSTANCEINFO_FULL) return false;
		if (name == LIBGENS_PAC_EXTENSION_TERRAIN_MODEL_FULL) return true;
		if (name == LIBGENS_PAC_EXTENSION_PAC_DEPEND_FULL) return false;
		if (name == LIBGENS_PAC_EXTENSION_PAC_FXCOL_FULL) return false;
		if (name == LIBGENS_PAC_EXTENSION_PAC_RAW_FULL) return false;
		if (name == LIBGENS_PAC_EXTENSION_PAC_MAT_ANIM_FULL) return true;
		if (name == LIBGENS_PAC_EXTENSION_PAC_ANM_HKX_FULL) return false;
		if (name == LIBGENS_PAC_EXTENSION_PAC_UV_ANIM_FULL) return true;
		if (name == LIBGENS_PAC_EXTENSION_PAC_VIS_ANIM_FULL) return true;
		if (name == LIBGENS_PAC_EXTENSION_PAC_EFFECT_FULL) return false;
		if (name == LIBGENS_PAC_EXTENSION_PAC_MODEL_FULL) return true;
		if (name == LIBGENS_PAC_EXTENSION_PAC_SKL_HKX_FULL) return false;
		if (name == LIBGENS_PAC_EXTENSION_PAC_DDS_FULL) return true;
		if (name == LIBGENS_PAC_EXTENSION_PAC_SHADOW_MODEL_FULL) return false;
		if (name == LIBGENS_PAC_EXTENSION_PAC_SWIF_FULL) return false;
		if (name == LIBGENS_PAC_EXTENSION_PAC_PATH2_FULL) return false;
		if (name == LIBGENS_PAC_EXTENSION_PAC_PIXELSHADER_FULL) return false;
		if (name == LIBGENS_PAC_EXTENSION_PAC_PIXELSHADERCODE_FULL) return false;
		if (name == LIBGENS_PAC_EXTENSION_PAC_PIXELSHADERPARAMETER_FULL) return false;
		if (name == LIBGENS_PAC_EXTENSION_PAC_SHADERLIST_FULL) return false;
		if (name == LIBGENS_PAC_EXTENSION_PAC_VERTEXSHADER_FULL) return false;
		if (name == LIBGENS_PAC_EXTENSION_PAC_VERTEXSHADERCODE_FULL) return false;
		if (name == LIBGENS_PAC_EXTENSION_PAC_VERTEXSHADERPARAMETER_FULL) return false;
		if (name == LIBGENS_PAC_EXTENSION_PAC_SM4SHADERCONTAINER_FULL) return false;
		if (name == LIBGENS_PAC_EXTENSION_PAC_XTB2DATA_FULL) return false;

		return false;
	}

	PacFile::PacFile() {
		name = "";
		data = NULL;
		data_size = 0;
		file_data_address = 0;
		meta_pac_extra_strings.clear();
	}

	PacFile::~PacFile() {
		free(data);
	}

	PacFile::PacFile(string filename) {
		file_data_address = 0;

		File file(filename, LIBGENS_FILE_READ_BINARY);
		if (file.valid()) {
			name = File::nameFromFilenameNoExtension(filename);

			// Read entire file into data buffer
			data_size = file.getFileSize();
			data = (unsigned char *) malloc(data_size);
			file.read(data, data_size);

			// Search data buffer for comparison string
			size_t metadata_address = 0;
			char comp_string[] = LIBGENS_PAC_METAINFO_HEADER;
			for (size_t i=0; i<data_size-16; i++) {
				if (!strncmp((char *) data + i, comp_string, strlen(comp_string))) {
					metadata_address = i;
					break;
				}
			}

			// If metadata was found, reallocate data buffer to be smaller
			// Also read metadata and add to the current table
			// Detect if each metadata entry either points inside the file or to the
			// string meta table.

			if (metadata_address) {
				data = (unsigned char *) realloc(data, metadata_address);
				data_size = metadata_address;

				file.goToAddress(metadata_address);
				file.moveAddress(16);
				file.fixPaddingRead(4);

				unsigned int meta_pac_count = 0;
				file.readInt32(&meta_pac_count);

				// Read Pac Metadata
				for (size_t i=0; i<meta_pac_count; i++) {
					PacMetaString pac_meta_string;

					unsigned int meta_pac_address = 0;
					file.readInt32(&meta_pac_address);
					pac_meta_string.name = "";
					pac_meta_string.relative_address = meta_pac_address;
					meta_pac_extra_strings.push_back(pac_meta_string);
				}

				// Detect if Pac Metadata points to either strings or other binary data
				for (size_t i=0; i<meta_pac_count; i++) {
					file.goToAddress(meta_pac_extra_strings[i].relative_address);
					file.readInt32(&meta_pac_extra_strings[i].address);

					// If it points to the PAC Metadata, it's a string
					if (meta_pac_extra_strings[i].address >= metadata_address) {
						file.goToAddress(meta_pac_extra_strings[i].address);
						file.readString(&meta_pac_extra_strings[i].name);
					}
				}
			}

			file.close();
		}
	}

	PacFile::PacFile(PacFile *pac_file) {
		name = pac_file->getName();
		data = NULL;
		data_size = pac_file->getDataSize();
		file_data_address = 0;
	}

	PacFile::PacFile(vector<string> pac_names) {
		name = "";
		data_size = 8 + pac_names.size() * 4;
		data = (unsigned char *) malloc(data_size);
		memset(data, 0, data_size);
		file_data_address = 0;

		unsigned int *count = (unsigned int *)(data + 4);
		*count = pac_names.size();

		PacMetaString base_offset;
		base_offset.address = 8;
		base_offset.relative_address = 0;
		base_offset.name = "";
		meta_pac_extra_strings.push_back(base_offset);

		for (size_t i=0; i<pac_names.size(); i++) {
			PacMetaString name_offset;
			name_offset.address = 0;
			name_offset.relative_address = 8 + i*4;
			name_offset.name = pac_names[i];
			meta_pac_extra_strings.push_back(name_offset);
		}
	}


	void PacFile::read(File *file) {
		size_t file_name_string_address=0;

		//LibGens::Error::printfMessage(LibGens::Error::LOG, "0x%x Header", file->getCurrentAddress());

		file->readInt32(&file_name_string_address);
		file->readInt32(&file_data_address);

		file->goToAddress(file_name_string_address);
		file->readString(&name);

		//LibGens::Error::printfMessage(LibGens::Error::LOG, "0x%x: %s", file_data_address, name.c_str());

		file->goToAddress(file_data_address);
		size_t empty_flag=0;
		file->readInt32(&data_size);
		file->moveAddress(8);
		file->readInt32(&empty_flag);

		if (empty_flag == 0x80) {
			data = NULL;
			return;
		}

		data = new unsigned char[data_size];
		file->read(data, data_size);

		size_t lower_limit = file_data_address + 16;
		size_t upper_limit = lower_limit + data_size;

		//LibGens::Error::printfMessage(LibGens::Error::LOG, "Lower Limit: %d Upper Limit: %d Name: %s", lower_limit, upper_limit, name.c_str());
	}

	vector<string> PacFile::getPacDependNames() {
		vector<string> names;

		for (size_t i=0; i<meta_pac_extra_strings.size(); i++) {
			if (meta_pac_extra_strings[i].name.size()) {
				names.push_back(meta_pac_extra_strings[i].name);
			}
		}

		return names;
	}

	void PacFile::save(string filename) {
		if (!data) {
			return;
		}

		bool meta_pac_strings = (meta_pac_extra_strings.size() != 0);

		File file(filename, LIBGENS_FILE_WRITE_BINARY);
		file.write(data, data_size);

		if (meta_pac_strings) {
			char meta_string[] = LIBGENS_PAC_METAINFO_HEADER;
			file.write(meta_string, 16);

			file.fixPadding(4);

			unsigned int meta_pac_count = meta_pac_extra_strings.size();
			file.writeInt32(&meta_pac_count);

			for (size_t i=0; i<meta_pac_count; i++) {
				file.writeInt32(&meta_pac_extra_strings[i].relative_address);
			}

			for (size_t i=0; i<meta_pac_count; i++) {
				// Fix address to string's location in the file
				if (meta_pac_extra_strings[i].name.size()) {
					meta_pac_extra_strings[i].address = file.getCurrentAddress();
					file.writeString(&meta_pac_extra_strings[i].name);
				}

				file.goToAddress(meta_pac_extra_strings[i].relative_address);
				file.writeInt32(&meta_pac_extra_strings[i].address);

				file.goToEnd();
			}
		}

		file.close();
	}

	string PacFile::getName() {
		return name;
	}

	void PacFile::setName(string v) {
		name = v;
	}

	unsigned int PacFile::getDataSize() {
		return data_size;
	}


	PacExtension::PacExtension() {
		name = "";
	}

	PacExtension::~PacExtension() {
		for (size_t i=0; i<files.size(); i++) {
			delete files[i];
		}
	}

	void PacExtension::setName(string v) {
		name = v;
	}

	string PacExtension::getName() {
		return name;
	}

	void PacExtension::addFile(PacFile *file) {
		if (!file) return;
		files.push_back(file);
	}

	size_t PacExtension::getInternalSize() {
		size_t internal_size = 0;
		for (size_t i=0; i<files.size(); i++) {
			internal_size += files[i]->getDataSize();
		}
		return internal_size;
	}

	vector<PacFile *> PacExtension::getFiles() {
		return files;
	}

	bool PacExtension::isEmpty() {
		return (files.size() == 0);
	}

	void PacExtension::extract(string folder, bool convert_textures, void (*callback)(string)) {
		// Don't extract depend files
		if (name == LIBGENS_PAC_EXTENSION_PAC_DEPEND_FULL) {
			return;
		}

		string file_extension = name;
		size_t double_dots=file_extension.find_first_of(":");
		if (double_dots!=string::npos) {
			file_extension.resize(double_dots);
		}

		for (size_t i=0; i<files.size(); i++) {
			string new_filename = folder + files[i]->getName() + "." + file_extension;

			if (convert_textures && (file_extension == "dds")) {
				string temp_filename="temp-tex.gtx";
				files[i]->save(temp_filename);

				string command="texconv2 -i " + temp_filename + " -o \"" + new_filename + "\"";
				system(command.c_str());
			}
			else {
				if (callback) {
					(*callback)(files[i]->getName());
				}

				files[i]->save(new_filename);
			}
		}
	}


	void PacPack::extract(string folder, bool convert_textures, void (*callback)(string)) {
		for (size_t i=0; i<extensions.size(); i++) {
			extensions[i]->extract(folder, convert_textures, callback);
		}
	}


	PacExtension *PacPack::getExtension(string ext) {
		ext = extensionToFull(ext);
		if (!ext.size()) {
			printf("Unrecognized extension %s, abort!\n", ext.c_str());
			getchar();
			return NULL;
		}
		return getExtensionFull(ext);
	}

	PacExtension *PacPack::getExtensionFull(string ext) {
		for (size_t i=0; i<extensions.size(); i++) {
			if (extensions[i]->getName() == ext) {
				return extensions[i];
			}
		}

		PacExtension *pac_extension = new PacExtension();
		pac_extension->setName(ext);
		extensions.push_back(pac_extension);
		return pac_extension;
	}

	void PacPack::deleteExtension(string ext) {
		ext = extensionToFull(ext);
		if (!ext.size()) {
			return;
		}

		for (size_t i=0; i<extensions.size(); i++) {
			if (extensions[i]->getName() == ext) {
				delete extensions[i];
				extensions.erase(extensions.begin() + i);
				return;
			}
		}
	}

	
	void PacPack::createExtensions() {
		getExtension(LIBGENS_PAC_EXTENSION_ANM);
		getExtension(LIBGENS_PAC_EXTENSION_LIGHT);
		getExtension(LIBGENS_PAC_EXTENSION_LFT);
		getExtension(LIBGENS_PAC_EXTENSION_GISM);
		getExtension(LIBGENS_PAC_EXTENSION_HHD);
		getExtension(LIBGENS_PAC_EXTENSION_PAC_PHY_HKX);
		getExtension(LIBGENS_PAC_EXTENSION_LUA);
		getExtension(LIBGENS_PAC_EXTENSION_MATERIAL);
		getExtension(LIBGENS_PAC_EXTENSION_TERAIN_INSTANCEINFO);
		getExtension(LIBGENS_PAC_EXTENSION_TERRAIN_MODEL);
		getExtension(LIBGENS_PAC_EXTENSION_PAC_DEPEND);
		getExtension(LIBGENS_PAC_EXTENSION_PAC_FXCOL);
		getExtension(LIBGENS_PAC_EXTENSION_PAC_RAW);
		getExtension(LIBGENS_PAC_EXTENSION_PAC_MAT_ANIM);
		getExtension(LIBGENS_PAC_EXTENSION_PAC_ANM_HKX);
		getExtension(LIBGENS_PAC_EXTENSION_PAC_UV_ANIM);
		getExtension(LIBGENS_PAC_EXTENSION_PAC_VIS_ANIM);
		getExtension(LIBGENS_PAC_EXTENSION_PAC_EFFECT);
		getExtension(LIBGENS_PAC_EXTENSION_PAC_MODEL);
		getExtension(LIBGENS_PAC_EXTENSION_PAC_SKL_HKX);
		getExtension(LIBGENS_PAC_EXTENSION_PAC_SWIF);
		getExtension(LIBGENS_PAC_EXTENSION_PAC_DDS);
		getExtension(LIBGENS_PAC_EXTENSION_PAC_PATH2);
		getExtension(LIBGENS_PAC_EXTENSION_PAC_SHADOW_MODEL);
		getExtension(LIBGENS_PAC_EXTENSION_PAC_PIXELSHADER);
		getExtension(LIBGENS_PAC_EXTENSION_PAC_PIXELSHADERCODE);
		getExtension(LIBGENS_PAC_EXTENSION_PAC_PIXELSHADERPARAMETER);
		getExtension(LIBGENS_PAC_EXTENSION_PAC_SHADERLIST);
		getExtension(LIBGENS_PAC_EXTENSION_PAC_VERTEXSHADER);
		getExtension(LIBGENS_PAC_EXTENSION_PAC_VERTEXSHADERCODE);
		getExtension(LIBGENS_PAC_EXTENSION_PAC_VERTEXSHADERPARAMETER);
		getExtension(LIBGENS_PAC_EXTENSION_PAC_SM4SHADERCONTAINER);
		getExtension(LIBGENS_PAC_EXTENSION_PAC_XTB2DATA);
	}

	void PacPack::cleanUnusedExtensions() {
		for (size_t i=0; i<extensions.size(); i++) {
			if (extensions[i]->isEmpty()) {
				delete extensions[i];
				extensions.erase(extensions.begin() + i);
				i-=1;
			}
		}
	}

	string PacPack::extensionToFull(string ext) {
		if (ext == LIBGENS_PAC_EXTENSION_ANM) {
			return LIBGENS_PAC_EXTENSION_ANM_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_LIGHT) {
			return LIBGENS_PAC_EXTENSION_LIGHT_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_LFT) {
			return LIBGENS_PAC_EXTENSION_LFT_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_GISM) {
			return LIBGENS_PAC_EXTENSION_GISM_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_HHD) {
			return LIBGENS_PAC_EXTENSION_HHD_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_PAC_PHY_HKX) {
			return LIBGENS_PAC_EXTENSION_PAC_PHY_HKX_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_LUA) {
			return LIBGENS_PAC_EXTENSION_PAC_LUA_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_MATERIAL) {
			return LIBGENS_PAC_EXTENSION_MATERIAL_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_TERAIN_INSTANCEINFO) {
			return LIBGENS_PAC_EXTENSION_TERAIN_INSTANCEINFO_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_TERRAIN_MODEL) {
			return LIBGENS_PAC_EXTENSION_TERRAIN_MODEL_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_PAC_DEPEND) {
			return LIBGENS_PAC_EXTENSION_PAC_DEPEND_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_PAC_FXCOL) {
			return LIBGENS_PAC_EXTENSION_PAC_FXCOL_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_PAC_RAW) {
			return LIBGENS_PAC_EXTENSION_PAC_RAW_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_PAC_MAT_ANIM) {
			return LIBGENS_PAC_EXTENSION_PAC_MAT_ANIM_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_PAC_ANM_HKX) {
			return LIBGENS_PAC_EXTENSION_PAC_ANM_HKX_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_PAC_UV_ANIM) {
			return LIBGENS_PAC_EXTENSION_PAC_UV_ANIM_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_PAC_VIS_ANIM) {
			return LIBGENS_PAC_EXTENSION_PAC_VIS_ANIM_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_PAC_EFFECT) {
			return LIBGENS_PAC_EXTENSION_PAC_EFFECT_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_PAC_MODEL) {
			return LIBGENS_PAC_EXTENSION_PAC_MODEL_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_PAC_SKL_HKX) {
			return LIBGENS_PAC_EXTENSION_PAC_SKL_HKX_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_PAC_DDS) {
			return LIBGENS_PAC_EXTENSION_PAC_DDS_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_PAC_SHADOW_MODEL) {
			return LIBGENS_PAC_EXTENSION_PAC_SHADOW_MODEL_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_PAC_SWIF) {
			return LIBGENS_PAC_EXTENSION_PAC_SWIF_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_PAC_PATH2) {
			return LIBGENS_PAC_EXTENSION_PAC_PATH2_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_PAC_PIXELSHADER) {
			return LIBGENS_PAC_EXTENSION_PAC_PIXELSHADER_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_PAC_PIXELSHADERCODE) {
			return LIBGENS_PAC_EXTENSION_PAC_PIXELSHADERCODE_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_PAC_PIXELSHADERPARAMETER) {
			return LIBGENS_PAC_EXTENSION_PAC_PIXELSHADERPARAMETER_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_PAC_SHADERLIST) {
			return LIBGENS_PAC_EXTENSION_PAC_SHADERLIST_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_PAC_VERTEXSHADER) {
			return LIBGENS_PAC_EXTENSION_PAC_VERTEXSHADER_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_PAC_VERTEXSHADERCODE) {
			return LIBGENS_PAC_EXTENSION_PAC_VERTEXSHADERCODE_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_PAC_VERTEXSHADERPARAMETER) {
			return LIBGENS_PAC_EXTENSION_PAC_VERTEXSHADERPARAMETER_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_PAC_SM4SHADERCONTAINER) {
			return LIBGENS_PAC_EXTENSION_PAC_SM4SHADERCONTAINER_FULL;
		}
		else if (ext == LIBGENS_PAC_EXTENSION_PAC_XTB2DATA) {
			return LIBGENS_PAC_EXTENSION_PAC_XTB2DATA_FULL;
		}
		else {
			return "";
		}  
	}


	void PacPack::addFile(string filename) {
		string extension = File::extensionFromFilename(filename);
		string name = File::nameFromFilenameNoExtension(filename);

		if (File::check(filename)) {
			PacExtension *pac_extension = getExtension(extension);

			if (pac_extension) {
				PacFile *pac_file = new PacFile(filename);

				if (pac_file) {
					pac_extension->addFile(pac_file);
				}
			}
		}
	}

	void PacPack::addFolder(string folder) {
		set<string> filenames;

		WIN32_FIND_DATA FindFileData;
		HANDLE hFind;
		hFind = FindFirstFile((folder+"*.*").c_str(), &FindFileData);
		if (hFind == INVALID_HANDLE_VALUE) {} 
		else {
			do {
				const char *name=FindFileData.cFileName;
				if (name[0]=='.') continue;

				string new_filename=folder+ToString(name);
				filenames.insert(new_filename);
			} while (FindNextFile(hFind, &FindFileData) != 0);
			FindClose(hFind);
		}

		for(set<string>::iterator it=filenames.begin(); it!=filenames.end(); it++) {
			addFile(*it);
		}
	}


	void PacFile::write(File *file, GensStringTable *string_table) {
		unsigned int empty_flag = (data ? 0 : 0x80);
		
		file_data_address = file->getCurrentAddress();

		// Write Header and Data
		file->writeInt32(&data_size);
		file->writeNull(8);
		file->writeInt32(&empty_flag);

		size_t data_address = file->getCurrentAddress();
		if (data) {
			file->write(data, data_size);
			// Fix offsets based on metadata PAC Table
			for (size_t i=0; i<meta_pac_extra_strings.size(); i++) {
				file->goToAddress(meta_pac_extra_strings[i].relative_address + data_address);

				if (meta_pac_extra_strings[i].name.size()) {
					string_table->writeString(file, meta_pac_extra_strings[i].name);
				}
				else {
					size_t new_address = meta_pac_extra_strings[i].address + data_address;
					file->writeInt32A(&new_address);
				}
			}
		}
	}

	void PacFile::hashInput(SHA1Context &sha1_context) {
		if (data) {
			SHA1Input(&sha1_context, data, data_size);
		}
	}

	PacProxyEntry::PacProxyEntry() {
		name = "";
		extension = "";
		index = 0;
	}

	void PacProxyEntry::setName(string v) {
		name = v;
	}

	void PacProxyEntry::setExtension(string v) {
		extension = v;
	}

	void PacProxyEntry::setIndex(unsigned int v) {
		index = v;
	}

	void PacProxyEntry::write(File *file, GensStringTable *string_table) {
		string_table->writeString(file, extension);
		string_table->writeString(file, name);
		file->writeInt32(&index);
	}

	void PacExtension::write(File *file, GensStringTable *string_table) {
		unsigned int files_count = files.size();
		unsigned int files_table_address = file->getCurrentAddress() + 8;

		file_address = file->getCurrentAddress();

		//LibGens::Error::printfMessage(LibGens::Error::LOG, "Extension Address: %d", file_address);

		// Reserve Space for Extensions section
		file->writeInt32(&files_count);
		file->writeInt32A(&files_table_address);

		// Write Table
		for (size_t i=0; i<files_count; i++) {
			string_table->writeString(file, files[i]->getName());
			file->writeNull(4);
		}
	}

	void PacExtension::writeData(File *file, GensStringTable *string_table) {
		for (size_t i=0; i<files.size(); i++) {
			file->fixPadding(16);
			files[i]->write(file, string_table);
			file->goToEnd();
		}
	}

	void PacExtension::proxyFiles() {
		for (size_t i=0; i<files.size(); i++) {
			PacFile *original = files[i];
			PacFile *proxy = new PacFile(original);
			files[i] = proxy;
		}
	}

	void PacExtension::deleteFiles() {
		for (size_t i=0; i<files.size(); i++) {
			delete files[i];
		}

		files.clear();
	}


	void PacFile::writeFixed(File *file) {
		file->writeInt32A(&file_data_address);
	}

	void PacExtension::writeFixed(File *file) {
		file->writeInt32A(&file_address);

		for (size_t i=0; i<files.size(); i++) {
			file->goToAddress(file_address + 12 + i*8);
			files[i]->writeFixed(file);
		}
	}

	void PacExtension::hashInput(SHA1Context &sha1_context) {
		for (size_t i=0; i<files.size(); i++) {
			files[i]->hashInput(sha1_context);
		}
	}

	/** PacPack */

	PacPack::PacPack() {
	}

	PacPack::~PacPack() {
		for (size_t i=0; i < extensions.size(); i++) {
			delete extensions[i];
		}

		for (size_t i=0; i < proxy_entries.size(); i++) {
			delete proxy_entries[i];
		}
	}

	void PacPack::save(string filename) {
		cleanUnusedExtensions();

		File file(filename, LIBGENS_FILE_WRITE_BINARY);
		if (file.valid()) {
			GensStringTable string_table;
			file.setRootNodeAddress(0);

			// Header Section
			unsigned int file_size = 0;
			char header[] = "PACx201L";
			file.write(header, strlen(header));
			file.writeNull(4);

			unsigned int unknown_1 = 0x1;
			file.writeInt32(&unknown_1);

			// Data Section
			unsigned int data_size=0;
			unsigned int file_data_size=0;
			unsigned int extension_table_size=0;
			unsigned int proxy_table_size=0;
			unsigned int string_table_data_size=0;
			unsigned int offset_table_size=0;

			char data_header[] = "DATA";
			file.write(data_header, strlen(data_header));

			unsigned int data_flag = 0x1;
			file.writeNull(24);
			file.writeInt32(&data_flag);

			// Extensions Section
			size_t extension_section_address = file.getCurrentAddress();
			{
				unsigned int extensions_count = extensions.size();
				unsigned int extensions_table_address = file.getCurrentAddress() + 8;

				// Reserve Space for Extensions section
				file.writeInt32(&extensions_count);
				file.writeInt32A(&extensions_table_address);

				for (size_t i=0; i<extensions_count; i++) {
					string_table.writeString(&file, extensions[i]->getName());
					file.writeNull(4);
				}

				// Reserve Space for Extension Sub Tables
				for (size_t i=0; i<extensions.size(); i++) {
					extensions[i]->write(&file, &string_table);
				}

				extension_table_size = file.getCurrentAddress() - extension_section_address;
			}

			// Data Section
			size_t data_section_address = file.getCurrentAddress();
			{
				for (size_t i=0; i<extensions.size(); i++) {
					extensions[i]->writeData(&file, &string_table);
				}

				file_data_size = file.getCurrentAddress() - data_section_address;
			}

			// Extension Section Fix Offsets
			{
				for (size_t i=0; i<extensions.size(); i++) {
					file.goToAddress(extension_section_address + i*8 + 12);
					extensions[i]->writeFixed(&file);
				}
				file.goToEnd();
			}

			// Proxy Entries Section
			size_t proxy_section_address = file.getCurrentAddress();
			if (proxy_entries.size()) {
				unsigned int proxy_table_count=proxy_entries.size();
				unsigned int proxy_table_offset=file.getCurrentAddress() + 8;
				file.writeInt32(&proxy_table_count);
				file.writeInt32A(&proxy_table_offset);

				for (size_t i=0; i<proxy_table_count; i++) {
					proxy_entries[i]->write(&file, &string_table);
				}

				proxy_table_size = file.getCurrentAddress() - proxy_section_address;
			}
			else {
				proxy_table_size = 0;
			}

			// String Table Section
			size_t string_table_section_address = file.getCurrentAddress();
			{
				string_table.write(&file, false);
				file.goToEnd();
				file.fixPadding(4);

				string_table_data_size = file.getCurrentAddress() - string_table_section_address;
			}

			// Offset Table Section
			size_t offset_table_section_address = file.getCurrentAddress();
			{
				file.sortAddressTable();
				file.writeAddressTableBBIN();
				file.fixPadding(4);
				offset_table_size = file.getCurrentAddress() - offset_table_section_address;
			}
			
			// Fix Sizes on header
			file.goToEnd();
			file_size = file.getCurrentAddress();
			file.goToAddress(8);
			file.writeInt32(&file_size);


			file.goToAddress(20);
			data_size = file_size - 16;

			file.writeInt32(&data_size);
			file.writeInt32(&file_data_size);
			file.writeInt32(&extension_table_size);
			file.writeInt32(&proxy_table_size);
			file.writeInt32(&string_table_data_size);
			file.writeInt32(&offset_table_size);
		}
	}

	vector<PacExtension *> PacPack::getExtensions() {
		return extensions;
	}

	void PacPack::addProxy(PacProxyEntry *entry) {
		proxy_entries.push_back(entry);
	}

	void PacPack::setName(string v) {
		name = v;
	}

	string PacPack::getName() {
		return name;
	}

	size_t PacPack::getInternalSize() {
		size_t internal_size = 0;
		for (size_t i=0; i<extensions.size(); i++) {
			internal_size += extensions[i]->getInternalSize();
		}

		return internal_size;
	}

	void PacPack::hashInput(SHA1Context &sha1_context) {
		for (size_t i=0; i<extensions.size(); i++) {
			extensions[i]->hashInput(sha1_context);
		}
	}


	PacSet::PacSet() {
		name = folder = "";
		for (int i=0; i<5; i++) {
			sha1_hash[i] = 0;
		}
	}

	PacSet::~PacSet() {
		for (size_t i=0; i<packs.size(); i++) {
			delete packs[i];
		}
	}

	PacSet::PacSet(string filename) {
		name = File::nameFromFilename(filename);
		folder = File::folderFromFilename(filename);

		PacPack *root_pack = new PacPack(filename);

		if (root_pack) {
			packs.push_back(root_pack);

			// Search for Pac Depend
			PacExtension *depend_extension = root_pack->getExtension(LIBGENS_PAC_EXTENSION_PAC_DEPEND);

			if (depend_extension) {
				vector<PacFile *> depend_files = depend_extension->getFiles();
				for (size_t i=0; i<depend_files.size(); i++) {
					openDependFile(depend_files[i]);
				}
			}
		}

		// Generate SHA1 Hash
		SHA1Reset(&sha1_context);
		for (size_t i=0; i < packs.size(); i++) {
			packs[i]->hashInput(sha1_context);
		}
		SHA1Result(&sha1_context);

		for (int i=0; i<5; i++) {
			sha1_hash[i] = sha1_context.Message_Digest[i];
		}
	}

	int PacSet::getSHA1Hash(int i) {
		return sha1_hash[i];
	}

	void PacSet::openDependFile(PacFile *file) {
		if (!file) return;

		vector<string> pac_depend_names = file->getPacDependNames();
		for (size_t i=0; i<pac_depend_names.size(); i++) {
			string filename = folder + pac_depend_names[i];

			PacPack *depend_pack = new PacPack(filename);
			if (depend_pack) {
				packs.push_back(depend_pack);
			}
		}
	}

	void PacSet::addFolder(string target_folder) {
		if (!packs.size()) {
			packs.push_back(new PacPack());
		}

		PacPack *root_pack = packs[0];
		if (root_pack) {
			root_pack->createExtensions();
			root_pack->addFolder(target_folder);
		}
	}

	void PacSet::splitPacks() {
		if (packs.size() == 1) {
			PacPack *root_pack = packs[0];

			// Fake Pack
			PacPack *split_pack = new PacPack();
			split_pack->createExtensions();

			// Move any of the special extensions into a secondary file
			// Proxy anything that is a special extension
			vector<PacExtension *> extensions = root_pack->getExtensions();
			for (size_t i=0; i<extensions.size(); i++) {
				if (extensions[i]->isSpecialExtension()) {
					vector<PacFile *> extension_files = extensions[i]->getFiles();
					extensions[i]->proxyFiles();

					PacExtension *new_extension = split_pack->getExtensionFull(extensions[i]->getName());
					if (new_extension) {
						for (size_t j=0; j<extension_files.size(); j++) {
							new_extension->addFile(extension_files[j]);

							PacProxyEntry *proxy_entry = new PacProxyEntry();
							proxy_entry->setName(extension_files[j]->getName());
							proxy_entry->setExtension(extensions[i]->getName());
							proxy_entry->setIndex(j);
							root_pack->addProxy(proxy_entry);
						}
					}
				}
			}

			// Split secondary file into new ones if it's bigger than the limit
			PacPack *current_split_pack = NULL;
			size_t extension_index = 0;
			size_t file_index = 0;

			extensions = split_pack->getExtensions();
			vector<PacFile *> extension_files;

			if (extensions.size()) {
				extension_files = extensions[extension_index]->getFiles();
			}
			
			size_t current_size = split_pack->getInternalSize();

			if (current_size < LIBGENS_PAC_SPLIT_BYTES_LIMIT) {
				packs.push_back(split_pack);
			}
			else {
				size_t next_file_size = 0;

				while (true) {
					if (!current_split_pack || (current_split_pack->getInternalSize() > LIBGENS_PAC_SPLIT_BYTES_LIMIT+next_file_size)) {
						current_split_pack = new PacPack();
						current_split_pack->createExtensions();
						packs.push_back(current_split_pack);
					}

					if (file_index < extension_files.size()) {
						PacExtension *current_extension = current_split_pack->getExtensionFull(extensions[extension_index]->getName());
						if (current_extension) {
							current_extension->addFile(extension_files[file_index]);
						}
						file_index++;
					}

					if (file_index >= extension_files.size()) {
						file_index = 0;
						extension_index++;

						if (extension_index >= extensions.size()) {
							break;
						}
						else {
							extension_files = extensions[extension_index]->getFiles();
						}
					}

					if (file_index < extension_files.size()) {
						next_file_size = extension_files[file_index]->getDataSize();
					}
				}
			}
		}
	}

	void PacSet::extract(string target_folder, bool convert_textures, void (*callback)(string)) {
		for (size_t i=0; i<packs.size(); i++) {
			packs[i]->extract(target_folder, convert_textures, callback);
		}
	}

	void PacSet::save(string filename) {
		if (!packs.size()) {
			return;
		}

		// Save the extra packs
		vector<string> extra_pack_names;

		for (size_t i=1; i<packs.size(); i++) {
			char extension[]=".00";
			sprintf(extension, ".%02d", i-1);

			string save_filename = filename + ToString(extension);
			packs[i]->save(save_filename);

			extra_pack_names.push_back(File::nameFromFilename(save_filename));
		}

		// Generate a new PAC Depend
		// If only a single pack, delete the extension
		if (packs.size() == 1) {
			packs[0]->deleteExtension(LIBGENS_PAC_EXTENSION_PAC_DEPEND);
		}
		else {
			PacExtension *depend_extension = packs[0]->getExtension(LIBGENS_PAC_EXTENSION_PAC_DEPEND);
			if (depend_extension) {
				depend_extension->deleteFiles();

				PacFile *depend_file = new PacFile(extra_pack_names);
				depend_file->setName(File::nameFromFilenameNoExtension(filename));
				depend_extension->addFile(depend_file);
			}
		}

		packs[0]->save(filename);
	}
};
