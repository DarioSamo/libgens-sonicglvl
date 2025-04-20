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

#include "Havok.h"
#include "HavokAnimationCache.h"
#include "HavokPhysicsCache.h"
#include "HavokSkeletonCache.h"
#include "HavokEnviroment.h"
#include "HavokPropertyDatabase.h"
#include <Common/Base/keycode.cxx>

#ifdef HAVOK_5_5_0
    #define HK_CLASSES_FILE <Common/Serialize/Classlist/hkCompleteClasses.h>
    #include <Common/Serialize/Util/hkBuiltinTypeRegistry.cxx>

    #define HK_COMPAT_FILE <Common/Compat/hkCompatVersions.h>
    #include <Common/Compat/hkCompat_All.cxx>
#else
    #include <Common/Base/Config/hkProductFeatures.cxx>
#endif