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
#include "AR.h"

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: pfdpack file folder_name\nExample: pfdpack Stage stage_folder");
		getchar();
        return 1;
    }

	string pack_name = ToString(argv[1]);
    string source = ToString(argv[2]);

	LibGens::ArPack pack(source + "/");

	if (pack_name.find(".ar") != string::npos) {
		pack.save(pack_name);
	}
	else {
		pack.save(pack_name + ".pfd");
		pack.savePFI(pack_name + ".pfi");
	}

    return 0;
}