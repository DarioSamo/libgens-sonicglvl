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

#define LIBGENS_BIXF_GO_TO_PARENT        0x00
#define LIBGENS_BIXF_NEW_NODE            0x21
#define LIBGENS_BIXF_NEW_PARAMETER       0x41
#define LIBGENS_BIXF_NEW_VALUE           0x61

#define LIBGENS_BIXF_NEW_NODE_TABLE      0x29
#define LIBGENS_BIXF_NEW_PARAMETER_TABLE 0x49
#define LIBGENS_BIXF_NEW_VALUE_TABLE     0x69
#define LIBGENS_BIXF_NEW_VALUE_BOOL      0x70
#define LIBGENS_BIXF_NEW_VALUE_INT       0x74
#define LIBGENS_BIXF_NEW_VALUE_UINT      0x75
#define LIBGENS_BIXF_NEW_VALUE_FLOAT     0x76

#define LIBGENS_BIXF_MODE_PARTICLES      0x00
#define LIBGENS_BIXF_MODE_EVENT          0x01

namespace LibGens {
	class BIXFConverter {
		public:
			static const size_t IDTableSize;
			static const string IDTable[];
			static const size_t IDTableEventSize;
			static const string IDTableEvent[];
			static const size_t ValueTableSize;
			static const string ValueTable[];
			static string nodeIDtoString(unsigned char id, int mode_flag);
			static string valueIDtoString(unsigned char id, int mode_flag);
			static bool isOnNodeIDTable(string v, unsigned char &id, int mode_flag);
			static bool isOnValueIDTable(string v, unsigned char &id);
			static void convertToXML(string source, string dest, int mode_flag=0);
			static void convertToBIXF(string source, string dest, int mode_flag=0);
			static void convertToBIXF(TiXmlElement *pElem, vector<string> &string_table, vector<unsigned char> &data, int mode_flag);
			static unsigned char createBIXFstring(string value, vector<string> &string_table);
	};
};
