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

#include "Tags.h"

namespace LibGens {
	/** Tag */

	const string Tag::KeyMaterialLayer = "LYR";
	const string Tag::KeyMaterialTexture = "TXTR";
	const string Tag::KeyMaterialShader = "SHDR";
	const string Tag::KeyMaterialParameter = "PMTR";
	const string Tag::KeyMaterialCulling = "CULL";
	const string Tag::KeyMaterialAdd = "ADD";
	const string Tag::KeyTerrainGroup = "TGRP";

	Tag::Tag() {
		key = "";
	}

	Tag::Tag(string str) {
		bool reading_values = false;
		size_t str_size = str.size();
		string value = "";
		key = "";

		for (size_t c = 0; c < str_size; c++) {
			if (str[c] == '(') {
				if (!reading_values) {
					key = value;
					reading_values = true;
					value = "";
				}
			}
			else if (str[c] == ')') {
				if (reading_values && value.size()) {
					values.push_back(value);
					value = "";
				}
				break;
			}
			else if (str[c] == ',') {
				if (reading_values && value.size()) {
					values.push_back(value);
					value = "";
				}
			}
			else if (str[c] == ' ') {
				// Only add space characters if the value already has characters
				if (value.size())
					value += str[c];
			}
			else {
				// Add all other characters otherwise
				value += str[c];
			}
		}

		// If this tag has no parameters, we assign the key as the entire string.
		if (!reading_values) {
			key = str;
		}
	}

	string Tag::getKey() {
		return key;
	}

	string Tag::getValue(int value_index, string default_value) {
		if ((value_index >= 0) && (value_index < getValueCount())) {
			return values[value_index];
		}
		return default_value;
	}

	int Tag::getValueInt(int value_index, int default_value) {
		string value = getValue(value_index);
		if (value.size()) {
			int value_i = 0;
			FromString<int>(value_i, value, std::dec);
			return value_i;
		}

		return default_value;
	}

	float Tag::getValueFloat(int value_index, float default_value) {
		string value = getValue(value_index);
		if (value.size()) {
			float value_f = 0.0f;
			FromString<float>(value_f, value, std::dec);
			return value_f;
		}

		return default_value;
	}

	bool Tag::getValueBool(int value_index, bool default_value) {
		string value = getValue(value_index);
		if (value.size()) {
			if ((value[0] == '1') || (value[0] == 't') || (value[0] == 'T') || (value[0] == 'y') || (value[0] == 'Y')) {
				return true;
			}
		}

		return default_value;
	}

	int Tag::getValueCount() {
		return values.size();
	}

	bool Tag::empty() {
		return (key.size() == 0);
	}

	/** Tags */

	Tags::Tags(string str) {
		bool read_tag_mode = false;
		size_t str_size = str.size();
		string tag = "";
		for (size_t c = 0; c < str_size; c++) {
			if (str[c] == '@') {
				if (read_tag_mode) {
					tags.push_back(Tag(tag));
				}
				else {
					name = tag;
					read_tag_mode = true;
				}

				tag = "";
			}
			else {
				tag += str[c];
			}
		}

		if (tag.size()) {
			if (read_tag_mode) {
				tags.push_back(Tag(tag));
			}
			else {
				name = tag;
			}
		}
	}

	string Tags::getName() {
		return name;
	}

	Tag Tags::getTag(int i) {
		if ((i >= 0) && (i < getTagCount())) {
			return tags[i];
		}
		else {
			return Tag();
		}
	}

	int Tags::getTagCount() {
		return tags.size();
	}

	bool Tags::empty() {
		return (name.size() == 0);
	}

	string Tags::getTagValue(string key, int value_index, string default_value) {
		for (size_t i=0; i < tags.size(); i++) {
			if (tags[i].getKey() == key) {
				if ((value_index >= 0) && (value_index < tags[i].getValueCount())) {
					return tags[i].getValue(value_index);
				}

				break;
			}
		}

		return default_value;
	}

	int Tags::getTagValueInt(string key, int value_index, int default_value) {
		string value = getTagValue(key, value_index);
		if (value.size()) {
			int value_i = 0;
			FromString<int>(value_i, value, std::dec);
			return value_i;
		}

		return default_value;
	}

	float Tags::getTagValueFloat(string key, int value_index, float default_value) {
		string value = getTagValue(key, value_index);
		if (value.size()) {
			float value_f = 0.0f;
			FromString<float>(value_f, value, std::dec);
			return value_f;
		}

		return default_value;
	}

	bool Tags::getTagValueBool(string key, int value_index, bool default_value) {
		string value = getTagValue(key, value_index);
		if (value.size())
			return (value[0] == '1') || (value[0] == 't') || (value[0] == 'T') || (value[0] == 'y') || (value[0] == 'Y');
		else
			return default_value;
	}

	vector<Tag> Tags::getTagsByKey(string key) {
		vector<Tag> match;
		for (size_t i = 0; i < tags.size(); i++) {
			if (tags[i].getKey() == key) {
				match.push_back(tags[i]);
			}
		}
		return match;
	}
};