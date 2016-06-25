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
	class Tag {
	protected:
		string key;
		vector<string> values;
	public:
		static const string KeyMaterialLayer;
		static const string KeyMaterialTexture;
		static const string KeyMaterialShader;
		static const string KeyMaterialParameter;
		static const string KeyMaterialCulling;
		static const string KeyMaterialAdd;
		static const string KeyTerrainGroup;

		Tag();
		Tag(string str);
		string getKey();
		string getValue(int value_index, string default_value = "");
		int getValueInt(int value_index, int default_value = 0);
		float getValueFloat(int value_index, float default_value = 0.0f);
		bool getValueBool(int value_index, bool default_value = false);
		int getValueCount();
		bool empty();
	};

	class Tags {
	protected:
		string name;
		vector<Tag> tags;
	public:
		Tags(string str);
		string getName();
		Tag getTag(int i);
		int getTagCount();
		string getTagValue(string key, int value_index = 0, string default_value = "");
		int getTagValueInt(string key, int value_index = 0, int default_value = 0);
		float getTagValueFloat(string key, int value_index = 0, float default_value = 0.0f);
		bool getTagValueBool(string key, int value_index = 0, bool default_value = false);
		vector<Tag> getTagsByKey(string key);
		bool empty();
	};
};