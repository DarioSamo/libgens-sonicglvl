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

#ifndef LIBGENS_OBJECT_PRODUCTION_H_INCLUDED
#define LIBGENS_OBJECT_PRODUCTION_H_INCLUDED

#define LIBGENS_OBJECT_PRODUCTION_ERROR_FILE        "No valid object production file found: "
#define LIBGENS_OBJECT_PRODUCTION_ERROR_FILE_ROOT   "Object production file doesn't have a valid root."

#define LIBGENS_OBJECT_PRODUCTION_MODEL             "Model"
#define LIBGENS_OBJECT_PRODUCTION_MOTION            "Motion"
#define LIBGENS_OBJECT_PRODUCTION_MOTION_SKELETON   "MotionSkeleton"

namespace LibGens {
	class ObjectPhysics {
		protected:
			vector<string> models;
			vector<string> motions;
			vector<string> motion_skeletons;
			string name;
		public:	
			ObjectPhysics();
			void readXML(TiXmlElement *root);
			string getName();
			vector<string> getModels();
			vector<string> getMotions();
			vector<string> getMotionSkeletons();
	};

	class ObjectProduction {
		protected:
			list<ObjectPhysics *> object_entries;
			std::set<std::string> sorted_entry_names;
			std::set<std::string>::iterator sorted_entry_name_iterator;
		public:
			ObjectProduction();
			void load(string filename);
			ObjectPhysics *getObjectPhysics(string name);
			void readySortedEntries();
			bool getNextEntryName(std::string &output);
	};
};

#endif