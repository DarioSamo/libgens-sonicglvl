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

#include "ObjectProduction.h"

namespace LibGens {
	ObjectPhysics::ObjectPhysics() {
		name = "";
	}

	void ObjectPhysics::readXML(TiXmlElement *root) {
		name = root->ValueStr();

		TiXmlElement *pElem=root->FirstChildElement();
		for(pElem; pElem; pElem=pElem->NextSiblingElement()) {
			string entry_name=pElem->ValueStr();
			char *text_ptr=(char *) pElem->GetText();
			string text="";

			if (text_ptr) {
				text = ToString(text_ptr);
			}

			if (entry_name == LIBGENS_OBJECT_PRODUCTION_MODEL) {
				models.push_back(text);
			}

			if (entry_name == LIBGENS_OBJECT_PRODUCTION_MOTION) {
				motions.push_back(text);
			}

			if (entry_name == LIBGENS_OBJECT_PRODUCTION_MOTION_SKELETON) {
				motion_skeletons.push_back(text);
			}
		}
	}

	string ObjectPhysics::getName() {
		return name;
	}

	vector<string> ObjectPhysics::getModels() {
		return models;
	}

	vector<string> ObjectPhysics::getMotions() {
		return motions;
	}

	vector<string> ObjectPhysics::getMotionSkeletons() {
		return motion_skeletons;
	}

	ObjectProduction::ObjectProduction() {
	}

	void ObjectProduction::load(string filename) {
		TiXmlDocument doc(filename);
		if (!doc.LoadFile()) {
			Error::addMessage(Error::FILE_NOT_FOUND, LIBGENS_OBJECT_PRODUCTION_ERROR_FILE + filename);
			return;
		}

		TiXmlHandle hDoc(&doc);
		TiXmlElement* pElem;
		TiXmlHandle hRoot(0);

		pElem=hDoc.FirstChildElement().Element();
		if (!pElem) {
			Error::addMessage(Error::EXCEPTION, LIBGENS_OBJECT_PRODUCTION_ERROR_FILE_ROOT);
			return;
		}

		sorted_entry_names.clear();

		pElem=pElem->FirstChildElement();
		for(pElem; pElem; pElem=pElem->NextSiblingElement()) {
			ObjectPhysics *object=new ObjectPhysics();
			object->readXML(pElem);
			object_entries.push_back(object);

			sorted_entry_names.insert(object->getName());
		}
	}

	ObjectPhysics *ObjectProduction::getObjectPhysics(string name) {
		for (list<ObjectPhysics *>::iterator it=object_entries.begin(); it!=object_entries.end(); it++) {
			if ((*it)->getName() == name) {
				return (*it);
			}
		}

		return NULL;
	}

	void ObjectProduction::readySortedEntries() {
		sorted_entry_name_iterator = sorted_entry_names.begin();
	}

	bool ObjectProduction::getNextEntryName(string &output) {
		if (sorted_entry_name_iterator == sorted_entry_names.end()) {
			return false;
		}

		output = (*sorted_entry_name_iterator);
		sorted_entry_name_iterator++;
		return true;
	}
};