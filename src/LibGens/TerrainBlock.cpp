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

#include "TerrainBlock.h"
#include "TerrainInstance.h"
#include "TerrainGroup.h"

namespace LibGens {
	TerrainBlock::TerrainBlock(string filename) {
		File file(filename, LIBGENS_FILE_READ_BINARY);

		if (file.valid()) {
			file.readHeader();
			read(&file);
			file.close();
		}
	}


	void TerrainBlock::save(string filename) {
		File file(filename, LIBGENS_FILE_WRITE_BINARY);

		if (file.valid()) {
			file.prepareHeader(LIBGENS_TERRAIN_BLOCK_ROOT_GENERATIONS);
			write(&file);
			file.writeHeader();
			file.close();
		}
	}


	void TerrainBlockInstance::read(File *file) {
		size_t header_address=file->getCurrentAddress();

		file->readInt32BE(&type);
		file->readInt32BE(&identifier_a);
		file->readInt32BE(&identifier_b);

		size_t sphere_address=0;
		file->readInt32BEA(&sphere_address);
		file->goToAddress(sphere_address);
		center.read(file);
		file->readFloat32BE(&radius);
	}


	void TerrainBlockInstance::write(File *file) {
		size_t header_address=file->getCurrentAddress();

		file->writeInt32BE(&type);
		file->writeInt32BE(&identifier_a);
		file->writeInt32BE(&identifier_b);

		size_t sphere_address=file->getCurrentAddress()+4;
		file->writeInt32BEA(&sphere_address);
		center.write(file);
		file->writeFloat32BE(&radius);

		file->goToEnd();
	}

	void TerrainBlockInstance::setType(unsigned int v) {
		type = v;
	}

	void TerrainBlockInstance::setIdentifierA(unsigned int v) {
		identifier_a = v;
	}

	void TerrainBlockInstance::setIdentifierB(unsigned int v) {
		identifier_b = v;
	}

	void TerrainBlockInstance::setCenter(Vector3 v) {
		center = v;
	}

	void TerrainBlockInstance::setRadius(float v) {
		radius = v;
	}
	
	void TerrainBlock::read(File *file) {
		size_t header_address=file->getCurrentAddress();

		unsigned int instance_count=0;
		size_t instance_table_address=0;
		file->readInt32BE(&instance_count);
		file->readInt32BEA(&instance_table_address);
		file->readInt32BE(&root_instance_index);

		blocks.reserve(instance_count);
		for (size_t i=0; i<instance_count; i++) {
			file->goToAddress(instance_table_address + i * file->getAddressSize());
			size_t address=0;
			file->readInt32BEA(&address);
			file->goToAddress(address);

			TerrainBlockInstance *block=new TerrainBlockInstance();
			block->read(file);
			blocks.push_back(block);
		}
	}


	void TerrainBlock::write(File *file) {
		size_t header_address=file->getCurrentAddress();

		unsigned int instance_count=blocks.size();
		size_t instance_table_address=36;
		file->writeInt32BE(&instance_count);
		file->writeInt32BEA(&instance_table_address);
		file->writeInt32BE(&root_instance_index);

		vector<size_t> instance_addresses;
		file->writeNull(instance_count*4);
		for (size_t i=0; i<instance_count; i++) {
			instance_addresses.push_back(file->getCurrentAddress());
			blocks[i]->write(file);
		}

		for (size_t i=0; i<instance_count; i++) {
			file->goToAddress(instance_table_address+i*4);
			file->writeInt32BEA(&instance_addresses[i]);
		}

		file->goToEnd();
	}

	void TerrainBlock::addBlockInstance(TerrainBlockInstance *instance) {
		blocks.push_back(instance);
	}

    size_t TerrainBlock::getBlockInstanceCount() {
		return blocks.size();
    }

    struct TerrainBlockInstanceCache {
		LibGens::AABB aabb;
		Vector3 center;
		uint32_t terrain_group_index;
		uint32_t instance_index;
	};

	int buildRecursively(const std::vector<const TerrainBlockInstanceCache*>& items, TerrainBlock* block) {
		if (items.empty()) {
			return -1;
		}

		LibGens::TerrainBlockInstance* instance = new LibGens::TerrainBlockInstance();

		LibGens::AABB aabb;
		aabb.reset();

		for (size_t i = 0; i < items.size(); i++)
			aabb.merge(items[i]->aabb);

		const Vector3 center = aabb.center();

		instance->setCenter(center);
		instance->setRadius(aabb.radius());

		if (items.size() == 1) {
			instance->setType(LIBGENS_TERRAIN_BLOCK_INSTANCE_TYPE_LEAF);
			instance->setIdentifierA(items[0]->terrain_group_index);
			instance->setIdentifierB(items[0]->instance_index);
		}

		else {
			instance->setType(LIBGENS_TERRAIN_BLOCK_INSTANCE_TYPE_BRANCH);

			size_t dimIndex = 0;
			float maxDim = 0;

			for (size_t i = 0; i < 3; i++) {
				const float currDim = i == 0 ? aabb.sizeX() : i == 1 ? aabb.sizeY() : aabb.sizeZ();
				if (currDim > maxDim) {
					dimIndex = i;
					maxDim = currDim;
				}
			}

			std::vector<const TerrainBlockInstanceCache*> left;
			std::vector<const TerrainBlockInstanceCache*> right;

			for (size_t i = 0; i < items.size(); i++) {
				if (((float*)&items[i]->center)[dimIndex] < ((float*)&center)[dimIndex])
					left.push_back(items[i]);
				else
					right.push_back(items[i]);
			}

			if (left.empty())
				std::swap(left, right);

			if (right.empty()) {
				for (size_t i = 0; i < left.size(); i++) {
					if ((i & 1) == 0) {
						right.push_back(left.back());
						left.pop_back();
					}
				}
			}

			const int index0 = buildRecursively(left, block);
			const int index1 = buildRecursively(right, block);

			if (index0 == -1) {
				delete instance;
				return index1;
			}

			if (index1 == -1) {
				delete instance;
				return index0;
			}

			instance->setIdentifierA(index0);
			instance->setIdentifierB(index1);
		}

		const int index = block->getBlockInstanceCount();
		block->addBlockInstance(instance);
		return index;
	}

	void TerrainBlock::build(const std::vector<TerrainGroup*>& groups) {
		std::vector<const TerrainBlockInstanceCache*> items;

		for (size_t i = 0; i < groups.size(); i++) {
			std::vector<std::vector<TerrainInstance*>> instance_vectors = groups[i]->getInstanceVectors();

			for (size_t j = 0; j < instance_vectors.size(); j++) {
				TerrainBlockInstanceCache* item = new TerrainBlockInstanceCache();

				item->aabb.reset();
				for (size_t k = 0; k < instance_vectors[j].size(); k++)
					item->aabb.merge(instance_vectors[j][k]->getAABB());

				item->center = item->aabb.center();
				item->terrain_group_index = i;
				item->instance_index = j;

				items.push_back(item);
			}
		}

		root_instance_index = buildRecursively(items, this);

		for (size_t i = 0; i < items.size(); i++)
			delete items[i];
	}
};