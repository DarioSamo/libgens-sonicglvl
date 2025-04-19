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


#include "Material.h"
#include "MaterialLibrary.h"


namespace LibGens {
	MaterialLibrary::MaterialLibrary(string folder_p) {
		folder = folder_p;
	}

	void MaterialLibrary::addMaterial(Material *material) {
		materials.push_back(material);
	}

	
	bool MaterialLibrary::checkMaterial(string id) {
		for (list<Material *>::iterator it=materials.begin(); it!=materials.end(); it++) {
			if ((*it)->getName() == id) {
				return true;
			}
		}

		return false;
	}


	Material *MaterialLibrary::getMaterial(string id) {
		for (list<Material *>::iterator it=materials.begin(); it!=materials.end(); it++) {
			if ((*it)->getName() == id) {
				return *it;
			}
		}
		
		Material *mat=new Material(folder + id + LIBGENS_MATERIAL_EXTENSION);
		materials.push_back(mat);
		return mat;
	}


	list<Material*> MaterialLibrary::getMaterials() {
		return materials;
	}

	void MaterialLibrary::merge(MaterialLibrary *library, bool overwrite) {
		list<Material *> to_push;

		for (list<Material *>::iterator it=library->materials.begin(); it!=library->materials.end(); it++) {
			bool found=false;

			for (list<Material *>::iterator it2=materials.begin(); it2!=materials.end(); it2++) {
				if ((*it)->getName() == (*it2)->getName()) {
					found = true;

					if (overwrite) {
						delete (*it2);
						(*it2) = (*it);
					}
					else {
						delete (*it);
					}
				}
			}

			if (!found) to_push.push_back(*it);
		}
		library->materials.clear();

		for (list<Material *>::iterator it=to_push.begin(); it!=to_push.end(); it++) {
			materials.push_back(*it);
		}

		delete library;
	}

	void MaterialLibrary::save(string folder_target, int root_type) {
		folder = folder_target;

		for (list<Material *>::iterator it=materials.begin(); it!=materials.end(); it++) {
			(*it)->save(folder + (*it)->getName() + LIBGENS_MATERIAL_EXTENSION, root_type);
		}
	}
}