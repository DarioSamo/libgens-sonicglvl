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

#include "FBX.h"
#include "FBXManager.h"

namespace LibGens {
	FBXManager::FBXManager() {
		sdk_manager = FbxManager::Create();
		FbxIOSettings *ios = FbxIOSettings::Create(sdk_manager, IOSROOT);
		ios->SetBoolProp(EXP_FBX_EMBEDDED, true);
		sdk_manager->SetIOSettings(ios);
	}

	FBX *FBXManager::importFBX(string filename, string scene_name) {
		FbxImporter* lImporter = FbxImporter::Create(sdk_manager, "");
		// Import
		bool lImportStatus = lImporter->Initialize(filename.c_str(), -1, sdk_manager->GetIOSettings());
		if(!lImportStatus) {
			// Handle error
			printf("Call to FbxImporter::Initialize() failed.\n");
			printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
			return NULL;
		}
		FbxScene* lScene = FbxScene::Create(sdk_manager, scene_name.c_str());
		lImporter->Import(lScene);
		int lFileMajor, lFileMinor, lFileRevision;
		lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

		// Create LibGens FBX from Scene
		FBX *fbx = new FBX(lScene);

		// Cleanup
		//lScene->Destroy();
		lImporter->Destroy();

		return fbx;
	}

	FbxManager *FBXManager::getManager() {
		return sdk_manager;
	}
};