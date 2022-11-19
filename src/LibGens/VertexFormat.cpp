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

#include "Vertex.h"
#include "VertexFormat.h"

namespace LibGens {
	VertexFormatElement::VertexFormatElement() {
	}

	VertexFormatElement::VertexFormatElement(unsigned int offset_p, VertexElementData data_p, VertexElementID id_p, size_t index_p) : offset(offset_p), data(data_p), id(id_p), index(index_p) {
	}

	unsigned int VertexFormatElement::getOffset() {
		return offset;
	}

	void VertexFormatElement::setOffset(unsigned int offset_p) {
		offset = offset_p;
	}

	VertexElementData VertexFormatElement::getData() {
		return data;
	}

	void VertexFormatElement::setData(VertexElementData data_p) {
		data = data_p;
	}

	VertexElementID VertexFormatElement::getID() {
		return id;
	}

	void VertexFormatElement::setID(VertexElementID id_p) {
		id = id_p;
	}

	unsigned short VertexFormatElement::getIndex() {
		return index;
	}

	void VertexFormatElement::setIndex(unsigned short index_p) {
		index = index_p;
	}

	VertexFormat::VertexFormat() {
		size=0;
	}

	VertexFormat::VertexFormat(unsigned int type) {
		switch (type) {
			case LIBGENS_VERTEX_FORMAT_PC:
			{
				addElement(VertexFormatElement(0, FLOAT3, POSITION, 0));
				addElement(VertexFormatElement(12, FLOAT3, NORMAL, 0));
				addElement(VertexFormatElement(24, FLOAT3, TANGENT, 0));
				addElement(VertexFormatElement(36, FLOAT3, BINORMAL, 0));
				addElement(VertexFormatElement(48, FLOAT2, UV, 0));
				addElement(VertexFormatElement(56, FLOAT2, UV, 1));
				addElement(VertexFormatElement(64, FLOAT2, UV, 2));
				addElement(VertexFormatElement(72, FLOAT2, UV, 3));
				addElement(VertexFormatElement(80, FLOAT4, COLOR, 0));
				addElement(VertexFormatElement(96, UBYTE4, BONE_INDICES, 0));
				addElement(VertexFormatElement(100, UBYTE4N, BONE_WEIGHTS, 0));
				size = 104;
				break;
			}

			case LIBGENS_VERTEX_FORMAT_PC_TERRAIN:
			{
				addElement(VertexFormatElement(0, FLOAT3, POSITION, 0));
				addElement(VertexFormatElement(12, FLOAT3, NORMAL, 0));
				addElement(VertexFormatElement(24, FLOAT3, TANGENT, 0));
				addElement(VertexFormatElement(36, FLOAT3, BINORMAL, 0));
				addElement(VertexFormatElement(48, FLOAT2, UV, 0));
				addElement(VertexFormatElement(56, FLOAT2, UV, 1));
				addElement(VertexFormatElement(64, FLOAT4, COLOR, 0));
				size = 80;
				break;
			}

			case LIBGENS_VERTEX_FORMAT_360:
			{
				addElement(VertexFormatElement(0, FLOAT3, POSITION, 0));
				addElement(VertexFormatElement(12, DEC3N_360, NORMAL, 0));
				addElement(VertexFormatElement(16, DEC3N_360, TANGENT, 0));
				addElement(VertexFormatElement(20, DEC3N_360, BINORMAL, 0));
				addElement(VertexFormatElement(24, FLOAT2_HALF, UV, 0));
				addElement(VertexFormatElement(28, FLOAT2_HALF, UV, 1));
				addElement(VertexFormatElement(32, FLOAT2_HALF, UV, 2));
				addElement(VertexFormatElement(36, FLOAT2_HALF, UV, 3));
				addElement(VertexFormatElement(40, UBYTE4N, COLOR, 0));
				addElement(VertexFormatElement(44, UBYTE4, BONE_INDICES, 0));
				addElement(VertexFormatElement(48, UBYTE4N, BONE_WEIGHTS, 0));
				size = 52;
				break;
			}

			case LIBGENS_VERTEX_FORMAT_360_TERRAIN:
			{
				addElement(VertexFormatElement(0, FLOAT3, POSITION, 0));
				addElement(VertexFormatElement(12, DEC3N_360, NORMAL, 0));
				addElement(VertexFormatElement(16, DEC3N_360, TANGENT, 0));
				addElement(VertexFormatElement(20, DEC3N_360, BINORMAL, 0));
				addElement(VertexFormatElement(24, FLOAT2_HALF, UV, 0));
				addElement(VertexFormatElement(28, FLOAT2_HALF, UV, 1));
				addElement(VertexFormatElement(32, UBYTE4N, COLOR, 0));
				size = 36;
				break;
			}

			case LIBGENS_VERTEX_FORMAT_FORCES:
			{
				addElement(VertexFormatElement(0, FLOAT3, POSITION, 0));
				addElement(VertexFormatElement(12, DEC3N, NORMAL, 0));
				addElement(VertexFormatElement(16, DEC3N, TANGENT, 0));
				addElement(VertexFormatElement(20, DEC3N, BINORMAL, 0));
				addElement(VertexFormatElement(24, FLOAT2_HALF, UV, 0));
				addElement(VertexFormatElement(28, UBYTE4N, COLOR, 0));
				addElement(VertexFormatElement(32, UBYTE4_2, BONE_INDICES, 0));
				addElement(VertexFormatElement(36, UBYTE4N, BONE_WEIGHTS, 0));
				size = 40;
				break;
			}

			case LIBGENS_VERTEX_FORMAT_FORCES_TERRAIN:
			{
				addElement(VertexFormatElement(0, FLOAT3, POSITION, 0));
				addElement(VertexFormatElement(12, DEC3N, NORMAL, 0));
				addElement(VertexFormatElement(16, DEC3N, TANGENT, 0));
				addElement(VertexFormatElement(20, DEC3N, BINORMAL, 0));
				addElement(VertexFormatElement(24, FLOAT2_HALF, UV, 0));
				addElement(VertexFormatElement(28, FLOAT2_HALF, UV, 1));
				addElement(VertexFormatElement(32, UBYTE4N, COLOR, 0));
				size = 36;
				break;
			}
		}
	}


	VertexFormat::VertexFormat(VertexFormat *clone) {
		elements = clone->elements;
		size = clone->size;
	}

	void VertexFormat::addElement(VertexFormatElement vfe) {
		elements.push_back(vfe);
	}

	
	void VertexFormat::read(File *file) {
		size_t header_address=file->getCurrentAddress();

		for (size_t i=0; i<LIBGENS_MAX_VERTEX_FORMAT_ENTRIES; i++) {
			file->goToAddress(header_address + i*12);

			unsigned int offset=0;
			unsigned int data=0;
			unsigned short id=0;
			unsigned char index=0;
			file->readInt32BE(&offset);

			if (offset > 1000) {
				break;
			}

			file->readInt32BE(&data);
			file->readInt16BE(&id);
			file->readUChar(&index);

			VertexFormatElement vfe;
			vfe.setOffset(offset);
			vfe.setData((VertexElementData) data);
			vfe.setID((VertexElementID) id);
			vfe.setIndex(index);
			elements.push_back(vfe);
		}
	}

	
	void VertexFormat::write(File *file) {
		size_t header_address=file->getCurrentAddress();
		for (list<VertexFormatElement>::iterator it=elements.begin(); it!=elements.end(); it++) {
			unsigned int offset=(*it).getOffset();
			unsigned int data=(*it).getData();
			unsigned short id=(*it).getID();
			unsigned char index=(*it).getIndex();
			file->writeInt32BE(&offset);
			file->writeInt32BE(&data);
			file->writeInt16BE(&id);
			file->writeUChar(&index);
			file->fixPadding();
		}

		unsigned int filler_offset=16711680;
		unsigned int filler_id=0xFFFFFFFF;
		file->writeInt32BE(&filler_offset);
		file->writeInt32BE(&filler_id);
		file->writeNull(4);
	}

	list<VertexFormatElement> VertexFormat::getElements() {
		return elements;
	}

	void VertexFormat::setSize(unsigned int v) {
		size=v;
	}

	unsigned int VertexFormat::getSize() {
		return size;
	}
};