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
	SonicXNFile::SonicXNFile(XNFileMode file_mode_parameter) {
		info=NULL;
		offset_table=NULL;
		footer=NULL;
		end=NULL;
		big_endian=false;

		file_mode = file_mode_parameter;

		setHeaders();

		// Create Essential Sections
		info = new SonicXNInfo();
		info->setHeader(header_info);
		info->setFileMode(file_mode);
		info->setBigEndian(big_endian);

		offset_table = new SonicXNOffsetTable();
		offset_table->setHeader(LIBGENS_XNSECTION_HEADER_OFFSET_TABLE);
		offset_table->setFileMode(file_mode);
		offset_table->setBigEndian(big_endian);

		footer=new SonicXNFooter();
		footer->setHeader(LIBGENS_XNSECTION_HEADER_FOOTER);
		footer->setFileMode(file_mode);
		footer->setBigEndian(big_endian);

		end=new SonicXNEnd();
		end->setHeader(LIBGENS_XNSECTION_HEADER_END);
		end->setFileMode(file_mode);
		end->setBigEndian(big_endian);
	}

	SonicXNFile::SonicXNFile(string filename, XNFileMode file_mode_parameter) {
		File file(filename, LIBGENS_FILE_READ_BINARY);

		info=NULL;
		offset_table=NULL;
		footer=NULL;
		end=NULL;
		big_endian=false;

		folder = filename;
		size_t sz=folder.size();
		int last_slash=0;
		for (size_t i=0; i<sz; i++) {
			if ((folder[i] == '\\') || (folder[i] == '/')) last_slash=i;
		}
		if (last_slash) folder.erase(last_slash+1, folder.size()-last_slash-1);
		else folder="";
		
		std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
		
		if ((filename.find(LIBGENS_XNO_EXTENSION)      != string::npos) || (filename.find(LIBGENS_XNM_EXTENSION)      != string::npos) || (file_mode_parameter == MODE_XNO)) {
			file_mode = MODE_XNO;
		}
		else if (filename.find(LIBGENS_ZNO_EXTENSION) != string::npos  || (filename.find(LIBGENS_ZNM_EXTENSION)      != string::npos) || (file_mode_parameter == MODE_ZNO)) {
			file_mode = MODE_ZNO;
		}
		else if (filename.find(LIBGENS_INO_EXTENSION) != string::npos  || (filename.find(LIBGENS_INM_EXTENSION)      != string::npos) || (file_mode_parameter == MODE_INO)) {
			file_mode = MODE_INO;
		}
		else if ((filename.find(LIBGENS_GNO_EXTENSION) != string::npos) || (filename.find(LIBGENS_GNM_EXTENSION) != string::npos) || (filename.find(LIBGENS_GNA_EXTENSION) != string::npos) || (file_mode_parameter == MODE_GNO)) {
			file_mode = MODE_GNO;
		}
		else if (filename.find(LIBGENS_ENO_EXTENSION) != string::npos || (file_mode_parameter == MODE_ENO)) {
			file_mode = MODE_ENO;
		}
		else if (file_mode_parameter == MODE_YNO) {
			file_mode = MODE_YNO;
		}

		setHeaders();

		if (file.valid()) {
			while (!end) {
				readSection(&file);
			}
			file.close();
		}

		SonicXNObject *object=getObject();
		if (object) {
			string name="Object";
			if (footer) name=footer->name;
			object->setNames(name);
		}
	}

	void SonicXNFile::setHeaders() {
		switch (file_mode) {
			case MODE_XNO:
				header_info    = LIBGENS_XNSECTION_HEADER_INFO_XNO;
				header_texture = LIBGENS_XNSECTION_HEADER_TEXTURE_XNO;
				header_effect  = LIBGENS_XNSECTION_HEADER_EFFECT_XNO;
				header_object  = LIBGENS_XNSECTION_HEADER_OBJECT_XNO;
				header_bones   = LIBGENS_XNSECTION_HEADER_BONES_XNO;
				header_motion  = LIBGENS_XNSECTION_HEADER_MOTION_XNO;
				break;

			case MODE_ZNO:
				header_info    = LIBGENS_XNSECTION_HEADER_INFO_ZNO;
				header_texture = LIBGENS_XNSECTION_HEADER_TEXTURE_ZNO;
				header_effect  = LIBGENS_XNSECTION_HEADER_EFFECT_ZNO;
				header_object  = LIBGENS_XNSECTION_HEADER_OBJECT_ZNO;
				header_bones   = LIBGENS_XNSECTION_HEADER_BONES_ZNO;
				header_motion  = LIBGENS_XNSECTION_HEADER_MOTION_ZNO;
				break;

			case MODE_INO:
				header_info    = LIBGENS_XNSECTION_HEADER_INFO_INO;
				header_texture = LIBGENS_XNSECTION_HEADER_TEXTURE_INO;
				header_effect  = LIBGENS_XNSECTION_HEADER_EFFECT_INO;
				header_object  = LIBGENS_XNSECTION_HEADER_OBJECT_INO;
				header_bones   = LIBGENS_XNSECTION_HEADER_BONES_INO;
				header_motion  = LIBGENS_XNSECTION_HEADER_MOTION_INO;
				break;

			case MODE_GNO:
				big_endian = true;
				header_info    = LIBGENS_XNSECTION_HEADER_INFO_GNO;
				header_texture = LIBGENS_XNSECTION_HEADER_TEXTURE_GNO;
				header_effect  = LIBGENS_XNSECTION_HEADER_EFFECT_GNO;
				header_object  = LIBGENS_XNSECTION_HEADER_OBJECT_GNO;
				header_bones   = LIBGENS_XNSECTION_HEADER_BONES_GNO;
				header_motion  = LIBGENS_XNSECTION_HEADER_MOTION_GNO;
				break;

			case MODE_ENO:
				big_endian = true;
				header_info    = LIBGENS_XNSECTION_HEADER_INFO_ENO;
				header_texture = LIBGENS_XNSECTION_HEADER_TEXTURE_ENO;
				header_effect  = LIBGENS_XNSECTION_HEADER_EFFECT_ENO;
				header_object  = LIBGENS_XNSECTION_HEADER_OBJECT_ENO;
				header_bones   = LIBGENS_XNSECTION_HEADER_BONES_ENO;
				header_motion  = LIBGENS_XNSECTION_HEADER_MOTION_ENO;
				break;

			case MODE_YNO:
				big_endian = true;
				header_info    = LIBGENS_XNSECTION_HEADER_INFO_YNO;
				header_texture = LIBGENS_XNSECTION_HEADER_TEXTURE_XNO;
				header_effect  = LIBGENS_XNSECTION_HEADER_EFFECT_XNO;
				header_object  = LIBGENS_XNSECTION_HEADER_OBJECT_XNO;
				header_bones   = LIBGENS_XNSECTION_HEADER_BONES_XNO;
				header_motion  = LIBGENS_XNSECTION_HEADER_MOTION_XNO;
				break;
		}
	}


	void SonicXNSection::read(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_S06_XNINFO_ERROR_MESSAGE_NULL_FILE);
			return;
		}
		
		head_address = file->getCurrentAddress()-4;
		file->readInt32(&section_size);
	}

	void SonicXNSection::write(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_S06_XNINFO_ERROR_MESSAGE_NULL_FILE);
			return;
		}
		
		head_address = file->getCurrentAddress();
		file->write((void *) header.c_str(), 4);
		file->writeNull(4);

		writeBody(file);

		file->fixPadding(LIBGENS_XNSECTION_PADDING);
		size_t bookmark=file->getCurrentAddress();

		
		section_size = bookmark - head_address - LIBGENS_XNSECTION_HEADER_SIZE;
		file->goToAddress(head_address + 4);
		file->writeInt32(&section_size);


		file->goToAddress(bookmark);
	}


	void SonicXNSection::goToEnd(File *file) {
		file->goToAddress(head_address + section_size + 8);
	}


	void SonicXNInfo::read(File *file) {
		SonicXNSection::read(file);

		unsigned int file_root_address=0;
		file->readInt32E(&section_count, big_endian);
		file->readInt32E(&file_root_address, big_endian);
		file->setRootNodeAddress(file_root_address);
	}

	void SonicXNInfo::writeBody(File *file) {
		file->writeInt32(&section_count);
		unsigned int root_address=32;
		file->writeInt32(&root_address);
		file->writeNull(12);

		unsigned int unknown_value=1;
		file->writeInt32(&unknown_value);
	}


	void SonicXNInfo::writeFixed(File *file) {
		size_t address=file->getCurrentAddress();

		file->goToAddress(head_address + 16);
		file->writeInt32A(&offset_table_address);
		file->writeInt32(&offset_table_address_raw);
		file->writeInt32(&offset_table_size);

		file->goToAddress(address);
	}


	void SonicXNOffsetTable::read(File *file) {
		SonicXNSection::read(file);

		unsigned int offset_count=0;
		file->readInt32E(&offset_count, big_endian);
		file->moveAddress(4);

		for (size_t i=0; i<offset_count; i++) {
			size_t address=0;
			file->readInt32E(&address, big_endian);
			addresses.push_back(address);
		}
	}

	void SonicXNOffsetTable::writeBody(File *file) {
		unsigned int offset_count=addresses.size();
		file->writeInt32(&offset_count);
		file->writeNull(4);

		for (size_t i=0; i<offset_count; i++) {
			size_t address=addresses[i];
			file->writeInt32(&address);
		}
	}

	
	void SonicXNFooter::read(File *file) {
		SonicXNSection::read(file);

		file->moveAddress(8);
		file->readString(&name);
	}

	void SonicXNFooter::writeBody(File *file) {
		file->writeNull(8);
		file->writeString(&name);
	}


	SonicXNSection *SonicXNFile::readSection(File *file) {
		string identifier="";
		file->readString(&identifier, 4);

		if (identifier == header_info) {
			info = new SonicXNInfo();
			info->setHeader(header_info);
			info->setFileMode(file_mode);
			info->setBigEndian(big_endian);
			info->read(file);
			info->goToEnd(file);
			
			for (size_t i=0; i<info->getSectionCount(); i++) {
				SonicXNSection *section=readSection(file);
				if (section) {
					section->goToEnd(file);
					sections.push_back(section);
				}
			}
		}
		else if (identifier == LIBGENS_XNSECTION_HEADER_OFFSET_TABLE) {
			offset_table = new SonicXNOffsetTable();
			offset_table->setHeader(LIBGENS_XNSECTION_HEADER_OFFSET_TABLE);
			offset_table->setFileMode(file_mode);
			offset_table->setBigEndian(big_endian);
			offset_table->read(file);
			offset_table->goToEnd(file);
		}
		else if (identifier == header_texture) {
			SonicXNTexture *section=new SonicXNTexture();
			section->setHeader(header_texture);
			section->setFileMode(file_mode);
			section->setBigEndian(big_endian);
			section->read(file);
			return section;
		}
		else if (identifier == header_effect) {
			SonicXNEffect *section=new SonicXNEffect();
			section->setHeader(header_effect);
			section->setFileMode(file_mode);
			section->setBigEndian(big_endian);
			section->read(file);
			return section;
		}
		else if (identifier == header_object) {
			SonicXNObject *section=new SonicXNObject(getTexture(), getEffect(), getBones());
			section->setHeader(header_object);
			section->setFileMode(file_mode);
			section->setBigEndian(big_endian);
			section->read(file);

			return section;
		}
		else if (identifier == header_bones) {
			SonicXNBones *section=new SonicXNBones();
			section->setHeader(header_bones);
			section->setFileMode(file_mode);
			section->setBigEndian(big_endian);
			section->read(file);
			return section;
		}
		else if (identifier == header_motion) {
			SonicXNMotion *section=new SonicXNMotion();
			section->setHeader(header_motion);
			section->setFileMode(file_mode);
			section->setBigEndian(big_endian);
			section->read(file);
			return section;
		}
		else if (identifier == LIBGENS_XNSECTION_HEADER_FOOTER) {
			footer=new SonicXNFooter();
			footer->setHeader(LIBGENS_XNSECTION_HEADER_FOOTER);
			footer->setFileMode(file_mode);
			footer->setBigEndian(big_endian);
			footer->read(file);
			footer->goToEnd(file);
			return footer;
		}
		else if (identifier == LIBGENS_XNSECTION_HEADER_END) {
			end=new SonicXNEnd();
			end->setHeader(LIBGENS_XNSECTION_HEADER_END);
			end->setFileMode(file_mode);
			end->setBigEndian(big_endian);
			end->read(file);
			end->goToEnd(file);
			return end;
		}

		return NULL;
	}

	
	void SonicXNFile::save(string filename) {
		File file(filename, LIBGENS_FILE_WRITE_BINARY);

		if (file.valid()) {
			write(&file);
			file.close();
		}
	}

	void SonicXNFile::write(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_S06_XNINFO_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}
		file->setRootNodeAddress(32);

		if (info) {
			info->setSectionCount(sections.size());
			info->write(file);
		}

		for (size_t i=0; i<sections.size(); i++) {
			sections[i]->write(file);
		}

		if (offset_table) {
			offset_table->clear();
			
			file->sortAddressTable();
			list<size_t> table=file->getAddressTable();
			for (list<size_t>::iterator it=table.begin(); it!=table.end(); it++) {
				offset_table->push(*it);
			}

			offset_table->write(file);

			info->setOffsetTableAddress(offset_table->getAddress());
			info->setOffsetTableSize(offset_table->getSectionSize() + 8);

			info->writeFixed(file);
		}

		if (footer) footer->write(file);
		if (end) end->write(file);
	}

	void SonicXNFile::setFileMode(XNFileMode target_file_mode) {
		SonicXNObject  *object=getObject();
		SonicXNTexture *texture=getTexture();
		SonicXNEffect  *effect=getEffect();
		SonicXNBones   *bones=getBones();
		SonicXNMotion  *motion=getMotion();

		file_mode = target_file_mode;

		setHeaders();

		if (info) {
			info->setHeader(header_info);
			info->setFileMode(file_mode);
		}

		if (object)  {
			object->setFileMode(file_mode);
			object->setHeader(header_object);
		}
		if (texture) {
			texture->setFileMode(file_mode);
			texture->setHeader(header_texture);
		}
		if (effect)  {
			effect->setFileMode(file_mode);
			effect->setHeader(header_effect);
		}
		if (bones)   {
			bones->setFileMode(file_mode);
			bones->setHeader(header_bones);
		}
		if (motion)  {
			motion->setFileMode(file_mode);
			motion->setHeader(header_motion);
		}

		if (offset_table) offset_table->setFileMode(file_mode);
		if (footer) footer->setFileMode(file_mode);
		if (end) end->setFileMode(file_mode);
	}


	void SonicXNFile::createTextureSection() {
		SonicXNTexture *section=new SonicXNTexture();
		section->setHeader(header_texture);
		section->setFileMode(file_mode);
		section->setBigEndian(big_endian);
		sections.push_back(section);
	}

	void SonicXNFile::createEffectSection() {
		SonicXNEffect *section=new SonicXNEffect();
		section->setHeader(header_effect);
		section->setFileMode(file_mode);
		section->setBigEndian(big_endian);
		sections.push_back(section);
	}

	void SonicXNFile::createBoneSection() {
		SonicXNBones *section=new SonicXNBones();
		section->setHeader(header_bones);
		section->setFileMode(file_mode);
		section->setBigEndian(big_endian);
		sections.push_back(section);
	}

	void SonicXNFile::createObjectSection() {
		SonicXNObject *section=new SonicXNObject(getTexture(), getEffect(), getBones());
		section->setHeader(header_object);
		section->setFileMode(file_mode);
		section->setBigEndian(big_endian);
		sections.push_back(section);
	}
}