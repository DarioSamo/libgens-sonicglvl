#include "stdafx.h"
#include "EditorModelConverter.h"
#include "Model.h"
#include "Mesh.h"
#include "Submesh.h"
#include "Vertex.h"

QList<Ogre::String> EditorModelConverter::convertModel(LibGens::Model *model, Ogre::String resource_group_name) {
	QList<Ogre::String> mesh_names;

	if (model) {
		vector<LibGens::Mesh *> meshes = model->getMeshes();
		for (size_t m = 0; m < meshes.size(); m++) {
			vector<LibGens::Submesh *> *submeshes = meshes[m]->getSubmeshSlots();
			for (size_t slot = 0; slot < LIBGENS_MODEL_SUBMESH_SLOTS; slot++) {
				Ogre::String mesh_name = model->getName() + "_" + ToString(m) + "_" + ToString(slot);
				Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().createManual(mesh_name, resource_group_name);
				Ogre::AxisAlignedBox mesh_aabb;
				mesh_aabb.setNull();

				for (size_t s = 0; s < submeshes[slot].size(); s++) {
					// Get LibGens Data
					LibGens::Submesh *submesh=submeshes[slot][s];
					vector<LibGens::Vertex *> submesh_vertices = submesh->getVertices();
					vector<LibGens::Polygon> submesh_faces = submesh->getFaces();

					const size_t nVertices = submesh_vertices.size();
					const size_t nVertCount = 24;
					const size_t vbufCount = nVertCount*nVertices;
					float *vertices = (float *) malloc(sizeof(float)*vbufCount);

					for (size_t i=0; i<submesh_vertices.size(); i++) {
						LibGens::Vector3 pos = submesh_vertices[i]->getPosition();
						LibGens::Vector3 norm = submesh_vertices[i]->getNormal();
						LibGens::Vector3 tan = submesh_vertices[i]->getTangent();
						LibGens::Vector3 binorm = submesh_vertices[i]->getBinormal();
						LibGens::Vector2 uv0 = submesh_vertices[i]->getUV(0);
						LibGens::Vector2 uv1 = submesh_vertices[i]->getUV(1);
						LibGens::Vector2 uv2 = submesh_vertices[i]->getUV(2);
						LibGens::Vector2 uv3 = submesh_vertices[i]->getUV(3);
						LibGens::Color col = submesh_vertices[i]->getColor();
						mesh_aabb.merge(Ogre::Vector3(pos.x, pos.y, pos.z));

						vertices[i*nVertCount]    = pos.x;    vertices[i*nVertCount+1]  = pos.y;     vertices[i*nVertCount+2]  = pos.z;
						vertices[i*nVertCount+3]  = norm.x;   vertices[i*nVertCount+4]  = norm.y;    vertices[i*nVertCount+5]  = norm.z;
						vertices[i*nVertCount+6]  = tan.x;    vertices[i*nVertCount+7]  = tan.y;     vertices[i*nVertCount+8]  = tan.z;
						vertices[i*nVertCount+9]  = binorm.x; vertices[i*nVertCount+10] = binorm.y;  vertices[i*nVertCount+11] = binorm.z;
						vertices[i*nVertCount+12] = uv0.x;    vertices[i*nVertCount+13] = uv0.y;
						vertices[i*nVertCount+14] = uv1.x;    vertices[i*nVertCount+15] = uv1.y;
						vertices[i*nVertCount+16] = uv2.x;    vertices[i*nVertCount+17] = uv2.y;
						vertices[i*nVertCount+18] = uv3.x;    vertices[i*nVertCount+19] = uv3.y;
						vertices[i*nVertCount+20] = col.r;    vertices[i*nVertCount+21] = col.g;     
						vertices[i*nVertCount+22] = col.b;    vertices[i*nVertCount+23] = col.a;
					}
 
					Ogre::RenderSystem* rs = Ogre::Root::getSingleton().getRenderSystem();
					const size_t ibufCount = submesh_faces.size() * 3;
					unsigned short *faces = (unsigned short *) malloc(sizeof(unsigned short) * ibufCount);

					for (size_t i = 0; i < submesh_faces.size(); i++) {
						faces[i*3]   = submesh_faces[i].a;
						faces[i*3+1] = submesh_faces[i].b;
						faces[i*3+2] = submesh_faces[i].c;
					}

					Ogre::SubMesh* sub = mesh->createSubMesh();
					sub->vertexData = new Ogre::VertexData();
					sub->vertexData->vertexCount = nVertices;

					Ogre::VertexDeclaration* decl = sub->vertexData->vertexDeclaration;
					size_t offset = 0;

					decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
					offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

					decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);
					offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

					decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_TANGENT);
					offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

					decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_BINORMAL);
					offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

					decl->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);
					offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT2);

					decl->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES, 1);
					offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT2);

					decl->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES, 2);
					offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT2);

					decl->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES, 3);
					offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT2);

					decl->addElement(0, offset, Ogre::VET_FLOAT4, Ogre::VES_DIFFUSE);
					offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT4);

					Ogre::HardwareVertexBufferSharedPtr vbuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(offset, sub->vertexData->vertexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
					vbuf->writeData(0, vbuf->getSizeInBytes(), vertices, true);
					Ogre::VertexBufferBinding* bind = sub->vertexData->vertexBufferBinding; 
					bind->setBinding(0, vbuf);
					Ogre::HardwareIndexBufferSharedPtr ibuf = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(Ogre::HardwareIndexBuffer::IT_16BIT, ibufCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
					ibuf->writeData(0, ibuf->getSizeInBytes(), faces, true);
					sub->useSharedVertices = false;
					sub->indexData->indexBuffer = ibuf;
					sub->indexData->indexCount = ibufCount;
					sub->indexData->indexStart = 0;

					// Assign Material to Submesh
					sub->setMaterialName(submesh->getMaterialName());
				}

				mesh->_setBounds(mesh_aabb);
				mesh->_setBoundingSphereRadius(mesh_aabb.getSize().length() / 2.0F);
				mesh->load();

				mesh_names.append(mesh_name);
			}
		}
	}

	return mesh_names;
}