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

#include "LibGens.h"
#include "PAC.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: pacpack file\n");
		getchar();
        return 1;
    }

	string pack_name = ToString(argv[1]);

	LibGens::Error::setLogging(true);
	LibGens::initialize();
	
	if (pack_name.find(".pac") != string::npos) {
		LibGens::PacSet *pac_set = new LibGens::PacSet(pack_name);
		string name = LibGens::File::nameFromFilenameNoExtension(pack_name);
		string folder = LibGens::File::folderFromFilename(pack_name);

		string new_folder = folder + name;
		CreateDirectory(new_folder.c_str(), NULL);
		pac_set->extract(new_folder + "/", false);
		delete pac_set;
	}
	else {
		LibGens::PacSet *pac_set = new LibGens::PacSet();
		pac_set->addFolder(pack_name + "/");
		pac_set->splitPacks();
		pac_set->save(pack_name + ".pac");
		delete pac_set;
	}

    return 0;
}