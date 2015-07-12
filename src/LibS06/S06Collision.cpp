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

#include "LibGens.h"
#include "S06Common.h"
#include "S06Collision.h"
#include "Havok.h"

namespace LibGens {
	SonicCollision::SonicCollision(string filename) {
		File file(filename, LIBGENS_FILE_READ_BINARY);

		if (file.valid()) {
			read(&file);
			file.close();
		}
	}

	void SonicCollisionFace::read(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_S06_COLLISION_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		size_t header_address=file->getCurrentAddress();
		file->readInt16BE(&v1);
		file->readInt16BE(&v2);
		file->readInt16BE(&v3);
		file->moveAddress(2);

		file->readInt32BE(&collision_flag);
	}

	void SonicCollisionFace::write(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_S06_COLLISION_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}

		size_t header_address=file->getCurrentAddress();
		file->writeInt16BE(&v1);
		file->writeInt16BE(&v2);
		file->writeInt16BE(&v3);
		file->writeNull(2);
		file->writeInt32BE(&collision_flag);
	}



	void SonicCollision::read(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_S06_COLLISION_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		size_t header_address=file->getCurrentAddress();
		size_t address=0;

		
		unsigned int file_size=0;
		file->setRootNodeAddress(32);
		file->readInt32BE(&file_size);

		file->goToAddress(32);

		size_t geometry_address=0;
		size_t mopp_code_address=0;
		size_t vertex_section_address=0;
		size_t face_section_address=0;

		file->readInt32BEA(&geometry_address);
		file->readInt32BEA(&mopp_code_address);

		// Geometry
		file->goToAddress(geometry_address);
		file->readInt32BEA(&vertex_section_address);
		file->readInt32BEA(&face_section_address);

		// Vertices
		file->goToAddress(vertex_section_address);
		unsigned int vertex_total=0;
		file->readInt32BE(&vertex_total);

		for (size_t i=0; i<vertex_total; i++) {
			Vector3 position;
			position.read(file);
			vertex_pool.push_back(position);
		}

		// Faces
		file->goToAddress(face_section_address);

		unsigned int face_total=0;
		file->readInt32BE(&face_total);

		for (size_t i=0; i<face_total; i++) {
			SonicCollisionFace face;
			face.read(file);
			face_pool.push_back(face);
		}

		// Mopp Code
		file->goToAddress(mopp_code_address);
		mopp_code_center.read(file);
		file->readFloat32BE(&mopp_code_w);
		file->readInt32BE(&mopp_code_size);

		mopp_code_data = (unsigned char *) malloc(mopp_code_size);
		file->read(mopp_code_data, mopp_code_size);
	}


	void SonicCollision::save(string filename) {
		File file(filename, LIBGENS_FILE_WRITE_BINARY);

		if (file.valid()) {
			write(&file);
			file.close();
		}
	}

	void SonicCollision::write(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_S06_COLLISION_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}

		file->setRootNodeAddress(32);

		unsigned int file_size=0;
		size_t offset_table_address=0;

		file->writeNull(22);
		string header="1BBINA";
		file->writeString(&header);
		file->fixPadding(32);

		SonicOffsetTable offset_table;

		size_t geometry_address=40;
		size_t unknown_address=0;
		size_t vertex_section_address=0;
		size_t face_section_address=0;

		file->writeNull(16);

		// Geometry
		// Vertices
		vertex_section_address = file->getCurrentAddress();

		unsigned int vertex_total=vertex_pool.size();
		file->writeInt32BE(&vertex_total);

		for (size_t i=0; i<vertex_total; i++) {
			vertex_pool[i].write(file);
		}

		// Faces
		face_section_address = file->getCurrentAddress();

		unsigned int face_total=face_pool.size();
		file->writeInt32BE(&face_total);
		for (size_t i=0; i<face_total; i++) {
			face_pool[i].write(file);
		}

		// Unknown
		unknown_address = file->getCurrentAddress();

		mopp_code_center.write(file);
		file->writeFloat32BE(&mopp_code_w);
		file->writeInt32BE(&mopp_code_size);
		file->write(mopp_code_data, mopp_code_size);
		file->fixPadding(4);


		file->goToAddress(32);
		offset_table.addEntry(0x40, file->getCurrentAddress());
		file->writeInt32BEA(&geometry_address);
		offset_table.addEntry(0x41, file->getCurrentAddress());
		file->writeInt32BEA(&unknown_address);
		offset_table.addEntry(0x41, file->getCurrentAddress());
		file->writeInt32BEA(&vertex_section_address);
		offset_table.addEntry(0x41, file->getCurrentAddress());
		file->writeInt32BEA(&face_section_address);
		file->goToEnd();

		offset_table_address = file->getCurrentAddress() - 32;
		file->writeAddressTableBBIN();
		
		file_size = file->getFileSize();

		file->goToAddress(0);
		file->writeInt32BE(&file_size);
		file->writeInt32BE(&offset_table_address);
		size_t offset_table_size = file_size - offset_table_address - 32;
		file->writeInt32BE(&offset_table_size);
	}


	SonicCollision::SonicCollision(FBX *fbx) {
		mopp_code_data = NULL;
		mopp_code_size = 0;

		FbxScene *lScene = fbx->getScene();

		if (lScene) {
			printf("Scene for collision found!\n");

			const int lNodeCount = lScene->GetSrcObjectCount<FbxNode>();
			FbxStatus lStatus;

			for (int lIndex=0; lIndex<lNodeCount; lIndex++) {
				FbxNode *lNode = lScene->GetSrcObject<FbxNode>(lIndex);
				printf("Collision Node found: %s\n", lNode->GetName());
				addFbxNode(lNode);
			}
		}

		buildMoppCode();
	}


	void SonicCollision::addFbxNode(FbxNode *lNode) {
		if (!lNode) return;

		FbxMesh *lMesh=lNode->GetMesh();

		printf("Looking for mesh\n");

		if (!lMesh) return;

		printf("Mesh found!\n");

		string node_name = ToString(lNode->GetName());
		unsigned int collision_flag = 0;

		size_t pos=node_name.find_last_of("@");

		if (pos != string::npos) {
			FromString<unsigned int>(collision_flag, node_name.substr(pos+1, node_name.size() - pos - 1), std::hex);
		}

		FbxAMatrix transform_matrix=lNode->EvaluateGlobalTransform();

		int lPolygonCount=lMesh->GetPolygonCount();
		int control_points_count=lMesh->GetControlPointsCount();
		FbxVector4 *control_points=lMesh->GetControlPoints();

		for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex) {
			int polygon_size=lMesh->GetPolygonSize(lPolygonIndex);
			if (polygon_size == 3) {
				SonicCollisionFace face;
				face.v1 = face.v2 = face.v3 = 0;
				face.collision_flag = collision_flag;

				for (int j=0; j<polygon_size; j++) {
					int control_point_index=lMesh->GetPolygonVertex(lPolygonIndex, j);
					FbxVector4 control_point=transform_matrix.MultT(control_points[control_point_index]);

					Vector3 vertex_position = Vector3(control_point[0], control_point[2], -control_point[1]);
					size_t vertex_index = 0;
					bool found = false;
					for (size_t i=0; i<vertex_pool.size(); i++) {
						if (vertex_position == vertex_pool[i]) {
							found = true;
							vertex_index = i;
							break;
						}
					}

					if (!found) {
						vertex_index = vertex_pool.size();
						vertex_pool.push_back(vertex_position);
					}

					if (j == 0) face.v1=vertex_index;
					if (j == 1) face.v2=vertex_index;
					if (j == 2) face.v3=vertex_index;
				}

				face_pool.push_back(face);
			}
			else printf("Unsupported polygon size.\n");
		}
	}

	void SonicCollision::buildMoppCode() {
		hkpSimpleMeshShape *m_storageMeshShape = new hkpSimpleMeshShape(0.01f);

		m_storageMeshShape->m_vertices.setSize(vertex_pool.size());
		for(size_t ti = 0; ti < vertex_pool.size(); ti++) {
			m_storageMeshShape->m_vertices[ti] = hkVector4(vertex_pool[ti].x/100.0f, vertex_pool[ti].y/100.0f, vertex_pool[ti].z/100.0f);
		}

		m_storageMeshShape->m_triangles.setSize(face_pool.size());
		for(size_t ti = 0; ti < face_pool.size(); ti++) {
			m_storageMeshShape->m_triangles[ti].m_a = face_pool[ti].v1;
			m_storageMeshShape->m_triangles[ti].m_b = face_pool[ti].v2;
			m_storageMeshShape->m_triangles[ti].m_c = face_pool[ti].v3;
		}


		hkpStorageExtendedMeshShape* extendedMesh = new hkpStorageExtendedMeshShape();
		hkpExtendedMeshShape::TrianglesSubpart part;
		part.m_numTriangleShapes = m_storageMeshShape->m_triangles.getSize();
		part.m_numVertices = m_storageMeshShape->m_vertices.getSize();
		part.m_vertexBase = (float*)m_storageMeshShape->m_vertices.begin();
		part.m_stridingType = hkpExtendedMeshShape::INDICES_INT32;
		part.m_vertexStriding = sizeof(hkVector4);
		part.m_indexBase = m_storageMeshShape->m_triangles.begin();
		part.m_indexStriding = sizeof(hkpSimpleMeshShape::Triangle);
		extendedMesh->addTrianglesSubpart( part );

		hkpMoppCompilerInput compiler_input;

		hkpMoppCode* moppCode = hkpMoppUtility::buildCode(extendedMesh, compiler_input);
		if (moppCode) {
			mopp_code_center.x = moppCode->m_info.m_offset(0);
			mopp_code_center.y = moppCode->m_info.m_offset(1);
			mopp_code_center.z = moppCode->m_info.m_offset(2);
			mopp_code_w        = moppCode->m_info.m_offset(3);
			mopp_code_size     = moppCode->m_data.getSize();
			mopp_code_data     = (unsigned char *) malloc(mopp_code_size);

			for (size_t i=0; i<mopp_code_size; i++) {
				mopp_code_data[i] = moppCode->m_data[i];
			}
		}
	}
}