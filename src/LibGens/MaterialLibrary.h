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

namespace LibGens {
	class Material;

	class MaterialLibrary {
		friend class MaterialLibrary;

		protected:
			list<Material *> materials;
			string folder;
		public:
			MaterialLibrary(string folder_p);
			void addMaterial(Material *material);
			Material *getMaterial(string id);
			list<Material*> getMaterials();
			bool checkMaterial(string id);
			void merge(MaterialLibrary *library, bool overwrite=false);
			void save(string folder_target);
	};
};