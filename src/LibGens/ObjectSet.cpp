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

#include "ObjectSet.h"
#include "Object.h"
#include "Level.h"

namespace LibGens {
	ObjectSet::ObjectSet() {
		need_update=false;
	}

	ObjectSet::ObjectSet(string filename_p) {
		filename = filename_p;

		need_update = false;

		TiXmlDocument doc(filename);
		if (!doc.LoadFile()) {
			Error::addMessage(Error::FILE_NOT_FOUND, LIBGENS_FILE_H_ERROR_READ_FILE_BEFORE + filename + LIBGENS_FILE_H_ERROR_READ_FILE_AFTER);
			return;
		}

		name = filename;
		size_t sep = name.find(LIBGENS_OBJECT_SET_NAME);
		if (sep != std::string::npos) {
			name = name.substr(sep + 8, name.size() - sep - 8);
		}
	
		size_t dot = name.find(LIBGENS_OBJECT_SET_EXTENSION);
		if (dot != string::npos) {
			name = name.substr(0, dot);
		}

		TiXmlHandle hDoc(&doc);
		TiXmlElement* pElem;
		TiXmlHandle hRoot(0);

		pElem=hDoc.FirstChildElement().Element();
		if (!pElem) {
			Error::addMessage(Error::EXCEPTION, LIBGENS_OBJECT_H_ERROR_READ_SET_BEFORE + filename + LIBGENS_OBJECT_H_ERROR_READ_SET_AFTER);
			return;
		}
		
		// Go past SetObject
		pElem=pElem->FirstChildElement();
		for(pElem; pElem; pElem=pElem->NextSiblingElement()) {
			if (pElem->ValueStr() != LIBGENS_OBJECT_SET_LAYER_DEFINE) {
				Object *obj=new Object(pElem->ValueStr());
				obj->readXML(pElem);
				obj->setParentSet(this);
				objects.push_back(obj);
			}
		}
	}


	Object *ObjectSet::getByID(size_t id, unsigned int *output_index) {
		size_t index = 0;
		for (list<Object *>::iterator it=objects.begin(); it!=objects.end(); it++) {
			if ((*it)->getID() == id) {
				if (output_index) {
					*output_index = index;
				}
				return *it;
			}
			index++;
		}

		return NULL;
	}


	void ObjectSet::learnFromLibrary(ObjectLibrary *library) {
		if (!library) return;

		for (list<Object *>::iterator it=objects.begin(); it!=objects.end(); it++) {
			(*it)->learnFromLibrary(library);
		}
	}


	void ObjectSet::saveXML(string filename) {
		TiXmlDocument doc;
		TiXmlDeclaration *decl = new TiXmlDeclaration( "1.0", "", "" );
		doc.LinkEndChild( decl );

		TiXmlElement *root=new TiXmlElement(LIBGENS_OBJECT_SET_ROOT);
		
		for (list<Object *>::iterator it=objects.begin(); it!=objects.end(); it++) {
			(*it)->writeXML(root);
		}

		doc.LinkEndChild(root);

		if (!doc.SaveFile(filename)) {
			Error::addMessage(Error::FILE_NOT_FOUND, LIBGENS_OBJECT_H_ERROR_WRITE_SET_XML + filename);
		}
	}

	void ObjectSet::getObjectsByName(string name, list<Object *> &total_list) {
		for (list<Object *>::iterator it=objects.begin(); it!=objects.end(); it++) {
			if ((*it)->getName() == name) {
				total_list.push_back(*it);
			}
		}
	}


	size_t ObjectSet::newObjectID() {
		size_t id;

		for (id=LIBGENS_LEVEL_START_ID_GENERATION; id<LIBGENS_LEVEL_END_ID_GENERATION; id++) {
			Object *object=getByID(id);
			if (!object) break;
		}

		return id;
	}

	void ObjectSet::setName(string nm) {
		name=nm;
	}

	void ObjectSet::setFilename(string nm) {
		filename = nm;
	}

	string ObjectSet::getName() {
		return name;
	}

	string ObjectSet::getFilename() {
		return filename;
	}

	void ObjectSet::addObject(Object *obj) {
		if (obj) {
			need_update = true;
			objects.push_back(obj);
			obj->setParentSet(this);
		}
	}

	bool ObjectSet::hasObject(Object *obj) {
		return std::find(objects.begin(), objects.end(), obj) != objects.end();
	}

	void ObjectSet::eraseObject(Object *obj) {
		for (list<Object *>::iterator it=objects.begin(); it!=objects.end(); it++) {
			if ((*it) == obj) {
				objects.erase(it);
				need_update = true;
				return;
			}
		}
	}

	list<Object *> ObjectSet::getObjects() {
		return objects;
	}
};