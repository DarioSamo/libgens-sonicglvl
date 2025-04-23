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


#include "ObjectCategory.h"
#include "Object.h"

namespace LibGens {
	Object *ObjectCategory::getTemplate(string name) {
		for (vector<Object *>::iterator it=templates.begin(); it!=templates.end(); it++) {
			if ((*it)->getName() == name) return *it;
		}

		return NULL;
	}

	Object *ObjectCategory::getTemplateByIndex(size_t index) {
		if (index < templates.size()) {
			return templates[index];
		}

		return NULL;
	}

	bool ObjectCategory::learnFromObject(Object *object) {
		for (vector<Object *>::iterator it=templates.begin(); it!=templates.end(); it++) {
			if ((*it)->getName() == object->getName()) {
				(*it)->learnFromObject(object);
				return true;
			}
		}

		return false;
	}

	void ObjectCategory::saveXMLTemplates(string folder) {
		CreateDirectory(folder.c_str(), nullptr);

		for (vector<Object *>::iterator it=templates.begin(); it!=templates.end(); it++) {
			string filename=folder + (*it)->getName() + LIBGENS_OBJECT_TEMPLATE_EXTENSION;
			(*it)->saveXMLTemplate(filename);
		}
	}

	ObjectCategory::ObjectCategory(string nm) : name(nm) {
	}

	void ObjectCategory::addTemplate(Object *obj) {
		templates.push_back(obj);
	}

	vector<Object *> ObjectCategory::getTemplates() {
		return templates;
	}

	string ObjectCategory::getName() {
		return name;
	}

	string ObjectCategory::getFolder() {
		return folder;
	}

	void ObjectCategory::setFolder(string folder_p) {
		folder = folder_p;
	}
};