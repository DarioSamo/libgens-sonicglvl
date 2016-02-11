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
		if (type == LIBGENS_VERTEX_FORMAT_PC) {
			VertexFormatElement position(0, VECTOR3, POSITION, 0);
			VertexFormatElement normal(12, VECTOR3, NORMAL, 0);
			VertexFormatElement tangent(24, VECTOR3, TANGENT, 0);
			VertexFormatElement binormal(36, VECTOR3, BINORMAL, 0);
			VertexFormatElement uv_1(48, VECTOR2, UV, 0);
			VertexFormatElement uv_2(56, VECTOR2, UV, 1);
			VertexFormatElement uv_3(64, VECTOR2, UV, 1);
			VertexFormatElement uv_4(72, VECTOR2, UV, 1);
			VertexFormatElement color(80, VECTOR4, RGBA, 0);
			VertexFormatElement bone_indices(96, INDICES, BONE_INDICES, 0);
			VertexFormatElement bone_weights(100, VECTOR4_CHAR, BONE_WEIGHTS, 0);
			
			addElement(position);
			addElement(normal);
			addElement(binormal);
			addElement(tangent);
			addElement(uv_1);
			addElement(uv_2);
			addElement(uv_3);
			addElement(uv_4);
			addElement(color);
			addElement(bone_indices);
			addElement(bone_weights);

			size = 104;
		}
		else if (type == LIBGENS_VERTEX_FORMAT_PC_TERRAIN) {
			VertexFormatElement position(0, VECTOR3, POSITION, 0);
			VertexFormatElement normal(12, VECTOR3, NORMAL, 0);
			VertexFormatElement tangent(24, VECTOR3, TANGENT, 0);
			VertexFormatElement binormal(36, VECTOR3, BINORMAL, 0);
			VertexFormatElement uv_1(48, VECTOR2, UV, 0);
			VertexFormatElement uv_2(56, VECTOR2, UV, 1);
			VertexFormatElement color(64, VECTOR4, RGBA, 0);
			
			addElement(position);
			addElement(normal);
			addElement(binormal);
			addElement(tangent);
			addElement(uv_1);
			addElement(uv_2);
			addElement(color);

			size = 80;
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