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

#include "UVAnimation.h"
#include "UVAnimationLibrary.h"

namespace LibGens {
	UVAnimationLibrary::UVAnimationLibrary(string folder_p) {
		addFolder(folder_p);
	}

	void UVAnimationLibrary::addFolder(string folder_p) {
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind;
		hFind = FindFirstFile((folder_p+"*"+LIBGENS_UV_ANIMATION_EXTENSION).c_str(), &FindFileData);
		if (hFind == INVALID_HANDLE_VALUE) {} 
		else {
			do {
				const char *name=FindFileData.cFileName;
				if (name[0]=='.') continue;

				string new_filename=folder_p+ToString(name);
				UVAnimation *uv_animation=new LibGens::UVAnimation(new_filename);
				animations.push_back(uv_animation);
			} while (FindNextFile(hFind, &FindFileData) != 0);
			FindClose(hFind);
		}
	}

	UVAnimation *UVAnimationLibrary::getUVAnimation(string material_name, string texset_name) {
		for (list<UVAnimation *>::iterator it=animations.begin(); it!=animations.end(); it++) {
			if (((*it)->getTexsetName() == texset_name) && ((*it)->getMaterialName() == material_name)) {
				return *it;
			}
		}

		return NULL;
	}
};