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

#include "StringTable.h"

namespace LibGens {
	GensStringTable::GensStringTable() {
		null_string_count = 0;
	}

	void GensStringTable::writeString(File *file, string str) {
		file->writeNull(4);

		if (!str.size()) {
			GensString new_string;
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

		GensString new_string;
		new_string.addresses.clear();
		new_string.addresses.push_back(file->getCurrentAddress()-4);
		new_string.value = str;
		strings.push_back(new_string);
	}

	void GensStringTable::write(File *file) {
		for (size_t i=0; i<strings.size(); i++) {
			size_t address=file->getCurrentAddress();
			if (strings[i].value.size()) file->writeString(&strings[i].value);
			else {
				file->writeNull(4);
			}

			for (size_t j=0; j<strings[i].addresses.size(); j++) {
				file->goToAddress(strings[i].addresses[j]);
				file->writeInt32EA(&address);
			}

			file->goToEnd();
		}
	}

	void GensStringTable::clear() {
		strings.clear();
		null_string_count = 0;
	}
}