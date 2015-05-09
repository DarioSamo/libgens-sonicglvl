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

#pragma once

namespace LibGens {
	enum ObjectElementType {
		OBJECT_ELEMENT_UNDEFINED,
		OBJECT_ELEMENT_BOOL,
		OBJECT_ELEMENT_INTEGER,
		OBJECT_ELEMENT_FLOAT,
		OBJECT_ELEMENT_STRING,
		OBJECT_ELEMENT_ID,
		OBJECT_ELEMENT_ID_LIST,
		OBJECT_ELEMENT_VECTOR,
		OBJECT_ELEMENT_VECTOR_LIST
	};

	class ObjectElement {
		protected:
			ObjectElementType type;
			string name;
			string description;
		public:
			ObjectElement() {
				type = OBJECT_ELEMENT_UNDEFINED;
				description = "";
				name = "";
			}

			void setName(string nm) {
				name = nm;
			}

			string getName() {
				return name;
			}

			void setDescription(string nm) {
				description = nm;
			}

			string getDescription() {
				return description;
			}

			ObjectElementType getType() {
				return type;
			}

			virtual void writeXML(TiXmlElement *root) {
			}

			virtual void writeXMLTemplate(TiXmlElement *root);
	};

	class ObjectElementBool : public ObjectElement {
		public:
			bool value;
			ObjectElementBool() {
				type=OBJECT_ELEMENT_BOOL;
				value=false;
			}

			void writeXML(TiXmlElement *root);
			void writeXMLTemplate(TiXmlElement *root);
	};

	class ObjectElementInteger : public ObjectElement {
		public:
			unsigned int value;
			ObjectElementInteger() {
				type=OBJECT_ELEMENT_INTEGER;
				value=false;
			}

			void writeXML(TiXmlElement *root);
			void writeXMLTemplate(TiXmlElement *root);
	};

	class ObjectElementFloat : public ObjectElement {
		public:
			float value;
			ObjectElementFloat() {
				type=OBJECT_ELEMENT_FLOAT;
				value=0.0;
			}

			void writeXML(TiXmlElement *root);
			void writeXMLTemplate(TiXmlElement *root);
	};

	class ObjectElementString : public ObjectElement {
		public:
			string value;
			ObjectElementString() {
				type=OBJECT_ELEMENT_STRING;
				value="";
			}

			void writeXML(TiXmlElement *root);
			void writeXMLTemplate(TiXmlElement *root);
	};

	class ObjectElementID : public ObjectElement {
		public:
			size_t value;
			ObjectElementID() {
				type=OBJECT_ELEMENT_ID;
				value=0;
			}

			void writeXML(TiXmlElement *root);
			void writeXMLTemplate(TiXmlElement *root);
	};

	class ObjectElementIDList : public ObjectElement {
		public:
			vector<size_t> value;
			ObjectElementIDList() {
				type=OBJECT_ELEMENT_ID_LIST;
			}

			void writeXML(TiXmlElement *root);
			void writeXMLTemplate(TiXmlElement *root);
	};

	class ObjectElementVector : public ObjectElement {
		public:
			Vector3 value;
			ObjectElementVector() {
				type=OBJECT_ELEMENT_VECTOR;
				value = Vector3();
			}

			void writeXML(TiXmlElement *root);
			void writeXMLTemplate(TiXmlElement *root);
	};

	class ObjectElementVectorList : public ObjectElement {
		public:
			vector<Vector3> value;
			ObjectElementVectorList() {
				type=OBJECT_ELEMENT_VECTOR_LIST;
			}

			void writeXML(TiXmlElement *root);
			void writeXMLTemplate(TiXmlElement *root);
	};

};