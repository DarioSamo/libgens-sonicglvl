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

#ifndef HAVOKCONVERTER_UNLEASHED
#include <Common/Internal/ConvexHull/hkGeometryUtility.h>
#include <Common/Internal/GeometryProcessing/ConvexHull/hkgpConvexHull.h>
#else
#include <Common\Base\Types\Geometry\hkStridedVertices.h>
#include <Physics\Internal\PreProcess\ConvexHull\hkpGeometryUtility.h>
#include <Physics\Collide\Util\Welding\hkpMeshWeldingUtility.h>
#endif

#include <Physics/Collide/Shape/Convex/ConvexVertices/hkpConvexVerticesConnectivity.h>
#include <Physics/Collide/Shape/Convex/ConvexVertices/hkpConvexVerticesConnectivityUtil.h>
#include <Physics/Collide/Shape/Convex/Sphere/hkpSphereShape.h>
#include <Physics/Collide/Shape/Convex/Cylinder/hkpCylinderShape.h>

// TODO: Migrate
// Taken from HavokAnimationExporter
#ifdef HAVOKCONVERTER_UNLEASHED
template<typename T>
void ToPtrArray(const hkArray<T>& array, T*& ptr, hkInt32& num)
{
	num = array.getSize();
	ptr = new T[num];

	memcpy(ptr, &array[0], num * sizeof(T));
}

template<typename T>
void ToPtrArray(const hkArray<T>& array, T**& ptr, hkInt32& num)
{
	num = array.getSize();
	ptr = new T * [num];

	for (int i = 0; i < num; i++)
		ptr[i] = new T(array[i]);
}
#endif

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
#ifndef HAVOKCONVERTER_UNLEASHED
#ifndef Release2012
	info.m_autoUpdateKdTree = false;
#endif
	info.m_maxConstraintViolation = FLT_MAX;
#endif // HAVOKCONVERTER_UNLEASHED
	hkpWorld* world = new hkpWorld(info);
	world->lock();
#endif // HAVOKCONVERTER_LOST_WORLD

	std::unordered_set<std::string> rigid_body_names;

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
		convertNodeTree(scene, scene->mRootNode, global_transform, world, rigid_body_names);
#endif
	}
	
#ifndef HAVOKCONVERTER_LOST_WORLD
	world->unlock();
#endif // !HAVOKCONVERTER_LOST_WORLD

	hkOstream outfile(output_file.toStdString().c_str());
	if (!outfile.isOk()) {
		logProgress(ProgressFatal, "Could not open output file for writing!");
		return false;
	}

	// Create root level container and push the physics data on to it.
	hkRootLevelContainer *root_level_container = new hkRootLevelContainer();
	hkArray<hkRootLevelContainer::NamedVariant> namedVariants;

#ifdef HAVOKCONVERTER_LOST_WORLD
	namedVariants.append(shapes);
	logProgress(ProgressNormal, QString("Conversion finished with %1 compressed shapes.").arg(shapes.getSize()));
#else
	// Create physics data.
	static hkpPhysicsData* physics_data = new hkpPhysicsData();

	// Populate physics data with the world we created.
	world->lock();
	hkpPhysicsSystem* system = world->getWorldAsOneSystem();
	system->setName("Default Physics System");
	physics_data->addPhysicsSystem(system);

	logProgress(ProgressNormal, QString("Physics system with %1 rigid bodies.").arg(system->getRigidBodies().getSize()));

	namedVariants.pushBack(hkRootLevelContainer::NamedVariant("Physics Data", physics_data, &hkpPhysicsDataClass));

#ifndef HAVOKCONVERTER_UNLEASHED
	root_level_container->m_namedVariants = std::move(namedVariants);
	physics_data->removeReference();
#else
	ToPtrArray(namedVariants, root_level_container->m_namedVariants, root_level_container->m_numNamedVariants);
#endif // HAVOKCONVERTER_UNLEASHED
#endif // HAVOKCONVERTER_LOST_WORLD
	
	// Serialize root level container to a binary packfile.
	hkPackfileWriter::Options pack_options;

#ifndef HAVOKCONVERTER_UNLEASHED
	pack_options.m_writeMetaInfo = false;
	hkSerializeUtil::SaveOptions options;
	options.useBinary(true);
	hkResult result = hkSerializeUtil::savePackfile(root_level_container, hkRootLevelContainerClass, outfile.getStreamWriter(), pack_options, HK_NULL, options);
#else

	hkBinaryPackfileWriter* writer = new hkBinaryPackfileWriter();
	writer->setContents(root_level_container, hkRootLevelContainerClass);
	pack_options.m_layout = hkStructureLayout::GccPs3LayoutRules; // PS3 also works on Xbox 360, but not vice versa.
	hkResult result = writer->save(outfile.getStreamWriter(), pack_options);

	physics_data->removeReference();
#endif

	//logProgress(ProgressNormal, QString("Variant info: %1").arg(writer->));

	bool convertResult = result == HK_SUCCESS;
	if (convertResult)  {
		logProgress(ProgressSuccess, "Serializer success! Saved file to " + output_file);
	}
	else {
		logProgress(ProgressFatal, "Serializer output error. File could not be saved properly.");
	}

#ifndef HAVOKCONVERTER_LOST_WORLD
	system->removeReference();
	world->unlock();
	delete world;
#endif // !HAVOKCONVERTER_LOST_WORLD

	delete root_level_container;
	return convertResult;
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

#ifndef HAVOKCONVERTER_UNLEASHED
	hkpMeshWeldingUtility::computeWeldingInfo(extended_shape, mopp_shape, hkpWeldingUtility::WELDING_TYPE_ANTICLOCKWISE, false);
#else
	// TODO: The above utility is not available in this version, which poses a problem,
	// as multi-shape welding does not seem to be handled. We need a way to fix this.
	extended_shape->computeWeldingInfo(mopp_shape, hkpWeldingUtility::WELDING_TYPE_ANTICLOCKWISE);
#endif

	logProgress(ProgressNormal, QString("Created shape with %1 vertices, %2 faces and a MOPP code of size %3.").arg(mesh->mNumVertices).arg(mesh->mNumFaces).arg(code->getCodeSize()));
	code->removeReference();
	extended_shape->removeReference();

	return mopp_shape;
}

hkpConvexShape* HKWindow::convertMeshToConvexShape(aiMesh* mesh, LibGens::Vector3 scale, LibGens::Tags& tags) {
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

	//const LibGens::Vector3 offset = position - aabb.center();

	// Can't make a switch statement out of tags, so we just have to check for each tag we care about in sequence and go from there.
	// The first time our tag matches our desired tag, we have our convex shape, so just return.

	for (int i = 0; i < tags.getTagCount(); ++i) {
		auto tag = tags.getTag(i);

		if (tag.getKey() == "BOX") {
			const hkVector4 halfExtents(aabb.sizeX() * 0.5f, aabb.sizeY() * 0.5f, aabb.sizeZ() * 0.5f, 1.0);
			return new hkpBoxShape(halfExtents);
		}
		if (tag.getKey() == "SPHERE") {

			// TODO: For accuracy's sake, instead of getting the largest AABB direction, loop through all points & get the longest squared distance to the center.
			// This cheap method will work OKAY for now, but won't guarantee a fit around the object.
			return new hkpSphereShape(aabb.sizeMax() * 0.5f);
		}
		if (tag.getKey() == "CYLINDER" || tag.getKey() == "CYL") {

			// This is a bit involved, but worth it.
			// Idea: Get the longest extent to define the axis of the cylinder, then second longest to get the radius.

			auto f_min = [](float a, float b) -> float
			{
				return a < b ? a : b;
			};
			auto f_max = [](float a, float b) -> float
			{
				return a > b ? a : b;
			};
			auto mid_element = [](float a, float b, float c, float min, float max) -> float
			{
				if (a > min && a < max)
					return a;
				if (b > min && b < max)
					return b;
				if (c > min && c < max)
					return c;

				// Fallback?
				return min;
			};

			enum CylAxis {
				NONE,
				AxisX,
				AxisY,
				AxisZ
			};

			CylAxis axis = NONE;
			string axisValue = tag.getValue(0, "");
			if (axisValue == "X" || axisValue == "x")
				axis = AxisX;
			if (axisValue == "Y" || axisValue == "y")
				axis = AxisY;
			if (axisValue == "Z" || axisValue == "z")
				axis = AxisZ;

			const float size_x = aabb.sizeX() * 0.5f;
			const float size_y = aabb.sizeY() * 0.5f;
			const float size_z = aabb.sizeZ() * 0.5f;

			hkVector4 start;
			hkVector4 end;
			float radius;

			switch (axis) {
				case AxisX: {
					start = hkVector4( size_x, 0, 0);
					end   = hkVector4(-size_x, 0, 0);
					radius = f_max(size_y, size_z);
					break;
				}
				case AxisY: {
					start = hkVector4(0,  size_y, 0);
					end   = hkVector4(0, -size_y, 0);
					radius = f_max(size_x, size_z);
					break;
				}
				case AxisZ: {
					start = hkVector4(0, 0,  size_z);
					end   = hkVector4(0, 0, -size_z);
					radius = f_max(size_x, size_y);
					break;
				}

				// Determine automatically by getting the longest or shortest axis & assuming that's the polar axis.
				// If longest axis is the polar axis, get the SECOND longest axis as the radius. Else, just get the longest axis.
				// It's a h4ck method for sure, but better than nothing.
				case NONE: {
					start = hkVector4(size_x, 0, 0);
					end   = hkVector4(-size_x, 0, 0);
					float min_size, max_size;
					const bool isLong = tag.getValueInt(0, 1) > 0; // if @CYL(0) then use short axis, else use long. Default is long.

					if (isLong) {

						max_size  = size_x;
						if (size_y > max_size) {
							max_size = size_y;
							start = hkVector4(0,  size_y, 0);
							end   = hkVector4(0, -size_y, 0);
						}

						if (size_z > max_size) {
							max_size = size_z;
							start = hkVector4(0, 0,  size_z);
							end   = hkVector4(0, 0, -size_z);
						}
						min_size = f_min(size_x, f_min(size_y, size_z));
						radius = mid_element(size_x, size_y, size_z, min_size, max_size);
					}
					else {

						min_size  = size_x;
						if (size_y < min_size) {
							min_size = size_y;
							start = hkVector4(0,  size_y, 0);
							end   = hkVector4(0, -size_y, 0);
						}

						if (size_z < min_size) {
							// No need to assign min, as that value is not used.
							start = hkVector4(0, 0,  size_z);
							end   = hkVector4(0, 0, -size_z);
						}
						radius = f_max(size_x, f_max(size_y, size_z));
					}
					break;
				}
			}

			return new hkpCylinderShape(start, end, radius);
		}
	}

#ifndef HAVOKCONVERTER_UNLEASHED
	hkStridedVertices verts((hkVector4*)vertex_buffer, (int)mesh->mNumVertices);
	hkpConvexShape* shape = new hkpConvexVerticesShape(verts);
#else
	hkStridedVertices verts;
	verts.m_vertices = vertex_buffer;
	verts.m_numVertices = (int)mesh->mNumVertices;
	verts.m_striding = sizeof(hkVector4);

	hkGeometry geometry;
	hkInplaceArrayAligned16<hkVector4, 32> transformedPlanes;

	hkpGeometryUtility::createConvexGeometry(verts, geometry, transformedPlanes);

	verts.m_numVertices = geometry.m_vertices.getSize();
	verts.m_vertices = &(geometry.m_vertices[0](0));

	hkpConvexShape* shape = new hkpConvexVerticesShape(verts, transformedPlanes);
#endif

	// TODO: make connectivity whenever possible if we need it.
	//hkpConvexVerticesConnectivity connectivity = new hkpConvexVerticesConnectivity()

	return shape;
}

QList<hkpRigidBody *> HKWindow::convertNodeToRigidBodies(const aiScene *scene, aiNode *node, LibGens::Matrix4 transform, std::unordered_set<std::string>& names) {
	QList<hkpRigidBody *> rigid_bodies;
	LibGens::Vector3 pos, sca;
	LibGens::Quaternion ori;
	transform.decomposition(pos, sca, ori);

	LibGens::Tags tags(node->mName.C_Str());

	auto GetConvexTag = [&tags]() -> bool {
		for (int i = 0; i < tags.getTagCount(); ++i) {
			auto tag = tags.getTag(i);

			if (tag.getKey() == "BOX"
				|| tag.getKey() == "SPHERE"
				|| tag.getKey() == "CYLINDER"
				|| tag.getKey() == "CYL")
				return true;
		}
		return false;
	};

	size_t index = 0;

	for (unsigned int m = 0; m < node->mNumMeshes; m++) {
		// Rigid body information.
		hkpRigidBodyCinfo rigid_body_info;
		rigid_body_info.m_position.set(pos.x, pos.y, pos.z);
		rigid_body_info.m_rotation.set(ori.x, ori.y, ori.z, ori.w);
		rigid_body_info.m_angularDamping = 0.049805f;
		rigid_body_info.m_linearDamping = 0.0f;

		bool isRigidBody = false;
		switch (converter_settings.mode) {  // NOLINT(clang-diagnostic-switch-enum)
			default:
			case Collision: {
				rigid_body_info.m_mass = 0.0f;
				rigid_body_info.m_motionType = hkpMotion::MOTION_FIXED;
				break;
			}
			case RigidBodies: {
				isRigidBody = true;
				const int motionTypeTagValue = tags.getTagValueInt("MOTION", 0, 1);

				rigid_body_info.m_mass = tags.getTagValueFloat("MASS", 0, 5.0f);
				rigid_body_info.m_motionType = motionTypeTagValue == 0
				                             ? hkpMotion::MOTION_FIXED
				                             : hkpMotion::MOTION_BOX_INERTIA;
				break;
			}
		}

		const bool isConvex = tags.getTagValueBool("CONVEX", 0, isRigidBody) || GetConvexTag();
		hkpShape* shape = isConvex
		                ? convertMeshToConvexShape(scene->mMeshes[node->mMeshes[m]], sca, tags)
		                : convertMeshToShape(scene->mMeshes[node->mMeshes[m]], sca);

		rigid_body_info.m_shape = shape;
		hkpRigidBody* rigid_body = new hkpRigidBody(rigid_body_info);

		// Prepare properties.
		QMap<hkUint32, int> properties;

		// remove tags from name if in rigidbody mode
		// TODO: Possibly implement a more robust tag-removal step; there may be niche cases we want terrain tags on object physics.
		std::string temp_name = converter_settings.mode == RigidBodies
		                      ? tags.getName().c_str()
		                      : node->mName.C_Str();

		std::string rigid_body_name = temp_name;
		while (names.find(rigid_body_name) != names.end()) {
			rigid_body_name = ToString(temp_name) + "_" + ToString(index++);
		}

		// The name is going to be kept alive by the unordered set.
		rigid_body->setName(names.insert(rigid_body_name).first->c_str());

		// TODO: See about finding a way to prevent iterating here *again?*
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
		if (!properties.isEmpty())
			logProgress(ProgressNormal, QString("Applying %1 properties to %2").arg(properties.size()).arg(rigid_body->getName()));

		foreach(hkUint32 key, properties.keys()) {
#ifndef HAVOKCONVERTER_UNLEASHED
			rigid_body->setProperty(key, properties[key]);
#else
			// setProperty was not added until later, so we have to do this ourselves...
			bool hasProperty = rigid_body->hasProperty(key);
			if (hasProperty)
				rigid_body->editProperty(key, properties[key]);
			else
				rigid_body->addProperty(key, properties[key]);
#endif
		}

		shape->removeReference();
		rigid_bodies.append(rigid_body);
	}

	return rigid_bodies;
}

void HKWindow::convertNodeTree(const aiScene *scene, aiNode *node, LibGens::Matrix4 parent_transform, hkpWorld *world, std::unordered_set<std::string>& names) {
	// Convert meshes to rigid bodies.
	LibGens::Matrix4 local_transform;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			local_transform[i][j] = node->mTransformation[i][j];

	if (node->mNumMeshes)
		logProgress(ProgressNormal, QString("Converting %1 to rigid bodies.").arg(node->mName.C_Str()));

	LibGens::Matrix4 transform = parent_transform * local_transform;
	QList<hkpRigidBody *> rigid_bodies = convertNodeToRigidBodies(scene, node, transform, names);
	
	if (!rigid_bodies.isEmpty())
		logProgress(ProgressNormal, QString("Adding %1 rigid bodies to World.").arg(rigid_bodies.size()));

	foreach(hkpRigidBody *rigid_body, rigid_bodies) {
		world->addEntity(rigid_body);
		rigid_body->removeReference();
	}

	// Convert children.
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		convertNodeTree(scene, node->mChildren[i], transform, world, names);
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