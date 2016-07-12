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

#include "LibGens.h"
#include <algorithm>
#include "S06XnFile.h"

namespace LibGens {
	void SonicTextureUnitZNO::read(File *file, bool big_endian) {
		file->readInt32E(&flag, big_endian);
		file->readInt32E(&index, big_endian);
		file->readInt32E(&enviroment_mode, big_endian);
		offset.read(file, big_endian);
		file->moveAddress(4);
		scale.read(file, big_endian);
		file->readInt32E(&wrap_s, big_endian);
		file->readInt32E(&wrap_t, big_endian);
		file->readFloat32E(&lod_bias, big_endian);
	}

	void SonicTextureUnitZNO::write(File *file) {
		file->writeInt32(&flag);
		file->writeInt32(&index);
		file->writeInt32(&enviroment_mode);
		offset.write(file, false);
		file->writeNull(4);
		scale.write(file, false);
		file->writeInt32(&wrap_s);
		file->writeInt32(&wrap_t);
		file->writeFloat32(&lod_bias);
		file->writeNull(20);
	}

	void SonicMaterialColor::read(File *file, XNFileMode file_mode, bool big_endian) {
		file->readInt32E(&flag, big_endian);
		ambient.read(file, big_endian);
		diffuse.read(file, big_endian);
		specular.read(file, big_endian);
		emission.read(file, big_endian);
		file->readFloat32E(&shininess, big_endian);
		file->readFloat32E(&specular_intensity, big_endian);
	}

	void SonicMaterialColor::write(File *file, XNFileMode file_mode) {
		file->writeInt32(&flag);
		ambient.write(file, false);
		diffuse.write(file, false);
		specular.write(file, false);
		emission.write(file, false);
		file->writeFloat32(&shininess);
		file->writeFloat32(&specular_intensity);
	}

	bool SonicMaterialColor::compare(SonicMaterialColor *color) {
		if (flag != color->flag) return false;
		if (ambient != color->ambient) return false;
		if (diffuse != color->diffuse) return false;
		if (specular != color->specular) return false;
		if (emission != color->emission) return false;
		if (shininess != color->shininess) return false;
		if (specular_intensity != color->specular_intensity) return false;

		return true;
	}

	void SonicMaterialProperties::read(File *file, XNFileMode file_mode, bool big_endian) {
		file->read(data, 28);
	}

	void SonicMaterialProperties::write(File *file, XNFileMode file_mode) {
		file->write(data, 28);
	}


	void SonicTextureUnit::read(File *file, bool big_endian) {
		file->readFloat32E(&flag_f, big_endian);
		file->readInt32E(&index, big_endian);
		file->readInt32E(&flag, big_endian);
		file->moveAddress(4);
		file->readFloat32E(&flag_2_f, big_endian);
		file->moveAddress(4);
		file->readInt32E(&flag_2, big_endian);
		file->readFloat32E(&flag_3_f, big_endian);
		file->readInt32E(&flag_3, big_endian);
		Error::printfMessage(Error::WARNING, "Instance Material: %f %d", flag_f, index);
	}

	void SonicTextureUnit::write(File *file) {
		file->writeFloat32(&flag_f);
		file->writeInt32(&index);
		file->writeInt32(&flag);
		file->writeNull(4);
		file->writeFloat32(&flag_2_f);
		file->writeNull(4);
		file->writeInt32(&flag_2);
		file->writeFloat32(&flag_3_f);
		file->writeInt32(&flag_3);
		file->writeNull(12);
	}

	bool SonicTextureUnit::compare(SonicTextureUnit *t) {
		if (flag_f    != t->flag_f)   return false;
		if (index     != t->index)    return false;
		if (flag_2_f  != t->flag_2_f) return false;
		if (flag      != t->flag)     return false;
		if (flag_2    != t->flag_2)   return false;
		if (flag_3_f  != t->flag_3_f) return false;
		if (flag_3    != t->flag_3)   return false;
		return true;
	}

	void SonicMaterialTable::read(File *file, XNFileMode file_mode, bool big_endian) {
		size_t table_address=0;
		colors=NULL;
		properties=NULL;

		if (file_mode == MODE_ENO) return;

		file->readInt32E(&count, big_endian);
		file->readInt32EA(&table_address, big_endian);
		file->goToAddress(table_address);

		data_block_1_length = 20;
		data_block_2_length = 16;

		size_t data_1_offset=0;
		size_t data_2_offset=0;
		size_t texture_units_offset=0;

		file->readInt32E(&flag_table, big_endian);
		file->readInt32E(&user_flag, big_endian);
		file->readInt32EA(&data_1_offset, big_endian);
		file->readInt32EA(&data_2_offset, big_endian);
		
		unsigned int texture_unit_zno_count=0;
		if (file_mode == MODE_ZNO) {
			file->readInt32E(&texture_unit_flag, big_endian);
			file->readInt32E(&texture_unit_zno_count, big_endian);
		}
		file->readInt32EA(&texture_units_offset, big_endian);
		if (file_mode == MODE_ZNO) {
			file->readInt32E(&texture_unit_flag_2, big_endian);
		}

		if (file_mode == MODE_ZNO) {
			file->goToAddress(data_1_offset);
			colors = new SonicMaterialColor();
			colors->read(file, file_mode, big_endian);

			file->goToAddress(data_2_offset);
			properties = new SonicMaterialProperties();
			properties->read(file, file_mode, big_endian);
		}
		else {
			file->goToAddress(data_1_offset);
			file->read(first_floats, data_block_1_length*4);

			file->goToAddress(data_2_offset);
			file->read(first_ints, data_block_2_length*4);
		}

		if (file_mode == MODE_ZNO) {
			if (texture_unit_zno_count) {
				for (size_t i=0; i<texture_unit_zno_count; i++) {
					file->goToAddress(texture_units_offset + i*64);
					SonicTextureUnitZNO *texture_unit = new SonicTextureUnitZNO();
					texture_unit->read(file, big_endian);
					texture_units_zno.push_back(texture_unit);
				}
			}
		}
		else if (count) {
			count -= 16;
			for (size_t i=0; i<count; i++) {
				file->goToAddress(texture_units_offset + i*48);
				SonicTextureUnit *texture_unit = new SonicTextureUnit();
				texture_unit->read(file, big_endian);
				texture_units.push_back(texture_unit);
			}
		}

		Error::addMessage(Error::WARNING, "Done with Material.");
	}

	void SonicMaterialTable::write(File *file, XNFileMode file_mode) {
		head_address = file->getCurrentAddress();

		if (file_mode == MODE_ZNO) {
			file->writeInt32(&count);
			file->writeInt32A(&table_address);
		}
		else {
			unsigned int file_count=16 + texture_units.size();
			if (!texture_units.size()) file->writeNull(8);
			else {
				file->writeInt32(&file_count);
				file->writeInt32A(&table_address);
			}
		}
	}

	void SonicMaterialTable::writeTable(File *file, XNFileMode file_mode) {
		table_address = file->getCurrentAddress();

		file->writeInt32(&flag_table);
		file->writeInt32(&user_flag);
		file->writeInt32A(&data_block_1_address);
		file->writeInt32A(&data_block_2_address);

		if (file_mode == MODE_ZNO) {
			file->writeInt32(&texture_unit_flag);
			unsigned int texture_unit_zno_count=texture_units_zno.size();
			file->writeInt32(&texture_unit_zno_count);
		}

		file->writeInt32A(&texture_units_address);
		if (file_mode == MODE_ZNO) {
			file->writeInt32(&texture_unit_flag_2);
		}
		else {
			file->writeNull(12);
		}
	}

	void SonicMaterialTable::writeDataBlock1(File *file, XNFileMode file_mode) {
		data_block_1_address = file->getCurrentAddress();
		if (file_mode == MODE_ZNO) {
			colors->write(file, file_mode);
		}
		else {
			file->write(first_floats, data_block_1_length*4);
		}
	}

	void SonicMaterialTable::writeDataBlock2(File *file, XNFileMode file_mode) {
		data_block_2_address = file->getCurrentAddress();
		if (file_mode == MODE_ZNO) {
			properties->write(file, file_mode);
		}
		else {
			file->write(first_ints, data_block_2_length*4);
		}
	}

	void SonicMaterialTable::writeTextureUnits(File *file, XNFileMode file_mode) {
		texture_units_address = file->getCurrentAddress();

		if (file_mode == MODE_ZNO) {
			for (size_t i=0; i<texture_units_zno.size(); i++) {
				texture_units_zno[i]->write(file);
			}
		}
		else {
			for (size_t i=0; i<texture_units.size(); i++) {
				texture_units[i]->write(file);
			}
		}
	}

	
	bool SonicMaterialTable::compareDataBlock1(SonicMaterialTable *table, XNFileMode file_mode) {
		if (file_mode == MODE_ZNO) {
			colors->compare(table->colors);
		}
		else {
			for (size_t i=0; i<data_block_1_length; i++) {
				if (first_floats[i] != table->first_floats[i]) {
					return false;
				}
			}
		}
		return true;
	}

	bool SonicMaterialTable::compareDataBlock2(SonicMaterialTable *table, XNFileMode file_mode) {
		for (size_t i=0; i<data_block_2_length; i++) {
			if (first_ints[i] != table->first_ints[i]) {
				return false;
			}
		}
		return true;
	}

	bool SonicMaterialTable::compareTextureUnits(SonicMaterialTable *table, XNFileMode file_mode) {
		if (texture_units.size() > table->texture_units.size()) return false;

		for (size_t index=0; index<texture_units.size(); index++) {
			if (!texture_units[index]->compare(table->texture_units[index])) {
				return false;
			}
		}

		return true;
	}
};
