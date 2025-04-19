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

#include "Havok.h"
#include "HavokPropertyDatabase.h"

namespace LibGens {
	unsigned int HavokPropertyDatabaseEntry::getKey() {
		return key;
	}

	unsigned int HavokPropertyDatabaseEntry::getValue() {
		return value;
	}

	string HavokPropertyDatabaseGroup::getName() {
		return name;
	}

	list<HavokPropertyDatabaseEntry *> HavokPropertyDatabaseGroup::getEntries() {
		return entries;
	}

	
	HavokPropertyDatabaseEntry::HavokPropertyDatabaseEntry(TiXmlElement *parent) {
		TiXmlElement *pElem=parent->FirstChildElement();

		for(pElem; pElem; pElem=pElem->NextSiblingElement()) {
			if (pElem->ValueStr() == LIBGENS_HAVOK_DATABASE_HKPARAM) {
				string param_name = "";
				char *text_ptr=(char *) pElem->GetText();

				pElem->QueryValueAttribute(LIBGENS_HAVOK_DATABASE_NAME, &param_name);

				if (param_name == LIBGENS_HAVOK_DATABASE_KEY) {
					if (text_ptr) {
						FromString<unsigned int>(key, ToString(text_ptr), std::dec);
					}
				}

				if (param_name == LIBGENS_HAVOK_DATABASE_VALUE) {
					TiXmlElement *pElem_i = pElem->FirstChildElement();
					if (pElem_i) {
						// hkobject
						pElem_i = pElem_i->FirstChildElement();
						if (pElem_i) {
							//hkparam
							pElem_i->QueryValueAttribute(LIBGENS_HAVOK_DATABASE_NAME, &param_name);
							if (param_name == LIBGENS_HAVOK_DATABASE_DATA) {
								text_ptr=(char *) pElem_i->GetText();

								if (text_ptr) {
									FromString<unsigned int>(value, ToString(text_ptr), std::dec);
								}
							}
						}
					}
				}
			}
		}
	}

	
	HavokPropertyDatabaseGroup::HavokPropertyDatabaseGroup(TiXmlElement *parent) {
		parent->QueryValueAttribute(LIBGENS_HAVOK_DATABASE_NAME, &name);

		TiXmlElement *pElem_i=parent->FirstChildElement();
		for(pElem_i; pElem_i; pElem_i=pElem_i->NextSiblingElement()) {
			if (pElem_i->ValueStr() == LIBGENS_HAVOK_DATABASE_HKOBJECT) {
				HavokPropertyDatabaseEntry *entry = new HavokPropertyDatabaseEntry(pElem_i);
				if (entry) {
					entries.push_back(entry);
				}
			}
		}
	}


	HavokPropertyDatabase::HavokPropertyDatabase(string filename) {
		TiXmlDocument doc(filename);

		if (!doc.LoadFile()) {
			LibGens::Error::addMessage(Error::FILE_NOT_FOUND, "Could not load Havok Property Database: " + filename);
			return;
		}

		TiXmlHandle hDoc(&doc);
		TiXmlElement* pElem;
		TiXmlHandle hRoot(0);

		pElem=hDoc.FirstChildElement().Element();
		if (!pElem) {
			Error::addMessage(Error::EXCEPTION, "Havok Property Database file doesn't have a valid root.");
			return;
		}

		pElem=pElem->FirstChildElement();
		for(pElem; pElem; pElem=pElem->NextSiblingElement()) {
			if ((pElem->ValueStr() == LIBGENS_HAVOK_DATABASE_ENTRY)) {
				HavokPropertyDatabaseGroup *group = new HavokPropertyDatabaseGroup(pElem);
				groups.push_back(group);
			}
		}
	}

#ifndef HAVOK_5_5_0
	void HavokPropertyDatabaseGroup::applyProperties(hkpRigidBody *rigid_body) {
		for (list<HavokPropertyDatabaseEntry *>::iterator it=entries.begin(); it!=entries.end(); it++) {
			rigid_body->setProperty((*it)->getKey(), hkpPropertyValue((int)(*it)->getValue()));
		}
	}
#endif

	void HavokPropertyDatabase::applyProperties(hkpRigidBody *rigid_body) {
		if (!rigid_body) {
			return;
		}

		string name = ToString(rigid_body->getName());
		for (list<HavokPropertyDatabaseGroup *>::iterator it=groups.begin(); it!=groups.end(); it++) {
			if (name.find("@" + (*it)->getName()) != string::npos) {
				(*it)->applyProperties(rigid_body);
				return;
			}
		}
	}
};