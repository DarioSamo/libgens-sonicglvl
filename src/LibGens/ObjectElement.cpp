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

#include "ObjectElement.h"
#include "Object.h"

namespace LibGens {

	// writeXML
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

	// writeXML() Lost World
	void ObjectElementSint8::writeXML(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);

		TiXmlText* eleValue=new TiXmlText(ToString((int) value));
		eleRoot->LinkEndChild(eleValue);

		root->LinkEndChild(eleRoot);
	}

	void ObjectElementUint8::writeXML(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);

		TiXmlText* eleValue=new TiXmlText(ToString((int) value));
		eleRoot->LinkEndChild(eleValue);

		root->LinkEndChild(eleRoot);
	}

	void ObjectElementSint16::writeXML(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);

		TiXmlText* eleValue=new TiXmlText(ToString(value));
		eleRoot->LinkEndChild(eleValue);

		root->LinkEndChild(eleRoot);
	}

	void ObjectElementUint16::writeXML(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);

		TiXmlText* eleValue=new TiXmlText(ToString(value));
		eleRoot->LinkEndChild(eleValue);

		root->LinkEndChild(eleRoot);
	}

	void ObjectElementSint32::writeXML(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);

		TiXmlText* eleValue=new TiXmlText(ToString(value));
		eleRoot->LinkEndChild(eleValue);

		root->LinkEndChild(eleRoot);
	}

	void ObjectElementUint32::writeXML(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);

		TiXmlText* eleValue=new TiXmlText(ToString(value));
		eleRoot->LinkEndChild(eleValue);

		root->LinkEndChild(eleRoot);
	}

	void ObjectElementEnum::writeXML(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);

		TiXmlText* eleValue=new TiXmlText(ToString((int) value));
		eleRoot->LinkEndChild(eleValue);

		root->LinkEndChild(eleRoot);
	}

	// writeXMLTemplate()
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
		const char *element = (getType() == OBJECT_ELEMENT_ID) ? LIBGENS_OBJECT_ELEMENT_ID_TEMPLATE : LIBGENS_OBJECT_ELEMENT_TARGET_TEMPLATE;
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_TYPE, element);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DESCRIPTION, description);
		root->LinkEndChild(eleRoot);
	}

	void ObjectElementIDList::writeXMLTemplate(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);
		const char *element = (getType() == OBJECT_ELEMENT_ID_LIST) ? LIBGENS_OBJECT_ELEMENT_ID_LIST_TEMPLATE : LIBGENS_OBJECT_ELEMENT_UINT32ARRAY_TEMPLATE;
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_TYPE, element);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DESCRIPTION, description);
		root->LinkEndChild(eleRoot);
	}

	void ObjectElementVector::writeXMLTemplate(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);

		const char *element;
		switch (getType()) {
		case OBJECT_ELEMENT_VECTOR:   element = LIBGENS_OBJECT_ELEMENT_VECTOR_TEMPLATE;     break;
		case OBJECT_ELEMENT_POSITION: element = LIBGENS_OBJECT_ELEMENT_POSITION_TEMPLATE;   break;
		case OBJECT_ELEMENT_VECTOR3:  element = LIBGENS_OBJECT_ELEMENT_VECTOR3_TEMPLATE;    break;
		}

		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_TYPE, element);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DESCRIPTION, description);
		root->LinkEndChild(eleRoot);
	}

	void ObjectElementVectorList::writeXMLTemplate(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_TYPE, LIBGENS_OBJECT_ELEMENT_VECTOR_LIST_TEMPLATE);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DESCRIPTION, description);
		root->LinkEndChild(eleRoot);
	}

	// writeXMLTemplate() Lost World
	void ObjectElementSint8::writeXMLTemplate(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_TYPE, LIBGENS_OBJECT_ELEMENT_SINT8_TEMPLATE);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DEFAULT, ToString((int) value));
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DESCRIPTION, description);
		root->LinkEndChild(eleRoot);
	}

	void ObjectElementUint8::writeXMLTemplate(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_TYPE, LIBGENS_OBJECT_ELEMENT_UINT8_TEMPLATE);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DEFAULT, ToString((int) value));
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DESCRIPTION, description);
		root->LinkEndChild(eleRoot);
	}

	void ObjectElementSint16::writeXMLTemplate(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_TYPE, LIBGENS_OBJECT_ELEMENT_SINT16_TEMPLATE);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DEFAULT, ToString(value));
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DESCRIPTION, description);
		root->LinkEndChild(eleRoot);
	}

	void ObjectElementUint16::writeXMLTemplate(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_TYPE, LIBGENS_OBJECT_ELEMENT_UINT16_TEMPLATE);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DEFAULT, ToString(value));
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DESCRIPTION, description);
		root->LinkEndChild(eleRoot);
	}

	void ObjectElementSint32::writeXMLTemplate(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_TYPE, LIBGENS_OBJECT_ELEMENT_SINT32_TEMPLATE);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DEFAULT, ToString(value));
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DESCRIPTION, description);
		root->LinkEndChild(eleRoot);
	}

	void ObjectElementUint32::writeXMLTemplate(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_TYPE, LIBGENS_OBJECT_ELEMENT_UINT32_TEMPLATE);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DEFAULT, ToString(value));
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DESCRIPTION, description);
		root->LinkEndChild(eleRoot);
	}

	void ObjectElementEnum::writeXMLTemplate(TiXmlElement *root) {
		TiXmlElement* eleRoot=new TiXmlElement(name);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_TYPE, LIBGENS_OBJECT_ELEMENT_ENUM_TEMPLATE);
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DEFAULT, ToString((int) value));
		eleRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DESCRIPTION, description);
		root->LinkEndChild(eleRoot);
	}
};