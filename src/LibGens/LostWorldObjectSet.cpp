#include "LostWorldObjectSet.h"
#include "Object.h"
#include "ObjectLibrary.h"
#include "Level.h"
#include "StringTable.h"

namespace LibGens {
	LostWorldObjectSet::LostWorldObjectSet(string filename_p, ObjectLibrary *library) : ObjectSet()
	{
		filename = filename_p;
		need_update = false;
		
		name = filename;
		size_t sep = name.find_last_of("\\/");
		if (sep != std::string::npos) {
			name = name.substr(sep + 1, name.size() - sep - 1);
		}
	
		size_t dot = name.find_last_of(".");
		if (dot != string::npos) {
			name = name.substr(0, dot);
		}
		
		File file(filename_p, LIBGENS_FILE_READ_BINARY, true);
		cout << "Reading " << filename_p << "\n";

		if (file.valid())	
			readORC(&file, library);
	}

	LostWorldObjectSet::LostWorldObjectSet() : ObjectSet()
	{
	}

	void LostWorldObjectSet::readORC(File *file, ObjectLibrary *library)
	{
		size_t BINA;
		size_t endian_flag;
		file->readInt32BE(&BINA);
		file->readInt32BE(&endian_flag);
		if (endian_flag == 0x3230304C) file->setBigEndian(false);

		file->setRootNodeAddress(0x40);
		file->goToAddress(0x48);
		
		// SOBJ header
		unsigned int numObjTypes, numObjects;
		size_t typeTableAddress, objOffsetsAddress;
		file->readInt32E(&numObjTypes);
		file->readInt32EA(&typeTableAddress);
		file->moveAddress(4);
		file->readInt32EA(&objOffsetsAddress);
		file->readInt32E(&numObjects);

		file->goToAddress(0x64);

		// Object types
		struct objType {
			string name;
			unsigned int count;
			vector<unsigned short> indices;
		};
		vector<objType> objTypes;
		objTypes.reserve(numObjTypes);

		for (int t = 0; t < numObjTypes; t++) {
			objType type;

			size_t nameOffset, indicesOffset;
			file->readInt32EA(&nameOffset);
			file->readInt32E(&type.count);
			file->readInt32EA(&indicesOffset);
			
			size_t curAddress = file->getCurrentAddress();
			file->goToAddress(nameOffset);
			file->readString(&type.name);
			file->goToAddress(indicesOffset);

			type.indices.resize(type.count);
			for (int o = 0; o < type.count; o++)
				file->readInt16E(&type.indices[o]);

			file->goToAddress(curAddress);
			objTypes.push_back(type);
		}

		// Object offsets
		vector<size_t> objOffsets(numObjects);
		file->goToAddress(objOffsetsAddress);

		for (int o = 0; o < numObjects; o++)
			file->readInt32EA(&objOffsets[o]);

		// Objects
		objects.resize(numObjects, 0);

		for (int t = 0; t < numObjTypes; t++)
		{
			objType& type = objTypes[t];
			Object *temp = library->getTemplate(type.name);

			if (!temp)
			{
				cout << "orc requested object that has no template: " << type.name << "\n";
				continue;
			}

			for (int o = 0; o < type.count; o++)
			{
				unsigned short index = type.indices[o];
				size_t offset = objOffsets[index];
				file->goToAddress(offset);

				Object *obj = new Object(temp);
				obj->readORC(file);
				obj->setParentSet(this);

				list<Object*>::iterator it = objects.begin();
				it = std::next(it, index);
				*it = obj;
			}
		}

		// Hack - erase invalid objects (ie objects with missing templates)
		// I do this because I'm trying to preserve the order objects are in in the source file
		// but having null objects in the list causes crashes
		for (list<Object*>::iterator it = objects.begin(); it != objects.end(); it++) {
			if (!*it) {
				it = objects.erase(it);
				it--;
			}
		}
	}

	void LostWorldObjectSet::saveORC(Level *level, bool PC)
	{
		File file(filename, LIBGENS_FILE_WRITE_BINARY, true);

		if (!file.valid()) cout << "Couldn't save " << filename << "\n";

		else
		{
			cout << "Saving " << filename << "\n";

			// Header
			int header_start = file.getCurrentAddress();
			unsigned int magicA = 0x42494E41;
			unsigned int magicB = PC ? 0x3230304C : 0x32303042;
			file.setBigEndian(!PC);

			unsigned short unk1 = 0x1;
			unsigned int DATA = 0x44415441;
			unsigned short unk2 = 0x18;
			unsigned int filler32 = 0;
			unsigned short filler16 = 0;

			file.writeInt32BE(&magicA);
			file.writeInt32BE(&magicB);
			file.writeNull(4);
			file.writeInt16E(&unk1);
			file.writeInt16E(&filler16);
			file.writeInt32BE(&DATA);
			file.writeNull(0x10);
			file.writeInt16E(&unk2);
			file.writeInt16E(&filler16);
			file.writeNull(0x18);

			file.setRootNodeAddress(file.getCurrentAddress());

			// Create object type list
			unsigned int num_nodes = 0;

			struct objType {
				string name;
				int num;
				vector<unsigned short> indices;
				int offset;
				int indicesOffset;

				bool operator<(const objType& other) {
					return (name < other.name);
				}
			};
			vector<objType> types;

			unsigned short obj_num = 0;
			for (list<Object*>::iterator o = objects.begin(); o != objects.end(); o++) {
				Object *obj = *o;
				string name = obj->getName();

				bool new_type = true;

				for (int t = 0; t < types.size(); t++) {
					objType *type = &types[t];

					if (type->name == name) {
						type->num++;
						type->indices.push_back(obj_num);
						new_type = false;
						break;
					}
				}

				if (new_type) {
					objType type;
					type.name = name;
					type.num = 1;
					type.indices.push_back(obj_num);
					types.push_back(type);
				}

				num_nodes += obj->getMultiSetParam()->getSize() + 1;
				obj_num++;
			}

			std::sort(types.begin(), types.end());

			// SOBJ
			vector<unsigned int> offsets;

			unsigned int v = 0x534F424A; // "SOBJ"
			unsigned int unk3 = 1;
			unsigned int num_types = types.size();
			unsigned int type_table_offset = 0x24;
			unsigned int unk4 = 0;
			unsigned int num_objects = objects.size();

			file.writeInt32E(&v);
			file.writeInt32E(&unk3);
			file.writeInt32E(&num_types);
			offsets.push_back(file.getCurrentAddress());
			file.writeInt32E(&type_table_offset);
			file.writeInt32E(&unk4);
			offsets.push_back(file.getCurrentAddress());
			file.writeNull(4);
			file.writeInt32E(&num_objects);
			file.writeNull(4);
			file.writeInt32E(&num_nodes);

			// Type Table
			GensStringTable table;

			for (int t = 0; t < types.size(); t++) {
				objType *type = &types[t];
				offsets.push_back(file.getCurrentAddress());
				type->offset = file.getCurrentAddress();
				table.writeString(&file, type->name);
				unsigned int uintnum = type->num;
				file.writeInt32E(&uintnum);
				offsets.push_back(file.getCurrentAddress());
				file.writeNull(4);
			}

			// Object Indices
			for (int t = 0; t < types.size(); t++) {
				objType *type = &types[t];

				unsigned int cur = file.getCurrentAddress();
				file.goToAddress(type->offset);
				file.moveAddress(8);
				file.writeInt32EA(&cur);
				file.goToAddress(cur);

				for (int i = 0; i < type->num; i++)
					file.writeInt16E(&type->indices[i]);

				file.fixPadding(4);
			}

			// Object Offsets
			unsigned int objOffsetsStart = file.getCurrentAddress();
			for (int o = 0; o < objects.size(); o++) {
				offsets.push_back(file.getCurrentAddress());
				file.writeNull(4);
			}
			
			// Objects
			vector<unsigned int> obj_offsets(objects.size());
			
			obj_num = 0;
			for (list<Object*>::iterator o = objects.begin(); o != objects.end(); o++) {
				file.fixPadding(16);
				obj_offsets[obj_num] = file.getCurrentAddress();
				Object *obj = *o;
				obj->writeORC(&file, offsets);
				obj_num++;
			}

			file.fixPadding(4);

			// Units
			for (list<Object*>::iterator o = objects.begin(); o != objects.end(); o++) {
				Object *obj = *o;
				obj->writeUnitsORC(&file, level);
			}
			file.fixPadding(4);

			// Strings
			unsigned int strings_start = file.getCurrentAddress();
			table.write(&file);
			file.fixPadding(4);

			// Offset table
			unsigned int offset_table_start = file.getCurrentAddress();

			unsigned int last = file.getRootNodeAddress();

			for (int o = 0; o < offsets.size(); o++) {
				unsigned int val = offsets[o] - last;

				if (val <= 0xFC) {
					unsigned char v = 0x40 | ((val >> 2) & 0xFF);
					file.write(&v, 1);
				}

				else if (val <= 0xFFFC) {
					unsigned short v = 0x8000 | ((val >> 2) & 0xFFFF);
					file.writeInt16BE(&v);
				}

				else {
					unsigned int v = 0xC0000000 | (val >> 2);
					file.writeInt32BE(&v);
				}

				last = offsets[o];
			}

			file.fixPadding(4);

			// Sizes
			unsigned int file_size = file.getCurrentAddress();
			unsigned int data_size = file.getCurrentAddress() - 0x10;
			unsigned int sobj_size = strings_start - 0x40;
			unsigned int strings_size = offset_table_start - strings_start;
			unsigned int offsets_size = file_size - offset_table_start;

			file.goToAddress(8);
			file.writeInt32E(&file_size);
			file.goToAddress(0x14);
			file.writeInt32E(&data_size);
			file.writeInt32E(&sobj_size);
			file.writeInt32E(&strings_size);
			file.writeInt32E(&offsets_size);

			// Offsets
			file.goToAddress(0x54);
			file.writeInt32EA(&objOffsetsStart);

			file.goToAddress(objOffsetsStart);

			for (int o = 0; o < objects.size(); o++)
				file.writeInt32EA(&obj_offsets[o]);

			// Done
			file.close();
		}
	}
	
	void LostWorldObjectSet::fixTransform(Level *level)
	{
		for (list<Object*>::iterator it = objects.begin(); it != objects.end(); it++)
			(*it)->recalcTransform(level);
	}
}