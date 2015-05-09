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

#include "Object.h"
#include "ObjectExtra.h"
#include "ObjectElement.h"
#include "ObjectLibrary.h"

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

	Object::Object(string nm) : name(nm), multi_set_param() {
		template_reference=NULL;
		position=Vector3();
		rotation=Quaternion();
		id=0;
		parent_set = NULL;
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
					element_sub->value = element_sub->value;
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
						else if ((sub_element_name==LIBGENS_MATH_AXIS_X_TEXT) || (sub_element_name==LIBGENS_MATH_AXIS_Y_TEXT) || (sub_element_name==LIBGENS_MATH_AXIS_Z_TEXT)) {
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
						if (first_value.size() && second_value.size() && operator_str.size()) {
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
							if (comparison_type == OBJECT_ELEMENT_BOOL) {
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
							}

							// Handle floats comparison
							if (comparison_type == OBJECT_ELEMENT_FLOAT) {
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
							}

							// Handle string comparison
							if (comparison_type == OBJECT_ELEMENT_STRING) {
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
};