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
	rotation_x.fromAngleAxis(converter_settings.rotation_x * LIBGENS_MATH_DEGREE_TO_RAD, LibGens::Vector3(1.0f, 0.0f, 0.0f));
	rotation_y.fromAngleAxis(converter_settings.rotation_y * LIBGENS_MATH_DEGREE_TO_RAD, LibGens::Vector3(0.0f, 1.0f, 0.0f));
	rotation_z.fromAngleAxis(converter_settings.rotation_z * LIBGENS_MATH_DEGREE_TO_RAD, LibGens::Vector3(0.0f, 0.0f, 1.0f));
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

#ifdef HAVOKCONVERTER_LOSTWORLD
	hkArray<hkRootLevelContainer::NamedVariant> shapes;
#else
	hkpWorldCinfo info;
	info.m_convexListFilter = HK_NULL;
	info.m_minDesiredIslandSize = 64;
#ifndef HAVOK_5_5_0
#ifndef HAVOK_2012
	info.m_autoUpdateKdTree = false;
#endif
	info.m_maxConstraintViolation = FLT_MAX;
#endif
	hkpWorld* world = new hkpWorld(info);
	world->lock();
#endif // HAVOKCONVERTER_LOSTWORLD

	std::set<std::string> rigid_body_names;

	//*************************
	//  Load Assimp Scene
	//*************************
	foreach(QString model_source_path, model_source_paths) {
		logProgress(ProgressNormal, "Assimp importer reading model " + model_source_path + " ...");
		flushProgress(true);

		Assimp::Importer importer;
		importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS | aiComponent_TANGENTS_AND_BITANGENTS | aiComponent_COLORS | 
															aiComponent_TEXCOORDS | aiComponent_BONEWEIGHTS | aiComponent_TEXTURES | aiComponent_MATERIALS);

		const aiScene *scene = importer.ReadFile(model_source_path.toStdString(), aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_JoinIdenticalVertices | 
																				  aiProcess_RemoveComponent);
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
		
#ifdef HAVOKCONVERTER_LOSTWORLD
		hkpStaticCompoundShape* shape = convertNodeTreeCompoundShape(scene, scene->mRootNode, global_transform);
		shapes.pushBack(hkRootLevelContainer::NamedVariant("shape",shape, &hkcdStaticTreeDefaultTreeStorage6Class));
#else
		convertNodeTree(scene, scene->mRootNode, global_transform, world, rigid_body_names);
#endif
	}
	
#ifndef HAVOKCONVERTER_LOSTWORLD
	world->unlock();
#endif // !HAVOKCONVERTER_LOSTWORLD

	hkOstream outfile(output_file.toStdString().c_str());
	if (outfile.isOk()) {
		// Create root level container and push the physics data on to it.
		hkRootLevelContainer *root_level_container = new hkRootLevelContainer();

#ifdef HAVOKCONVERTER_LOSTWORLD
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

		hkRootLevelContainer::NamedVariant namedVariant("Physics Data", physics_data, &hkpPhysicsDataClass);

#ifdef HAVOK_5_5_0
		root_level_container->m_namedVariants = &namedVariant;
		root_level_container->m_numNamedVariants = 1;
#else
		root_level_container->m_namedVariants.pushBack(namedVariant);
		physics_data->removeReference();
#endif

#endif

		// Serialize root level container to a binary packfile.
		hkBinaryPackfileWriter writer;
		writer.setContents(root_level_container, hkRootLevelContainerClass);

		hkBinaryPackfileWriter::Options options = {};
#ifdef HAVOKCONVERTER_UNLEASHED
		options.m_layout = hkStructureLayout::Xbox360LayoutRules;
#else
		options.m_layout = hkStructureLayout::MsvcWin32LayoutRules;
#endif
		hkResult result = writer.save(outfile.getStreamWriter(), options);
		bool success = (result == HK_SUCCESS);
		if (success)  {
			logProgress(ProgressSuccess, "Serializer success! Saved file to " + output_file);
		}
		else {
			logProgress(ProgressFatal, "Serializer output error. File could not be saved properly.");
		}

#ifdef HAVOK_5_5_0
		physics_data->removeReference();
#endif

#ifndef HAVOKCONVERTER_LOSTWORLD
		system->removeReference();
		world->unlock();
		delete world;
#endif // !HAVOKCONVERTER_LOSTWORLD

		delete root_level_container;

		return success;
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

hkpRigidBody* HKWindow::convertNodeToRigidBody(const aiScene *scene, aiNode *node, LibGens::Matrix4 transform, std::set<std::string>& names) {
	LibGens::Tags tags(node->mName.C_Str());

	LibGens::Vector3 pos, sca;
	LibGens::Quaternion ori;
	transform.decomposition(pos, sca, ori);

	LibGens::AABB aabb;
	aabb.reset();

	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

		for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
			LibGens::Vector3 vertex(mesh->mVertices[j].x * sca.x, mesh->mVertices[j].y * sca.y, mesh->mVertices[j].z * sca.z);
			aabb.addPoint(vertex);
		}
	}

	hkpShape* shape = NULL;

	for (int i = 0; i < tags.getTagCount(); i++) {
		auto tag = tags.getTag(i);

		if (tag.getKey() == "BOX") {
			const hkVector4 half_extents(aabb.sizeX() * 0.5f, aabb.sizeY() * 0.5f, aabb.sizeZ() * 0.5f, 1.0f);
			shape = new hkpBoxShape(half_extents);
			pos = (pos + ori * aabb.center());
			logProgress(ProgressNormal, QString("Created box shape with half extents [%1, %2, %3].").arg(half_extents(0)).arg(half_extents(1)).arg(half_extents(2)));
			break;
		}

		else if (tag.getKey() == "SPHERE") {
			shape = new hkpSphereShape(aabb.radius());
			pos = (pos + ori * aabb.center());
			logProgress(ProgressNormal, QString("Created sphere shape with radius %1.").arg(aabb.radius()));
			break;
		}

		else if (tag.getKey() == "CYLINDER" || tag.getKey() == "CYL" || tag.getKey() == "CAPSULE") {
			// Get the longest extent to define the axis of the cylinder, then second longest to get the radius.

			auto mid_element = [](float a, float b, float c, float min, float max) -> float {
				if (a > min && a < max)
					return a;
				if (b > min && b < max)
					return b;
				if (c > min && c < max)
					return c;

				return min;
			};

			enum LongAxis {
				NONE,
				AXIS_X,
				AXIS_Y,
				AXIS_Z
			};

			LongAxis axis = NONE;
			string axis_value = tag.getValue(0, "");

			if (axis_value == "X" || axis_value == "x")
				axis = AXIS_X;
			if (axis_value == "Y" || axis_value == "y")
				axis = AXIS_Y;
			if (axis_value == "Z" || axis_value == "z")
				axis = AXIS_Z;

			const float size_x = aabb.sizeX() * 0.5f;
			const float size_y = aabb.sizeY() * 0.5f;
			const float size_z = aabb.sizeZ() * 0.5f;

			hkVector4 start;
			hkVector4 end;
			float radius;

			switch (axis) {
			case AXIS_X: {
				start = hkVector4(size_x, 0, 0);
				end = hkVector4(-size_x, 0, 0);
				radius = max(size_y, size_z);
				break;
			}
			case AXIS_Y: {
				start = hkVector4(0, size_y, 0);
				end = hkVector4(0, -size_y, 0);
				radius = max(size_x, size_z);
				break;
			}
			case AXIS_Z: {
				start = hkVector4(0, 0, size_z);
				end = hkVector4(0, 0, -size_z);
				radius = max(size_x, size_y);
				break;
			}

			// Determine automatically by getting the longest or shortest axis & assuming that's the polar axis.
			// If longest axis is the polar axis, get the SECOND longest axis as the radius. Else, just get the longest axis.
			// It's a hack method for sure, but better than nothing.
			case NONE: {
				start = hkVector4(size_x, 0, 0);
				end = hkVector4(-size_x, 0, 0);
				float min_size, max_size;
				const bool is_long = tag.getValueInt(0, 1) > 0; // if @CYL(0) then use short axis, else use long. Default is long.

				if (is_long) {
					max_size = size_x;
					if (size_y > max_size) {
						max_size = size_y;
						start = hkVector4(0, size_y, 0);
						end = hkVector4(0, -size_y, 0);
					}

					if (size_z > max_size) {
						max_size = size_z;
						start = hkVector4(0, 0, size_z);
						end = hkVector4(0, 0, -size_z);
					}
					min_size = min(size_x, min(size_y, size_z));
					radius = mid_element(size_x, size_y, size_z, min_size, max_size);
				}
				else {
					min_size = size_x;
					if (size_y < min_size) {
						min_size = size_y;
						start = hkVector4(0, size_y, 0);
						end = hkVector4(0, -size_y, 0);
					}

					if (size_z < min_size) {
						// No need to assign min, as that value is not used.
						start = hkVector4(0, 0, size_z);
						end = hkVector4(0, 0, -size_z);
					}
					radius = max(size_x, max(size_y, size_z));
				}
				break;
			}
			}

			if (tag.getKey() == "CAPSULE") {
				shape = new hkpCapsuleShape(start, end, radius);
				logProgress(ProgressNormal, QString("Created capsule shape with start [%1, %2, %3], end [%4, %5, %6] and radius %7.").arg(start(0)).arg(start(1)).arg(start(2)).arg(end(0)).arg(end(1)).arg(end(2)).arg(radius));
			}
			else {
				shape = new hkpCylinderShape(start, end, radius);
				logProgress(ProgressNormal, QString("Created cylinder shape with start [%1, %2, %3], end [%4, %5, %6] and radius %7.").arg(start(0)).arg(start(1)).arg(start(2)).arg(end(0)).arg(end(1)).arg(end(2)).arg(radius));
			}

			pos = (pos + ori * aabb.center());
			break;
		}
	}

	if (shape == NULL) {
		bool is_convex = tags.getTagValueBool("CONVEX", 0, converter_settings.mode == RigidBodies);

		if (is_convex) {
			int num_vertices = 0;

			for (unsigned int i = 0; i < node->mNumMeshes; i++) {
				aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
				num_vertices += mesh->mNumVertices;
			}
			
			float* vertex_buffer = new float[num_vertices * 4];
			int vertex_index = 0;

			for (unsigned int i = 0; i < node->mNumMeshes; i++) {
				aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

				for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
					LibGens::Vector3 vertex(mesh->mVertices[v].x * sca.x, mesh->mVertices[v].y * sca.y, mesh->mVertices[v].z * sca.z);

					vertex_buffer[vertex_index++] = vertex.x;
					vertex_buffer[vertex_index++] = vertex.y;
					vertex_buffer[vertex_index++] = vertex.z;
					vertex_buffer[vertex_index++] = 0.0f;
				}
			}

			vertices_to_free.push_back(vertex_buffer);

#ifdef HAVOK_5_5_0
			hkStridedVertices verts;
			verts.m_vertices = vertex_buffer;
			verts.m_numVertices = num_vertices;
			verts.m_striding = sizeof(hkVector4);

			hkGeometry geometry;
			hkInplaceArrayAligned16<hkVector4, 32> transformedPlanes;

			hkpGeometryUtility::createConvexGeometry(verts, geometry, transformedPlanes);

			verts.m_numVertices = geometry.m_vertices.getSize();
			verts.m_vertices = &(geometry.m_vertices[0](0));

			shape = new hkpConvexVerticesShape(verts, transformedPlanes);
#else
			hkStridedVertices verts((hkVector4*)vertex_buffer, num_vertices);
			shape = new hkpConvexVerticesShape(verts);
#endif
			logProgress(ProgressNormal, QString("Created convex shape with %1 vertices.").arg(num_vertices));
		}
		else {
			// Create extended mesh shape.
			hkpStorageExtendedMeshShape* extended_shape = new hkpStorageExtendedMeshShape();
			int num_vertices = 0;
			int num_faces = 0;

			for (unsigned int i = 0; i < node->mNumMeshes; i++) {
				aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

				// Get triangle count.
				int triangles = 0;
				for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
					if (mesh->mFaces[f].mNumIndices == 3) {
						triangles++;
					}
				}

				// Fill index buffer.
				size_t index_count = 0;
				int* index_buffer = new int[triangles * 3];
				for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
					if (mesh->mFaces[f].mNumIndices == 3) {
						index_buffer[index_count++] = mesh->mFaces[f].mIndices[0];
						index_buffer[index_count++] = mesh->mFaces[f].mIndices[1];
						index_buffer[index_count++] = mesh->mFaces[f].mIndices[2];
						num_faces += 3;
					}
				}

				indices_to_free.push_back(index_buffer);

				// Fill vertex buffer.
				float* vertex_buffer = new float[mesh->mNumVertices * 4];
				size_t vertex_count = 0;

				for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
					LibGens::Vector3 vertex(mesh->mVertices[v].x * sca.x, mesh->mVertices[v].y * sca.y, mesh->mVertices[v].z * sca.z);

					vertex_buffer[vertex_count++] = vertex.x;
					vertex_buffer[vertex_count++] = vertex.y;
					vertex_buffer[vertex_count++] = vertex.z;
					vertex_buffer[vertex_count++] = 0.0f;

					++num_vertices;
				}

				vertices_to_free.push_back(vertex_buffer);

				hkpExtendedMeshShape::TrianglesSubpart part;
				part.m_numTriangleShapes = triangles;
				part.m_numVertices = mesh->mNumVertices;
				part.m_vertexBase = vertex_buffer;
				part.m_stridingType = hkpExtendedMeshShape::INDICES_INT32;
				part.m_vertexStriding = sizeof(float) * 4;
				part.m_indexBase = index_buffer;
				part.m_indexStriding = sizeof(int) * 3;
				extended_shape->addTrianglesSubpart(part);
			}

			// Compile mopp and return it as the shape.
			hkpMoppCompilerInput mopp_input;
			mopp_input.setAbsoluteFitToleranceOfTriangles(TriangleAbsoluteTolerance);
			mopp_input.setAbsoluteFitToleranceOfAxisAlignedTriangles(hkVector4(TriangleAbsoluteTolerance, TriangleAbsoluteTolerance, TriangleAbsoluteTolerance));
			hkpMoppCode* code = hkpMoppUtility::buildCode(extended_shape, mopp_input);
			hkpMoppBvTreeShape* mopp_shape = new hkpMoppBvTreeShape(extended_shape, code);
#ifdef HAVOK_5_5_0
			extended_shape->computeWeldingInfo(mopp_shape, hkpWeldingUtility::WELDING_TYPE_ANTICLOCKWISE);
#else
			hkpMeshWeldingUtility::computeWeldingInfo(extended_shape, mopp_shape, hkpWeldingUtility::WELDING_TYPE_ANTICLOCKWISE, false);
#endif
			logProgress(ProgressNormal, QString("Created shape with %1 vertices, %2 faces and a MOPP code of size %3.").arg(num_vertices).arg(num_faces).arg(code->getCodeSize()));
			code->removeReference();
			extended_shape->removeReference();

			shape = mopp_shape;
		}
	}

	// Rigid body information.
	hkpRigidBodyCinfo rigid_body_info;
	rigid_body_info.m_position.set(pos.x, pos.y, pos.z);
	rigid_body_info.m_rotation.set(ori.x, ori.y, ori.z, ori.w);
	rigid_body_info.m_angularDamping = 0.049805f;
	rigid_body_info.m_linearDamping = 0.0f;

	switch (converter_settings.mode) {
	default:
	case Collision: {
		rigid_body_info.m_mass = 0.0f;
		rigid_body_info.m_motionType = hkpMotion::MOTION_FIXED;
		break;
	}
	case RigidBodies: {
		rigid_body_info.m_mass = tags.getTagValueFloat("MASS", 0, 5.0f);
		rigid_body_info.m_motionType = (tags.getTagValueInt("MOTION", 0, 1) == 0) ? hkpMotion::MOTION_FIXED : hkpMotion::MOTION_BOX_INERTIA;
		break;
	}
	}
	rigid_body_info.m_shape = shape;

	hkpRigidBody* rigid_body = new hkpRigidBody(rigid_body_info);

	std::string node_name;
	if (converter_settings.mode == RigidBodies) {
		node_name = tags.getName();
	}
	else {
		node_name = node->mName.C_Str();
	}

	std::string rigid_body_name = node_name;
	size_t rigid_body_name_index = 0;
	while (names.find(rigid_body_name) != names.end()) {
		rigid_body_name = node_name;
		rigid_body_name += "_";
		rigid_body_name += ToString(rigid_body_name_index);
		++rigid_body_name_index;
	}

	rigid_body->setName(names.insert(rigid_body_name).first->c_str());

	// Prepare properties.
	QMap<hkUint32, int> properties;
	for (int i = 0; i < tags.getTagCount(); i++) {
		QString tag_match = tags.getTag(i).getKey().c_str();

		// Search for tag in existing properties. We only use it if it matches the bitwise operation.
		for (int bitwise = 0; bitwise < 2; bitwise++) {
			foreach(const HKPropertyTag & tag, converter_settings.property_tags) {
				if (tag.tag == tag_match) {
					foreach(const HKPropertyValue & value, tag.values) {
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
	if (!properties.isEmpty()) {
		logProgress(ProgressNormal, QString("Applying %1 properties to %2").arg(properties.size()).arg(rigid_body->getName()));
	}

	foreach(hkUint32 key, properties.keys()) {
		if (rigid_body->hasProperty(key)) {
			rigid_body->editProperty(key, properties[key]);
		}
		else {
			rigid_body->addProperty(key, properties[key]);
		}
	}
	
	shape->removeReference();

	return rigid_body;
}

void HKWindow::convertNodeTree(const aiScene *scene, aiNode *node, LibGens::Matrix4 parent_transform, hkpWorld *world, std::set<std::string>& names) {
	// Convert meshes to rigid bodies.
	LibGens::Matrix4 local_transform;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			local_transform[i][j] = node->mTransformation[i][j];

	LibGens::Matrix4 transform = parent_transform * local_transform;

	if (node->mNumMeshes) {
		logProgress(ProgressNormal, QString("Converting %1 to rigid body.").arg(node->mName.C_Str()));

		hkpRigidBody* rigid_body = convertNodeToRigidBody(scene, node, transform, names);
		world->addEntity(rigid_body);
		rigid_body->removeReference();
	}

	// Convert children.
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		convertNodeTree(scene, node->mChildren[i], transform, world, names);
	}
}

#ifdef HAVOKCONVERTER_LOSTWORLD

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