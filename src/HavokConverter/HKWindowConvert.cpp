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

#include "HKWindow.h"
#include "LibGens.h"
#include <QFileInfo>
#include <QDir>
#include <QTemporaryDir>
#include <QProcess>
#include "assimp/postprocess.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/config.h"
#include "HavokEnviroment.h"
#include "Tags.h"
#include "Havok.h"

const float HKWindow::TriangleAbsoluteTolerance = 0.01f;
const int HKWindow::HavokBufferSizeMB = 256;

bool HKWindow::convert() {
	QStringList model_source_paths = converter_settings.model_source_paths;
	QString output_file = converter_settings.output_file;

	//*************************
	//  Verifications
	//*************************
	if (model_source_paths.isEmpty()) {
		logProgress(ProgressWarning, "No source model paths have been added! An empty file will be created.");
	}

	if (output_file.isEmpty()) {
		logProgress(ProgressFatal, "Output path can't be empty!");
		return false;
	}

	foreach(QString model_source_path, model_source_paths) {
		if (!QFileInfo(model_source_path).exists()) {
			logProgress(ProgressFatal, "Source model file " + model_source_path + " doesn't exist! Verify if it's there or if it's open by another program.");
			return false;
		}
	}

	//*************************
	//  Global Transform
	//*************************

	LibGens::Matrix4 global_transform;
	LibGens::Vector3 position(converter_settings.position_x, converter_settings.position_y, converter_settings.position_z);
	LibGens::Vector3 scale(converter_settings.scale_x * 0.01, converter_settings.scale_y * 0.01, converter_settings.scale_z * 0.01);
	LibGens::Quaternion rotation_x, rotation_y, rotation_z;
	rotation_x.fromAngleAxis(converter_settings.rotation_x * LIBGENS_MATH_RAD_TO_DEGREE, LibGens::Vector3(1.0f, 0.0f, 0.0f));
	rotation_y.fromAngleAxis(converter_settings.rotation_y * LIBGENS_MATH_RAD_TO_DEGREE, LibGens::Vector3(0.0f, 1.0f, 0.0f));
	rotation_z.fromAngleAxis(converter_settings.rotation_z * LIBGENS_MATH_RAD_TO_DEGREE, LibGens::Vector3(0.0f, 0.0f, 1.0f));
	LibGens::Quaternion orientation = rotation_x * rotation_y * rotation_z;
	global_transform.makeTransform(position, scale, orientation);

	logProgress(ProgressNormal, "Using Global Transform Matrix...");
	for (int x = 0; x < 4; x++) {
		logProgress(ProgressNormal, QString(" [%1, %2, %3, %4]").arg(global_transform[x][0]).arg(global_transform[x][1]).arg(global_transform[x][2]).arg(global_transform[x][3]));
	}

	//*************************
	//  Setup Havok Enviroment
	//*************************
	if (!havok_enviroment)
		havok_enviroment = new LibGens::HavokEnviroment(HavokBufferSizeMB * 1024 * 1024);

#ifdef HAVOKCONVERTER_LOST_WORLD
	hkArray<hkRootLevelContainer::NamedVariant> shapes;
#else
	hkpWorldCinfo info;
	info.m_convexListFilter = HK_NULL;
	info.m_minDesiredIslandSize = 64;
#ifndef Release2012
	info.m_autoUpdateKdTree = false;
#endif
	info.m_maxConstraintViolation = FLT_MAX;
	hkpWorld* world = new hkpWorld(info);
	world->lock();
#endif // HAVOKCONVERTER_LOST_WORLD



	//*************************
	//  Load Assimp Scene
	//*************************
	foreach(QString model_source_path, model_source_paths) {
		logProgress(ProgressNormal, "Assimp importer reading model " + model_source_path + " ...");

		Assimp::Importer importer;
		importer.SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT, 0x8000);
		importer.SetPropertyInteger(AI_CONFIG_PP_SLM_TRIANGLE_LIMIT, 0x80000);
		importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS | aiComponent_TANGENTS_AND_BITANGENTS | aiComponent_COLORS | 
															aiComponent_TEXCOORDS | aiComponent_BONEWEIGHTS | aiComponent_TEXTURES | aiComponent_MATERIALS);

		const aiScene *scene = importer.ReadFile(model_source_path.toStdString(), aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_JoinIdenticalVertices | 
																				  aiProcess_FindInstances | aiProcess_SplitLargeMeshes | aiProcess_RemoveComponent);
		if (!scene) {
			logProgress(ProgressFatal, QString("Assimp failed to open %1: %2").arg(model_source_path).arg(importer.GetErrorString()));
			return false;
		}

		logProgress(ProgressNormal, "Assimp importer loaded scene sucessfully.");
		logProgress(ProgressNormal, "aiScene:");
		logProgress(ProgressNormal, QString("* Animations: %1").arg(scene->mNumAnimations));
		logProgress(ProgressNormal, QString("* Meshes: %1").arg(scene->mNumMeshes));
		logProgress(ProgressNormal, QString("* Node Tree:"));
		logNodeTree(scene->mRootNode, "**");
		
#ifdef HAVOKCONVERTER_LOST_WORLD
		hkpStaticCompoundShape* shape = convertNodeTreeCompoundShape(scene, scene->mRootNode, global_transform);
		shapes.pushBack(hkRootLevelContainer::NamedVariant("shape",shape, &hkcdStaticTreeDefaultTreeStorage6Class));
#else
		convertNodeTree(scene, scene->mRootNode, global_transform, world);
#endif
	}
	
#ifndef HAVOKCONVERTER_LOST_WORLD
	world->unlock();
#endif // !HAVOKCONVERTER_LOST_WORLD

	hkOstream outfile(output_file.toStdString().c_str());
	if (outfile.isOk()) {
		// Create root level container and push the physics data on to it.
		hkRootLevelContainer *root_level_container = new hkRootLevelContainer();

#ifdef HAVOKCONVERTER_LOST_WORLD
		root_level_container->m_namedVariants.append(shapes);
		logProgress(ProgressNormal, QString("Conversion finished with %1 compressed shapes.").arg(shapes.getSize()));
#else
		// Create physics data.
		hkpPhysicsData* physics_data = new hkpPhysicsData();

		// Populate physics data with the world we created.
		world->lock();
		hkpPhysicsSystem* system = world->getWorldAsOneSystem();
		system->setName("Default Physics System");
		physics_data->addPhysicsSystem(system);

		logProgress(ProgressNormal, QString("Physics system with %1 rigid bodies.").arg(system->getRigidBodies().getSize()));

		root_level_container->m_namedVariants.pushBack(hkRootLevelContainer::NamedVariant("Physics Data", physics_data, &hkpPhysicsDataClass));
		physics_data->removeReference();
#endif

		// Serialize root level container to a binary packfile.
		hkPackfileWriter::Options pack_options;
		hkSerializeUtil::SaveOptions options; 
		pack_options.m_writeMetaInfo = false;
		options.useBinary(true);
		hkResult result = hkSerializeUtil::savePackfile(root_level_container, hkRootLevelContainerClass, outfile.getStreamWriter(), pack_options, HK_NULL, options);
		if (result == HK_SUCCESS)  {
			logProgress(ProgressSuccess, "Serializer success! Saved file to " + output_file);
			return true;
		}
		else {
			logProgress(ProgressFatal, "Serializer output error. File could not be saved properly.");
			return false;
		}

#ifndef HAVOKCONVERTER_LOST_WORLD
		system->removeReference();
		world->unlock();
		delete world;
#endif // !HAVOKCONVERTER_LOST_WORLD

		delete root_level_container;
	}
	else {
		logProgress(ProgressFatal, "Could not open output file for writing!");
		return false;
	}
}

void HKWindow::logNodeTree(aiNode *node, QString prefix) {
	if (node) {
		logProgress(ProgressNormal, QString("%1%2 (%3 Children, %4 Meshes)").arg(prefix).arg(node->mName.C_Str()).arg(node->mNumChildren).arg(node->mNumMeshes));

		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			logNodeTree(node->mChildren[i], prefix + "*");
		}
	}
}


hkpShape *HKWindow::convertMeshToShape(aiMesh *mesh, LibGens::Vector3 scale) {
	// Get triangle count.
	int triangles = 0;
	for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
		if (mesh->mFaces[f].mNumIndices == 3) {
			triangles++;
		}
	}

	// Fill index buffer.
	size_t index_count = 0;
	int *index_buffer = new int[triangles * 3];
	for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
		if (mesh->mFaces[f].mNumIndices == 3) {
			index_buffer[index_count++] = mesh->mFaces[f].mIndices[0];
			index_buffer[index_count++] = mesh->mFaces[f].mIndices[1];
			index_buffer[index_count++] = mesh->mFaces[f].mIndices[2];
		}
	}

	// Fill vertex buffer.
	float *vertex_buffer = new float[mesh->mNumVertices * 4];
	size_t vertex_count = 0;
	LibGens::AABB aabb;
	aabb.reset();

	for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
		LibGens::Vector3 vertex(mesh->mVertices[v].x * scale.x, mesh->mVertices[v].y * scale.y, mesh->mVertices[v].z * scale.z);
		aabb.addPoint(vertex);

		vertex_buffer[vertex_count++] = vertex.x;
		vertex_buffer[vertex_count++] = vertex.y;
		vertex_buffer[vertex_count++] = vertex.z;
		vertex_buffer[vertex_count++] = 0.0f;
	}

	// Create extended mesh shape.
	hkpStorageExtendedMeshShape *extended_shape = new hkpStorageExtendedMeshShape();
	hkpExtendedMeshShape::TrianglesSubpart part;
	part.m_numTriangleShapes = triangles;
	part.m_numVertices = mesh->mNumVertices;
	part.m_vertexBase = vertex_buffer;
	part.m_stridingType = hkpExtendedMeshShape::INDICES_INT32;
	part.m_vertexStriding = sizeof(float) * 4;
	part.m_indexBase = index_buffer;
	part.m_indexStriding = sizeof(int) * 3;
	extended_shape->addTrianglesSubpart(part);

	// Compile mopp and return it as the shape.
	hkpMoppCompilerInput mopp_input;
	mopp_input.m_enableChunkSubdivision = true;
	mopp_input.setAbsoluteFitToleranceOfTriangles(TriangleAbsoluteTolerance);
	mopp_input.setAbsoluteFitToleranceOfAxisAlignedTriangles(hkVector4(TriangleAbsoluteTolerance, TriangleAbsoluteTolerance, TriangleAbsoluteTolerance));
	hkpMoppCode *code = hkpMoppUtility::buildCode(extended_shape, mopp_input);
	hkpMoppBvTreeShape *mopp_shape = new hkpMoppBvTreeShape(extended_shape, code);
	hkpMeshWeldingUtility::computeWeldingInfo(extended_shape, mopp_shape, hkpWeldingUtility::WELDING_TYPE_ANTICLOCKWISE, false);
	logProgress(ProgressNormal, QString("Created shape with %1 vertices, %2 faces and a MOPP code of size %3.").arg(mesh->mNumVertices).arg(mesh->mNumFaces).arg(code->getCodeSize()));
	code->removeReference();
	extended_shape->removeReference();

	return mopp_shape;
}

QList<hkpRigidBody *> HKWindow::convertNodeToRigidBodies(const aiScene *scene, aiNode *node, LibGens::Matrix4 transform) {
	QList<hkpRigidBody *> rigid_bodies;
	LibGens::Vector3 pos, sca;
	LibGens::Quaternion ori;
	transform.decomposition(pos, sca, ori);

	for (unsigned int m = 0; m < node->mNumMeshes; m++) {
		// Rigid body information.
		hkpRigidBodyCinfo rigid_body_info;
		rigid_body_info.m_position.set(pos.x, pos.y, pos.z);
		rigid_body_info.m_rotation.set(ori.x, ori.y, ori.z, ori.w);
		rigid_body_info.m_angularDamping = 0.049805f;
		rigid_body_info.m_linearDamping = 0.0f;
		hkpShape *shape = convertMeshToShape(scene->mMeshes[node->mMeshes[m]], sca);
		rigid_body_info.m_shape = shape;
		rigid_body_info.m_mass = 0.0f;
		rigid_body_info.m_motionType = hkpMotion::MOTION_FIXED;
		hkpRigidBody* rigid_body = new hkpRigidBody(rigid_body_info);
		rigid_body->setName(node->mName.C_Str());

		// Prepare properties.
		QMap<hkUint32, int> properties;
		LibGens::Tags tags(node->mName.C_Str());
		for (int i = 0; i < tags.getTagCount(); i++) {
			QString tag_match = tags.getTag(i).getKey().c_str();

			// Search for tag in existing properties. We only use it if it matches the bitwise operation.
			for (int bitwise = 0; bitwise < 2; bitwise++) {
				foreach(const HKPropertyTag &tag, converter_settings.property_tags) {
					if (tag.tag == tag_match) {
						foreach(const HKPropertyValue &value, tag.values) {
							if (value.bitwise == bitwise) {
								if (value.bitwise == HKBitwise_SET)
									properties[value.key] = value.value;
								else if (value.bitwise == HKBitwise_OR)
									properties[value.key] |= value.value;
							}
						}
					}
				}
			}
		}
		
		// Set properties.
		if (!properties.isEmpty())
			logProgress(ProgressNormal, QString("Applying %1 properties to %2").arg(properties.size()).arg(rigid_body->getName()));

		foreach(hkUint32 key, properties.keys()) {
			rigid_body->setProperty(key, properties[key]);
		}

		shape->removeReference();
		rigid_bodies.append(rigid_body);
	}

	return rigid_bodies;
}

void HKWindow::convertNodeTree(const aiScene *scene, aiNode *node, LibGens::Matrix4 parent_transform, hkpWorld *world) {
	// Convert meshes to rigid bodies.
	LibGens::Matrix4 local_transform;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			local_transform[i][j] = node->mTransformation[i][j];

	if (node->mNumMeshes)
		logProgress(ProgressNormal, QString("Converting %1 to rigid bodies.").arg(node->mName.C_Str()));

	LibGens::Matrix4 transform = parent_transform * local_transform;
	QList<hkpRigidBody *> rigid_bodies = convertNodeToRigidBodies(scene, node, transform);
	
	if (!rigid_bodies.isEmpty())
		logProgress(ProgressNormal, QString("Adding %1 rigid bodies to World.").arg(rigid_bodies.size()));

	foreach(hkpRigidBody *rigid_body, rigid_bodies) {
		world->addEntity(rigid_body);
		rigid_body->removeReference();
	}

	// Convert children.
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		convertNodeTree(scene, node->mChildren[i], transform, world);
	}
}

#ifdef HAVOKCONVERTER_LOST_WORLD

hkpShape* HKWindow::convertMeshToCompressedShape(aiMesh* mesh, int userdata = 0)
{
	hkArray<hkGeometry::Triangle> triangle_buffer;
	hkArray<hkVector4> vertex_buffer;

	for (unsigned int f = 0; f < mesh->mNumFaces; f++)
	{
		if (mesh->mFaces[f].mNumIndices == 3)
		{
			hkGeometry::Triangle triangle;
			triangle.m_a = mesh->mFaces[f].mIndices[0];
			triangle.m_b = mesh->mFaces[f].mIndices[1];
			triangle.m_c = mesh->mFaces[f].mIndices[2];
			triangle.m_material = userdata;
			triangle_buffer.pushBack(triangle);
		}
	}

	for (unsigned int v = 0; v < mesh->mNumVertices; v++)
	{
		hkVector4 vert = hkVector4(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z);
		vertex_buffer.pushBack(vert);
	}

	hkGeometry geometry;
	geometry.m_triangles = triangle_buffer;
	geometry.m_vertices = vertex_buffer;

	hkpDefaultBvCompressedMeshShapeCinfo cinfo(&geometry);
	cinfo.m_collisionFilterInfoMode = hkpBvCompressedMeshShape::PerPrimitiveDataMode::PER_PRIMITIVE_DATA_PALETTE;
	cinfo.m_userDataMode = hkpBvCompressedMeshShape::PerPrimitiveDataMode::PER_PRIMITIVE_DATA_PALETTE;
	
	hkpShape* shape = new hkpBvCompressedMeshShape(cinfo);
	shape->setUserData(userdata);
	return shape;
}

hkQsTransform HKWindow::createHKTransform(LibGens::Matrix4 transform)
{
	hkQsTransform trans(hkQsTransform::IDENTITY);
	LibGens::Vector3 pos, scale;
	LibGens::Quaternion rot;
	transform.decomposition(pos, scale, rot);

	trans.setTranslation(hkVector4(pos.x, pos.y, pos.z));
	trans.setRotation(hkQuaternion(rot.x, rot.y, rot.z, rot.w));
	trans.setScale(hkVector4(scale.x, scale.y, scale.z, 1));

	trans.m_translation.mul(10);
	trans.m_scale.mul(10);
	return trans;
}

void HKWindow::convertNodeToCompressedShape(const aiScene* scene, aiNode* node, LibGens::Matrix4 parent_transform, hkpStaticCompoundShape* compound)
{
	LibGens::Matrix4 local_transform;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			local_transform[i][j] = node->mTransformation[i][j];

	LibGens::Matrix4 transform = parent_transform * local_transform;
	hkGeometry geometry;
	
	for (unsigned int m = 0; m < node->mNumMeshes; m++)
	{
		hkQsTransform trans = createHKTransform(transform);
		int userdata = 0;

		LibGens::Tags tags(node->mName.C_Str());
		for (int i = 0; i < tags.getTagCount(); i++) {
			QString tag_match = tags.getTag(i).getKey().c_str();

			// Search for tag in existing properties. We only use it if it matches the bitwise operation.
			for (int bitwise = 0; bitwise < 2; bitwise++) {
				foreach(const HKPropertyTag & tag, converter_settings.property_tags) {
					if (tag.tag == tag_match) {
						foreach(const HKPropertyValue & value, tag.values) {
							if (value.bitwise == bitwise) {
								if (value.bitwise == HKBitwise_SET)
									userdata = value.value;
								else if (value.bitwise == HKBitwise_OR)
									userdata |= value.value;
							}
						}
					}
				}
			}
		}

		hkpShape* shape = convertMeshToCompressedShape(scene->mMeshes[node->mMeshes[m]], userdata);
		int instance = compound->addInstance(shape, trans);
		compound->setInstanceUserData(instance, 1042652845 + compound->getInstances().getSize() - 1);
		compound->setInstanceFilterInfo(instance, 11);
		logProgress(ProgressNormal, QString("Adding instance with user data %1").arg(compound->getInstanceUserData(instance)));
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		convertNodeToCompressedShape(scene, node->mChildren[i], transform, compound);
	}
}

hkpStaticCompoundShape* HKWindow::convertNodeTreeCompoundShape(const aiScene* scene, aiNode* node, LibGens::Matrix4 parent_transform)
{
	hkpStaticCompoundShape* shapeContainer = new hkpStaticCompoundShape();

	convertNodeToCompressedShape(scene, node, parent_transform, shapeContainer);

	shapeContainer->bake();
	return shapeContainer;
}

#endif