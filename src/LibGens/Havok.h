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

#pragma once

#include <Common/Base/hkBase.h>
#include <Common/Base/System/hkBaseSystem.h>
#include <Common/Base/System/Error/hkDefaultError.h>
#ifndef Release2005
#include <Common/Base/Memory/System/Util/hkMemoryInitUtil.h>
#include <Common/Base/Memory/System/hkMemorySystem.h>
#include <Common/Base/Memory/Allocator/Malloc/hkMallocAllocator.h>
#else
#include <Common/Base/Memory/Memory/FreeList/hkLargeBlockAllocator.h>
#include <Common/Base/Memory/Memory/FreeList/hkFreeListMemory.h>
#include <Common/Base/Memory/Memory/FreeList/SystemMemoryBlockServer/hkSystemMemoryBlockServer.h>
#include <Common/Base/Memory/Memory/Pool/hkPoolMemory.h>
#include <Common/Base/Memory/MemoryClasses/hkMemoryClassDefinitions.h>
#endif



#include <Common/Base/Monitor/hkMonitorStream.h>
#include <Common/Base/System/Io/IStream/hkIStream.h>
#include <Common/Base/Reflection/hkClass.h>
#include <Common/Base/Reflection/Registry/hkTypeInfoRegistry.h>
#include <Common/SceneData/Graph/hkxNode.h>
#include <Common/SceneData/Scene/hkxScene.h>

#include <Common/Serialize/Util/hkStructureLayout.h>
#include <Common/Serialize/Util/hkRootLevelContainer.h>
#ifndef Release2005
#include <Common/Serialize/Util/hkSerializeUtil.h>
#else
#include <Common/Serialize/Packfile/Binary/hkBinaryPackfileReader.h>
#endif
#include <Common/Serialize/Packfile/Binary/hkBinaryPackfileWriter.h>
#include <Physics/Utilities/Serialize/hkpPhysicsData.h>
#include <Physics/Utilities/Dynamics/Inertia/hkpInertiaTensorComputer.h>
#include <Physics/Dynamics/hkpDynamics.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>
#include <Physics/Dynamics/Entity/hkpRigidBodyCinfo.h>
#include <Physics/Collide/hkpCollide.h>
#include <Physics/Collide/Shape/hkpShape.h>
#ifndef Release2005
#include <Physics/Collide/Shape/hkpShapeBuffer.h>
#endif
#include <Physics/Collide/Shape/hkpShapeContainer.h>
#include <Physics/Collide/Shape/hkpShapeType.h>
#include <Physics/Collide/Shape/Convex/Box/hkpBoxShape.h>
#include <Physics/Collide/Shape/Convex/ConvexVertices/hkpConvexVerticesShape.h>
#include <Physics/Collide/Shape/Compound/Collection/StorageExtendedMesh/hkpStorageExtendedMeshShape.h>
#include <Physics/Collide/Util/Welding/hkpMeshWeldingUtility.h>

#include <Physics/Collide/Shape/Compound/Tree/Mopp/hkpMoppBvTreeShape.h>
#include <Physics/Collide/Shape/Compound/Tree/Mopp/hkpMoppCompilerInput.h>
#include <Physics/Collide/Shape/Compound/Tree/Mopp/hkpMoppUtility.h>

#ifdef Release2012
#include <Physics/Internal/Collide/StaticCompound/hkpStaticCompoundShape.h>
#include <Physics/Internal/Collide/BvCompressedMesh/hkpBvCompressedMeshShape.h>
#include <Physics/Internal/Collide/BvCompressedMesh/hkpBvCompressedMeshShapeCinfo.h>
#include <Physics/Collide/Shape/Convex/ConvexTranslate/hkpConvexTranslateShape.h>
#include <Physics/Collide/Shape/Compound/Collection/List/hkpListShape.h>
#include <Physics/Collide/Shape/Convex/ConvexVertices/hkpConvexVerticesShape.h>
#endif

#include <Physics/Collide/Shape/Compound/Collection/SimpleMesh/hkpSimpleMeshShape.h>
#include <Physics/Collide/Shape/Compound/Collection/StorageExtendedMesh/hkpStorageExtendedMeshShape.h>
#include <Common/Base/Math/Matrix/hkTransform.h>
#include <Common/Base/Types/Geometry/hkGeometry.h>
#include <Physics/Utilities/Collide/ShapeUtils/ShapeConverter/hkpShapeConverter.h>

// Animation includes
#ifndef Release2005
#include <Animation/Animation/Animation/Quantized/hkaQuantizedAnimation.h>
#include <Animation/Animation/Animation/Mirrored/hkaMirroredAnimation.h>
#include <Animation/Animation/Animation/Mirrored/hkaMirroredSkeleton.h>
#include <Animation/Animation/Animation/SplineCompressed/hkaSplineCompressedAnimation.h>
#endif
#include <Animation/Animation/Animation/Util/hkaAdditiveAnimationUtility.h>
#include <Animation/Animation/hkaAnimationContainer.h>
#include <Animation/Animation/Playback/Control/Default/hkaDefaultAnimationControl.h>
#include <Animation/Animation/Playback/hkaAnimatedSkeleton.h>
#include <Animation/Animation/Playback/Multithreaded/hkaMultithreadedAnimationUtils.h>
#ifndef Release2005
#include <Animation/Animation/Playback/Multithreaded/Common/hkaCommonAnimation.h>
#include <Animation/Animation/Playback/Multithreaded/SampleAndCombine/hkaAnimationSampleAndCombineJobQueueUtils.h>
#endif
#include <Animation/Animation/Rig/hkaPose.h>
#include <Animation/Animation/Rig/hkaSkeletonUtils.h>

#define LIBGENS_HAVOK_PHYSICS_EXTENSION     ".phy.hkx"
#define LIBGENS_HAVOK_SKELETON_EXTENSION    ".skl.hkx"
#define LIBGENS_HAVOK_ANIMATION_EXTENSION   ".anm.hkx"

#define LIBGENS_HAVOK_DATABASE_NAME         "name"
#define LIBGENS_HAVOK_DATABASE_ENTRY        "Entry"
#define LIBGENS_HAVOK_DATABASE_HKPARAM      "hkparam"
#define LIBGENS_HAVOK_DATABASE_HKOBJECT     "hkobject"
#define LIBGENS_HAVOK_DATABASE_NUMELEMENTS  "numelements"
#define LIBGENS_HAVOK_DATABASE_KEY          "key"
#define LIBGENS_HAVOK_DATABASE_VALUE        "value"
#define LIBGENS_HAVOK_DATABASE_DATA         "data"

namespace LibGens {
	static void HK_CALL HavokErrorReport(const char*, void*) {
	}
};