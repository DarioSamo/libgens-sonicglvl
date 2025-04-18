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

#include "Submesh.h"

#include "Model.h"
#include "Vertex.h"
#include "VertexFormat.h"

namespace LibGens {
	Submesh::Submesh() {
		material_name=LIBGENS_MODEL_SUBMESH_UNKNOWN_MATERIAL;
		vertex_format=NULL;
		extra="";
	}

	Submesh::Submesh(Submesh *clone, LibGens::Matrix4 transform, float uv2_left, float uv2_right, float uv2_top, float uv2_bottom) {
		size_t vertex_sz = clone->vertices.size();
		for (size_t v=0; v<vertex_sz; v++) {
			 Vertex *vertex = new Vertex(clone->vertices[v], transform, uv2_left, uv2_right, uv2_top, uv2_bottom);
			 vertex->setParent(this);
			 vertices.push_back(vertex);
		}

		faces = clone->faces;
		faces_vectors = clone->faces_vectors;
		bone_table = clone->bone_table;
		texture_units = clone->texture_units;
		texture_ids = clone->texture_ids;
		material_name = clone->material_name;
		vertex_format = new VertexFormat(clone->vertex_format);
		buildAABB();
	}

	Submesh::~Submesh() {
		for (vector<Vertex *>::iterator it=vertices.begin(); it!=vertices.end(); it++) {
			delete (*it);
		}
		vertices.clear();

		if (vertex_format) delete vertex_format;
	}
	
	void Submesh::read(File *file, Topology topology) {
		size_t header_address=file->getCurrentAddress();

		// Header
		size_t material_name_address=0;
		unsigned int faces_count=0;
		size_t faces_address=0;
		unsigned int vertices_count=0;
		unsigned int vertex_size=0;
		size_t vertices_address=0;
		size_t vertex_format_address=0;
		unsigned int bones_size=0;
		size_t bones_address=0;
		unsigned int texture_units_size=0;
		size_t texture_units_address=0;

		file->readInt32BEA(&material_name_address);
		file->readInt32BE(&faces_count);
		file->readInt32BEA(&faces_address);
		file->readInt32BE(&vertices_count);
		file->readInt32BE(&vertex_size);
		file->readInt32BEA(&vertices_address);
		file->readInt32BEA(&vertex_format_address);
		file->readInt32BE(&bones_size);
		file->readInt32BEA(&bones_address);

		// LibGens reads the original texture units, but clears them if it's linked to a valid material when resaving the model
		file->readInt32BE(&texture_units_size);
		file->readInt32BEA(&texture_units_address);

		// Faces in Triangle Strip format
		file->goToAddress(faces_address);
		for (size_t i=0; i<faces_count; i++) {
			unsigned short face=0;
			file->readInt16BE(&face);
			faces.push_back(face);
		}

		// Convert Faces in Triangle Strip to Face Vectors
		if (topology == TRIANGLE_STRIP) {
			unsigned short int face_1 = 0;
			unsigned short int face_2 = 0;
			unsigned short int face_3 = 0;
			unsigned short int last_face = 0;

			int new_strip = 3;
			int new_index = 0;

			for (size_t i = 0; i < faces_count; i++) {
				unsigned short int t = faces[i];
				if (t == 0xFFFF) {
					new_strip = 3;
					new_index = 0;
				}
				else {
					if (new_strip == 3) last_face = t;
					new_strip -= 1;
					face_3 = face_2;
					face_2 = face_1;
					face_1 = t;
					if (new_strip == 0) {
						Polygon poly = { face_1, face_2, face_3 };
						Polygon inv_poly = { face_3, face_2, face_1 };

						if ((face_1 != face_2) && (face_2 != face_3) && (face_1 != face_3)) {
							faces_vectors.push_back((new_index % 2) ? inv_poly : poly);
						}
						new_strip = 1;
						new_index++;
					}
				}
			}
		}
		else if (topology == TRIANGLE_LIST) {
			for (size_t i = 0; i < faces_count; i += 3) {
				Polygon poly = { faces[i + 2], faces[i + 1], faces[i] };
				faces_vectors.push_back(poly);
			}
		}

		// Vertex Format
		file->goToAddress(vertex_format_address);
		vertex_format=new VertexFormat();
		vertex_format->read(file);
		vertex_format->setSize(vertex_size);

		// Vertices
		for (size_t i=0; i<vertices_count; i++) {
			file->goToAddress(vertices_address + i*vertex_size);

			Vertex *v=new Vertex();
			v->read(file, vertex_format);
			v->setParent(this);
			vertices.push_back(v);
		}

		// Bone Table
		file->goToAddress(bones_address);

		if (file->getRootNodeType() >= 6) {
			for (size_t i = 0; i < bones_size; i++) {
				unsigned short bone = 0;
				file->readInt16BE(&bone);
				bone_table.push_back(bone);
			}
		}
		else {
			for (size_t i = 0; i < bones_size; i++) {
				unsigned char bone = 0;
				file->readUChar(&bone);
				bone_table.push_back(bone);
			}
		}

		// Material Texture Units
		for (size_t i=0; i<texture_units_size; i++) {
			file->goToAddress(texture_units_address + i * file->getAddressSize());
			size_t texture_unit_address=0;
			file->readInt32BEA(&texture_unit_address);
			file->goToAddress(texture_unit_address);

			size_t texture_sub_unit_address=0;
			unsigned int texture_id=0;
			file->readInt32BEA(&texture_sub_unit_address);
			file->readInt32BE(&texture_id);
			file->goToAddress(texture_sub_unit_address);

			string texture_unit="";
			file->readString(&texture_unit);
			texture_units.push_back(texture_unit);
			texture_ids.push_back(texture_id);
		}

		// Material Name
		file->goToAddress(material_name_address);
		file->readString(&material_name);
	}

	
	void Submesh::write(File *file) {
		size_t header_address=file->getCurrentAddress();

		// Header
		size_t material_name_address=0;
		unsigned int faces_count=faces.size();
		size_t faces_address=0;
		unsigned int vertices_count=vertices.size();
		unsigned int vertex_size=vertex_format->getSize();
		size_t vertices_address=0;
		size_t vertex_format_address=0;
		unsigned int bones_size=bone_table.size();
		size_t bones_address=0;
		unsigned int texture_units_size=texture_units.size();
		size_t texture_units_address=0;

		file->writeNull(4);
		file->writeInt32BE(&faces_count);
		file->writeNull(4);
		file->writeInt32BE(&vertices_count);
		file->writeInt32BE(&vertex_size);
		file->writeNull(4);
		file->writeNull(4);
		file->writeInt32BE(&bones_size);
		file->writeNull(4);
		file->writeInt32BE(&texture_units_size);
		file->writeNull(4);

		// Faces in Triangle Strip format
		faces_address = file->getCurrentAddress();
		for (size_t i=0; i<faces_count; i++) {
			file->writeInt16BE(&faces[i]);
		}
		file->fixPadding();

		// Vertices
		vertices_address = file->getCurrentAddress();
		for (size_t i=0; i<vertices_count; i++) {
			vertices[i]->write(file, vertex_format);
		}

		// Vertex Format
		vertex_format_address = file->getCurrentAddress();
		vertex_format->write(file);


		// Bone Table
		bones_address = file->getCurrentAddress();

		if (file->getRootNodeType() >= 6) {
			for (size_t i = 0; i < bones_size; i++) {
				file->writeInt16BE(&bone_table[i]);
			}
		}
		else {
			for (size_t i = 0; i < bones_size; i++) {
				file->writeUChar((unsigned char*)&bone_table[i]);
			}
		}

		file->fixPadding();

		// Texture Units
		texture_units_address = file->getCurrentAddress();
		vector<unsigned int> texture_units_addresses;
		file->writeNull(texture_units_size*4);

		for (size_t i=0; i<texture_units_size; i++) {
			texture_units_addresses.push_back(file->getCurrentAddress());

			size_t texture_unit_address=file->getCurrentAddress()+8;
			file->writeInt32BEA(&texture_unit_address);
			file->writeInt32BE(&texture_ids[i]);
			file->writeString(&texture_units[i]);
			file->fixPadding();
		}

		for (size_t i=0; i<texture_units_size; i++) {
			file->goToAddress(texture_units_address + i*4);
			file->writeInt32BEA(&texture_units_addresses[i]);
		}
		file->goToEnd();


		// Material Name
		material_name_address = file->getCurrentAddress();
		file->writeString(&material_name);
		file->fixPadding();


		// Fix header
		file->goToAddress(header_address);
		file->writeInt32BEA(&material_name_address);
		file->moveAddress(4);
		file->writeInt32BEA(&faces_address);
		file->moveAddress(4);
		file->moveAddress(4);
		file->writeInt32BEA(&vertices_address);
		file->writeInt32BEA(&vertex_format_address);
		file->moveAddress(4);
		file->writeInt32BEA(&bones_address);
		file->moveAddress(4);
		file->writeInt32BEA(&texture_units_address);
		file->goToEnd();
	}


	list<Vertex *> Submesh::getVertexList() {
		list<Vertex *> new_verts;
		copy(vertices.begin(), vertices.end(), back_inserter(new_verts));
		return new_verts;
	}

	
	list<unsigned int> Submesh::getFaceList() {
		list<unsigned int> new_faces;
		for (size_t i=0; i<faces_vectors.size(); i++) {
			new_faces.push_back((unsigned int) faces_vectors[i].a);
			new_faces.push_back((unsigned int) faces_vectors[i].b);
			new_faces.push_back((unsigned int) faces_vectors[i].c);
		}
		return new_faces;
	}


	void Submesh::build(vector<Vertex *> vertices_p, vector<Polygon> faces_vectors_p) {
		vertices = vertices_p;
		faces_vectors = faces_vectors_p;

		vector<LibGens::Vertex *> new_vertices;
		new_vertices.clear();

		vector<unsigned short> new_face_map;
		new_face_map.clear();

		for (size_t x=0; x<vertices.size(); x++) {
			LibGens::Vertex *v=vertices[x];

			bool clone=false;
			for (unsigned int y=0; y<new_vertices.size(); y++) {
				if ((*new_vertices[y])==(*v)) {
					clone = true;
					new_face_map.push_back(y);
					delete v;
					break;
				}
			}

			if (!clone) {
				new_vertices.push_back(v);
				new_face_map.push_back(new_vertices.size()-1);
			}
		}

		vertices = new_vertices;

		for (size_t i=0; i<faces_vectors.size(); i++) {
			faces_vectors[i].a = new_face_map[(int)faces_vectors[i].a];
			faces_vectors[i].b = new_face_map[(int)faces_vectors[i].b];
			faces_vectors[i].c = new_face_map[(int)faces_vectors[i].c];
		}

		
		triangle_stripper::indices tri_indices;
		for (size_t i=0; i<faces_vectors.size(); i++) {
			tri_indices.push_back((int)faces_vectors[i].a);
			tri_indices.push_back((int)faces_vectors[i].b);
			tri_indices.push_back((int)faces_vectors[i].c);
		}

		triangle_stripper::tri_stripper stripper(tri_indices);
		stripper.SetCacheSize(0);
		stripper.SetBackwardSearch(false);
		triangle_stripper::primitive_vector out_vector;
		stripper.Strip(&out_vector);

		for (size_t i=0; i<out_vector.size(); i+=1) {
			if (out_vector[i].Type == triangle_stripper::TRIANGLE_STRIP) {
				for (size_t j=0; j<out_vector[i].Indices.size(); j++) {
					faces.push_back(out_vector[i].Indices[j]);
				}
				faces.push_back(0xFFFF);
			}
			else {
				for (size_t j=0; j<out_vector[i].Indices.size(); j+=3) {
					faces.push_back(out_vector[i].Indices[j]);
					faces.push_back(out_vector[i].Indices[j+1]);
					faces.push_back(out_vector[i].Indices[j+2]);
					faces.push_back(0xFFFF);
				}
			}
		}

		if (faces[faces.size()-1]==0xFFFF) faces.resize(faces.size()-1);

		buildAABB();
	}

	void Submesh::fixVertexFormatForPC() {
		if (vertex_format) {
			vertex_format->fixForPC();
		}
	}

	vector<Vertex *> Submesh::getVertices() {
		return vertices;
	}

	size_t Submesh::getVerticesSize() {
		return vertices.size();
	}

	vector<unsigned short> Submesh::getFacesIndices() {
		return faces;
	}

	size_t Submesh::getFacesIndicesSize() {
		return faces.size();
	}

	vector<Polygon> Submesh::getFaces() {
		return faces_vectors;
	}

	size_t Submesh::getFacesSize() {
		return faces_vectors.size();
	}

	void Submesh::buildAABB() {
		aabb.reset();
		for (size_t i=0; i<vertices.size(); i++) {
			aabb.addPoint(vertices[i]->getPosition());
		}
	}

	AABB Submesh::getAABB() {
		return aabb;
	}

	void Submesh::setExtra(string v) {
		extra=v;
	}

	string Submesh::getExtra() {
		return extra;
	}

	bool Submesh::hasExtra() {
		return (extra.size() > 0);
	}

	string Submesh::getMaterialName() {
		return material_name;
	}

	void Submesh::setMaterialName(string v) {
		material_name = v;
	}

	void Submesh::setVertexFormat(VertexFormat *v) {
		delete vertex_format;
		vertex_format = new VertexFormat(v);
	}

	void Submesh::addBone(unsigned short bone) {
		bone_table.push_back(bone);
	}

	unsigned short Submesh::getBone(unsigned int index) {
		return bone_table[index];
	}

	vector<unsigned short> Submesh::getBoneTable() {
		return bone_table;
	}

	void Submesh::addTextureUnit(string v) {
		texture_units.push_back(v);
	}

	void Submesh::addTextureID(unsigned int v) {
		texture_ids.push_back(v);
	}

	unsigned int Submesh::getEstimatedMemorySize() {
		return vertex_format->getSize() * vertices.size() + faces.size() * 2;
	}

	void Submesh::changeVertexFormat(int format) {
		delete vertex_format;
		vertex_format = new VertexFormat(format);
	}

	VertexFormat* Submesh::getVertexFormat() {
		return vertex_format;
	}
};