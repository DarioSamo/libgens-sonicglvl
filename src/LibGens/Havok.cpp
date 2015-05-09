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

#include "Havok.h"
#include "HavokAnimationCache.h"
#include "HavokPhysicsCache.h"
#include "HavokSkeletonCache.h"
#include "HavokEnviroment.h"
#include "HavokPropertyDatabase.h"
#include <Common/Base/keycode.cxx>
#include <Common/Base/Config/hkProductFeatures.cxx>

// Keycode

// This excludes libraries that are not going to be linked
// from the project configuration, even if the keycodes are
// present
#define HK_FEATURE_REFLECTION_PHYSICS
#define HK_FEATURE_REFLECTION_ANIMATION
#define HK_FEATURE_REFLECTION_BEHAVIOUR
#define HK_CLASSES_FILE <Common/Serialize/Classlist/hkClasses.h>
#define HK_EXCLUDE_FEATURE_MemoryTracker
#define HK_EXCLUDE_FEATURE_RegisterVersionPatches 
#define HK_EXCLUDE_LIBRARY_hkGeometryUtilities

namespace LibGens {
};