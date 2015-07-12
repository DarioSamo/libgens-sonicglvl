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

#include "S06Common.h"

#define LIBGENS_S06_SET_ERROR_MESSAGE_NULL_FILE       "Trying to read set data from unreferenced file."
#define LIBGENS_S06_SET_ERROR_MESSAGE_WRITE_NULL_FILE "Trying to write set data to an unreferenced file."

#define LIBGENS_S06_SET_OFFSET_ROOT                   0x4C
#define LIBGENS_S06_SET_OFFSET_STRING_OBJECT          0x41
#define LIBGENS_S06_SET_OFFSET_PARAMETER_OBJECT       0x4E

#define LIBGENS_S06_SET_PARAMETER_BOOLEAN             0
#define LIBGENS_S06_SET_PARAMETER_INTEGER             1
#define LIBGENS_S06_SET_PARAMETER_FLOAT               2
#define LIBGENS_S06_SET_PARAMETER_STRING              3
#define LIBGENS_S06_SET_PARAMETER_VECTOR3             4
#define LIBGENS_S06_SET_PARAMETER_ID                  6


namespace LibGens {
	class SonicSetObjectParameter {
		friend SonicSetObjectParameter;

		protected:
			unsigned int type;
			float value_f;
			string value_s;
			unsigned int value_i;
			Vector3 value_v;
		public:
			SonicSetObjectParameter() {
			}

			SonicSetObjectParameter(SonicSetObjectParameter *clone);

			void read(File *file);

			void write(File *file, SonicStringTable *string_table);

			unsigned int getType() {
				return type;
			}

			void setValueInt(unsigned int v) {
				value_i = v;
			}

			unsigned int getValueInt() {
				return value_i;
			}

			float getValueFloat() {
				return value_f;
			}

			string getValueString() {
				return value_s;
			}

			bool getValueBool() {
				return value_i;
			}

			Vector3 getValueVector() {
				return value_v;
			}
	};

	class SonicSetObject {
		friend SonicSetObject;

		protected:
			Vector3 position;
			Quaternion rotation;

			string name;
			string type;

			float unknown;
			float unknown_2;
			vector<SonicSetObjectParameter *> parameters;

			size_t file_address;
			size_t parameter_address;
		public:
			SonicSetObject() {
			}

			void read(File *file);
			SonicSetObject(SonicSetObject *clone);

			vector<SonicSetObjectParameter *> getParameters() {
				return parameters;
			}

			void setAddress(size_t v) {
				parameter_address = v;
			}

			void write(File *file, SonicStringTable *string_table);
			void writeFixed(File *file);

			void setPosition(Vector3 v) {
				position=v;
			}

			Vector3 getPosition() {
				return position;
			}

			void setRotation(Quaternion v) {
				rotation=v;
			}

			Quaternion getRotation() {
				return rotation;
			}

			string getType() {
				return type;
			}

			string getName() {
				return name;
			}

			void setName(string v) {
				name = v;
			}

			float getUnknown() {
				return unknown;
			}

			float getUnknown2() {
				return unknown_2;
			}
	};

	class SonicSetGroup {
		protected:
			string name;
			string type;
			vector<unsigned int> values;

			size_t file_address;
			size_t parameter_address;
		public:
			SonicSetGroup() {
			}
			void read(File *file);
			void write(File *file, SonicStringTable *string_table);
			void writeValues(File *file);
			void writeFixed(File *file);

			void clearValues() {
				values.clear();
			}

			vector<unsigned int> getValues() {
				return values;
			}
	};


	class SonicSet {
		protected:
			vector<SonicSetObject *> objects;
			vector<SonicSetGroup *>  groups;
			unsigned int table_size;
			string name;
			SonicStringTable string_table;
		public:
			SonicSet() {

			}

			SonicSet(string filename);
			void read(File *file);
			void save(string filename);
			void write(File *file);

			void setName(string v) {
				name = v;
			}

			vector<SonicSetObject *> getObjects() {
				return objects;
			}

			void addObject(SonicSetObject *object) {
				if (!object) return;

				objects.push_back(object);
			}

			void deleteObjects() {
				for (size_t i=0; i<objects.size(); i++) {
					delete objects[i];
				}

				objects.clear();
			}

			void fixDuplicateNames();

			void clearGroupIDs() {
				for (size_t i=0; i<groups.size(); i++) {
					groups[i]->clearValues();
				}
			}

			void deleteGroups() {
				for (size_t i=0; i<groups.size(); i++) {
					delete groups[i];
				}
				groups.clear();
			}
	};
};
