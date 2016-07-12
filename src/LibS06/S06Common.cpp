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

namespace LibGens {
	void SonicStringTable::writeString(File *file, string str) {
		file->writeNull(4);

		if (!str.size()) {
			SonicString new_string;
			new_string.addresses.clear();
			new_string.addresses.push_back(file->getCurrentAddress()-4);
			new_string.value = "";
			strings.insert(strings.begin()+null_string_count, new_string);
			null_string_count++;
			return;
		}

		for (size_t i=0; i<strings.size(); i++) {
			if (strings[i].value == str) {
				strings[i].addresses.push_back(file->getCurrentAddress()-4);
				return;
			}
		}

		SonicString new_string;
		new_string.addresses.clear();
		new_string.addresses.push_back(file->getCurrentAddress()-4);
		new_string.value = str;
		strings.push_back(new_string);
	}

	void SonicStringTable::write(File *file) {
		for (size_t i=0; i<strings.size(); i++) {
			size_t address=file->getCurrentAddress();
			if (strings[i].value.size()) file->writeString(&strings[i].value);
			else {
				file->writeNull(4);
			}

			for (size_t j=0; j<strings[i].addresses.size(); j++) {
				file->goToAddress(strings[i].addresses[j]);
				file->writeInt32BEA(&address);
			}

			file->goToEnd();
		}
	}

	void SonicOffsetTable::addEntry(unsigned char c, size_t of) {
		SonicOffsetTableEntry entry(c, of);

		bool found=false;
		for (list<SonicOffsetTableEntry>::iterator it=entries.begin(); it!=entries.end(); it++) {
			if (entry.offset <= (*it).offset) {
				entries.insert(it, entry);
				found = true;
				break;
			}
		}

		if (!found) entries.push_back(entry);
	}


	void SonicOffsetTable::printList() {
		size_t index=0;
		for (list<SonicOffsetTableEntry>::iterator it=entries.begin(); it!=entries.end(); it++) {
			Error::addMessage(Error::WARNING, ToString((*it).code) + "  (" + ToString((*it).offset) + ") " + ToString(index));
			index++;
		}

		Error::addMessage(Error::WARNING, "Done");
	}

	void SonicOffsetTableEntry::write(File *file) {
		file->writeUChar(&code);
	}

	void SonicOffsetTable::write(File *file) {
		for (list<SonicOffsetTableEntry>::iterator it=entries.begin(); it!=entries.end(); it++) {
			(*it).write(file);
		}

		file->fixPadding();
	}

	void SonicOffsetTable::writeSize(File *file) {
		unsigned int total_entries=entries.size();
		file->writeInt32BE(&total_entries);
	}
}