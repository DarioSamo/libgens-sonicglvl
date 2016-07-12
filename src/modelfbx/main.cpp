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

#include "LibGens.h"
#include "FBX.h"
#include "FBXManager.h"
#include "HavokEnviroment.h"
#include "MaterialLibrary.h"
#include "Model.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: modelfbx [mesh.model] [skeleton.skl.hkx] [animation.anm.hkx] [output.fbx]\n - First 3 parameters can be in any order. You can omit any parameter for excluding elements from the output.\n");
		getchar();
        return 1;
    }
	LibGens::initialize();
	LibGens::Error::setLogging(true);
	
	string source_model="";
	string source_skeleton="";
	string source_animation="";
	string output_file="";
	size_t pos=0;

	for (int i=1; i<argc; i++) {
		string parameter = argv[i];
		std::transform(parameter.begin(), parameter.end(), parameter.begin(), ::tolower);

		if ((parameter.find(".model") != string::npos) && (!source_model.size())) {
			source_model = ToString(argv[i]);
		}

		if ((parameter.find(".terrain-model") != string::npos) && (!source_model.size())) {
			source_model = ToString(argv[i]);
		}

		if (((pos=parameter.find(".skl.hkx")) != string::npos) && (!source_skeleton.size())) {
			source_skeleton = ToString(argv[i]);
			source_skeleton.resize(pos);
		}

		if (((pos=parameter.find(".anm.hkx")) != string::npos) && (!source_animation.size())) {
			source_animation = ToString(argv[i]);
			source_animation.resize(pos);
		}

		if ((parameter.find(".fbx") != string::npos) && (!output_file.size())) {
			output_file = ToString(argv[i]);
		}
	}

	if (source_model.size() && !output_file.size()) {
		output_file = source_model + ".fbx";
	}

	if (source_skeleton.size() && !output_file.size()) {
		output_file = source_skeleton + ".fbx";
	}

	// Initialize FBX Package
	LibGens::FBXManager *fbx_manager = new LibGens::FBXManager();
	LibGens::FBX *fbx_pack = new LibGens::FBX(fbx_manager->getManager());
	LibGens::HavokEnviroment havok_enviroment(100 * 1024 * 1024);

	havok_enviroment.addFolder("./");
	
	LibGens::Model *model = NULL;

	if (source_model.size()) {
		model = new LibGens::Model(source_model);

		string folder="";
		size_t sep = source_model.find_last_of("\\/");
		if (sep != std::string::npos) {
			folder = source_model.substr(0, sep+1);
		}

		LibGens::MaterialLibrary *material_library = new LibGens::MaterialLibrary(folder);
		fbx_pack->setMaterialLibrary(material_library);
	}

	LibGens::HavokSkeletonCache *havok_skeleton_cache = NULL;
	LibGens::HavokAnimationCache *havok_animation_cache = NULL;

	if (source_skeleton.size()) {
		havok_skeleton_cache = havok_enviroment.getSkeleton(source_skeleton);
	}

	if (source_animation.size()) {
		havok_animation_cache = havok_enviroment.getAnimation(source_animation);
	}

	FbxMesh *model_node=fbx_pack->addNode(model, havok_skeleton_cache, havok_animation_cache);
	fbx_manager->exportFBX(fbx_pack, output_file);
    return 0;
}