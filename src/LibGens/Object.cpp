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

#include "Object.h"
#include "ObjectExtra.h"
#include "ObjectElement.h"
#include "ObjectLibrary.h"
#include "Level.h"
#include "StringTable.h"

namespace LibGens {
	void MultiSetNode::readXML(TiXmlElement *root) {
		for (TiXmlElement *pElem=root->FirstChildElement(); pElem; pElem=pElem->NextSiblingElement()) {
			string element_name=pElem->ValueStr();

			if (element_name == LIBGENS_OBJECT_ELEMENT_POSITION) {
				position.readXML(pElem);
			}
			else if (element_name == LIBGENS_OBJECT_ELEMENT_ROTATION) {
				rotation.readXML(pElem);
			}
		}
	}
	
	void MultiSetNode::writeXML(TiXmlElement *root, size_t index) {
		TiXmlElement* eleRoot=new TiXmlElement(LIBGENS_OBJECT_ELEMENT_MULTI_SET_PARAM_ELEMENT);

		TiXmlElement* indRoot=new TiXmlElement(LIBGENS_OBJECT_ELEMENT_MULTI_SET_PARAM_INDEX);
		TiXmlText* indValue=new TiXmlText(ToString(index));
		indRoot->LinkEndChild(indValue);
		eleRoot->LinkEndChild(indRoot);

		TiXmlElement* posRoot=new TiXmlElement(LIBGENS_OBJECT_ELEMENT_POSITION);
		position.writeXML(posRoot);
		eleRoot->LinkEndChild(posRoot);

		TiXmlElement* rotRoot=new TiXmlElement(LIBGENS_OBJECT_ELEMENT_ROTATION);
		rotation.writeXML(rotRoot);
		eleRoot->LinkEndChild(rotRoot);

		root->LinkEndChild(eleRoot);
	}

	void MultiSetNode::readORC(File *file) {
		position.read(file);
		position = position;

		Vector3 rot;
		rot.read(file);
		rotation.fromLostWorldEuler(rot);

		local_position.read(file);
		local_position = local_position;

		rot.read(file);
		local_rotation.fromLostWorldEuler(rot);
	}

	void MultiSetNode::recalcTransform(Object *parent) {
		// Recalculate position
		position = local_position;
		position = parent->getRotation() * position;
		position = position + parent->getPosition();
				
		// Recalculate rotation
		rotation = parent->getRotation() * local_rotation;
	}

	void MultiSetParam::readXML(TiXmlElement *root) {
		for (TiXmlElement *pElem=root->FirstChildElement(); pElem; pElem=pElem->NextSiblingElement()) {
			string element_name=pElem->ValueStr();
			char *text_ptr=(char *) pElem->GetText();

			if (text_ptr) {
				string text=ToString(text_ptr);

				if (element_name == LIBGENS_OBJECT_ELEMENT_MULTI_SET_PARAM_BASE_LINE) {
					FromString<float>(base_line, text, std::dec);
				}
				else if (element_name == LIBGENS_OBJECT_ELEMENT_MULTI_SET_PARAM_DIRECTION) {
					FromString<float>(direction, text, std::dec);
				}
				else if (element_name == LIBGENS_OBJECT_ELEMENT_MULTI_SET_PARAM_INTERVAL) {
					FromString<float>(interval, text, std::dec);
				}
				else if (element_name == LIBGENS_OBJECT_ELEMENT_MULTI_SET_PARAM_INTERVAL_BASE) {
					FromString<float>(interval_base, text, std::dec);
				}
				else if (element_name == LIBGENS_OBJECT_ELEMENT_MULTI_SET_PARAM_POSITION_BASE) {
					FromString<float>(position_base, text, std::dec);
				}
				else if (element_name == LIBGENS_OBJECT_ELEMENT_MULTI_SET_PARAM_ROTATION_BASE) {
					FromString<float>(rotation_base, text, std::dec);
				}
			}
			else {
				if (element_name == LIBGENS_OBJECT_ELEMENT_MULTI_SET_PARAM_ELEMENT) {
					MultiSetNode *node=new MultiSetNode();
					node->readXML(pElem);
					nodes.push_back(node);
				}
			}
		}
	}

	void MultiSetParam::writeXML(TiXmlElement *root) {
		if (!getSize()) return;

		TiXmlElement* mspRoot=new TiXmlElement(LIBGENS_OBJECT_ELEMENT_MULTI_SET_PARAM);

		size_t index=1;
		for (list<MultiSetNode *>::iterator it=nodes.begin(); it!=nodes.end(); it++) {
			(*it)->writeXML(mspRoot, index);
			index++;
		}

		TiXmlElement* pParameter=NULL;
		TiXmlText* pValue=NULL;

		pParameter=new TiXmlElement(LIBGENS_OBJECT_ELEMENT_MULTI_SET_PARAM_BASE_LINE);
		pValue=new TiXmlText(ToString(base_line));
		pParameter->LinkEndChild(pValue);
		mspRoot->LinkEndChild(pParameter);

		pParameter=new TiXmlElement(LIBGENS_OBJECT_ELEMENT_MULTI_SET_PARAM_DIRECTION);
		pValue=new TiXmlText(ToString(direction));
		pParameter->LinkEndChild(pValue);
		mspRoot->LinkEndChild(pParameter);

		pParameter=new TiXmlElement(LIBGENS_OBJECT_ELEMENT_MULTI_SET_PARAM_INTERVAL);
		pValue=new TiXmlText(ToString(interval));
		pParameter->LinkEndChild(pValue);
		mspRoot->LinkEndChild(pParameter);

		pParameter=new TiXmlElement(LIBGENS_OBJECT_ELEMENT_MULTI_SET_PARAM_INTERVAL_BASE);
		pValue=new TiXmlText(ToString(interval_base));
		pParameter->LinkEndChild(pValue);
		mspRoot->LinkEndChild(pParameter);

		pParameter=new TiXmlElement(LIBGENS_OBJECT_ELEMENT_MULTI_SET_PARAM_POSITION_BASE);
		pValue=new TiXmlText(ToString(position_base));
		pParameter->LinkEndChild(pValue);
		mspRoot->LinkEndChild(pParameter);

		pParameter=new TiXmlElement(LIBGENS_OBJECT_ELEMENT_MULTI_SET_PARAM_ROTATION_BASE);
		pValue=new TiXmlText(ToString(rotation_base));
		pParameter->LinkEndChild(pValue);
		mspRoot->LinkEndChild(pParameter);

		root->LinkEndChild(mspRoot);
	}

	void MultiSetParam::readORC(File *file, unsigned int count) {
		for (int i = 0; i < count; i++) {
			LibGens::MultiSetNode *node = new LibGens::MultiSetNode();
			node->readORC(file);
			nodes.push_back(node);
		}
	}
	
	void MultiSetParam::recalcTransform(Object *parent) {
		for (list<MultiSetNode*>::iterator it = nodes.begin(); it != nodes.end(); it++)
			(*it)->recalcTransform(parent);
	}

	Object::Object(string nm) : name(nm), multi_set_param() {
		template_reference=NULL;
		position=Vector3();
		rotation=Quaternion();
		id=0;
		parent_set = NULL;
		needs_transform_recalc = false;
	}

	Object::Object(Object *obj) {
		template_reference = obj;
		parent_set = NULL;

		name=obj->name;
		multi_set_param.clear();
		
		list<ObjectElement *> template_elements=obj->getElements();
		for (list<ObjectElement *>::iterator it=template_elements.begin(); it!=template_elements.end(); it++) {
			ObjectElement *element=cloneElement(*it);
			if (element) elements.push_back(element);
		}

		list<ObjectExtra *> template_extras=obj->getExtras();
		for (list<ObjectExtra *>::iterator it=template_extras.begin(); it!=template_extras.end(); it++) {
			ObjectExtra *extra=new ObjectExtra(*it);
			extras.push_back(extra);
		}

		position = obj->getPosition();
		rotation = obj->getRotation();
		id = obj->getID();
		needs_transform_recalc = false;
		// FIXME: Clone MultiSetParam
	}

	ObjectElement *Object::cloneElement(ObjectElement *element) {
		switch (element->getType()) {
			case OBJECT_ELEMENT_UNDEFINED :
				{
					ObjectElement *element_sub = new ObjectElement();
					element_sub->setName(element->getName());
					element_sub->setDescription(element->getDescription());
					return element_sub;
				}

			case OBJECT_ELEMENT_BOOL :
				{
					ObjectElementBool *element_sub = new ObjectElementBool();
					ObjectElementBool *element_src = (ObjectElementBool *) element;
					element_sub->setName(element->getName());
					element_sub->setDescription(element->getDescription());
					element_sub->value = element_src->value;
					return element_sub;
				}

			case OBJECT_ELEMENT_INTEGER :
				{
					ObjectElementInteger *element_sub = new ObjectElementInteger();
					ObjectElementInteger *element_src = (ObjectElementInteger *) element;
					element_sub->setName(element->getName());
					element_sub->setDescription(element->getDescription());
					element_sub->value = element_src->value;
					return element_sub;
				}

			case OBJECT_ELEMENT_FLOAT :
				{
					ObjectElementFloat *element_sub = new ObjectElementFloat();
					ObjectElementFloat *element_src = (ObjectElementFloat *) element;
					element_sub->setName(element->getName());
					element_sub->setDescription(element->getDescription());
					element_sub->value = element_src->value;
					return element_sub;
				}

			case OBJECT_ELEMENT_STRING :
				{
					ObjectElementString *element_sub = new ObjectElementString();
					ObjectElementString *element_src = (ObjectElementString *) element;
					element_sub->setName(element->getName());
					element_sub->setDescription(element->getDescription());
					element_sub->value = element_src->value;
					return element_sub;
				}

			case OBJECT_ELEMENT_ID :
				{
					ObjectElementID *element_sub = new ObjectElementID();
					ObjectElementID *element_src = (ObjectElementID *) element;
					element_sub->setName(element->getName());
					element_sub->value = element_src->value;
					return element_sub;
				}

			case OBJECT_ELEMENT_ID_LIST :
				{
					ObjectElementIDList *element_sub = new ObjectElementIDList();
					ObjectElementIDList *element_src = (ObjectElementIDList *) element;
					element_sub->setName(element->getName());
					element_sub->setDescription(element->getDescription());
					element_sub->value = element_src->value;
					return element_sub;
				}

			case OBJECT_ELEMENT_VECTOR :
				{
					ObjectElementVector *element_sub = new ObjectElementVector();
					ObjectElementVector *element_src = (ObjectElementVector *) element;
					element_sub->setName(element->getName());
					element_sub->value = element_src->value;
					return element_sub;
				}

			case OBJECT_ELEMENT_VECTOR_LIST :
				{
					ObjectElementVectorList *element_sub = new ObjectElementVectorList();
					ObjectElementVectorList *element_src = (ObjectElementVectorList *) element;
					element_sub->setName(element->getName());
					element_sub->setDescription(element->getDescription());
					element_sub->value = element_src->value;
					return element_sub;
				}

			case OBJECT_ELEMENT_SINT8 :
				{
					ObjectElementSint8 *element_sub = new ObjectElementSint8();
					ObjectElementSint8 *element_src = (ObjectElementSint8 *) element;
					element_sub->setName(element->getName());
					element_sub->setDescription(element->getDescription());
					element_sub->value = element_src->value;
					return element_sub;
				}

			case OBJECT_ELEMENT_UINT8 :
				{
					ObjectElementUint8 *element_sub = new ObjectElementUint8();
					ObjectElementUint8 *element_src = (ObjectElementUint8 *) element;
					element_sub->setName(element->getName());
					element_sub->setDescription(element->getDescription());
					element_sub->value = element_src->value;
					return element_sub;
				}

			case OBJECT_ELEMENT_SINT16 :
				{
					ObjectElementSint16 *element_sub = new ObjectElementSint16();
					ObjectElementSint16 *element_src = (ObjectElementSint16 *) element;
					element_sub->setName(element->getName());
					element_sub->setDescription(element->getDescription());
					element_sub->value = element_src->value;
					return element_sub;
				}

			case OBJECT_ELEMENT_UINT16 :
				{
					ObjectElementUint16 *element_sub = new ObjectElementUint16();
					ObjectElementUint16 *element_src = (ObjectElementUint16 *) element;
					element_sub->setName(element->getName());
					element_sub->setDescription(element->getDescription());
					element_sub->value = element_src->value;
					return element_sub;
				}

			case OBJECT_ELEMENT_SINT32 :
				{
					ObjectElementSint32 *element_sub = new ObjectElementSint32();
					ObjectElementSint32 *element_src = (ObjectElementSint32 *) element;
					element_sub->setName(element->getName());
					element_sub->setDescription(element->getDescription());
					element_sub->value = element_src->value;
					return element_sub;
				}

			case OBJECT_ELEMENT_UINT32 :
				{
					ObjectElementUint32 *element_sub = new ObjectElementUint32();
					ObjectElementUint32 *element_src = (ObjectElementUint32 *) element;
					element_sub->setName(element->getName());
					element_sub->setDescription(element->getDescription());
					element_sub->value = element_src->value;
					return element_sub;
				}

			case OBJECT_ELEMENT_ENUM :
				{
					ObjectElementEnum *element_sub = new ObjectElementEnum();
					ObjectElementEnum *element_src = (ObjectElementEnum *) element;
					element_sub->setName(element->getName());
					element_sub->setDescription(element->getDescription());
					element_sub->value = element_src->value;
					return element_sub;
				}

			case OBJECT_ELEMENT_TARGET :
				{
					ObjectElementTarget *element_sub = new ObjectElementTarget();
					ObjectElementTarget *element_src = (ObjectElementTarget *) element;
					element_sub->setName(element->getName());
					element_sub->setDescription(element->getDescription());
					element_sub->value = element_src->value;
					return element_sub;
				}

			case OBJECT_ELEMENT_POSITION :
				{
					ObjectElementPosition *element_sub = new ObjectElementPosition();
					ObjectElementPosition *element_src = (ObjectElementPosition *) element;
					element_sub->setName(element->getName());
					element_sub->setDescription(element->getDescription());
					element_sub->value = element_src->value;
					return element_sub;
				}

			case OBJECT_ELEMENT_VECTOR3 :
				{
					ObjectElementVector3 *element_sub = new ObjectElementVector3();
					ObjectElementVector3 *element_src = (ObjectElementVector3 *) element;
					element_sub->setName(element->getName());
					element_sub->setDescription(element->getDescription());
					element_sub->value = element_src->value;
					return element_sub;
				}

			case OBJECT_ELEMENT_UINT32ARRAY :
				{
					ObjectElementUint32Array *element_sub = new ObjectElementUint32Array();
					ObjectElementUint32Array *element_src = (ObjectElementUint32Array *) element;
					element_sub->setName(element->getName());
					element_sub->setDescription(element->getDescription());
					element_sub->value = element_src->value;
					return element_sub;
				}
		}

		return NULL;
	}

	ObjectExtra *Object::cloneExtra(ObjectExtra *extra) {
		if (!extra) return NULL;

		ObjectExtra *extra_sub = new ObjectExtra();
		extra_sub->setName(extra->getName());
		extra_sub->setType(extra->getType());
		extra_sub->setParameterNames(extra->getParameterNames());
		extra_sub->setParameters(extra->getParameters());
		return extra_sub;
	}

	void Object::readXML(TiXmlElement *root) {
		for(TiXmlElement *pElem=root->FirstChildElement(); pElem; pElem=pElem->NextSiblingElement()) {
			string element_name=pElem->ValueStr();
			char *text_ptr=(char *) pElem->GetText();

			if (element_name == LIBGENS_OBJECT_ELEMENT_POSITION) {
				position.readXML(pElem);
			}
			else if (element_name == LIBGENS_OBJECT_ELEMENT_ROTATION) {
				rotation.readXML(pElem);
			}
			else if (element_name == LIBGENS_OBJECT_ELEMENT_SET_ID) {
				if (text_ptr) {
					FromString<size_t>(id, ToString(text_ptr), std::dec);
				}
			}
			else if (element_name == LIBGENS_OBJECT_ELEMENT_MULTI_SET_PARAM) {
				multi_set_param.readXML(pElem);
			}
			else {
				if (text_ptr) {
					string text=ToString(text_ptr);

					bool is_number=true;
					for (size_t i=0; i<text.size(); i++) {
						if (!isdigit(text[i]) && (text[i] != '-') && (text[i] != '.') && (text[i] != 'e')) {
							is_number = false;
							break;
						}
					}

					if ((text==LIBGENS_OBJECT_ELEMENT_BOOL_TRUE) || (text==LIBGENS_OBJECT_ELEMENT_BOOL_FALSE)) {
						ObjectElementBool *element=new ObjectElementBool();
						element->setName(element_name);
						element->value = (text == LIBGENS_OBJECT_ELEMENT_BOOL_TRUE);
						elements.push_back(element);
					}
					else if (text.size() && is_number) {
						if (element_name.find("Integer") != string::npos) {
							ObjectElementInteger *element=new ObjectElementInteger();
							element->setName(element_name);
							FromString<unsigned int>(element->value, text, std::dec);
							elements.push_back(element);
						}
						else {
							ObjectElementFloat *element=new ObjectElementFloat();
							element->setName(element_name);
							FromString<float>(element->value, text, std::dec);
							elements.push_back(element);
						}
					}
					else {
						if (element_name == "TargetOffset_Up") {
							printf("%s\n", text.c_str());
							getchar();
						}

						ObjectElementString *element=new ObjectElementString();
						element->setName(element_name);
						element->value=text;
						elements.push_back(element);
					}
				}
				else {
					TiXmlElement *subElem=pElem->FirstChildElement();

					if (subElem) {
						string sub_element_name=subElem->ValueStr();

						if (sub_element_name==LIBGENS_OBJECT_ELEMENT_SET_ID) {
							if (subElem->NextSiblingElement()) {
								ObjectElementIDList *element=new ObjectElementIDList();
								element->setName(element_name);

								for(subElem; subElem; subElem=subElem->NextSiblingElement()) {
									char *sub_text_ptr=(char *) subElem->GetText();

									if (sub_text_ptr) {
										size_t new_id=0;
										FromString<size_t>(new_id, ToString(sub_text_ptr), std::dec);
										element->value.push_back(new_id);
									}
								}

								elements.push_back(element);
							}
							else {
								char *sub_text_ptr=(char *) subElem->GetText();
								if (sub_text_ptr) {
									ObjectElementID *element=new ObjectElementID();
									element->setName(element_name);
									FromString<size_t>(element->value, ToString(sub_text_ptr), std::dec);
									elements.push_back(element);
								}
							}
						}
						else if ((sub_element_name==LIBGENS_MATH_AXIS_X_TEXT) || (sub_element_name==LIBGENS_MATH_AXIS_Y_TEXT) || (sub_element_name==LIBGENS_MATH_AXIS_Z_TEXT) || (sub_element_name== LIBGENS_MATH_AXIS_W_TEXT)) {
							ObjectElementVector *element=new ObjectElementVector();
							element->setName(element_name);
							element->value.readXML(pElem);
							elements.push_back(element);
						}
						else if (!sub_element_name.compare(0, LIBGENS_OBJECT_ELEMENT_POSITION_STR_SIZE, LIBGENS_OBJECT_ELEMENT_POSITION)) {
							ObjectElementVectorList *element=new ObjectElementVectorList();
							Vector3 position;
							element->setName(element_name);
							
							for(subElem; subElem; subElem=subElem->NextSiblingElement()) {
								position.readXML(subElem);
								element->value.push_back(position);
							}

							elements.push_back(element);
						}
					}
					else {
						ObjectElement *element=new ObjectElement();
						element->setName(element_name);
						elements.push_back(element);
					}
				}
			}
		}
	}


	void Object::readXMLTemplateElement(TiXmlElement *root) {
		string element_name=root->ValueStr();
		string type="";
		string description="";
		root->QueryValueAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_TYPE, &type);
		root->QueryValueAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DESCRIPTION, &description);
		
		if (element_name == LIBGENS_OBJECT_ELEMENT_EXTRA) {
			string value="";
			root->QueryValueAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_NAME, &value);

			ObjectExtra *extra = new ObjectExtra();
			extra->setType(type);
			extra->setName(value);

			for (TiXmlAttribute *attribute=root->FirstAttribute(); attribute; attribute=attribute->Next()) {
				string attribute_name=attribute->NameTStr();
				string attribute_value=attribute->ValueStr();
				if ((attribute_name != LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_NAME) && (attribute_name != LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_TYPE)) {
					extra->addParameter(attribute_name, attribute_value);
				}
			}

			extras.push_back(extra);
		}
		else {
			if (type == LIBGENS_OBJECT_ELEMENT_UNDEFINED_TEMPLATE) {
				ObjectElement *element = new ObjectElement();
				element->setName(element_name);
				element->setDescription(description);
				elements.push_back(element);
			}
			else if (type == LIBGENS_OBJECT_ELEMENT_BOOL_TEMPLATE) {
				ObjectElementBool *element = new ObjectElementBool();
				string value="";
				root->QueryValueAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DEFAULT, &value);
				element->value = (value == LIBGENS_OBJECT_ELEMENT_BOOL_TRUE);
				element->setName(element_name);
				element->setDescription(description);
				elements.push_back(element);
			}
			else if (type == LIBGENS_OBJECT_ELEMENT_INTEGER_TEMPLATE) {
				ObjectElementInteger *element = new ObjectElementInteger();
				root->QueryUnsignedAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DEFAULT, &element->value);
				element->setName(element_name);
				element->setDescription(description);
				elements.push_back(element);
			}
			else if (type == LIBGENS_OBJECT_ELEMENT_FLOAT_TEMPLATE) {
				ObjectElementFloat *element = new ObjectElementFloat();
				root->QueryFloatAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DEFAULT, &element->value);
				element->setName(element_name);
				element->setDescription(description);
				elements.push_back(element);
			}
			else if (type == LIBGENS_OBJECT_ELEMENT_STRING_TEMPLATE) {
				ObjectElementString *element = new ObjectElementString();
				root->QueryValueAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DEFAULT, &element->value);
				element->setName(element_name);
				element->setDescription(description);
				elements.push_back(element);
			}
			else if (type == LIBGENS_OBJECT_ELEMENT_ID_TEMPLATE) {
				ObjectElementID *element = new ObjectElementID();
				element->setName(element_name);
				element->setDescription(description);
				elements.push_back(element);
			}
			else if (type == LIBGENS_OBJECT_ELEMENT_ID_LIST_TEMPLATE) {
				ObjectElementIDList *element = new ObjectElementIDList();
				element->setName(element_name);
				element->setDescription(description);
				elements.push_back(element);
			}
			else if (type == LIBGENS_OBJECT_ELEMENT_VECTOR_TEMPLATE) {
				ObjectElementVector *element = new ObjectElementVector();
				element->setName(element_name);
				element->setDescription(description);
				elements.push_back(element);
			}
			else if (type == LIBGENS_OBJECT_ELEMENT_VECTOR_LIST_TEMPLATE) {
				ObjectElementVectorList *element = new ObjectElementVectorList();
				element->setName(element_name);
				element->setDescription(description);
				elements.push_back(element);
			}
			else if (type == LIBGENS_OBJECT_ELEMENT_SINT8_TEMPLATE) {
				ObjectElementSint8 *element = new ObjectElementSint8();
				element->setName(element_name);
				element->setDescription(description);
				elements.push_back(element);
			}
			else if (type == LIBGENS_OBJECT_ELEMENT_UINT8_TEMPLATE) {
				ObjectElementUint8 *element = new ObjectElementUint8();
				element->setName(element_name);
				element->setDescription(description);
				elements.push_back(element);
			}
			else if (type == LIBGENS_OBJECT_ELEMENT_SINT16_TEMPLATE) {
				ObjectElementSint16 *element = new ObjectElementSint16();
				element->setName(element_name);
				element->setDescription(description);
				elements.push_back(element);
			}
			else if (type == LIBGENS_OBJECT_ELEMENT_UINT16_TEMPLATE) {
				ObjectElementUint16 *element = new ObjectElementUint16();
				element->setName(element_name);
				element->setDescription(description);
				elements.push_back(element);
			}
			else if (type == LIBGENS_OBJECT_ELEMENT_SINT32_TEMPLATE) {
				ObjectElementSint32 *element = new ObjectElementSint32();
				element->setName(element_name);
				element->setDescription(description);
				elements.push_back(element);
			}
			else if (type == LIBGENS_OBJECT_ELEMENT_UINT32_TEMPLATE) {
				ObjectElementUint32 *element = new ObjectElementUint32();
				element->setName(element_name);
				element->setDescription(description);
				elements.push_back(element);
			}
			else if (type == LIBGENS_OBJECT_ELEMENT_ENUM_TEMPLATE) {
				ObjectElementEnum *element = new ObjectElementEnum();
				element->setName(element_name);
				element->setDescription(description);
				elements.push_back(element);
			}
			else if (type == LIBGENS_OBJECT_ELEMENT_POSITION_TEMPLATE) {
				ObjectElementPosition *element = new ObjectElementPosition();
				element->setName(element_name);
				element->setDescription(description);
				elements.push_back(element);
			}
			else if (type == LIBGENS_OBJECT_ELEMENT_TARGET_TEMPLATE) {
				ObjectElementTarget *element = new ObjectElementTarget();
				element->setName(element_name);
				element->setDescription(description);
				elements.push_back(element);
			}
			else if (type == LIBGENS_OBJECT_ELEMENT_VECTOR3_TEMPLATE) {
				ObjectElementVector3 *element = new ObjectElementVector3();
				element->setName(element_name);
				element->setDescription(description);
				elements.push_back(element);
			}
			else if (type == LIBGENS_OBJECT_ELEMENT_UINT32ARRAY_TEMPLATE) {
				ObjectElementUint32Array *element = new ObjectElementUint32Array();
				element->setName(element_name);
				element->setDescription(description);
				elements.push_back(element);
			}
		}
	}


	void Object::readXMLTemplate(string filename) {
		TiXmlDocument doc(filename);
		if (!doc.LoadFile()) {
			Error::addMessage(Error::FILE_NOT_FOUND, LIBGENS_FILE_H_ERROR_READ_FILE_BEFORE + filename + LIBGENS_FILE_H_ERROR_READ_FILE_AFTER);
			return;
		}

		TiXmlHandle hDoc(&doc);
		TiXmlElement* pElem;
		TiXmlHandle hRoot(0);

		pElem=hDoc.FirstChildElement().Element();
		if (!pElem) {
			Error::addMessage(Error::EXCEPTION, LIBGENS_OBJECT_H_ERROR_READ_TEMPLATE_BEFORE + filename + LIBGENS_OBJECT_H_ERROR_READ_TEMPLATE_AFTER);
			return;
		}

		pElem=pElem->FirstChildElement();
		for(pElem; pElem; pElem=pElem->NextSiblingElement()) {
			readXMLTemplateElement(pElem);
		}
	}


	void Object::writeXML(TiXmlElement *root) {
		TiXmlElement* objRoot=new TiXmlElement(name);

		for (list<ObjectElement *>::iterator it=elements.begin(); it!=elements.end(); it++) {
			(*it)->writeXML(objRoot);
		}

		TiXmlElement* positionRoot=new TiXmlElement(LIBGENS_OBJECT_ELEMENT_POSITION);
		position.writeXML(positionRoot);
		objRoot->LinkEndChild(positionRoot);

		TiXmlElement* rotationRoot=new TiXmlElement(LIBGENS_OBJECT_ELEMENT_ROTATION);
		rotation.writeXML(rotationRoot);
		objRoot->LinkEndChild(rotationRoot);

		TiXmlElement* idRoot=new TiXmlElement(LIBGENS_OBJECT_ELEMENT_SET_ID);
		TiXmlText* idValue=new TiXmlText(ToString(id));
		idRoot->LinkEndChild(idValue);
		objRoot->LinkEndChild(idRoot);

		multi_set_param.writeXML(objRoot);

		root->LinkEndChild(objRoot);
	}

	void Object::saveXMLTemplate(string filename) {
		TiXmlDocument doc;
		TiXmlDeclaration *decl = new TiXmlDeclaration( "1.0", "", "" );
		doc.LinkEndChild( decl );

		TiXmlElement *root=new TiXmlElement(LIBGENS_OBJECT_TEMPLATE_ROOT);
		
		for (list<ObjectElement *>::iterator it=elements.begin(); it!=elements.end(); it++) {
			(*it)->writeXMLTemplate(root);
		}

		for (list<ObjectExtra *>::iterator it=extras.begin(); it!=extras.end(); it++) {
			(*it)->writeXMLTemplate(root);
		}

		doc.LinkEndChild(root);

		if (!doc.SaveFile(filename)) {
			Error::addMessage(Error::FILE_NOT_FOUND, LIBGENS_OBJECT_H_ERROR_WRITE_TEMPLATE_XML + filename);
		}
	}


	void Object::learnFromObject(Object *object) {
		list<ObjectElement *> template_elements=object->getElements();
		list<ObjectExtra *> template_extras=object->getExtras();

		for (list<ObjectElement *>::iterator it=template_elements.begin(); it!=template_elements.end(); it++) {
			bool found=false;

			for (list<ObjectElement *>::iterator it_e=elements.begin(); it_e!=elements.end(); it_e++) {
				if ((*it) == NULL) {
					printf("One of this template's elements is NULL!\n");
					getchar();
				}

				if ((*it_e) == NULL) {
					printf("One of this object's elements is NULL!\n");
					getchar();
				}

				if ((*it)->getName() == (*it_e)->getName()) {
					ObjectElementType type_current  = (*it_e)->getType();
					ObjectElementType type_template = (*it)->getType();

					if ((!(*it_e)->getDescription().size()) && (*it)->getDescription().size()) {
						(*it_e)->setDescription((*it)->getDescription());
					}

					if ((type_current == OBJECT_ELEMENT_UNDEFINED) && (type_template != OBJECT_ELEMENT_UNDEFINED)) {
						ObjectElement *new_element=cloneElement(*it);
						delete (*it_e);
						(*it_e) = new_element;
					}

					if ((type_current == OBJECT_ELEMENT_ID) && (type_template == OBJECT_ELEMENT_ID_LIST)) {
						ObjectElementIDList *new_element=(ObjectElementIDList *) cloneElement(*it);
						new_element->value.push_back(((ObjectElementID *)(*it_e))->value);
						delete (*it_e);
						(*it_e) = new_element;
					}

					if ((type_current == OBJECT_ELEMENT_VECTOR) && (type_template == OBJECT_ELEMENT_VECTOR_LIST)) {
						ObjectElementVectorList *new_element=(ObjectElementVectorList *) cloneElement(*it);
						new_element->value.push_back(((ObjectElementVector *)(*it_e))->value);
						delete (*it_e);
						(*it_e) = new_element;
					}

					if ((type_current == OBJECT_ELEMENT_FLOAT) && (type_template == OBJECT_ELEMENT_STRING)) {
						ObjectElementString *new_element=(ObjectElementString *) cloneElement(*it);
						new_element->value = ToString(((ObjectElementFloat *)(*it_e))->value);
						delete (*it_e);
						(*it_e) = new_element;
					}

					found=true;
					break;
				}
			}

			if (!found) {
				ObjectElement *new_element=cloneElement(*it);
				elements.push_back(new_element);
			}
		}


		for (list<ObjectExtra *>::iterator it=template_extras.begin(); it!=template_extras.end(); it++) {
			bool found=false;

			for (list<ObjectExtra *>::iterator it_e=extras.begin(); it_e!=extras.end(); it_e++) {
				if ((*it)->compare(*it_e)) {
					found=true;
					break;
				}
			}

			if (!found) {
				ObjectExtra *new_extra=cloneExtra(*it);
				if (new_extra) extras.push_back(new_extra);
			}
		}
	}


	void Object::learnFromLibrary(ObjectLibrary *library) {
		if (!library) return;

		Object *obj=library->getTemplate(name);
		if (obj) learnFromObject(obj);
	}


	string Object::queryEditorValue(string value_type, string slot_id, string default_value) {
		string model_name=default_value;

		for (list<ObjectExtra *>::iterator it=extras.begin(); it!=extras.end(); it++) {
			if ((*it)->getType() == value_type) {
				vector<string> parameter_names=(*it)->getParameterNames();
				vector<string> parameters=(*it)->getParameters();

				bool conditions_met=true;
				for (size_t i=0; i<parameter_names.size(); i++) {
					if (parameter_names[i].find(LIBGENS_OBJECT_EXTRA_CONDITION) != string::npos) {
						// Parse condition attribute into logic comparisons
						string first_value="";
						string second_value="";
						string operator_str="";
						unsigned int detection_step=0;

						// FIXME: Add Slot ID Comparison.

						for (size_t c=0; c<parameters[i].size(); c++) {
							char letter=parameters[i][c];

							// Ignore any Spaces
							if (letter == ' ') {
								continue;
							}

							// If Operator symbols are detected, add to operator string
							if ((letter == '=') || (letter == '<') || (letter == '>') || (letter == '!')) {
								if (detection_step == 0) {
									detection_step = 1;
								}
							}
							else {
								if (detection_step == 1) {
									detection_step = 2;
								}
							}

							if (detection_step == 0) first_value += letter;
							if (detection_step == 1) operator_str += letter;
							if (detection_step == 2) second_value += letter;
							if (detection_step > 2 ) break;
						}

						// Check if all strings are valid
						if (!first_value.empty() && !second_value.empty() && !operator_str.empty()) {
							// Run a cross-check for first and second strings on all the elements
							ObjectElement *first_element=getElement(first_value);
							ObjectElement *second_element=getElement(second_value);

							if (first_element && second_element) {
								// This type of comparisons should never be necessary, unsupported
								continue;
							}

							if (!first_element && !second_element) {
								// This type of comparisons should never be necessary, unsupported
								continue;
							}

							// Determine operator type
							ObjectConditionType condition_type=OBJECT_CONDITION_UNDEFINED;
							if ((operator_str == "=") || (operator_str == "==")) condition_type=OBJECT_CONDITION_EQUAL;
							if ((operator_str == "!") || (operator_str == "!=")) condition_type=OBJECT_CONDITION_NOT_EQUAL;
							if (operator_str == ">")                             condition_type=OBJECT_CONDITION_BIGGER;
							if (operator_str == "<")                             condition_type=OBJECT_CONDITION_LESSER;
							if (operator_str == ">=")                            condition_type=OBJECT_CONDITION_BIGGER_EQUAL;
							if (operator_str == "<=")                            condition_type=OBJECT_CONDITION_LESSER_EQUAL;

							ObjectElementType comparison_type=OBJECT_ELEMENT_UNDEFINED;
							if (first_element) comparison_type=first_element->getType();
							if (second_element) comparison_type=second_element->getType();

							// Handle boolean comparison
							switch (comparison_type) {

							case OBJECT_ELEMENT_BOOL:
								{
									bool b1, b2;

									if (first_element) b1=static_cast<ObjectElementBool *>(first_element)->value;
									else b1=(first_value == LIBGENS_OBJECT_ELEMENT_BOOL_TRUE);

									if (second_element) b2=static_cast<ObjectElementBool *>(second_element)->value;
									else b2=(second_value == LIBGENS_OBJECT_ELEMENT_BOOL_TRUE);
								
									if (condition_type == OBJECT_CONDITION_EQUAL) {
										if (!(b1 == b2)) {
											conditions_met = false;
											break;
										}
									}

									if (condition_type == OBJECT_CONDITION_NOT_EQUAL) {
										if (!(b1 != b2)) {
											conditions_met = false;
											break;
										}
									}
									break;
								}

							// Handle floats comparison
							case OBJECT_ELEMENT_FLOAT:
								{
									float f1, f2;

									if (first_element) f1=static_cast<ObjectElementFloat *>(first_element)->value;
									else FromString<float>(f1, first_value, std::dec);

									if (second_element) f2=static_cast<ObjectElementFloat *>(second_element)->value;
									else FromString<float>(f2, second_value, std::dec);

								
									if (condition_type == OBJECT_CONDITION_EQUAL) {
										if (!(f1 == f2)) {
											conditions_met = false;
											break;
										}
									}

									if (condition_type == OBJECT_CONDITION_NOT_EQUAL) {
										if (!(f1 != f2)) {
											conditions_met = false;
											break;
										}
									}

									if (condition_type == OBJECT_CONDITION_BIGGER) {
										if (!(f1 > f2)) {
											conditions_met = false;
											break;
										}
									}

									if (condition_type == OBJECT_CONDITION_LESSER) {
										if (!(f1 < f2)) {
											conditions_met = false;
											break;
										}
									}

									if (condition_type == OBJECT_CONDITION_BIGGER_EQUAL) {
										if (!(f1 >= f2)) {
											conditions_met = false;
											break;
										}
									}

									if (condition_type == OBJECT_CONDITION_LESSER_EQUAL) {
										if (!(f1 <= f2)) {
											conditions_met = false;
											break;
										}
									}
									break;
									}

								// Handle string comparison
								case OBJECT_ELEMENT_STRING:
									{
									string s1=first_value;
									string s2=second_value;

									if (first_element) s1=static_cast<ObjectElementString *>(first_element)->value;
									if (second_element) s2=static_cast<ObjectElementString *>(second_element)->value;
								
									if (condition_type == OBJECT_CONDITION_EQUAL) {
										if (!(s1 == s2)) {
											conditions_met = false;
											break;
										}
									}

									if (condition_type == OBJECT_CONDITION_NOT_EQUAL) {
										if (!(s1 != s2)) {
											conditions_met = false;
											break;
										}
									}
									break;
								}

							// Handle unsigned integer comparison
							case OBJECT_ELEMENT_INTEGER:
							case OBJECT_ELEMENT_UINT8:
							case OBJECT_ELEMENT_UINT16:
							case OBJECT_ELEMENT_UINT32:
							case OBJECT_ELEMENT_ENUM:
								{
									unsigned int i1, i2;
									
									if (first_element) {
										switch (comparison_type) {
											case OBJECT_ELEMENT_INTEGER: i1 = static_cast<ObjectElementInteger*>(first_element)->value; break;
											case OBJECT_ELEMENT_UINT8:   i1 = static_cast<ObjectElementUint8*>  (first_element)->value; break;
											case OBJECT_ELEMENT_UINT16:  i1 = static_cast<ObjectElementUint16*> (first_element)->value; break;
											case OBJECT_ELEMENT_UINT32:  i1 = static_cast<ObjectElementUint32*> (first_element)->value; break;
											case OBJECT_ELEMENT_ENUM:    i1 = static_cast<ObjectElementEnum*>   (first_element)->value; break;
										}
									}
									else FromString<unsigned int>(i1, first_value, std::dec);

									if (second_element) {
										switch (comparison_type) {
											case OBJECT_ELEMENT_INTEGER: i1 = static_cast<ObjectElementInteger*>(second_element)->value; break;
											case OBJECT_ELEMENT_UINT8:   i1 = static_cast<ObjectElementUint8*>  (second_element)->value; break;
											case OBJECT_ELEMENT_UINT16:  i1 = static_cast<ObjectElementUint16*> (second_element)->value; break;
											case OBJECT_ELEMENT_UINT32:  i1 = static_cast<ObjectElementUint32*> (second_element)->value; break;
											case OBJECT_ELEMENT_ENUM:    i1 = static_cast<ObjectElementEnum*>   (second_element)->value; break;
										}
									}
									else FromString<unsigned int>(i2, second_value, std::dec);

									if (condition_type == OBJECT_CONDITION_EQUAL) {
										if (!(i1 == i2)) {
											conditions_met = false;
											break;
										}
									}

									if (condition_type == OBJECT_CONDITION_NOT_EQUAL) {
										if (!(i1 != i2)) {
											conditions_met = false;
											break;
										}
									}

									if (condition_type == OBJECT_CONDITION_BIGGER) {
										if (!(i1 > i2)) {
											conditions_met = false;
											break;
										}
									}

									if (condition_type == OBJECT_CONDITION_LESSER) {
										if (!(i1 < i2)) {
											conditions_met = false;
											break;
										}
									}

									if (condition_type == OBJECT_CONDITION_BIGGER_EQUAL) {
										if (!(i1 >= i2)) {
											conditions_met = false;
											break;
										}
									}

									if (condition_type == OBJECT_CONDITION_LESSER_EQUAL) {
										if (!(i1 <= i2)) {
											conditions_met = false;
											break;
										}
									}
									break;
								}
								
							// Handle signed integer comparison
							case OBJECT_ELEMENT_SINT8:
							case OBJECT_ELEMENT_SINT16:
							case OBJECT_ELEMENT_SINT32:
								{
									signed int i1, i2;
									
									if (first_element) {
										switch (comparison_type) {
											case OBJECT_ELEMENT_SINT8:   i1 = static_cast<ObjectElementSint8*>  (first_element)->value; break;
											case OBJECT_ELEMENT_SINT16:  i1 = static_cast<ObjectElementSint16*> (first_element)->value; break;
											case OBJECT_ELEMENT_SINT32:  i1 = static_cast<ObjectElementSint32*> (first_element)->value; break;
										}
									}
									else FromString<signed int>(i1, first_value, std::dec);

									if (second_element) {
										switch (comparison_type) {
											case OBJECT_ELEMENT_SINT8:   i1 = static_cast<ObjectElementSint8*>  (second_element)->value; break;
											case OBJECT_ELEMENT_SINT16:  i1 = static_cast<ObjectElementSint16*> (second_element)->value; break;
											case OBJECT_ELEMENT_SINT32:  i1 = static_cast<ObjectElementSint32*> (second_element)->value; break;
										}
									}
									else FromString<signed int>(i2, second_value, std::dec);

									if (condition_type == OBJECT_CONDITION_EQUAL) {
										if (!(i1 == i2)) {
											conditions_met = false;
											break;
										}
									}

									if (condition_type == OBJECT_CONDITION_NOT_EQUAL) {
										if (!(i1 != i2)) {
											conditions_met = false;
											break;
										}
									}

									if (condition_type == OBJECT_CONDITION_BIGGER) {
										if (!(i1 > i2)) {
											conditions_met = false;
											break;
										}
									}

									if (condition_type == OBJECT_CONDITION_LESSER) {
										if (!(i1 < i2)) {
											conditions_met = false;
											break;
										}
									}

									if (condition_type == OBJECT_CONDITION_BIGGER_EQUAL) {
										if (!(i1 >= i2)) {
											conditions_met = false;
											break;
										}
									}

									if (condition_type == OBJECT_CONDITION_LESSER_EQUAL) {
										if (!(i1 <= i2)) {
											conditions_met = false;
											break;
										}
									}
									break;
								}
							}
						}
					}
				}

				if (conditions_met) {
					model_name = (*it)->getName();
				}
			}
		}

		return model_name;
	}

	string Object::queryEditorModel(string slot_id, string default_value) {
		return queryEditorValue(LIBGENS_OBJECT_EXTRA_TYPE_MODEL, slot_id, default_value);
	}

	string Object::queryEditorSkeleton(string slot_id, string default_value) {
		return queryEditorValue(LIBGENS_OBJECT_EXTRA_TYPE_SKELETON, slot_id, default_value);
	}

	string Object::queryEditorAnimation(string slot_id, string default_value) {
		return queryEditorValue(LIBGENS_OBJECT_EXTRA_TYPE_ANIMATION, slot_id, default_value);
	}

	string Object::getName() {
		return name;
	}

	size_t Object::getID() {
		return id;
	}

	void Object::setID(size_t v) {
		id=v;
	}

	void Object::setPosition(Vector3 v) {
		position=v;
	}

	Vector3 Object::getPosition() {
		return position;
	}

	void Object::setRotation(Quaternion v) {
		rotation=v;
	}

	Quaternion Object::getRotation() {
		return rotation;
	}

	list<ObjectElement *> Object::getElements() {
		return elements;
	}

	ObjectElement *Object::getElement(string nm) {
		for (list<ObjectElement *>::iterator it=elements.begin(); it!=elements.end(); it++) {
			if ((*it)->getName() == nm) {
				return (*it);
			}
		}
		return NULL;
	}

	list<ObjectExtra *> Object::getExtras() {
		return extras;
	}

	void Object::deleteExtras() {
		for (list<ObjectExtra *>::iterator it=extras.begin(); it!=extras.end(); it++) {
			delete (*it);
		}
		extras.clear();
	}

	string Object::queryExtraName(string type, string def) {
		for (list<ObjectExtra *>::iterator it=extras.begin(); it!=extras.end(); it++) {
			if ((*it)->getType() == type) {
				return (*it)->getName();
			}
		}
		return def;
	}

	void Object::setParentSet(ObjectSet *v) {
		parent_set = v;
	}

	ObjectSet *Object::getParentSet() {
		return parent_set;
	}

	MultiSetParam *Object::getMultiSetParam() {
		return &multi_set_param;
	}

	void Object::readORC(File *file) {
		size_t nodeTransformOffset;
		int transformCount;
		
		ObjectElementFloat *RangeIn = (ObjectElementFloat*) getElement("RangeIn");
		ObjectElementFloat *RangeOut = (ObjectElementFloat*) getElement("RangeOut");
		ObjectElementTarget *Parent = (ObjectElementTarget*) getElement("Parent");
		ObjectElementUint16 *Unknown1 = (ObjectElementUint16*) getElement("Unknown1");
		ObjectElementUint32 *Unknown2 = (ObjectElementUint32*) getElement("Unknown2");
		ObjectElementFloat *Unknown3 = (ObjectElementFloat*) getElement("Unknown3");
		
		file->readInt16BE(&Unknown1->value);
		unsigned short _id;
		file->readInt16BE(&_id);
		id = (int) _id;

		file->readInt32BE((unsigned int*) &Unknown2->value);
		file->moveAddress(4);
		file->readFloat32BE(&Unknown3->value);
		file->readFloat32BE(&RangeIn->value);
		file->readFloat32BE(&RangeOut->value);
		file->readInt32BE(&Parent->value);
		file->readInt32BEA(&nodeTransformOffset);
		file->readInt32BE(&transformCount);
		file->moveAddress(0xC);

		// Objects with parents use a local offset relative to the parent instead of the normal global position
		if (Parent->value != 0) needs_transform_recalc = true;

		int paramsStart = file->getCurrentAddress();
		file->goToAddress(nodeTransformOffset);
		position.read(file);
		position = position;

		// Rotation is stored as Euler XYZ angles (rad)
		Vector3 rot;
		rot.read(file);
		rotation.fromLostWorldEuler(rot);

		local_position.read(file);
		local_position = local_position;

		rot.read(file);
		local_rotation.fromLostWorldEuler(rot);
		
		if (transformCount > 1) multi_set_param.readORC(file, transformCount - 1);

		file->goToAddress(paramsStart);
		
		for (auto it = elements.begin(); it != elements.end(); it++)
		{
			ObjectElement *elem = *it;
			if (elem->getName() == "RangeIn" ) continue;
			if (elem->getName() == "RangeOut") continue;
			if (elem->getName() == "Parent"  ) continue;
			if (elem->getName() == "Unknown1") continue;
			if (elem->getName() == "Unknown2") continue;
			if (elem->getName() == "Unknown3") continue;

			switch (elem->getType())
			{

			case OBJECT_ELEMENT_BOOL: 
				{
				ObjectElementBool *bool_cast = static_cast<ObjectElementBool*>(elem);
				file->read(&bool_cast->value, 1);
				break;
				}

			case OBJECT_ELEMENT_FLOAT:
				{
				file->fixPaddingRead(4);
				ObjectElementFloat *float_cast = static_cast<ObjectElementFloat*>(elem);
				file->readFloat32BE(&float_cast->value);
				break;
				}

			case OBJECT_ELEMENT_STRING:
				{
				unsigned int offset, unknown;
				file->readInt32BEA(&offset);
				file->readInt32BE(&unknown);

				if (offset != file->getRootNodeAddress()) {
					ObjectElementString *string_cast = static_cast<ObjectElementString*>(elem);
					unsigned int curAddr = file->getCurrentAddress();
					file->goToAddress(offset);
					file->readString(&string_cast->value);
					file->goToAddress(curAddr);
				}
				
				if (unknown != 0)
					cout << "0x" << std::hex << file->getCurrentAddress() - 0x10 << " " << getName() << " : String unknown is not zero: " << std::dec << unknown << "\n";

				break;
				}
				
			case OBJECT_ELEMENT_SINT8:
			case OBJECT_ELEMENT_UINT8:
			case OBJECT_ELEMENT_ENUM:
				{
				ObjectElementUint8 *uint8_cast = static_cast<ObjectElementUint8*>(elem);
				file->readUChar(&uint8_cast->value);
				break;
				}

			case OBJECT_ELEMENT_SINT16:
			case OBJECT_ELEMENT_UINT16:
				{
				// There's only one uint16 property, and no sint16 ones, and it's already 4-byte-aligned
				// so there's no way to check if this type needs padding (and no reason to)
				ObjectElementUint16 *uint16_cast = static_cast<ObjectElementUint16*>(elem);
				file->readInt16BE(&uint16_cast->value);
				break;
				}

			case OBJECT_ELEMENT_SINT32:
			case OBJECT_ELEMENT_UINT32:
			case OBJECT_ELEMENT_TARGET:
				{
				file->fixPaddingRead(4);
				ObjectElementInteger *int_cast = static_cast<ObjectElementInteger*>(elem);
				file->readInt32BE(&int_cast->value);
				break;
				}

			case OBJECT_ELEMENT_POSITION:
			case OBJECT_ELEMENT_VECTOR3:
				{
				file->fixPaddingRead(16);
				ObjectElementVector *vector_cast = static_cast<ObjectElementVector*>(elem);
				vector_cast->value.read(file);
				if (elem->getType() == OBJECT_ELEMENT_POSITION) vector_cast->value = vector_cast->value;
				
				unsigned int unknown;
				file->readInt32BE(&unknown);

				if (unknown != 0)
				{
					string type = (elem->getType() == OBJECT_ELEMENT_POSITION ? "Position" : "Vector3");
					cout << "0x" << std::hex << file->getCurrentAddress() - 0x10 << " " << getName() << " : " << type << " unknown is not zero: " << std::dec << unknown << "\n";
				}

				break;
				}

			case OBJECT_ELEMENT_UINT32ARRAY:
				{
				file->fixPaddingRead(4);
				unsigned int offset = 0, count = 0, unknown = 0;
				file->readInt32BEA(&offset);
				file->readInt32BE(&count);
				file->readInt32BE(&unknown);
				ObjectElementUint32Array *array_cast = static_cast<ObjectElementUint32Array*>(elem);
				array_cast->value.resize(count);

				if (count > 0) {
					unsigned int curAddr = file->getCurrentAddress();
					file->goToAddress(offset);

					for (int a = 0; a < count; a++)
						file->readInt32BE(&array_cast->value[a]);

					file->goToAddress(curAddr);
				}

				if (unknown != 0)
					cout << "0x" << std::hex << file->getCurrentAddress() - 0xC << " " << getName() << " : Uint32Array unknown is not zero: " << std::dec << unknown << "\n";

				break;
				}
			}
		}
	}
	
	void Object::writeORC(File *file, vector<unsigned int>& offset_vector) {
		orc_offset = file->getCurrentAddress();

		int zero = 0;
		ObjectElementFloat *range_in_element  = (ObjectElementFloat*) getElement("RangeIn");
		ObjectElementFloat *range_out_element = (ObjectElementFloat*) getElement("RangeOut");
		ObjectElementTarget *parent_element = (ObjectElementTarget*) getElement("Parent");
		ObjectElementUint16 *unknown1_element = (ObjectElementUint16*) getElement("Unknown1");
		ObjectElementUint32 *unknown2_element = (ObjectElementUint32*) getElement("Unknown2");
		ObjectElementFloat *unknown3_element = (ObjectElementFloat*) getElement("Unknown3");
		float range_in = range_in_element->value;
		float range_out = range_out_element->value;
		size_t parent = parent_element->value;
		unsigned short unknown1 = unknown1_element->value;
		unsigned int unknown2 = unknown2_element->value;
		float unknown3 = unknown3_element->value;

		int num_nodes = getMultiSetParam()->getSize() + 1;
		unsigned short id16 = (unsigned short) id;

		file->writeInt16BE(&unknown1); // unknown value that precedes ID
		file->writeInt16BE(&id16);
		file->writeInt32BE(&unknown2); // unknown value - no clue what it is
		file->writeInt32BE(&zero); // unknown value - always 0
		file->writeFloat32BE(&unknown3); // unknown float
		file->writeFloat32BE(&range_in);
		file->writeFloat32BE(&range_out);
		file->writeInt32BE(&parent); // unknown value - usually 0
		offset_vector.push_back(file->getCurrentAddress());
		file->writeNull(4); // filler for units offset
		file->writeInt32BE(&num_nodes);
		file->writeNull(0xC); // padding to 16 bytes

		GensStringTable strings;

		struct uint32array {
			ObjectElementUint32Array *elem;
			int offset;
		};
		vector<uint32array> arrays;
		
		for (list<ObjectElement*>::iterator it = elements.begin(); it != elements.end(); it++) {
			ObjectElement *elem = *it;
			if (elem->getName() == "RangeIn") continue;
			if (elem->getName() == "RangeOut") continue;
			if (elem->getName() == "Parent") continue;
			if (elem->getName() == "Unknown1") continue;
			if (elem->getName() == "Unknown2") continue;
			if (elem->getName() == "Unknown3") continue;

			switch (elem->getType()) {
				
			case OBJECT_ELEMENT_FLOAT:
				{
				file->fixPadding(4);
				ObjectElementFloat *float_cast =static_cast<ObjectElementFloat*>(elem);
				file->writeFloat32BE(&float_cast->value);
				break;
				}

			case OBJECT_ELEMENT_STRING:
				{
				file->fixPadding(4);
				ObjectElementString *string_cast = static_cast<ObjectElementString*>(elem);

				if (!string_cast->value.empty()) {
					offset_vector.push_back(file->getCurrentAddress());
					strings.writeString(file, string_cast->value);
					file->writeNull(4);
				}

				else {
					file->writeNull(8);
				}

				break;
				}

			case OBJECT_ELEMENT_SINT8:
			case OBJECT_ELEMENT_UINT8:
			case OBJECT_ELEMENT_BOOL:
			case OBJECT_ELEMENT_ENUM:
				{
				ObjectElementUint8 *uint8_cast = static_cast<ObjectElementUint8*>(elem);
				file->write(&uint8_cast->value, 1);
				break;
				}

			case OBJECT_ELEMENT_SINT16:
			case OBJECT_ELEMENT_UINT16:
				{
				file->fixPadding(2);
				ObjectElementUint16 *uint16_cast = static_cast<ObjectElementUint16*>(elem);
				file->writeInt16BE(&uint16_cast->value);
				break;
				}

			case OBJECT_ELEMENT_SINT32:
			case OBJECT_ELEMENT_UINT32:
			case OBJECT_ELEMENT_TARGET:
				{
				file->fixPadding(4);
				ObjectElementInteger *int_cast = static_cast<ObjectElementInteger*>(elem);
				file->writeInt32BE(&int_cast->value);
				break;
				}

			case OBJECT_ELEMENT_POSITION:
			case OBJECT_ELEMENT_VECTOR3:
				{
				file->fixPadding(16);
				ObjectElementVector3 *vector3_cast = static_cast<ObjectElementVector3*>(elem);
				Vector3 value = vector3_cast->value;
				if (elem->getType() == OBJECT_ELEMENT_POSITION) value = value;
				value.write(file);
				file->writeNull(4);
				break;
				}

			case OBJECT_ELEMENT_UINT32ARRAY:
				{
					file->fixPadding(4);
					ObjectElementUint32Array *array_cast = static_cast<ObjectElementUint32Array*>(elem);

					if (array_cast->value.size() > 0) {
						uint32array new_array;
						new_array.elem = array_cast;
						new_array.offset = file->getCurrentAddress();
						arrays.push_back(new_array);
						offset_vector.push_back(file->getCurrentAddress());
					}

					file->writeNull(0xC);
					break;
				}
			}
		}

		// Write strings
		file->fixPadding(4);
		strings.write(file, false);

		// Write arrays
		file->fixPadding(4);
		for (int a = 0; a < arrays.size(); a++) {
			uint32array *array_p = &arrays[a];
			unsigned int cur = file->getCurrentAddress();
			unsigned int num = array_p->elem->value.size();

			file->goToAddress(array_p->offset);
			file->writeInt32BEA(&cur);
			file->writeInt32BE(&num);
			file->goToAddress(cur);

			for (int i = 0; i < num; i++)
				file->writeInt32BE(&array_p->elem->value[i]);
		}
	}

	void Object::writeUnitsORC(File *file, Level *level) {
		unsigned int units_start = file->getCurrentAddress();
		file->goToAddress(orc_offset);
		file->moveAddress(0x1C);
		file->writeInt32BEA(&units_start);
		file->goToAddress(units_start);
		
		int num_nodes = multi_set_param.getSize() + 1;
		std::list<MultiSetNode*> nodes = multi_set_param.getNodes();
		std::list<MultiSetNode*>::iterator it = nodes.begin();

		for (int n = 0; n < num_nodes; n++) {
			Vector3 pos;
			Quaternion rotquat;

			if (n == 0) {
				pos = position;
				rotquat = rotation;
			}
			else {
				pos = (*it)->position;
				rotquat = (*it)->rotation;
				it++;
			}

			pos.write(file);
		
			Vector3 rot = rotquat.toLostWorldEuler();
			rot.write(file);

			// Local transform is for objects with parents - world position isn't used (except for range) and object is transformed relative to its parent
			ObjectElementTarget *parent_elem = (ObjectElementTarget*) getElement("Parent");

			if (parent_elem && parent_elem->value != 0) {
				Object *parent = level->getObjectByID(parent_elem->value);
				Vector3 parent_pos = parent->getPosition();
				Quaternion parent_rot = parent->getRotation();
				Quaternion inv_parent = parent_rot.inverse();

				pos = parent_rot.inverse() * (pos - parent_pos);
				pos.write(file);

				Quaternion parent_conjugate(parent_rot.w, -parent_rot.x, -parent_rot.y, -parent_rot.z);
				rot = (parent_conjugate * rotquat).toLostWorldEuler();
				rot.write(file);
			}

			else
				file->writeNull(0x18);
		}
	}

	void Object::recalcTransform(Level *level)
	{
		if (needs_transform_recalc) {
			// Reset the bool immediately to prevent potential recursion
			needs_transform_recalc = false;

			ObjectElementTarget *parent_elem = (ObjectElementTarget*) getElement("Parent");

			if (parent_elem && parent_elem->value != 0) {
				Object *parent = level->getObjectByID(parent_elem->value);
				parent->recalcTransform(level);

				// Recalculate position
				position = local_position;
				position = parent->getRotation() * position;
				position = position + parent->getPosition();
				
				// Recalculate rotation
				rotation = parent->getRotation() * local_rotation;

				// Recalculate MSP nodes
				multi_set_param.recalcTransform(parent);
			}
		}
	}
};