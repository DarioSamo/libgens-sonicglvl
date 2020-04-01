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

#include "EditorConfiguration.h"

EditorConfiguration::EditorConfiguration() {
	object_production_path = "";
	game_path = "";
}

void EditorConfiguration::load(string filename) {
	TiXmlDocument doc(filename);
	if (!doc.LoadFile()) {
		LibGens::Error::addMessage(LibGens::Error::FILE_NOT_FOUND, SONICGLVL_CONFIGURATION_ERROR_FILE + filename);
		return;
	}

	TiXmlHandle hDoc(&doc);
	TiXmlElement* pElem;
	TiXmlHandle hRoot(0);

	pElem=hDoc.FirstChildElement().Element();
	if (!pElem) {
		LibGens::Error::addMessage(LibGens::Error::EXCEPTION, SONICGLVL_CONFIGURATION_ERROR_FILE_ROOT);
		return;
	}

	pElem=pElem->FirstChildElement();
	for(pElem; pElem; pElem=pElem->NextSiblingElement()) {
		string entry_name="";
		string filename="";
		string value="";

		entry_name = pElem->ValueStr();
		pElem->QueryValueAttribute(SONICGLVL_CONFIGURATION_FILENAME_ATTRIBUTE, &filename);
		pElem->QueryValueAttribute(SONICGLVL_CONFIGURATION_VALUE_ATTRIBUTE, &value);

		if (entry_name==SONICGLVL_CONFIGURATION_OBJECT_PRODUCTION_PATH) {
			object_production_path = filename;
		}

		if (entry_name==SONICGLVL_CONFIGURATION_TERRAIN_VIEW_DISTANCE) {
			FromString<float>(terrain_view_distance, value, std::dec);
		}

		if (entry_name==SONICGLVL_CONFIGURATION_TERRAIN_CAMERA_OFFSET) {
			FromString<float>(terrain_camera_offset, value, std::dec);
		}

		if (entry_name==SONICGLVL_CONFIGURATION_GI_LEVEL_1_QUALITY_OFFSET) {
			FromString<float>(gi_level_1_quality_offset, value, std::dec);
		}

		if (entry_name==SONICGLVL_CONFIGURATION_GI_LEVEL_2_QUALITY_OFFSET) {
			FromString<float>(gi_level_2_quality_offset, value, std::dec);
		}

		if (entry_name==SONICGLVL_CONFIGURATION_TERRAIN_UPDATE_INTERVAL_S) {
			FromString<float>(terrain_update_interval, value, std::dec);
		}

		if (entry_name==SONICGLVL_CONFIGURATION_VISIBILITY_FLAGS) {
			FromString<unsigned int>(visibility_flags, value, std::dec);
		}

		if (entry_name == SONICGLVL_CONFIGURATION_GAME_PATH) {
			game_path = filename;
		}
	}
}

void EditorConfiguration::save(string filename) {

#define CREATE_ELEMENT_FILENAME_ATTRIBUTE(parent, elemName, value) \
	((TiXmlElement*)parent->InsertEndChild(TiXmlElement(elemName)))->SetAttribute(SONICGLVL_CONFIGURATION_FILENAME_ATTRIBUTE, value)

#define CREATE_ELEMENT_VALUE_ATTRIBUTE(parent, elemName, value) \
	((TiXmlElement*)parent->InsertEndChild(TiXmlElement(elemName)))->SetAttribute(SONICGLVL_CONFIGURATION_VALUE_ATTRIBUTE, value)

	TiXmlDocument doc(filename);
	TiXmlNode* root = doc.InsertEndChild(TiXmlElement("Configuration"));
	CREATE_ELEMENT_FILENAME_ATTRIBUTE(root, SONICGLVL_CONFIGURATION_OBJECT_PRODUCTION_PATH, object_production_path);
	CREATE_ELEMENT_FILENAME_ATTRIBUTE(root, SONICGLVL_CONFIGURATION_GAME_PATH, game_path);
	CREATE_ELEMENT_VALUE_ATTRIBUTE(root, SONICGLVL_CONFIGURATION_TERRAIN_VIEW_DISTANCE, terrain_view_distance);
	CREATE_ELEMENT_VALUE_ATTRIBUTE(root, SONICGLVL_CONFIGURATION_TERRAIN_CAMERA_OFFSET, terrain_camera_offset);
	CREATE_ELEMENT_VALUE_ATTRIBUTE(root, SONICGLVL_CONFIGURATION_GI_LEVEL_1_QUALITY_OFFSET, gi_level_1_quality_offset);
	CREATE_ELEMENT_VALUE_ATTRIBUTE(root, SONICGLVL_CONFIGURATION_GI_LEVEL_2_QUALITY_OFFSET, gi_level_2_quality_offset);
	CREATE_ELEMENT_VALUE_ATTRIBUTE(root, SONICGLVL_CONFIGURATION_TERRAIN_UPDATE_INTERVAL_S, terrain_update_interval);
	CREATE_ELEMENT_VALUE_ATTRIBUTE(root, SONICGLVL_CONFIGURATION_VISIBILITY_FLAGS, visibility_flags);

	doc.SaveFile(filename);
}