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

namespace LibGens {
	class SonicString {
		public:
			vector<size_t> addresses;
			string value;
	};

	class SonicStringTable {
		protected:
			vector<SonicString> strings;
			size_t null_string_count;
		public:
			SonicStringTable() {
				null_string_count = 0;
			}

			void writeString(File *file, string str);
			void write(File *file);

			void clear() {
				strings.clear();
				null_string_count = 0;
			}
	};

	class SonicOffsetTableEntry {
		public:
			unsigned char code;
			size_t offset;

			SonicOffsetTableEntry(unsigned char c, size_t of) {
				code = c;
				offset = of;
			}

			void write(File *file);
	};

	class SonicOffsetTable {
		protected:
			list<SonicOffsetTableEntry> entries;
		public:
			SonicOffsetTable() {
			}

			void addEntry(unsigned char c, size_t of);
			void printList();

			void write(File *file);
			void writeSize(File *file);
	};
};
