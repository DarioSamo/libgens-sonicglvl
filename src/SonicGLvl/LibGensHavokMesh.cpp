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

#include "Common.h"


void buildHavokMesh(Ogre::SceneNode *scene_node, string name, hkGeometry* geometry, Ogre::SceneManager *scene_manager, Ogre::uint32 query_flags, string resource_group) {
	Ogre::MeshPtr msh = Ogre::MeshManager::getSingleton().createManual(name, resource_group);

	LibGens::AABB aabb;
	aabb.reset();

	// Transform to Ogre Mesh
    Ogre::SubMesh* sub = msh->createSubMesh();
	const size_t nVertices = geometry->m_vertices.getSize();
	const size_t nVertCount = 8;
    const size_t vbufCount = nVertCount*nVertices;
    float *vertices = (float *) malloc(sizeof(float)*vbufCount);

	for (size_t i=0; i<nVertices; i++) {
		vertices[i*nVertCount]   = geometry->m_vertices[i](0);
		vertices[i*nVertCount+1] = geometry->m_vertices[i](1);
		vertices[i*nVertCount+2] = geometry->m_vertices[i](2);

		vertices[i*nVertCount+3] = 0;
		vertices[i*nVertCount+4] = 0;
		vertices[i*nVertCount+5] = 0;

		vertices[i*nVertCount+6] = geometry->m_vertices[i](0)/5.0;
		vertices[i*nVertCount+7] = geometry->m_vertices[i](1)/5.0;

		aabb.addPoint(LibGens::Vector3(geometry->m_vertices[i](0), geometry->m_vertices[i](1), geometry->m_vertices[i](2)));
	}
 
    Ogre::RenderSystem* rs = Ogre::Root::getSingleton().getRenderSystem();
	const size_t ibufCount = geometry->m_triangles.getSize()*3;
    unsigned short *faces = (unsigned short *) malloc(sizeof(unsigned short) * ibufCount);

	for (size_t i=0; i<geometry->m_triangles.getSize(); i++) {
		faces[i*3]   = geometry->m_triangles[i].m_a;
		faces[i*3+1] = geometry->m_triangles[i].m_b;
		faces[i*3+2] = geometry->m_triangles[i].m_c;

		size_t i1=faces[i*3];
		size_t i2=faces[i*3+1];
		size_t i3=faces[i*3+2];

		Ogre::Vector3 v0(vertices[i1*nVertCount], vertices[i1*nVertCount+1], vertices[i1*nVertCount+2]);
		Ogre::Vector3 v1(vertices[i2*nVertCount], vertices[i2*nVertCount+1], vertices[i2*nVertCount+2]);
		Ogre::Vector3 v2(vertices[i3*nVertCount], vertices[i3*nVertCount+1], vertices[i3*nVertCount+2]);

		Ogre::Vector3 dir0 = v2 - v0;
		Ogre::Vector3 dir1 = v0 - v1;
		Ogre::Vector3 normal = dir0.crossProduct(dir1).normalisedCopy();

		vertices[i1*nVertCount+3] = normal.x;
		vertices[i1*nVertCount+4] = normal.y;
		vertices[i1*nVertCount+5] = normal.z;

		vertices[i2*nVertCount+3] = normal.x;
		vertices[i2*nVertCount+4] = normal.y;
		vertices[i2*nVertCount+5] = normal.z;

		vertices[i3*nVertCount+3] = normal.x;
		vertices[i3*nVertCount+4] = normal.y;
		vertices[i3*nVertCount+5] = normal.z;
	}

    msh->sharedVertexData = new Ogre::VertexData();
    msh->sharedVertexData->vertexCount = nVertices;
 
    Ogre::VertexDeclaration* decl = msh->sharedVertexData->vertexDeclaration;
    size_t offset = 0;

    decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

	decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

	decl->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);
	offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT2);

    Ogre::HardwareVertexBufferSharedPtr vbuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(offset, msh->sharedVertexData->vertexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    vbuf->writeData(0, vbuf->getSizeInBytes(), vertices, true);
    Ogre::VertexBufferBinding* bind = msh->sharedVertexData->vertexBufferBinding; 
    bind->setBinding(0, vbuf);
	Ogre::HardwareIndexBufferSharedPtr ibuf = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(Ogre::HardwareIndexBuffer::IT_16BIT, ibufCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
	ibuf->writeData(0, ibuf->getSizeInBytes(), faces, true);
	sub->useSharedVertices = true;
	sub->indexData->indexBuffer = ibuf;
	sub->indexData->indexCount = ibufCount;
	sub->indexData->indexStart = 0;
	
	msh->_setBounds(Ogre::AxisAlignedBox(aabb.start.x, aabb.start.y, aabb.start.z, aabb.end.x, aabb.end.y, aabb.end.z));
	msh->_setBoundingSphereRadius(((aabb.end.x-aabb.start.x) + (aabb.end.y-aabb.start.y) + (aabb.end.z-aabb.start.z))/6.0f);
	msh->load();

	free(faces);
	free(vertices);

	Ogre::Entity* entity = scene_manager->createEntity(name);
	entity->setQueryFlags(query_flags);
	entity->setMaterialName("havok");

	if (scene_node) {
		scene_node->attachObject(entity);
	}
}