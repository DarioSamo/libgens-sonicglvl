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
	class Object;
	class ObjectCategory;
	class ObjectSet;
	class Level;

	class ObjectLibrary {
		protected:
			vector<ObjectCategory *> categories;
			string folder;
		public:
			ObjectLibrary(string folder_p);
			ObjectCategory *getCategory(string name);
			ObjectCategory *getCategoryByIndex(size_t index);
			vector<ObjectCategory *> getCategories();
			void loadCategory(string category_name, string folder_name);
			Object *createObject(string name);
			Object *getTemplate(string name);
			void learnFromSet(ObjectSet *set, ObjectCategory *default_category=NULL);
			void learnFromLevel(Level *level, ObjectCategory *default_category=NULL);
			void loadDatabase(string filename);
			void saveDatabase(string filename);
	};
};