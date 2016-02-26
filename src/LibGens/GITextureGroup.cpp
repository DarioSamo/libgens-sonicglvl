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

#include "AR.h"
#include "GITextureGroup.h"
#include "Material.h"
#include <map>

namespace LibGens {
	GISubtexture::GISubtexture() {
	}

	void GISubtexture::setPixelSize(unsigned int v) {
		pixel_size = v;
	}

	unsigned int GISubtexture::getPixelSize() {
		return pixel_size;
	}

	string GISubtexture::getName() {
		return name;
	}

	string &GISubtexture::getPath() {
		return full_path;
	}

	void GISubtexture::setPath(string v) {
		full_path = v;
	}

	void GISubtexture::setName(string v) {
		name = v;
	}

	void GISubtexture::setParent(GITexture *v, string parent_folder, string parent_name) {
		parent = v;
		if (parent) {
			full_path=parent_folder + "-" + parent_name + LIBGENS_TEXTURE_FILE_EXTENSION;
		}
	}

	GITexture *GISubtexture::getParent() {
		return parent;
	}

	float &GISubtexture::getX() {
		return x;
	}

	float &GISubtexture::getY() {
		return y;
	}

	float &GISubtexture::getWidth() {
		return w;
	}

	float &GISubtexture::getHeight() {
		return h;
	}

	void GISubtexture::setX(float v) {
		x = v;
	}

	void GISubtexture::setY(float v) {
		y = v;
	}

	void GISubtexture::setWidth(float v) {
		w = v;
	}

	void GISubtexture::setHeight(float v) {
		h = v;
	}

	GITexture::GITexture() {

	}

	GITexture::GITexture(string folder_p) {
		folder = folder_p;
	}

	string GITexture::getFolder() {
		return folder;
	}

	string GITexture::getName() {
		return texture_name;
	}

	void GITexture::setName(string v) {
		texture_name = v;
	}

	void GITexture::addSubtexture(GISubtexture *v) {
		subtextures.push_back(v);
	}

	list<GISubtexture *> GITexture::getSubtextures() {
		return subtextures;
	}

	string GITexture::getFilename() {
		return folder + "/" + texture_name + LIBGENS_TEXTURE_FILE_EXTENSION;
	}

	GITextureGroupInfo::GITextureGroupInfo(string filename, string terrain_folder) {
		File file(filename, LIBGENS_FILE_READ_BINARY);

		if (file.valid()) {
			file.readHeader();
			read(&file, terrain_folder);
			file.close();
		}
	}

	void GITextureGroupInfo::save(string filename_p) {
		File file(filename_p, LIBGENS_FILE_WRITE_BINARY);

		if (file.valid()) {
			file.prepareHeader(LIBGENS_GI_TEXTURE_GROUP_ROOT_TYPE);
			write(&file);
			file.writeHeader();
			file.close();
		}
	}

	GITextureGroupInfo::GITextureGroupInfo() {

	}

	void GITextureGroupInfo::addInstance(string name, Vector3 center, float radius) {
		instance_names.push_back(name);
		instance_centers.push_back(center);
		instance_radius.push_back(radius);
	}

	void GISubtexture::read(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_GI_TEXTURE_GROUP_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		unsigned char texture_name_size=0;
		file->readUChar(&texture_name_size);
		file->readString(&name, texture_name_size);
		unsigned char w_c=0, h_c=0;
		file->readUChar(&w_c);
		file->readUChar(&h_c);
		file->readFloat8(&x);
		file->readFloat8(&y);

		if (w_c > 0) w = 1.0f / pow(2.0f, (float) w_c);
		else w = 1.0f;

		if (h_c > 0) h = 1.0f / pow(2.0f, (float) h_c);
		else h = 1.0f;
	}

	void GISubtexture::write(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_GI_TEXTURE_GROUP_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		unsigned char texture_name_size = name.size();
		file->writeUChar(&texture_name_size);
		file->writeString(name.c_str());

		unsigned char w_c = (int)(log(1.0f / w) / log(2.0f));
		unsigned char h_c = (int)(log(1.0f / h) / log(2.0f));
		file->writeUChar(&w_c);
		file->writeUChar(&h_c);
		file->writeFloat8(&x);
		file->writeFloat8(&y);
	}
	
	void GITexture::read(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_GI_TEXTURE_GROUP_ERROR_MESSAGE_NULL_FILE);
			return;
		}
		
		unsigned char texture_name_size=0;
		unsigned char subtexture_count=0;
		file->readUChar(&texture_name_size);
		file->readString(&texture_name, texture_name_size);
		file->readUChar(&subtexture_count);
		file->moveAddress(1);

		for (size_t i=0; i<subtexture_count; i++) {
			GISubtexture *subtexture=new GISubtexture();
			subtexture->read(file);
			subtexture->setParent(this, getFolder(), getName());
			subtextures.push_back(subtexture);
		}
	}

	void GITexture::write(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_GI_TEXTURE_GROUP_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		unsigned char texture_name_size = texture_name.size();
		size_t subtexture_count_sz = subtextures.size();
		unsigned char subtexture_count = subtexture_count_sz;
		file->writeUChar(&texture_name_size);
		file->writeString(texture_name.c_str());
		file->writeUChar(&subtexture_count);
		file->writeNull(1);

		for (list<GISubtexture *>::iterator it = subtextures.begin(); it != subtextures.end(); it++) {
			(*it)->write(file);
		}
	}

	void GITexture::setWidth(unsigned int v) {
		width = v;
	}

	void GITexture::setHeight(unsigned int v) {
		height = v;
	}

	int GITexture::getWidth() {
		return width;
	}

	int GITexture::getHeight() {
		return height;
	}

	GITextureTree::GITextureTree() {
		width = height = 0;
		node_type = -1;
		left = right = NULL;
		subtexture = NULL;
	}

	GITextureTree::~GITextureTree() {
		delete left;
		delete right;
	}

	bool GITextureTree::fitTexture(GISubtexture *subt) {
		// Bigger than current node, split to either left or right of this. Create children if necessary.
		if (((width > subt->getPixelSize()) || (height > subt->getPixelSize())) && ((node_type == 0) || (node_type == 1))) {
			if (!left && !right) {
				left = new GITextureTree();
				right = new GITextureTree();

				if (node_type == 0) {
					left->x = x;
					left->y = y;
					left->width = width / 2;
					left->height = height;
					left->node_type = 1;

					right->x = x + width / 2;
					right->y = y;
					right->width = width / 2;
					right->height = height;
					right->node_type = 1;
				}
				else if (node_type == 1) {
					left->x = x;
					left->y = y;
					left->width = width;
					left->height = height / 2;
					left->node_type = 0;

					right->x = x;
					right->y = y + height / 2;
					right->width = width;
					right->height = height / 2;
					right->node_type = 0;
				}
			}

			if (left && left->fitTexture(subt))
				return true;

			if (right && right->fitTexture(subt))
				return true;
		}
		// If width and height match, this node is not occupied, and it has no children
		else if ((width == subt->getPixelSize()) && (height == subt->getPixelSize()) && (node_type != 2) && !left && !right && !subtexture) {
			subtexture = subt;
			node_type = 2;
			return true;
		}

		return false;
	}

	void GITextureTree::setSubtextures(unsigned int texture_width, unsigned int texture_height) {
		if (subtexture) {
			subtexture->setX((float) x / (float) texture_width);
			subtexture->setY((float) y / (float) texture_height);
			subtexture->setWidth((float) width / (float) texture_width);
			subtexture->setHeight((float) height / (float) texture_height);
		}

		if (left)
			left->setSubtextures(texture_width, texture_height);

		if (right)
			right->setSubtextures(texture_width, texture_height);
	}

	void GITexture::organizeSubtextures() {
		GITextureTree texture_tree;
		texture_tree.x = 0;
		texture_tree.y = 0;
		texture_tree.width = getWidth();
		texture_tree.height = getHeight();
		texture_tree.node_type = (texture_tree.height > texture_tree.width) ? 1 : 0;

		// Fit subtextures as best as possible on the current pixel size
		for (list<GISubtexture *>::iterator it = subtextures.begin(); it != subtextures.end(); it++) {
			texture_tree.fitTexture(*it);
		}

		texture_tree.setSubtextures(getWidth(), getHeight());
	}

	void GITextureGroup::readAtlasinfo(File *file, string terrain_folder, vector<string> instance_names) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_GI_TEXTURE_GROUP_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		unsigned short texture_count=0;
		file->readInt16BE(&texture_count);
		file->moveAddress(1);

		for (size_t i=0; i<texture_count; i++) {
			GITexture *texture=new GITexture(terrain_folder);
			texture->read(file);
			textures.push_back(texture);
		}

		if (instance_names.size() != texture_count) {
			for (size_t i=0; i<instance_names.size(); i++) {
				string tex_name=instance_names[i] + LIBGENS_GI_TEXTURE_GROUP_SUBTEXTURE_LEVEL + ToString(quality_level);

				bool found=false;
				for (list<GITexture *>::iterator it=textures.begin(); it!=textures.end(); it++) {
					if ((*it)->getTextureByInstance(tex_name)) {
						found = true;
						break;
					}
				}

				if (!found) {
					GITexture *texture=new GITexture(terrain_folder);
					texture->setName(tex_name);

					GISubtexture *subtexture=new GISubtexture();
					subtexture->setParent(texture, texture->getFolder(), texture->getName());
					subtexture->setName(tex_name);
					subtexture->setX(0);
					subtexture->setY(0);
					subtexture->setWidth(1);
					subtexture->setHeight(1);

					texture->addSubtexture(subtexture);
					textures.push_back(texture);
				}
			}
		}
	}

	void GITextureGroup::saveAtlasinfo(string atlasinfo_filename) {
		File file(atlasinfo_filename, LIBGENS_FILE_WRITE_BINARY);
		if (file.valid()) {
			writeAtlasinfo(&file);
			file.close();
		}
	}

	void GITextureGroup::writeAtlasinfo(File *file) {
		unsigned short texture_count=textures.size();
		file->writeInt16BE(&texture_count);
		file->writeNull(1);

		for (list<GITexture *>::iterator it = textures.begin(); it != textures.end(); it++) {
			(*it)->write(file);
		}
	}

	void GITextureGroup::read(File *file, string terrain_folder, string group_folder, vector<string> &global_instance_names) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_GI_TEXTURE_GROUP_ERROR_MESSAGE_NULL_FILE);
			return;
		}
		filename=group_folder;

		unsigned int instance_count=0;
		size_t index_table_address;
        size_t bounding_sphere_address;

		file->readInt32BE(&quality_level);
		file->readInt32BE(&instance_count);
		file->readInt32BEA(&index_table_address);
		file->readInt32BEA(&bounding_sphere_address);
		file->readInt32BE(&folder_size);

		vector<string> instance_names;
		file->goToAddress(index_table_address);
		for (size_t i=0; i<instance_count; i++) {
			unsigned int instance_index=0;
			file->readInt32BE(&instance_index);

			if (quality_level == 0) {
				if (instance_index < global_instance_names.size()) {
					instance_names.push_back(global_instance_names[instance_index]);
				}
				else {
					Error::addMessage(Error::EXCEPTION, "GITextureGroup::read: Instance Index " + ToString(instance_index) + " is bigger than the Global Instance Names list.");
				}
			}

			instance_indices.push_back(instance_index);
		}

		file->goToAddress(bounding_sphere_address);
		center.read(file);
		file->readFloat32BE(&radius);


		// Read atlasinfo directly from pack file
		ArPack *ar_pack = NULL;
		File *ar_pack_file = NULL;
		if (terrain_folder.size()) {
			ar_pack = new ArPack(terrain_folder, false);
			ar_pack_file = new File(terrain_folder, LIBGENS_FILE_READ_BINARY);
		}

		if (ar_pack && ar_pack_file) {
			ArFile *entry=ar_pack->getFile(LIBGENS_GI_TEXTURE_GROUP_ATLASINFO_FILE);
			if (entry) {
				ar_pack_file->setGlobalOffset(entry->getAbsoluteDataAddress());
				readAtlasinfo(ar_pack_file, group_folder, instance_names);
			 }
		}

		if (ar_pack_file) {
			ar_pack_file->close();
			delete ar_pack_file;
		}

		if (ar_pack) {
			delete ar_pack;
		}
	}

	GITextureGroup::GITextureGroup() {
	}

	list<GITexture *> GITextureGroup::getTextures() {
		return textures;
	}

	void GITextureGroup::setQualityLevel(unsigned int v) {
		quality_level = v;
	}

	unsigned int GITextureGroup::getQualityLevel() {
		return quality_level;
	}

	string GITextureGroup::getFilename() {
		return filename;
	}

	string GITextureGroup::getAtlasinfoFilename() {
		return atlasinfo_filename;
	}

	vector<string> GITextureGroupInfo::getInstanceNames() {
		return instance_names;
	}

	GITextureGroup *GITextureGroupInfo::getGroupByIndex(size_t index) {
		if (index < groups.size()) {
			return groups[index];
		}

		return NULL;
	}

	vector<GITextureGroup *> GITextureGroupInfo::getGroups() {
		return groups;
	}

	void GITextureGroup::write(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_GI_TEXTURE_GROUP_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}

		size_t header_address=file->getCurrentAddress();

		unsigned int instance_count=instance_indices.size();
		size_t index_table_address=0;
        size_t bounding_sphere_address=0;

		file->writeInt32BE(&quality_level);
		file->writeInt32BE(&instance_count);
		file->writeNull(8);
		file->writeInt32BE(&folder_size);
		
		index_table_address = file->getCurrentAddress();
		for (size_t i=0; i<instance_count; i++) {
			file->writeInt32BE(&instance_indices[i]);
		}

		bounding_sphere_address = file->getCurrentAddress();
		center.write(file);
		file->writeFloat32BE(&radius);

		file->goToAddress(header_address+8);
		file->writeInt32BEA(&index_table_address);
		file->writeInt32BEA(&bounding_sphere_address);
		file->goToEnd();
	}

	void GITextureGroup::fixIndices(std::map<int, int> index_map) {
		size_t sz = instance_indices.size();
		for (size_t i = 0; i < sz; i++) {
			instance_indices[i] = index_map[instance_indices[i]];
		}
	}

	void GITextureGroup::organizeSubtextures(unsigned int max_texture_size) {
		// Sort from biggest to smallest subtextures
		list<GISubtexture *> sorted_subtextures;
		for (list<GISubtexture *>::iterator it = subtextures_to_organize.begin(); it != subtextures_to_organize.end(); it++) {
			(*it)->setName((*it)->getName() + "-level" + ToString(quality_level));

			bool added = false;
			for (list<GISubtexture *>::iterator it2 = sorted_subtextures.begin(); it2 != sorted_subtextures.end(); it2++) {
				if ((*it2)->getPixelSize() < (*it)->getPixelSize()) {
					sorted_subtextures.insert(it2, *it);
					added = true;
					break;
				}
			}

			if (!added)
				sorted_subtextures.push_back(*it);
		}

		subtextures_to_organize.clear();

		// Keep generating textures until there's no more subtextures to go through
		const int max_pixels = max_texture_size * max_texture_size;
		while (sorted_subtextures.size()) {
			LibGens::GITexture *gi_texture = new LibGens::GITexture();
			char gi_texture_name[256];
			sprintf(gi_texture_name, "a%04d", textures.size());
			gi_texture->setName(gi_texture_name);

			// Remove all used subtextures from list
			list<GISubtexture *> subtextures_used;
			list<GISubtexture *> new_sorted_subtextures;
			int filled_pixels = 0;
			size_t subtextures_used_count = 0;
			for (list<GISubtexture *>::iterator it = sorted_subtextures.begin(); it != sorted_subtextures.end(); it++) {
				int texture_pixels = (*it)->getPixelSize() * (*it)->getPixelSize();
				if ((filled_pixels + texture_pixels) <= max_pixels) {
					filled_pixels += texture_pixels;
					subtextures_used.push_back(*it);
					subtextures_used_count++;
				}
				else {
					new_sorted_subtextures.push_back(*it);
				}
			}

			sorted_subtextures = new_sorted_subtextures;

			// Add all used subtextures to the GI texture. Detect the maximum width and height needed.
			int width = 4;
			int height = 4;
			bool increased_axis = false;
			while (((width * height) < filled_pixels) && (width <= max_texture_size) && (height <= max_texture_size)) {
				if (increased_axis)
					height *= 2;
				else
					width *= 2;

				increased_axis = !increased_axis;
			}

			gi_texture->setWidth(width);
			gi_texture->setHeight(height);

			// Organize all the subtextures that were used
			for (list<GISubtexture *>::iterator it = subtextures_used.begin(); it != subtextures_used.end(); it++) {
				gi_texture->addSubtexture(*it);
			}
			gi_texture->organizeSubtextures();

			textures.push_back(gi_texture);
		}
	}
	
	void GITextureGroupInfo::read(File *file, string terrain_folder) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_GI_TEXTURE_GROUP_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		size_t header_address=file->getCurrentAddress();

		unsigned int name_total=0;
		size_t name_table_address;
		size_t sphere_table_address;
		unsigned int gia_total=0;
		size_t gia_table_address;
		unsigned int low_gia_total=0;
		size_t low_gia_table_address;

		file->readInt32BE(&name_total);
		file->readInt32BEA(&name_table_address);
		file->readInt32BEA(&sphere_table_address);
		file->readInt32BE(&gia_total);
		file->readInt32BEA(&gia_table_address);
		file->readInt32BE(&low_gia_total);
		file->readInt32BEA(&low_gia_table_address);

		for (size_t i=0; i<name_total; i++) {
			// Names
			file->goToAddress(name_table_address + i*4);
			size_t address=0;
			file->readInt32BEA(&address);
			file->goToAddress(address);

			string instance="";
			file->readString(&instance);
			instance_names.push_back(instance);


			// Spheres
			file->goToAddress(sphere_table_address + i*4);
			address=0;
			file->readInt32BEA(&address);
			file->goToAddress(address);

			Vector3 center;
			float radius;
			center.read(file);
			file->readFloat32BE(&radius);

			instance_centers.push_back(center);
			instance_radius.push_back(radius);
		}


		for (size_t i=0; i<gia_total; i++) {
			file->goToAddress(gia_table_address + i*4);
			size_t address=0;
			file->readInt32BEA(&address);
			file->goToAddress(address);

			string group_folder=LIBGENS_GI_TEXTURE_GROUP_FOLDER_BEFORE + ToString(i) + LIBGENS_GI_TEXTURE_GROUP_FOLDER_AFTER;
			GITextureGroup *group=new GITextureGroup();
			group->read(file, terrain_folder + group_folder, group_folder, instance_names);
			groups.push_back(group);
		}
	}


	void GITextureGroupInfo::write(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_GI_TEXTURE_GROUP_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}

		size_t header_address=file->getCurrentAddress();

		unsigned int name_total=instance_names.size();
		size_t name_table_address=0;
		size_t sphere_table_address=0;
		unsigned int gia_total=groups.size();
		size_t gia_table_address=0;
		unsigned int low_gia_total=0;
		size_t low_gia_table_address=0;

		file->writeInt32BE(&name_total);
		file->writeNull(8);
		file->writeInt32BE(&gia_total);
		file->writeNull(12);

		// Names
		name_table_address = file->getCurrentAddress();
		vector<size_t> name_addresses;
		file->writeNull(name_total * 4);
		for (size_t i=0; i<name_total; i++) {
			name_addresses.push_back(file->getCurrentAddress());
			file->writeString(&instance_names[i]);
			file->fixPadding();
		}

		for (size_t i=0; i<name_total; i++) {
			file->goToAddress(name_table_address + i*4);
			file->writeInt32BEA(&name_addresses[i]);
		}
		file->goToEnd();


		// Spheres
		sphere_table_address = file->getCurrentAddress();
		vector<size_t> sphere_addresses;
		file->writeNull(name_total * 4);
		for (size_t i=0; i<name_total; i++) {
			sphere_addresses.push_back(file->getCurrentAddress());
			instance_centers[i].write(file);
			file->writeFloat32BE(&instance_radius[i]);
		}

		for (size_t i=0; i<name_total; i++) {
			file->goToAddress(sphere_table_address + i*4);
			file->writeInt32BEA(&sphere_addresses[i]);
		}
		file->goToEnd();

		// GIA Groups
		gia_table_address = file->getCurrentAddress();
		vector<size_t> group_addresses;
		file->writeNull(gia_total * 4);
		for (size_t i=0; i<gia_total; i++) {
			group_addresses.push_back(file->getCurrentAddress());
			groups[i]->write(file);
		}

		for (size_t i=0; i<gia_total; i++) {
			file->goToAddress(gia_table_address + i*4);
			file->writeInt32BEA(&group_addresses[i]);
		}
		file->goToEnd();

		// Low Quality GIA Groups
		low_gia_table_address = file->getCurrentAddress();
		for (size_t i=0; i<gia_total; i++) {
			if (groups[i]->getQualityLevel() == LIBGENS_GI_TEXTURE_GROUP_LOWEST_QUALITY) {
				unsigned int index=i;
				file->writeInt32BE(&index);
				low_gia_total++;
			}
		}

		// Fix Header
		file->goToAddress(header_address+4);
		file->writeInt32BEA(&name_table_address);
		file->writeInt32BEA(&sphere_table_address);
		file->moveAddress(4);
		file->writeInt32BEA(&gia_table_address);
		file->writeInt32BE(&low_gia_total);
		file->writeInt32BEA(&low_gia_table_address);
		file->goToEnd();
	}

	
	GISubtexture *GITexture::getTextureByInstance(string instance) {
		for (list<GISubtexture *>::iterator it=subtextures.begin(); it!=subtextures.end(); it++) {
			string subtexture_name=(*it)->getName();

			if (subtexture_name.find(instance) != string::npos) {
				return *it;
			}
		}

		return NULL;
	}

	GISubtexture *GITextureGroup::getTextureByInstance(size_t instance_index, string instance, size_t quality_level_p, vector<GITextureGroup *> &groups) {
		bool found=false;
		if (quality_level_p != quality_level) return NULL;

		if (hasInstanceIndex(instance_index, groups)) {
			for (list<GITexture *>::iterator it=textures.begin(); it!=textures.end(); it++) {
				GISubtexture *result=(*it)->getTextureByInstance(instance+LIBGENS_GI_TEXTURE_GROUP_SUBTEXTURE_LEVEL+ToString(quality_level));
				if (result) return result;
			}
		}

		return NULL;
	}

	bool GITextureGroup::hasInstanceIndex(size_t instance_index, vector<GITextureGroup *> &groups) {
		for (size_t i=0; i<instance_indices.size(); i++) {
			if (quality_level) {
				if (instance_indices[i] < groups.size()) {
					if (groups[instance_indices[i]]->hasInstanceIndex(instance_index, groups)) {
						return true;
					}
				}
			}
			else {
				if (instance_indices[i] == instance_index) {
					return true;
				}
			}
		}

		return false;
	}

	void GITextureGroup::addInstanceIndex(unsigned int instance_index) {
		instance_indices.push_back(instance_index);
	}

	void GITextureGroup::addSubtextureToOrganize(GISubtexture *subtexture) {
		subtextures_to_organize.push_back(subtexture);
	}

	void GITextureGroup::addSubtextureToOrganize(GITextureGroup *clone_group, float downscale_factor) {
		list<GISubtexture *> clone_subtextures = clone_group->getSubtexturesToOrganize();
		for (list<GISubtexture *>::iterator it=clone_subtextures.begin(); it!=clone_subtextures.end(); it++) {
			GISubtexture *clone = new GISubtexture();
			clone->setName((*it)->getName());
			clone->setPixelSize(max((*it)->getPixelSize() * downscale_factor, 4.0f));
			subtextures_to_organize.push_back(clone);
		}
	}

	list<GISubtexture *> GITextureGroup::getSubtexturesToOrganize() {
		return subtextures_to_organize;
	}

	size_t GITextureGroup::getInstanceIndexCount() {
		return instance_indices.size();
	}

	void GITextureGroup::setCenter(Vector3 v) {
		center = v;
	}

	Vector3 GITextureGroup::getCenter() {
		return center;
	}

	void GITextureGroup::setRadius(float v) {
		radius = v;
	}

	float GITextureGroup::getRadius() {
		return radius;
	}

	void GITextureGroup::setFolderSize(unsigned int v) {
		folder_size = v;
	}

	int GITextureGroup::getFolderSize() {
		return folder_size;
	}

	GISubtexture *GITextureGroupInfo::getTextureByInstance(string instance, size_t quality_level) {
		size_t instance_index=0;
		bool found=false;

		for (size_t i=0; i<instance_names.size(); i++) {
			if (instance_names[i] == instance) {
				instance_index = i;
				found=true;
				break;
			}
		}

		if (found) {
			for (vector<GITextureGroup *>::iterator it=groups.begin(); it!=groups.end(); it++) {
				GISubtexture *result=(*it)->getTextureByInstance(instance_index, instance, quality_level, groups);
				if (result) return result;
			}
		}
		else Error::addMessage(Error::WARNING, "Couldn't find the instance name " + instance + " in the GITextureGroupInfo.");

		return NULL;
	}

	GITexture::~GITexture() {
		for (list<GISubtexture *>::iterator it=subtextures.begin(); it!=subtextures.end(); it++) {
			delete (*it);
		}
		subtextures.clear();
	}


	GITextureGroup::~GITextureGroup() {
		for (list<GITexture *>::iterator it=textures.begin(); it!=textures.end(); it++) {
			delete (*it);
		}
		textures.clear();
	}

	void GITextureGroupInfo::clean() {
		for (vector<GITextureGroup *>::iterator it=groups.begin(); it!=groups.end(); it++) {
			delete (*it);
		}
		groups.clear();
		instance_names.clear();
		instance_centers.clear();
		instance_radius.clear();
	}

	GITextureGroup *GITextureGroupInfo::createGroup() {
		GITextureGroup *group = new GITextureGroup();
		groups.push_back(group);
		return group;
	}

	int GITextureGroupInfo::getGroupIndex(GITextureGroup *group) {
		int groups_size = groups.size();
		for (int i = 0; i < groups_size; i++) {
			if (groups[i] == group) {
				return i;
			}
		}

		return -1;
	}

	int GITextureGroupInfo::getInstanceIndex(string instance_name) {
		int instances_size = instance_names.size();
		for (int i = 0; i < instances_size; i++) {
			if (instance_names[i] == instance_name) {
				return i;
			}
		}

		return -1;
	}

	void GITextureGroupInfo::sortGroupsByQualityLevel() {
		std::map<int, int> index_map;
		vector<GITextureGroup *> sorted_groups;
		int groups_size = groups.size();
		for (int level = 0; level < 3; level++) {
			for (int i = 0; i < groups_size; i++) {
				if (groups[i]->getQualityLevel() == level) {
					index_map[i] = sorted_groups.size();
					sorted_groups.push_back(groups[i]);
				}
			}
		}

		// Fix the group indices to point to the new ones if over quality level 0
		for (int i = 0; i < groups_size; i++) {
			if (sorted_groups[i]->getQualityLevel() != 0) {
				sorted_groups[i]->fixIndices(index_map);
			}
		}

		groups = sorted_groups;
	}
};