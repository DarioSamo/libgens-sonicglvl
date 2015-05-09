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

#include "ObjectLibrary.h"
#include "ObjectCategory.h"
#include "ObjectSet.h"
#include "Object.h"
#include "Level.h"

namespace LibGens {
	ObjectLibrary::ObjectLibrary(string folder_p) {
		folder = folder_p;
	}

	ObjectCategory *ObjectLibrary::getCategoryByIndex(size_t index) {
		if (index < categories.size()) return categories[index];
		else return NULL;
	}

	vector<ObjectCategory *> ObjectLibrary::getCategories() {
		return categories;
	}

	Object *ObjectLibrary::getTemplate(string name) {
		for (vector<ObjectCategory *>::iterator it=categories.begin(); it!=categories.end(); it++) {
			Object *templ=(*it)->getTemplate(name);
			if (templ) return templ;
		}

		return NULL;
	}

	Object *ObjectLibrary::createObject(string name) {
		Object *templ=getTemplate(name);

		if (!templ) {
			Error::addMessage(Error::EXCEPTION, LIBGENS_OBJECT_H_ERROR_FIND_TEMPLATE + name);
			return NULL;
		}

		Object *obj=new Object(templ);
		return obj;
	}


	ObjectCategory *ObjectLibrary::getCategory(string name) {
		for (vector<ObjectCategory *>::iterator it=categories.begin(); it!=categories.end(); it++) {
			if ((*it)->getName() == name) {
				return *it;
			}
		}

		ObjectCategory *cat=new ObjectCategory(name);
		categories.push_back(cat);
		return cat;
	}


	void ObjectLibrary::learnFromSet(ObjectSet *set, ObjectCategory *default_category) {
		if (!categories.size()) {
			Error::addMessage(Error::EXCEPTION, LIBGENS_OBJECT_H_ERROR_NO_CATEGORIES);
			return;
		}

		list<Object *> objects=set->getObjects();

		for (list<Object *>::iterator it=objects.begin(); it!=objects.end(); it++) {
			bool result=false;

			for (vector<ObjectCategory *>::iterator it_c=categories.begin(); it_c!=categories.end(); it_c++) {
				if ((*it_c)->learnFromObject(*it)) {
					result=true;
					break;
				}
			}

			if (!result) {
				Object *new_template = new Object(*it);
				ObjectCategory *category=(default_category ? default_category : *(categories.begin()));
				if (category) category->addTemplate(new_template);
			}
		}
	}


	void ObjectLibrary::learnFromLevel(Level *level, ObjectCategory *default_category) {
		list<ObjectSet *> sets=level->getSets();
		for (list<ObjectSet *>::iterator it=sets.begin(); it!=sets.end(); it++) {
			learnFromSet(*it, default_category);
		}
	}


	void ObjectLibrary::loadDatabase(string filename) {
		TiXmlDocument doc(filename);
		if (!doc.LoadFile()) {
			Error::addMessage(Error::FILE_NOT_FOUND, LIBGENS_LIBRARY_ERROR_FILE + filename);
			return;
		}

		TiXmlHandle hDoc(&doc);
		TiXmlElement* pElem;
		TiXmlHandle hRoot(0);

		pElem=hDoc.FirstChildElement().Element();
		if (!pElem) {
			Error::addMessage(Error::EXCEPTION, LIBGENS_LIBRARY_ERROR_FILE_ROOT);
			return;
		}

		pElem=pElem->FirstChildElement();
		for(pElem; pElem; pElem=pElem->NextSiblingElement()) {
			string entry_name="";
			string category_name="";
			string folder_name="";

			entry_name = pElem->ValueStr();
			pElem->QueryValueAttribute(LIBGENS_LIBRARY_NAME_ATTRIBUTE, &category_name);
			pElem->QueryValueAttribute(LIBGENS_LIBRARY_FOLDER_ATTRIBUTE, &folder_name);

			if ((entry_name==LIBGENS_LIBRARY_ENTRY) && category_name.size() && folder_name.size()) {
				loadCategory(category_name, folder_name);
			}
		}
	}


	void ObjectLibrary::loadCategory(string category_name, string folder_name) {
		LibGens::ObjectCategory *category=getCategory(category_name);
		category->setFolder(folder_name);
		string search_string=folder + folder_name + "/*.xml";

		WIN32_FIND_DATA FindFileData;
		HANDLE hFind;
		hFind = FindFirstFile(search_string.c_str(), &FindFileData);
		if (hFind == INVALID_HANDLE_VALUE) {} 
		else {
			do {
				const char *name=FindFileData.cFileName;
				if (name[0]=='.') continue;

				string object_name=ToString(name);
				string object_internal_name=object_name;
				object_internal_name.resize(object_internal_name.size()-((string)LIBGENS_OBJECT_TEMPLATE_EXTENSION).size());
				LibGens::Object *templ=new LibGens::Object(object_internal_name);
				templ->readXMLTemplate(folder + folder_name + "/" + object_name);
				category->addTemplate(templ);
			} while (FindNextFile(hFind, &FindFileData) != 0);
			FindClose(hFind);
		}
	}


	void ObjectLibrary::saveDatabase(string filename) {
		TiXmlDocument doc;
		TiXmlDeclaration *decl = new TiXmlDeclaration( "1.0", "", "" );
		doc.LinkEndChild( decl );

		TiXmlElement *libraryRoot = new TiXmlElement(LIBGENS_LIBRARY_ROOT);

		for (vector<ObjectCategory *>::iterator it=categories.begin(); it!=categories.end(); it++) {
			TiXmlElement *categoryRoot = new TiXmlElement(LIBGENS_LIBRARY_ENTRY);
			categoryRoot->SetAttribute(LIBGENS_LIBRARY_NAME_ATTRIBUTE, (*it)->getName());
			categoryRoot->SetAttribute(LIBGENS_LIBRARY_FOLDER_ATTRIBUTE, (*it)->getFolder());
			libraryRoot->LinkEndChild(categoryRoot);

			(*it)->saveXMLTemplates(folder + (*it)->getFolder() + "/");
		}
		
		doc.LinkEndChild(libraryRoot);
		doc.SaveFile(filename);
	}
};