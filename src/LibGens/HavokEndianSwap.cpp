#include "HavokEndianSwap.h"
#include "Endian.h"

namespace {
	enum {
		TYPE_VOID,
		TYPE_BOOL,
		TYPE_CHAR,
		TYPE_INT8,
		TYPE_UINT8,
		TYPE_INT16,
		TYPE_UINT16,
		TYPE_INT32,
		TYPE_UINT32,
		TYPE_INT64,
		TYPE_UINT64,
		TYPE_REAL,
		TYPE_VECTOR4,
		TYPE_QUATERNION,
		TYPE_MATRIX3,
		TYPE_ROTATION,
		TYPE_QSTRANSFORM,
		TYPE_MATRIX4,
		TYPE_TRANSFORM,
		TYPE_ZERO,
		TYPE_POINTER,
		TYPE_FUNCTION_POINTER,
		TYPE_ARRAY,
		TYPE_INPLACE_ARRAY,
		TYPE_ENUM,
		TYPE_STRUCT,
		TYPE_SIMPLE_ARRAY,
		TYPE_HOMOGENEOUS_ARRAY,
		TYPE_VARIANT,
		TYPE_CSTRING,
		TYPE_ULONG,
		TYPE_FLAGS,
		TYPE_HALF,
		TYPE_STRING_POINTER
	};

	class HavokClassName {
	public:
		unsigned int tag;
		string name;

		HavokClassName() {}
	};

	class HavokTypeMember {
	public:
		unsigned char tag[2];
		unsigned short array_size;
		unsigned short struct_type;
		unsigned short offset;
		unsigned int structure_address;
		string name;
		string structure;

		HavokTypeMember() {}
	};

	class HavokEnum {
	public:
		unsigned int id;
		string name;
	};

	class HavokType {
	public:
		unsigned int object_size;
		string name;
		string class_name;
		unsigned int described_version;
		unsigned int num_implemented_interfaces;
		unsigned int declared_enums;
		unsigned int address;

		vector<HavokTypeMember> members;
		vector<HavokEnum> enums;
		vector<vector<HavokEnum>> sub_enums;
		vector<string> sub_enum_names;

		HavokType* parent;
		unsigned int parent_address;

		void reset() {
			members.clear();
			enums.clear();
			sub_enums.clear();
			sub_enum_names.clear();
			name = "";

			parent = NULL;
			parent_address = 0;
		}
	};

	class HavokLink {
	public:
		unsigned int type;
		unsigned int address_1;
		unsigned int address_2;
		HavokType* type_parent;
		HavokType* type_node;
	};

	class HavokPointer {
	public:
		unsigned abs_address;
		unsigned target_address;
	};

	struct HavokPackfileHeader {
		int magic[2];
		int user_tag;
		int file_version;
		unsigned char layout_rules[4];
		int num_sections;
		int contents_section_index;
		int contents_section_offset;
		int contents_class_name_section_index;
		int contents_class_name_section_offset;
		char contents_version[16];
		int flags;
		int pad[1];
	};

	struct HavokPackfileSectionHeader {
		char section_tag[19];
		char null_byte;
		unsigned int absolute_data_start;
		unsigned int local_fixups_offset;
		unsigned int global_fixups_offset;
		unsigned int virtual_fixups_offset;
		unsigned int exports_offset;
		unsigned int imports_offset;
		unsigned int end_offset;

		void endianSwap() {
			Endian::swap(absolute_data_start);
			Endian::swap(local_fixups_offset);
			Endian::swap(global_fixups_offset);
			Endian::swap(virtual_fixups_offset);
			Endian::swap(exports_offset);
			Endian::swap(imports_offset);
			Endian::swap(end_offset);
		}
	};

	struct HavokEndianSwapImpl {
		vector<unsigned char> out;
		unsigned int class_name_global_address = 0;

		void endianSwap(int i, int sz) {
			char t = 0;

			for (int c = 0; c < sz / 2; c++) {
				t = out[i + c];
				out[i + c] = out[i + (sz - c) - 1];
				out[i + (sz - c) - 1] = t;
			}
		}

		list<HavokClassName> class_names;
		list<HavokType> types;
		list<HavokLink> type_links;
		list<HavokLink> data_links;
		list<HavokPointer> data_pointers;
		list<HavokPointer> data_global_pointers;

		void endianSwap(HavokPackfileHeader& header) {
			header.user_tag = 0;
			Endian::swap(header.file_version);
			header.layout_rules[0] = 4;
			header.layout_rules[1] = 1;
			header.layout_rules[2] = 0;
			header.layout_rules[3] = 1;

			Endian::swap(header.num_sections);
			Endian::swap(header.contents_section_index);
			Endian::swap(header.contents_section_offset);

			Endian::swap(header.contents_class_name_section_index);
			Endian::swap(header.contents_class_name_section_offset);

			Endian::swap(header.flags);

			endianSwap(8, 4);
			endianSwap(12, 4);
			endianSwap(20, 4);
			endianSwap(24, 4);
			endianSwap(28, 4);
			endianSwap(32, 4);
			endianSwap(36, 4);

			out[17] = 1;
		}

		void convertElement(LibGens::File* fp, int main_type) {
			if ((main_type == TYPE_INT16) || (main_type == TYPE_UINT16) || (main_type == TYPE_HALF)) {
				endianSwap(fp->getCurrentAddress(), 2);
				fp->seek(2, SEEK_CUR);
			}
			else if ((main_type == TYPE_INT32) || (main_type == TYPE_UINT32) || (main_type == TYPE_REAL) || (main_type == TYPE_POINTER) || (main_type == TYPE_ULONG) || (main_type == TYPE_STRING_POINTER) || (main_type == TYPE_CSTRING)) {
				endianSwap(fp->getCurrentAddress(), 4);
				fp->seek(4, SEEK_CUR);
			}
			else if (main_type == TYPE_VARIANT) {
				endianSwap(fp->getCurrentAddress(), 4);
				endianSwap(fp->getCurrentAddress() + 4, 4);
				fp->seek(8, SEEK_CUR);
			}
			else if ((main_type == TYPE_INT64) || (main_type == TYPE_UINT64)) {
				endianSwap(fp->getCurrentAddress(), 8);
				fp->seek(8, SEEK_CUR);
			}
			else if ((main_type == TYPE_VECTOR4) || (main_type == TYPE_QUATERNION)) {
				endianSwap(fp->getCurrentAddress(), 4);
				endianSwap(fp->getCurrentAddress() + 4, 4);
				endianSwap(fp->getCurrentAddress() + 8, 4);
				endianSwap(fp->getCurrentAddress() + 12, 4);
				fp->seek(16, SEEK_CUR);
			}
			else if ((main_type == TYPE_MATRIX3) || (main_type == TYPE_ROTATION) || (main_type == TYPE_QSTRANSFORM)) {
				for (int j = 0; j < 12; j++) endianSwap(fp->getCurrentAddress() + j * 4, 4);

				fp->seek(4 * 12, SEEK_CUR);
			}
			else if ((main_type == TYPE_MATRIX4) || (main_type == TYPE_TRANSFORM)) {
				for (int j = 0; j < 16; j++) endianSwap(fp->getCurrentAddress() + j * 4, 4);

				fp->seek(4 * 16, SEEK_CUR);
			}
			else if ((main_type == TYPE_ARRAY) || (main_type == TYPE_HOMOGENEOUS_ARRAY)) {
				endianSwap(fp->getCurrentAddress(), 4);
				endianSwap(fp->getCurrentAddress() + 4, 4);
				endianSwap(fp->getCurrentAddress() + 8, 4);
			}
			else fp->seek(1, SEEK_CUR);
		}

		void convertStructure(LibGens::File* fp, string type_name) {
			unsigned int start = fp->getCurrentAddress();
			bool found = false;

			for (list<HavokType>::iterator it = types.begin(); it != types.end(); it++) {
				if ((*it).name != type_name) continue;
				unsigned int array_offset = start + (*it).object_size;
				found = true;

				if ((*it).parent) convertStructure(fp, (*it).parent->name);

				for (int i = 0; i < (*it).members.size(); i++) {
					int main_type = (*it).members[i].tag[0];
					int sub_type =(*it).members[i].tag[1];

					fp->seek(start + (*it).members[i].offset, SEEK_SET);

					if (main_type == TYPE_ENUM) main_type = sub_type;

					if (main_type == TYPE_STRUCT) convertStructure(fp, (*it).members[i].structure);

					else if (main_type == TYPE_POINTER) {
						endianSwap(fp->getCurrentAddress(), 4);

						for (list<HavokPointer>::iterator it2 = data_pointers.begin(); it2 != data_pointers.end(); it2++) {
							if (fp->getCurrentAddress() == (*it2).abs_address) {
								fp->seek((*it2).target_address, SEEK_SET);
								break;
							}
						}
					}
					else if ((main_type == TYPE_ARRAY) || (main_type == TYPE_SIMPLE_ARRAY)) {
						unsigned int count = 0;

						fp->seek(4, SEEK_CUR);
						fp->read(&count, sizeof(unsigned int));
						Endian::swap(count);

						endianSwap(fp->getCurrentAddress() - 8, 4);
						endianSwap(fp->getCurrentAddress() - 4, 4);

						if (main_type == TYPE_ARRAY) endianSwap(fp->getCurrentAddress(), 4);

						if (count == 0) {
							continue;
						}

						for (list<HavokPointer>::iterator it2 = data_pointers.begin(); it2 != data_pointers.end(); it2++) {
							if ((fp->getCurrentAddress() - 8) == (*it2).abs_address) {
								fp->seek((*it2).target_address, SEEK_SET);
								break;
							}
						}

						unsigned int newaddr = fp->getCurrentAddress();
						unsigned int sz = 1;

						for (list<HavokType>::iterator it2 = types.begin(); it2 != types.end(); it2++) {
							if ((*it2).name == (*it).members[i].structure) {
								sz = (*it2).object_size;
								break;
							}
						}

						for (int j = 0; j < count; j++) {
							if (sub_type == TYPE_STRUCT) {
								fp->seek(newaddr + j * sz, SEEK_SET);
								convertStructure(fp, (*it).members[i].structure);
							}
							else {
								if (sub_type == TYPE_POINTER) {
								}
								else {
									convertElement(fp, sub_type);
								}
							}
						}
					}
					else {
						unsigned int count = (*it).members[i].array_size;
						if (count == 0) count = 1;

						for (int j = 0; j < count; j++) {
							convertElement(fp, main_type);
						}
					}
				}
			}
		}

		void readData(HavokPackfileSectionHeader& header, LibGens::File* fp) {
			fp->seek(header.absolute_data_start, SEEK_SET);

			if (!strcmp(header.section_tag, "__classnames__")) {
				HavokClassName classname;

				class_name_global_address = header.absolute_data_start;

				int i = 0;
				while (classname.tag != (unsigned int)-1) {
					classname.name = "";

					fp->read(&classname.tag, sizeof(unsigned int));
					Endian::swap(classname.tag);
					endianSwap(fp->getCurrentAddress() - 4, 4);

					if ((classname.tag == (unsigned int)-1) || (classname.tag == 0)) {
						break;
					}
					else {
						fp->seek(1, SEEK_CUR);
						char c = 0;
						for (int i = 0; i < 256; i++) {
							fp->read(&c, sizeof(char));
							if (!c) break;

							classname.name += c;
						}

						class_names.push_back(classname);
					}

					i++;
				}
			}

			if (!strcmp(header.section_tag, "__types__")) {
				int i = 0;
				while (true) {
					fp->seek(header.absolute_data_start + header.local_fixups_offset + i * 4, SEEK_SET);

					unsigned int address = 0;
					fp->read(&address, sizeof(unsigned int));
					Endian::swap(address);
					if (address == (unsigned int)-1) break;

					endianSwap(fp->getCurrentAddress() - 4, 4);

					if (fp->getCurrentAddress() >= header.absolute_data_start + header.global_fixups_offset) break;

					i++;
				}

				i = 0;
				while (true) {
					fp->seek(header.absolute_data_start + header.global_fixups_offset + i * 12, SEEK_SET);

					unsigned int address = 0;
					unsigned int type = 0;
					unsigned int meta_address = 0;
					fp->read(&address, sizeof(unsigned int));
					Endian::swap(address);
					if (address == (unsigned int)-1) break;

					fp->read(&type, sizeof(unsigned int));
					Endian::swap(type);

					fp->read(&meta_address, sizeof(unsigned int));
					Endian::swap(meta_address);

					HavokLink link;
					link.address_1 = header.absolute_data_start + address;
					link.address_2 = header.absolute_data_start + meta_address;
					link.type = type;
					type_links.push_back(link);

					endianSwap(fp->getCurrentAddress() - 12, 4);
					endianSwap(fp->getCurrentAddress() - 8, 4);
					endianSwap(fp->getCurrentAddress() - 4, 4);

					if (fp->getCurrentAddress() >= header.absolute_data_start + header.virtual_fixups_offset) break;

					i++;
				}

				i = 0;
				while (true) {
					fp->seek(header.absolute_data_start + header.virtual_fixups_offset + i * 12, SEEK_SET);

					unsigned int address = 0;
					unsigned int name_address = 0;
					string type_name = "";

					fp->read(&address, sizeof(unsigned int));
					fp->seek(4, SEEK_CUR);

					if (((address == 0) && (i != 0)) || (address == (unsigned int)-1)) break;

					fp->read(&name_address, sizeof(unsigned int));

					Endian::swap(address);
					Endian::swap(name_address);

					endianSwap(fp->getCurrentAddress() - 4, 4);
					endianSwap(fp->getCurrentAddress() - 8, 4);
					endianSwap(fp->getCurrentAddress() - 12, 4);

					fp->seek(class_name_global_address + name_address, SEEK_SET);
					for (int k = 0; k < 256; k++) {
						char c = 0;
						fp->read(&c, sizeof(char));
						if (c) type_name += c;
						else break;
					}

					fp->seek(header.absolute_data_start + address, SEEK_SET);

					HavokType type;
					type.reset();
					type.address = header.absolute_data_start + address;
					type.class_name = type_name;

					if ((type_name != "hkClass") && (type_name != "hkClassEnum")) {
						i++;
						continue;
					}

					fp->seek(4, SEEK_CUR);

					for (list<HavokLink>::iterator it = type_links.begin(); it != type_links.end(); it++) {
						if ((*it).address_1 == fp->getCurrentAddress()) {
							type.parent_address = (*it).address_2;
							break;
						}
					}

					fp->seek(4, SEEK_CUR);
					fp->read(&type.object_size, sizeof(unsigned int));
					Endian::swap(type.object_size);
					endianSwap(fp->getCurrentAddress() - 4, 4);

					fp->read(&type.num_implemented_interfaces, sizeof(unsigned int));
					Endian::swap(type.num_implemented_interfaces);
					endianSwap(fp->getCurrentAddress() - 4, 4);

					fp->seek(4, SEEK_CUR);

					fp->read(&type.declared_enums, sizeof(unsigned int));
					Endian::swap(type.declared_enums);
					endianSwap(fp->getCurrentAddress() - 4, 4);

					fp->seek(4, SEEK_CUR);

					unsigned int membernum = 0;
					fp->read(&membernum, sizeof(unsigned int));
					Endian::swap(membernum);
					endianSwap(fp->getCurrentAddress() - 4, 4);

					if (type_name == "hkClass") {
						fp->seek(12, SEEK_CUR);
						fp->read(&type.described_version, sizeof(unsigned int));
						Endian::swap(type.described_version);
						endianSwap(fp->getCurrentAddress() - 4, 4);
					}

					char c = 0;
					type.name = "";
					for (int k = 0; k < 256; k++) {
						fp->read(&c, sizeof(char));
						if (!c) break;

						type.name += c;
					}

					fp->fixPaddingRead(16);

					if (type_name == "hkClass") {
						vector<int> subs_sizes;
						for (int j = 0; j < type.declared_enums; j++) {
							fp->seek(8, SEEK_CUR);
							int sz = 0;
							fp->read(&sz, sizeof(int));
							Endian::swap(sz);
							endianSwap(fp->getCurrentAddress() - 4, 4);

							fp->seek(8, SEEK_CUR);

							subs_sizes.push_back(sz);
						}

						for (int j = 0; j < type.declared_enums; j++) {
							string nm = "";
							for (int k = 0; k < 256; k++) {
								char c = 0;
								fp->read(&c, sizeof(char));

								if (c) nm += c;
								else break;
							}
							fp->fixPaddingRead(16);

							vector<HavokEnum> subs;
							subs.clear();
							type.sub_enum_names.push_back(nm);

							fp->fixPaddingRead(16);

							HavokEnum en;
							for (int x = 0; x < subs_sizes[j]; x++) {
								fp->read(&en.id, sizeof(unsigned int));
								Endian::swap(en.id);
								endianSwap(fp->getCurrentAddress() - 4, 4);

								en.name = "";
								fp->seek(4, SEEK_CUR);

								subs.push_back(en);
							}

							fp->fixPaddingRead(16);

							for (int x = 0; x < subs_sizes[j]; x++) {
								for (int k = 0; k < 256; k++) {
									char c = 0;
									fp->read(&c, sizeof(char));

									if (c) subs[x].name += c;
									else break;
								}

								fp->fixPaddingRead(16);

								type.sub_enums.push_back(subs);
							}
						}

						HavokTypeMember typemember;
						for (int j = 0; j < membernum; j++) {
							fp->seek(4, SEEK_CUR);

							typemember.structure_address = 0;

							for (list<HavokLink>::iterator it = type_links.begin(); it != type_links.end(); it++) {
								if ((*it).address_1 == fp->getCurrentAddress()) {
									typemember.structure_address = (*it).address_2;
									break;
								}
							}

							fp->seek(8, SEEK_CUR);

							fp->read(typemember.tag, 2);

							fp->read(&typemember.array_size, sizeof(unsigned short));
							Endian::swap(typemember.array_size);

							fp->read(&typemember.struct_type, sizeof(unsigned short));
							Endian::swap(typemember.struct_type);

							fp->read(&typemember.offset, sizeof(unsigned short));
							Endian::swap(typemember.offset);

							endianSwap(fp->getCurrentAddress() - 6, 2);
							endianSwap(fp->getCurrentAddress() - 4, 2);
							endianSwap(fp->getCurrentAddress() - 2, 2);

							typemember.name = "";
							typemember.structure = "";

							type.members.push_back(typemember);
							fp->seek(4, SEEK_CUR);
						}

						for (int j = 0; j < membernum; j++) {
							type.members[j].name = "";
							for (int k = 0; k < 256; k++) {
								char c = 0;
								fp->read(&c, sizeof(char));

								if (c) type.members[j].name += c;
								else break;
							}

							fp->fixPaddingRead(16);
						}
					}
					else {
						HavokEnum en;

						for (int j = 0; j < type.object_size; j++) {
							fp->read(&en.id, sizeof(unsigned int));
							Endian::swap(en.id);
							endianSwap(fp->getCurrentAddress() - 4, 4);

							en.name = "";

							fp->seek(4, SEEK_CUR);

							type.enums.push_back(en);
						}
						fp->fixPaddingRead(16);

						for (int j = 0; j < type.object_size; j++) {
							for (int k = 0; k < 256; k++) {
								char c = 0;
								fp->read(&c, sizeof(char));

								if (c) type.enums[j].name += c;
								else break;
							}

							fp->fixPaddingRead(16);
						}
					}

					types.push_back(type);
					i++;
				}

				i = 0;
				for (list<HavokLink>::iterator it = type_links.begin(); it != type_links.end(); it++) {
					(*it).type_parent = NULL;
					(*it).type_node = NULL;

					i++;
				}

				int j = 0;
				for (list<HavokType>::iterator it = types.begin(); it != types.end(); it++) {
					if ((*it).class_name != "hkClass") continue;

					if ((*it).parent_address) {
						for (list<HavokType>::iterator it2 = types.begin(); it2 != types.end(); it2++) {
							if ((*it2).address == (*it).parent_address) {
								(*it).parent = &(*it2);
								break;
							}
						}
					}

					for (int x = 0; x < (*it).members.size(); x++) {
						if ((*it).members[x].structure_address) {
							for (list<HavokType>::iterator it2 = types.begin(); it2 != types.end(); it2++) {
								if ((*it2).address == (*it).members[x].structure_address) {
									(*it).members[x].structure = (*it2).name;
									break;
								}
							}
						}
					}

					j++;
				}
			}

			if (!strcmp(header.section_tag, "__data__")) {
				int i = 0;
				while (true) {
					fp->seek(header.absolute_data_start + header.local_fixups_offset + i * 8, SEEK_SET);

					unsigned int address = 0;
					unsigned int address_2 = 0;
					fp->read(&address, sizeof(unsigned int));
					Endian::swap(address);
					if (address == (unsigned int)-1) break;

					fp->read(&address_2, sizeof(unsigned int));
					Endian::swap(address_2);

					endianSwap(fp->getCurrentAddress() - 8, 4);
					endianSwap(fp->getCurrentAddress() - 4, 4);

					HavokPointer pointer;

					pointer.abs_address = address + header.absolute_data_start;
					pointer.target_address = address_2 + header.absolute_data_start;

					data_pointers.push_back(pointer);

					if (fp->getCurrentAddress() >= header.absolute_data_start + header.global_fixups_offset) break;

					i++;
				}

				i = 0;
				while (true) {
					fp->seek(header.absolute_data_start + header.global_fixups_offset + i * 12, SEEK_SET);

					unsigned int address = 0;
					unsigned int type = 0;
					unsigned int meta_address = 0;
					fp->read(&address, sizeof(unsigned int));
					Endian::swap(address);
					if (address == (unsigned int)-1) break;

					fp->read(&type, sizeof(unsigned int));
					Endian::swap(type);

					fp->read(&meta_address, sizeof(unsigned int));
					Endian::swap(meta_address);

					endianSwap(fp->getCurrentAddress() - 12, 4);
					endianSwap(fp->getCurrentAddress() - 8, 4);
					endianSwap(fp->getCurrentAddress() - 4, 4);

					HavokPointer pointer;
					pointer.abs_address = address + header.absolute_data_start;
					pointer.target_address = meta_address + header.absolute_data_start;
					data_global_pointers.push_back(pointer);

					if (fp->getCurrentAddress() >= header.absolute_data_start + header.virtual_fixups_offset) break;

					i++;
				}

				i = 0;
				while (true) {
					fp->seek(header.absolute_data_start + header.virtual_fixups_offset + i * 12, SEEK_SET);

					unsigned int address = 0;
					unsigned int name_address = 0;
					string type_name = "";

					fp->read(&address, sizeof(unsigned int));
					fp->seek(4, SEEK_CUR);
					Endian::swap(address);
					if (address == (unsigned int)-1) break;

					fp->read(&name_address, sizeof(unsigned int));

					endianSwap(fp->getCurrentAddress() - 4, 4);
					endianSwap(fp->getCurrentAddress() - 8, 4);
					endianSwap(fp->getCurrentAddress() - 12, 4);

					Endian::swap(name_address);
					unsigned back = fp->getCurrentAddress();

					fp->seek(class_name_global_address + name_address, SEEK_SET);
					for (int k = 0; k < 256; k++) {
						char c = 0;
						fp->read(&c, sizeof(char));
						if (c) type_name += c;
						else break;
					}

					fp->seek(header.absolute_data_start + address, SEEK_SET);

					convertStructure(fp, type_name);

					if (back >= header.absolute_data_start + header.exports_offset) break;

					i++;
				}
			}
		}
	};
}

namespace LibGens {
	vector<unsigned char> endianSwapHKX(File* file) {
		HavokEndianSwapImpl impl;
		impl.out.resize(file->getFileSize());
		file->read(impl.out.data(), impl.out.size());

		HavokPackfileHeader header;
		file->goToAddress(0);
		file->read(&header, sizeof(HavokPackfileHeader));
		impl.endianSwap(header);

		for (int i = 0; i < header.num_sections; i++) {
			HavokPackfileSectionHeader section_header;
			file->read(&section_header, sizeof(HavokPackfileSectionHeader));
			section_header.endianSwap();

			impl.endianSwap(file->getCurrentAddress() - 28, 4);
			impl.endianSwap(file->getCurrentAddress() - 24, 4);
			impl.endianSwap(file->getCurrentAddress() - 20, 4);
			impl.endianSwap(file->getCurrentAddress() - 16, 4);
			impl.endianSwap(file->getCurrentAddress() - 12, 4);
			impl.endianSwap(file->getCurrentAddress() - 8, 4);
			impl.endianSwap(file->getCurrentAddress() - 4, 4);

			unsigned int address = file->getCurrentAddress();
			impl.readData(section_header, file);
			file->seek(address, SEEK_SET);
		}

		return move(impl.out);
	}
}