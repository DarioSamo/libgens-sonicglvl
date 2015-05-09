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

#pragma once

namespace LibGens {
	class Object;
	class ObjectLibrary;

	class ObjectSet {
		protected:
			list<Object *> objects;
			string name;
			string filename;
			bool need_update;
		public:
			ObjectSet(string filename_p);
			ObjectSet();
			void setName(string nm);
			string getName();
			string getFilename();
			void addObject(Object *obj);

			// Doesn't delete the object, only removes it from the list
			void eraseObject(Object *obj);
			Object *getByID(size_t id, unsigned int *output_index=NULL);
			void getObjectsByName(string name, list<Object *> &total_list);
			void saveXML(string filename);
			list<Object *> getObjects();
			void learnFromLibrary(ObjectLibrary *library);
			size_t newObjectID();
	};
};