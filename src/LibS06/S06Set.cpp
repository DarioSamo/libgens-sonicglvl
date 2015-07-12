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

#include "LibGens.h"
#include "S06Set.h"

namespace LibGens {
	SonicSet::SonicSet(string filename) {
		File file(filename, LIBGENS_FILE_READ_BINARY);

		if (file.valid()) {
			read(&file);
			file.close();
		}
	}

	SonicSetObjectParameter::SonicSetObjectParameter(SonicSetObjectParameter *clone) {
		value_f = clone->value_f;
		value_s = clone->value_s;
		value_i = clone->value_i;
		value_v = clone->value_v;
		type    = clone->type;
	}

	void SonicSetObjectParameter::read(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_S06_SET_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		size_t header_address=file->getCurrentAddress();
		size_t address=0;
		unsigned int total=0;

		file->readInt32BE(&type);

		// Boolean
		if (type == 0) {
			file->readInt32BE(&value_i);
		}
		// Integer
		else if (type == 1) {
			file->readInt32BE(&value_i);
		}
		// Float
		else if (type == 2) {
			file->readFloat32BE(&value_f);
		}
		// Read string
		else if (type == 3) {
			size_t offset_address=file->getCurrentAddress();
			file->readInt32BEA(&address);
			file->readInt32BE(&total);
			file->goToAddress(address);
			file->readString(&value_s);
		}
		// Vector3
		else if (type == 4) {
			value_v.read(file);
		}
		// Target another object
		else if (type == 6) {
			file->readInt32BE(&value_i);
		}
	}

	
	void SonicSetObjectParameter::write(File *file, SonicStringTable *string_table) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_S06_SET_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}
		
		size_t header_address=file->getCurrentAddress();
		file->writeInt32BE(&type);

		if (type == 0) {
			file->writeInt32BE(&value_i);
			file->writeNull(12);
		}
		else if (type == 1) {
			file->writeInt32BE(&value_i);
			file->writeNull(12);
		}
		else if (type == 2) {
			file->writeFloat32BE(&value_f);
			file->writeNull(12);
		}
		else if (type == 3) {
			string_table->writeString(file, value_s);
			unsigned int total=1;
			unsigned int size=value_s.size()+1;
			file->writeInt32BE(&total);
			file->writeNull(4);
			file->writeInt32BE(&size);
		}
		else if (type == 4) {
			value_v.write(file);
			file->writeNull(4);
		}
		else if (type == 6) {
			file->writeInt32BE(&value_i);
			file->writeNull(12);
		}
	}


	SonicSetObject::SonicSetObject(SonicSetObject *clone) {
		position = clone->position;
		rotation = clone->rotation;
		name = clone->name;
		type = clone->type;
		unknown = clone->unknown;
		unknown_2 = clone->unknown_2;

		for (size_t i=0; i<clone->parameters.size(); i++) {
			SonicSetObjectParameter *parameter = new SonicSetObjectParameter(clone->parameters[i]);
			parameters.push_back(parameter);
		}
	}

	
	void SonicSetObject::read(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_S06_SET_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		size_t header_address=file->getCurrentAddress();
		size_t address=0;

		size_t name_1_address=0;
		size_t name_2_address=0;
		unsigned int parameter_total=0;
		size_t parameter_address=0;

		file->readInt32BEA(&name_1_address);
		size_t name_2_offset_address=file->getCurrentAddress();
		file->readInt32BEA(&name_2_address);
		file->readFloat32BE(&unknown);
		file->moveAddress(12);
		position.read(file);
		file->readFloat32BE(&unknown_2);
		rotation.read(file);
		file->readInt32BE(&parameter_total);

		if (parameter_total) {
			file->readInt32BEA(&parameter_address);
		}

		file->goToAddress(name_1_address);
		file->readString(&name);
		file->goToAddress(name_2_address);
		file->readString(&type);

		if (!parameter_total) {

		}
		else {

		}

		for (size_t i=0; i<parameter_total; i++) {
			file->goToAddress(parameter_address + i*20);

			SonicSetObjectParameter *parameter = new SonicSetObjectParameter();
			parameter->read(file);
			parameters.push_back(parameter);
		}
	}


	
	void SonicSetObject::write(File *file, SonicStringTable *string_table) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_S06_SET_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}
		
		size_t header_address=file->getCurrentAddress();
		file_address = header_address;

		string_table->writeString(file, name);
		string_table->writeString(file, type);
		file->writeFloat32BE(&unknown);
		file->writeNull(12);
		position.write(file);
		file->writeFloat32BE(&unknown_2);
		rotation.write(file);
		unsigned int parameter_total=parameters.size();
		file->writeInt32BE(&parameter_total);
		file->writeNull(4);
	}

	void SonicSetObject::writeFixed(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_S06_SET_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}

		file->goToAddress(file_address+60);

		if (parameters.size() > 0) {
			file->writeInt32BEA(&parameter_address);
		}
	}



	void SonicSetGroup::read(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_S06_SET_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		size_t header_address=file->getCurrentAddress();

		size_t name_address=0;
		size_t type_address=0;
		unsigned int values_total=0;
		size_t values_address=0;

		file->readInt32BEA(&name_address);
		file->readInt32BEA(&type_address);
		file->readInt32BE(&values_total);
		file->readInt32BEA(&values_address);

		file->goToAddress(name_address);
		file->readString(&name);

		file->goToAddress(type_address);
		file->readString(&type);

		for (unsigned int i=0; i<values_total; i++) {
			unsigned int value=0;
			file->goToAddress(values_address + i*8 + 4);
			file->readInt32BE(&value);
			values.push_back(value);
		}

		Error::printfMessage(Error::LOG, "%s - %s:", name.c_str(), type.c_str());
	}

	
	
	void SonicSetGroup::write(File *file, SonicStringTable *string_table) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_S06_SET_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}
		
		size_t header_address=file->getCurrentAddress();
		file_address = header_address;

		string_table->writeString(file, name);
		string_table->writeString(file, type);
		unsigned int values_total=values.size();
		file->writeInt32BE(&values_total);
		file->writeNull(4);
	}

	void SonicSetGroup::writeValues(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_S06_SET_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}
		
		parameter_address = file->getCurrentAddress();
		for (size_t i=0; i<values.size(); i++) {
			unsigned int value=values[i];
			file->writeNull(4);
			file->writeInt32BE(&value);
		}
	}

	void SonicSetGroup::writeFixed(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_S06_SET_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}

		file->goToAddress(file_address + 12);

		if (values.size()) {
			file->writeInt32BEA(&parameter_address);
		}
	}


	void SonicSet::read(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_S06_SET_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		size_t header_address=file->getCurrentAddress();
		size_t address=0;

		
		unsigned int file_size=0;
		size_t banana_table_address=0;
		file->setRootNodeAddress(32);
		file->readInt32BE(&file_size);
		file->readInt32BEA(&banana_table_address);
		file->readInt32BE(&table_size);

		file->goToAddress(44);
		file->readString(&name);

		file->goToAddress(76);

		unsigned int object_total=0;
		size_t object_address=0;
		file->readInt32BE(&object_total);
		file->readInt32BEA(&object_address);

		unsigned int group_total=0;
		size_t group_address=0;
		file->readInt32BE(&group_total);
		file->readInt32BEA(&group_address);

		for (size_t i=0; i<object_total; i++) {
			file->goToAddress(object_address + i*64);
			SonicSetObject *object=new SonicSetObject();
			object->read(file);
			objects.push_back(object);
		}


		for (size_t i=0; i<group_total; i++) {
			file->goToAddress(group_address + i*16);
			SonicSetGroup *group=new SonicSetGroup();
			group->read(file);
			groups.push_back(group);

			vector<unsigned int> values = group->getValues();

			for (size_t j=0; j<values.size(); j++) {
				Error::printfMessage(Error::LOG, "  %d: %s", values[j], objects[values[j]]->getName().c_str());
			}
		}

		file->goToAddress(banana_table_address);
		file->readAddressTableBBIN(table_size);
	}

	
	void SonicSet::save(string filename) {
		File file(filename, LIBGENS_FILE_WRITE_BINARY);

		if (file.valid()) {
			write(&file);
			file.close();
		}
	}

	void SonicSet::write(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_S06_SET_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}

		string_table.clear();

		file->setRootNodeAddress(32);

		file->writeNull(22);
		string header="1BBINA";
		file->writeString(&header);
		file->fixPadding(44);
		file->writeString(&name);
		file->fixPadding(76);

		unsigned int object_total=objects.size();
		file->writeInt32BE(&object_total);
		size_t object_address=file->getCurrentAddress();
		file->writeNull(4);

		unsigned int group_total=groups.size();
		file->writeInt32BE(&group_total);
		size_t group_address=file->getCurrentAddress();
		file->writeNull(4);

		size_t object_table_address=file->getCurrentAddress();
		for (size_t i=0; i<object_total; i++) objects[i]->write(file, &string_table);
		for (size_t i=0; i<object_total; i++) {
			vector<SonicSetObjectParameter *> parameters=objects[i]->getParameters();
			objects[i]->setAddress(file->getCurrentAddress());
			for (size_t j=0; j<parameters.size(); j++) {
				parameters[j]->write(file, &string_table);
			}
		}
		for (size_t i=0; i<object_total; i++) objects[i]->writeFixed(file);
		file->goToEnd();

		size_t group_table_address=file->getCurrentAddress();
		for (size_t i=0; i<group_total; i++) groups[i]->write(file, &string_table);
		for (size_t i=0; i<group_total; i++) groups[i]->writeValues(file);
		for (size_t i=0; i<group_total; i++) groups[i]->writeFixed(file);
		file->goToEnd();

		string_table.write(file);
		file->fixPadding(4);

		file->goToAddress(object_address);
		file->writeInt32BEA(&object_table_address);

		file->goToAddress(group_address);
		if (group_total) {
			file->writeInt32BEA(&group_table_address);
		}
		
		file->goToEnd();
		size_t table_address=file->getCurrentAddress() - 32;
		file->sortAddressTable();

		list<size_t> file_address_table = file->getAddressTable();
		file->writeAddressTableBBIN();
		file->fixPadding(8);

		unsigned int file_size=file->getFileSize();
		unsigned int table_size = (file_size - 32) - table_address;
		file->goToAddress(0);
		file->writeInt32BE(&file_size);
		file->writeInt32BE(&table_address);
		file->writeInt32BE(&table_size);
	}

	void SonicSet::fixDuplicateNames() {
		for (size_t i=0; i<objects.size(); i++) {
			bool found=true;
			string object_name = objects[i]->getName();
			size_t index = 0;

			while (found) {
				found = false;
				for (size_t j=0; j<i; j++) {
					if (objects[j]->getName() == object_name) {
						found = true;
						break;
					}
				}

				if (!found) {
					objects[i]->setName(object_name);
				}
				else {
					object_name = objects[i]->getType() + ToString(index);
					index++;
				}
			}
		}
	}
};