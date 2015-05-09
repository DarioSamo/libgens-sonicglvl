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
	
	enum ObjectConditionType {
		OBJECT_CONDITION_UNDEFINED,
		OBJECT_CONDITION_EQUAL,
		OBJECT_CONDITION_NOT_EQUAL,
		OBJECT_CONDITION_BIGGER,
		OBJECT_CONDITION_LESSER,
		OBJECT_CONDITION_BIGGER_EQUAL,
		OBJECT_CONDITION_LESSER_EQUAL
	};

	class ObjectExtra {
		protected:
			string type;
			string name;
			vector<string> parameter_names;
			vector<string> parameters;
		public:
			ObjectExtra(ObjectExtra *extra);
			ObjectExtra();
			void writeXMLTemplate(TiXmlElement *root);
			bool compare(ObjectExtra *extra);
			void setName(string nm);
			string getName();
			void setType(string nm);
			string getType();
			void addParameter(string p, string v);
			void setParameterNames(vector<string> v);
			vector<string> getParameterNames();
			void setParameters(vector<string> v);
			vector<string> getParameters();
			size_t getParameterSize();
			size_t getParameterNameSize();
			string getParameter(size_t index);
			string getParameterName(size_t index);
	};
};