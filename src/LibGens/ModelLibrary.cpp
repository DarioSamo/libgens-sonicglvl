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

#include "Model.h"
#include "ModelLibrary.h"

namespace LibGens {
	ModelLibrary::ModelLibrary(string folder_p) {
		folder = folder_p;
	}

	Model *ModelLibrary::getModel(string filename) {
		for (list<Model *>::iterator it=models.begin(); it!=models.end(); it++) {
			if ((*it)->getName() == filename) {
				return (*it);
			}
		}

		string new_filename=folder+filename+".model";
		if (File::check(new_filename)) {
			File file(new_filename, LIBGENS_FILE_READ_BINARY);
			if (!file.valid()) return NULL;
			file.close();

			Model *model = new Model(new_filename);
			model->setName(filename);
			models.push_back(model);
			return model;
		}

		return NULL;
	}
};