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

#include "ObjectElement.h"
#include "Object.h"

namespace LibGens {
	void ObjectElementBool::writeXML(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);

		TiXmlText* eleValue=new TiXmlText((value ? LIBGENS_OBJECT_ELEMENT_BOOL_TRUE : LIBGENS_OBJECT_ELEMENT_BOOL_FALSE));
		eleRoot->LinkEndChild(eleValue);

		root->LinkEndChild(eleRoot);
	}

	void ObjectElementInteger::writeXML(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);

		TiXmlText* eleValue=new TiXmlText(ToString(value));
		eleRoot->LinkEndChild(eleValue);

		root->LinkEndChild(eleRoot);
	}

	void ObjectElementFloat::writeXML(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);

		TiXmlText* eleValue=new TiXmlText(ToString(value));
		eleRoot->LinkEndChild(eleValue);

		root->LinkEndChild(eleRoot);
	}

	void ObjectElementString::writeXML(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);

		TiXmlText* eleValue=new TiXmlText(value);
		eleRoot->LinkEndChild(eleValue);

		root->LinkEndChild(eleRoot);
	}

	void ObjectElementID::writeXML(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);
		TiXmlElement* eleId=new TiXmlElement(LIBGENS_OBJECT_ELEMENT_SET_ID);

		TiXmlText* eleValue=new TiXmlText(ToString(value));
		eleId->LinkEndChild(eleValue);

		eleRoot->LinkEndChild(eleId);
		root->LinkEndChild(eleRoot);
	}

	void ObjectElementIDList::writeXML(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);

		for (size_t i=0; i<value.size(); i++) {
			TiXmlElement* eleId=new TiXmlElement(LIBGENS_OBJECT_ELEMENT_SET_ID);
			TiXmlText* eleValue=new TiXmlText(ToString(value[i]));
			eleId->LinkEndChild(eleValue);
			eleRoot->LinkEndChild(eleId);
		}


		root->LinkEndChild(eleRoot);
	}

	void ObjectElementVector::writeXML(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);
		value.writeXML(eleRoot);
		root->LinkEndChild(eleRoot);
	}

	void ObjectElementVectorList::writeXML(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);

		for (size_t i=0; i<value.size(); i++) {
			TiXmlElement* elePos=new TiXmlElement(LIBGENS_OBJECT_ELEMENT_POSITION + ToString(i));
			value[i].writeXML(elePos);
			eleRoot->LinkEndChild(elePos);
		}

		root->LinkEndChild(eleRoot);
	}

	void ObjectElement::writeXMLTemplate(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_TYPE, LIBGENS_OBJECT_ELEMENT_UNDEFINED_TEMPLATE);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DESCRIPTION, description);
		root->LinkEndChild(eleRoot);
	}

	void ObjectElementBool::writeXMLTemplate(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_TYPE, LIBGENS_OBJECT_ELEMENT_BOOL_TEMPLATE);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DEFAULT, (value ? LIBGENS_OBJECT_ELEMENT_BOOL_TRUE : LIBGENS_OBJECT_ELEMENT_BOOL_FALSE));
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DESCRIPTION, description);
		root->LinkEndChild(eleRoot);
	}

	void ObjectElementInteger::writeXMLTemplate(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_TYPE, LIBGENS_OBJECT_ELEMENT_INTEGER_TEMPLATE);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DEFAULT, ToString(value));
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DESCRIPTION, description);
		root->LinkEndChild(eleRoot);
	}

	void ObjectElementFloat::writeXMLTemplate(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_TYPE, LIBGENS_OBJECT_ELEMENT_FLOAT_TEMPLATE);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DEFAULT, ToString(value));
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DESCRIPTION, description);
		root->LinkEndChild(eleRoot);
	}

	void ObjectElementString::writeXMLTemplate(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_TYPE, LIBGENS_OBJECT_ELEMENT_STRING_TEMPLATE);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DEFAULT, value);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DESCRIPTION, description);
		root->LinkEndChild(eleRoot);
	}

	void ObjectElementID::writeXMLTemplate(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_TYPE, LIBGENS_OBJECT_ELEMENT_ID_TEMPLATE);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DESCRIPTION, description);
		root->LinkEndChild(eleRoot);
	}

	void ObjectElementIDList::writeXMLTemplate(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_TYPE, LIBGENS_OBJECT_ELEMENT_ID_LIST_TEMPLATE);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DESCRIPTION, description);
		root->LinkEndChild(eleRoot);
	}

	void ObjectElementVector::writeXMLTemplate(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_TYPE, LIBGENS_OBJECT_ELEMENT_VECTOR_TEMPLATE);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DESCRIPTION, description);
		root->LinkEndChild(eleRoot);
	}

	void ObjectElementVectorList::writeXMLTemplate(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_TYPE, LIBGENS_OBJECT_ELEMENT_VECTOR_LIST_TEMPLATE);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DESCRIPTION, description);
		root->LinkEndChild(eleRoot);
	}
};