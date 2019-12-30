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

#pragma once

#define LIBGENS_OBJECT_H_ERROR_READ_TEMPLATE_BEFORE   "The template "
#define LIBGENS_OBJECT_H_ERROR_READ_TEMPLATE_AFTER    " has no valid elements."
#define LIBGENS_OBJECT_H_ERROR_READ_SET_BEFORE        "The set file  "
#define LIBGENS_OBJECT_H_ERROR_READ_SET_AFTER         " has no valid elements."
#define LIBGENS_OBJECT_H_ERROR_FIND_TEMPLATE          "Couldn't find the requested template in the object library: "
#define LIBGENS_OBJECT_H_ERROR_WRITE_SET_XML          "Couldn't write the object set to the following file: "
#define LIBGENS_OBJECT_H_ERROR_WRITE_TEMPLATE_XML     "Couldn't write the object template to the following file: "
#define LIBGENS_OBJECT_H_ERROR_NO_CATEGORIES          "This library has no available categories. It can't auto-learn templates without a category available."

#define LIBGENS_OBJECT_ELEMENT_UNDEFINED_TEMPLATE     "undefined"
#define LIBGENS_OBJECT_ELEMENT_BOOL_TEMPLATE          "bool"
#define LIBGENS_OBJECT_ELEMENT_INTEGER_TEMPLATE		  "integer"
#define LIBGENS_OBJECT_ELEMENT_FLOAT_TEMPLATE		  "float"
#define LIBGENS_OBJECT_ELEMENT_STRING_TEMPLATE		  "string"
#define LIBGENS_OBJECT_ELEMENT_ID_TEMPLATE			  "id"
#define LIBGENS_OBJECT_ELEMENT_ID_LIST_TEMPLATE		  "id_list"
#define LIBGENS_OBJECT_ELEMENT_VECTOR_TEMPLATE		  "vector"
#define LIBGENS_OBJECT_ELEMENT_VECTOR_LIST_TEMPLATE   "vector_list"

#define LIBGENS_OBJECT_ELEMENT_SINT8_TEMPLATE         "sint8"
#define LIBGENS_OBJECT_ELEMENT_UINT8_TEMPLATE         "uint8"
#define LIBGENS_OBJECT_ELEMENT_SINT16_TEMPLATE        "sint16"
#define LIBGENS_OBJECT_ELEMENT_UINT16_TEMPLATE        "uint16"
#define LIBGENS_OBJECT_ELEMENT_SINT32_TEMPLATE        "sint32"
#define LIBGENS_OBJECT_ELEMENT_UINT32_TEMPLATE        "uint32"
#define LIBGENS_OBJECT_ELEMENT_ENUM_TEMPLATE          "enum"
#define LIBGENS_OBJECT_ELEMENT_TARGET_TEMPLATE        "target"
#define LIBGENS_OBJECT_ELEMENT_POSITION_TEMPLATE      "position"
#define LIBGENS_OBJECT_ELEMENT_VECTOR3_TEMPLATE       "vector3"
#define LIBGENS_OBJECT_ELEMENT_UINT32ARRAY_TEMPLATE   "uint32array"

#define LIBGENS_OBJECT_ELEMENT_POSITION               "Position"
#define LIBGENS_OBJECT_ELEMENT_POSITION_STR_SIZE      8
#define LIBGENS_OBJECT_ELEMENT_ROTATION               "Rotation"
#define LIBGENS_OBJECT_ELEMENT_SET_ID                 "SetObjectID"

#define LIBGENS_OBJECT_ELEMENT_MULTI_SET_PARAM                  "MultiSetParam"
#define LIBGENS_OBJECT_ELEMENT_MULTI_SET_PARAM_ELEMENT          "Element"
#define LIBGENS_OBJECT_ELEMENT_MULTI_SET_PARAM_INDEX            "Index"
#define LIBGENS_OBJECT_ELEMENT_MULTI_SET_PARAM_BASE_LINE        "BaseLine"
#define LIBGENS_OBJECT_ELEMENT_MULTI_SET_PARAM_COUNT            "Count"
#define LIBGENS_OBJECT_ELEMENT_MULTI_SET_PARAM_DIRECTION        "Direction"
#define LIBGENS_OBJECT_ELEMENT_MULTI_SET_PARAM_INTERVAL         "Interval"
#define LIBGENS_OBJECT_ELEMENT_MULTI_SET_PARAM_INTERVAL_BASE    "IntervalBase"
#define LIBGENS_OBJECT_ELEMENT_MULTI_SET_PARAM_POSITION_BASE    "PositionBase"
#define LIBGENS_OBJECT_ELEMENT_MULTI_SET_PARAM_ROTATION_BASE    "RotationBase"

#define LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_TYPE         "type"
#define LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_NAME         "name"
#define LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DEFAULT      "default"
#define LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_DESCRIPTION  "description"
#define LIBGENS_OBJECT_ELEMENT_BOOL_TRUE              "true"
#define LIBGENS_OBJECT_ELEMENT_BOOL_FALSE             "false"
#define LIBGENS_OBJECT_ELEMENT_EXTRA                  "Extra"

#define LIBGENS_OBJECT_TEMPLATE_ROOT                  "Template"
#define LIBGENS_OBJECT_SET_ROOT                       "SetObject"
#define LIBGENS_OBJECT_SET_LAYER_DEFINE               "LayerDefine"

#define LIBGENS_OBJECT_TEMPLATE_EXTENSION             ".xml"
#define LIBGENS_OBJECT_SET_NAME                       "setdata_"
#define LIBGENS_OBJECT_SET_EXTENSION                  ".set.xml"

#define LIBGENS_OBJECT_EXTRA_TYPE_MODEL               "model"
#define LIBGENS_OBJECT_EXTRA_TYPE_SKELETON            "skeleton"
#define LIBGENS_OBJECT_EXTRA_TYPE_ANIMATION           "animation"
#define LIBGENS_OBJECT_EXTRA_CONDITION                "condition"

#define LIBGENS_SPAWN_POINT_OBJECT_NAME      "SonicSpawn"
#define LIBGENS_SPAWN_POINT_OBJECT_FLAG      "Active"

#define LIBGENS_LIBRARY_ERROR_FILE        "No valid library file found: "
#define LIBGENS_LIBRARY_ERROR_FILE_ROOT   "Library file doesn't have a valid root."
#define LIBGENS_LIBRARY_ENTRY             "Entry"
#define LIBGENS_LIBRARY_NAME_ATTRIBUTE    "name"
#define LIBGENS_LIBRARY_FOLDER_ATTRIBUTE  "folder"
#define LIBGENS_LIBRARY_ROOT              "LevelDatabase"


namespace LibGens {
	class Object;

	class MultiSetNode {
		public:
			Vector3 position;
			Quaternion rotation;

			// Local transform is used for Lost World objects with parents; see below for the reason for including them
			Vector3 local_position;
			Quaternion local_rotation;

			MultiSetNode() : position(), rotation() {

			}

			void readXML(TiXmlElement *root);
			void writeXML(TiXmlElement *root, size_t index);
			void readORC(File *file);
			void recalcTransform(Object *parent);
	};

	class MultiSetParam {
		protected:
			list<MultiSetNode *> nodes;

			float base_line;
			float direction;
			float interval;
			float interval_base;
			float position_base;
			float rotation_base;
		public:
			MultiSetParam() {
				clear();
			}

			void clear() {
				base_line = 1.0;
				direction = 0.0;
				interval = 1.0;
				interval_base = 0.0;
				position_base = 0.0;
				rotation_base = 0.0;
			}

			void addNode(MultiSetNode *node) {
				if (!node) {
					return;
				}

				nodes.push_back(node);
			}

			void removeNode(MultiSetNode* node)
			{
				for (list<MultiSetNode*>::iterator it = nodes.begin(); it != nodes.end(); ++it)
					if ((*it) == node)
					{
						nodes.erase(it);
						delete (*it);
						return;
					}
			}

			void removeAllNodes()
			{
				nodes.clear();
			}

			list<MultiSetNode *> getNodes() {
				return nodes;
			}

			void readXML(TiXmlElement *root);

			void writeXML(TiXmlElement *root);

			void readORC(File *file, unsigned int count);

			void recalcTransform(Object *parent);

			void setDirection(float direction);
			void setInterval(float interval);
			void setPositionBase(float positionBase);
			void setRotationBase(float rotationBase);

			size_t getSize() {
				return nodes.size();
			}
	};

	class ObjectElement;
	class ObjectExtra;
	class ObjectSet;
	class ObjectLibrary;
	class Level;

	class Object {
		protected:
			list<ObjectElement *> elements;
			list<ObjectExtra *> extras;
			MultiSetParam multi_set_param;
			string name;
			Object *template_reference;
			Vector3 position;
			Quaternion rotation;
			size_t id;
			ObjectSet *parent_set;
			
			// Local Transform is used for Lost World levels - only preent so that in the event that
			// two objects are parented to each other, the generated world position can fall back to
			// the file's world position rather than 0,0,0 or something
			Vector3 local_position;
			Quaternion local_rotation;
			bool needs_transform_recalc; //  used for objects with parents in SLW
			unsigned int orc_offset; // used for orc saving. not the best solution, but whatever

		public:
			Object(string nm);
			Object(Object *obj);
			string getName();
			size_t getID();
			void setID(size_t v);
			void setPosition(Vector3 v);
			Vector3 getPosition();
			void setRotation(Quaternion v);
			Quaternion getRotation();
			list<ObjectElement *> getElements();
			ObjectElement *getElement(string nm);
			list<ObjectExtra *> getExtras();
			void deleteExtras();
			string queryExtraName(string type, string def="");
			string queryEditorValue(string value_type, string slot_id, string default_value);
			string queryEditorModel(string slot_id, string default_value);
			string queryEditorSkeleton(string slot_id, string default_value);
			string queryEditorAnimation(string slot_id, string default_value);
			ObjectElement *cloneElement(ObjectElement *element);
			ObjectExtra *cloneExtra(ObjectExtra *extra);
			void readXML(TiXmlElement *root);
			void readXMLTemplate(string filename);
			void readXMLTemplateElement(TiXmlElement *root);
			void writeXML(TiXmlElement *root);
			void saveXMLTemplate(string filename);
			void learnFromObject(Object *object);
			void learnFromLibrary(ObjectLibrary *library);
			void setParentSet(ObjectSet *v);
			ObjectSet *getParentSet();
			MultiSetParam *getMultiSetParam();

			void readORC(File *file);
			void writeORC(File *file, vector<unsigned int>& offset_vector);
			void writeUnitsORC(File *file, Level *level);
			void recalcTransform(Level *level);
	};
};

