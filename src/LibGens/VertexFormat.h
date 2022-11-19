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

namespace LibGens {
	enum VertexElementData {
		FLOAT1 = 0x2C83A4,
		FLOAT2 = 0x2C23A5,
		FLOAT3 = 0x2A23B9,
		FLOAT4 = 0x1A23A6,
		D3DCOLOR = 0x182886,
		UBYTE4 = 0x1A2286,
		UBYTE4_2 = 0x1A2386,
		SHORT2 = 0x2C2359,
		SHORT4 = 0x1A235A,
		UBYTE4N = 0x1A2086,
		UBYTE4N_2 = 0x1A2186,
		SHORT2N = 0x2C2159,
		SHORT4N = 0x1A215A,
		USHORT2N = 0x2C2059,
		USHORT4N = 0x1A205A,
		UDEC3 = 0x2A2287,
		DEC3N = 0x2A2187,
		DEC3N_360 = 0x2A2190,
		FLOAT2_HALF = 0x2C235F,
		FLOAT4_HALF = 0x1A2360,
		USHORT4 = 0x1A225A
	};

	enum VertexElementID {	
		POSITION = 0,
		BONE_WEIGHTS = 1,
		BONE_INDICES = 2,
		NORMAL = 3,
		PSIZE = 4,
		UV = 5,
		TANGENT = 6,
		BINORMAL = 7,
		TESSFACTOR = 8,
		POSITION_T = 9,
		COLOR = 10,
		FOG = 11,
		DEPTH = 12,
		SAMPLE = 13
	};

	// Vertex Format Table declarations
	class VertexFormatElement {
		protected:
			unsigned int offset;
			VertexElementData data;
			VertexElementID id;
			unsigned short index;
		public:
			VertexFormatElement();
			VertexFormatElement(unsigned int offset_p, VertexElementData data_p, VertexElementID id_p, size_t index_p);
			unsigned int getOffset();
			void setOffset(unsigned int offset_p);
			VertexElementData getData();
			void setData(VertexElementData data_p);
			VertexElementID getID();
			void setID(VertexElementID id_p);
			unsigned short getIndex();
			void setIndex(unsigned short index_p);
	};

	class VertexFormat {
		friend class VertexFormat;

		protected:
			list<VertexFormatElement> elements;
			unsigned int size;
		public:
			VertexFormat();
			VertexFormat(VertexFormat *clone);
			VertexFormat(unsigned int type);
			void addElement(VertexFormatElement vfe);
			list<VertexFormatElement> getElements();
			void read(File *file);
			void write(File *file);
			void setSize(unsigned int v);
			unsigned int getSize();
			void fixForPC();

	};
};