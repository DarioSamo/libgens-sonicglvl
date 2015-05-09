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
	Vertex::Vertex() {
		position=Vector3();
		normal=Vector3();
		tangent=Vector3();
		binormal=Vector3();
		for (size_t i=0; i<4; i++) {
			uv[i]=Vector2();
			bone_indices[i]=(i==0 ? 0    : 0xFF);
			bone_weights[i]=(i==0 ? 0xFF : 0);
		}
		color=Color();
	}

	void Vertex::read(File *file, VertexFormat *vformat) {
		size_t header_address=file->getCurrentAddress();

		readElements(file, vformat);

		// Verify Bone Weights
		unsigned char total_weight = 0;
		for (size_t i=0; i<4; i++) {
			total_weight += bone_weights[i];
		}

		if (total_weight != 0xFF) {
			bone_weights[0] += 0xFF - total_weight;
		}
	}

	
	void Vertex::readElements(File *file, VertexFormat *vformat) {
		size_t header_address=file->getCurrentAddress();

		list<VertexFormatElement> reference=vformat->getElements();
		for (list<VertexFormatElement>::iterator it=reference.begin(); it!=reference.end(); it++) {
			file->goToAddress(header_address+(*it).getOffset());

			switch ((*it).getID()) {
				case POSITION :
					if ((*it).getData() == VECTOR3) position.read(file);
					break;
				case BONE_WEIGHTS :
					if ((*it).getData() == VECTOR4_CHAR) file->read(bone_weights, 4);
					break;
				case BONE_INDICES :
					if ((*it).getData() == INDICES) file->read(bone_indices, 4);
					break;
				case NORMAL :
					if ((*it).getData() == VECTOR3) normal.read(file);
					if ((*it).getData() == VECTOR3_360) normal.readNormal360(file);
					break;
				case UV :
					if ((*it).getData() == VECTOR2) uv[(*it).getIndex()].read(file);
					if ((*it).getData() == VECTOR2_HALF) uv[(*it).getIndex()].readHalf(file);
					break;
				case BINORMAL :
					if ((*it).getData() == VECTOR3) binormal.read(file);
					if ((*it).getData() == VECTOR3_360) binormal.readNormal360(file);
					break;
				case TANGENT :
					if ((*it).getData() == VECTOR3) tangent.read(file);
					if ((*it).getData() == VECTOR3_360) tangent.readNormal360(file);
					break;
				case RGBA :
					if ((*it).getData() == VECTOR4) color.read(file, true);
					if ((*it).getData() == VECTOR4_CHAR) color.readABGR8(file);
					break;
			}
		}
	}


	void Vertex::write(File *file, VertexFormat *vformat) {
		writeElements(file, vformat);

		file->goToEnd();
	}

	
	void Vertex::writeElements(File *file, VertexFormat *vformat) {
		size_t header_address=file->getCurrentAddress();
		file->writeNull(vformat->getSize());

		list<VertexFormatElement> reference=vformat->getElements();
		for (list<VertexFormatElement>::iterator it=reference.begin(); it!=reference.end(); it++) {
			file->goToAddress(header_address+(*it).getOffset());

			switch ((*it).getID()) {
				case POSITION :
					if ((*it).getData() == VECTOR3) position.write(file);
					break;
				case BONE_WEIGHTS :
					if ((*it).getData() == VECTOR4_CHAR) file->write(bone_weights, 4);
					break;
				case BONE_INDICES :
					if ((*it).getData() == INDICES) file->write(bone_indices, 4);
					break;
				case NORMAL :
					if ((*it).getData() == VECTOR3) normal.write(file);
					break;
				case UV :
					if ((*it).getData() == VECTOR2) uv[(*it).getIndex()].write(file);
					break;
				case BINORMAL :
					if ((*it).getData() == VECTOR3) binormal.write(file);
					break;
				case TANGENT :
					if ((*it).getData() == VECTOR3) tangent.write(file);
					break;
				case RGBA :
					if ((*it).getData() == VECTOR4) color.write(file, true);
					break;
			}
		}
	}

	void Vertex::fixBinormalAndTangent() {
	}

	void VertexFormat::fixForPC() {
		size_t current_offset = 0;

		for (list<VertexFormatElement>::iterator it=elements.begin(); it!=elements.end(); it++) {
			(*it).setOffset((*it).getOffset() + current_offset);

			if ((*it).getData() == VECTOR3_360) {
				(*it).setData(VECTOR3);
				current_offset += 8;
			}

			if ((*it).getData() == VECTOR2_HALF) {
				(*it).setData(VECTOR2);
				current_offset += 4;
			}

			if (((*it).getData() == VECTOR4_CHAR) && ((*it).getID() == RGBA)) {
				(*it).setData(VECTOR4);
				current_offset += 12;
			}
		}

		size += current_offset;
	}

	

	void Vertex::setParent(Submesh *v) {
		parent = v;
	}

	Submesh *Vertex::getParent() {
		return parent;
	}

	bool Vertex::operator == (const Vertex& vertex) {
		if (position != vertex.position) return false;
		if (normal != vertex.normal) return false;
		if (tangent != vertex.tangent) return false;
		if (binormal != vertex.binormal) return false;

		for (size_t i=0; i<4; i++) {
			if (uv[i] != vertex.uv[i]) return false;
			if (bone_indices[i] != vertex.bone_indices[i]) return false;
			if (bone_weights[i] != vertex.bone_weights[i]) return false;
		}

		if (color != vertex.color) return false;
           return true;
	}
			
	void Vertex::transform(const Matrix4& matrix) {
		position = matrix * position;
	}

	Vector3 Vertex::getTPosition(const Matrix4& matrix) {
		return matrix * position;
	}

	Vector3 Vertex::getPosition() {
		return position;
	}

	Vector3 Vertex::getNormal() {
		return normal;
	}

	Vector3 Vertex::getTangent() {
		return tangent;
	}

	Vector3 Vertex::getBinormal() {
		return binormal;
	}

	Color Vertex::getColor() {
		return color;
	}

	unsigned char Vertex::getBoneIndex(size_t index) {
		return bone_indices[index];
	}

	unsigned char Vertex::getBoneWeight(size_t index) {
		return bone_weights[index];
	}

	Vector2 Vertex::getUV(size_t channel) {
		return uv[channel];
	}

	void Vertex::setPosition(Vector3 v) {
		position=v;
	}

	void Vertex::setNormal(Vector3 v) {
		normal=v;
	}

	void Vertex::setTangent(Vector3 v) {
		tangent=v;
	}

	void Vertex::setBinormal(Vector3 v) {
		binormal=v;
	}

	void Vertex::setUV(Vector2 v, size_t channel) {
		uv[channel] = v;
	}

	void Vertex::setBoneIndex(unsigned char v, size_t index) {
		bone_indices[index] = v;
	}

	void Vertex::setBoneWeight(unsigned char v, size_t index) {
		bone_weights[index] = v;
	}
};

