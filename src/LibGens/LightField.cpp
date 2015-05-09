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

#include "LightField.h"
#include "VRMap.h"
#include "Vertex.h"

namespace LibGens {
	void ColorPoint::read(File *file) {
	}

	void ColorPoint::write(File *file) {
		for (size_t x=0; x<8; x++) {
			for (size_t y=0; y<3; y++) {
				file->writeUChar(&rgb[x][y]);
			}
		}
		file->writeUChar(&flag);
	}


	void LightFieldCube::read(File *file, size_t head_address, AABB aabb) {
		file->readInt32BE(&type);
		file->readInt32BE(&value);

		point = aabb.center();

		if (type != LIBGENS_LIGHTFIELD_CUBE_NO_SPLIT) {
			left = new LightFieldCube();
			right = new LightFieldCube();

			file->goToAddress(head_address + LIBGENS_LIGHTFIELD_CUBE_SIZE * value);
			left->read(file, head_address, aabb.half(type, LIBGENS_MATH_SIDE_LEFT));

			file->goToAddress(head_address + LIBGENS_LIGHTFIELD_CUBE_SIZE * (value+1));
			right->read(file, head_address, aabb.half(type, LIBGENS_MATH_SIDE_RIGHT));
		}
		else {

		}
	}

	void LightFieldCube::write(File *file) {
		file->writeInt32BE(&type);
		file->writeInt32BE(&value);
	}

	void LightFieldCube::getCubeList(vector<LightFieldCube *> *list) {
		if (!list) {
			return;
		}

		if (type != LIBGENS_LIGHTFIELD_CUBE_NO_SPLIT) {
			value=list->size();
			list->push_back(left);
			list->push_back(right);

			left->getCubeList(list);
			right->getCubeList(list);
		}
		else {
		}
	}

	float total_area_last_progress=0;
	float total_area_progress=0;
	float total_area=0;

	SamplingPoint *LightField::createSamplingPoint(Vector3 point) {
		int x=0;
		int y=0;
		int z=0;
		x = (int)((point.x - world_aabb.start.x) / grid_size);
		y = (int)((point.y - world_aabb.start.y) / grid_size);
		z = (int)((point.z - world_aabb.start.z) / grid_size);

		for (size_t i=0; i<grid[x][y][z].size(); i++) {
			if ((point - grid[x][y][z][i]->point).length() < LIBGENS_LIGHTFIELD_SAMPLING_EPSILON) {
				return grid[x][y][z][i];
			}
		}


		SamplingPoint *sample=new SamplingPoint();
		sample->point = point;
		sampling_points.push_back(sample);
		grid[x][y][z].push_back(sample);
		return sample;
	}


	LightField::LightField(string filename) {
		File file(filename, LIBGENS_FILE_READ_BINARY);

		if (file.valid()) {
			file.readHeader();
			read(&file);
			file.close();
		}
	}

	void LightField::read(File *file) {
		size_t header_address=file->getCurrentAddress();
		world_aabb.read(file);

		int cube_count=0;
		size_t cube_address=0;
		int color_count=0;
		size_t color_address=0;
		int index_count=0;
		size_t index_address=0;

		file->readInt32BE(&cube_count);
		file->readInt32BEA(&cube_address);
		file->readInt32BE(&color_count);
		file->readInt32BEA(&color_address);
		file->readInt32BE(&index_count);
		file->readInt32BEA(&index_address);

		// LightFieldCubes
		// Recursive method traverses the entire Octree
		file->goToAddress(cube_address);
		cube=new LightFieldCube();
		cube->read(file, cube_address, world_aabb);
	}

	void LightField::save(string filename) {
		File file(filename, LIBGENS_FILE_WRITE_BINARY);

		if (file.valid()) {
			file.prepareHeader(LIBGENS_LIGHTFIELD_FILE_ROOT_TYPE);
			write(&file);
			file.writeHeader();
			file.close();
		}
	}

	void LightField::write(File *file) {
		world_aabb.write(file);

		vector<unsigned int> indices;

		size_t root_address=file->getCurrentAddress();
		file->writeNull(24);

		unsigned int cube_count=0;
		size_t cube_address=0;
		unsigned int color_count=0;
		size_t color_address=0;
		unsigned int index_count=0;
		size_t index_address=0;

		vector<LightFieldCube *> cubes;
		cubes.push_back(cube);
		cube->getCubeList(&cubes);
		cube_count = cubes.size();
		cube_address = file->getCurrentAddress();

		for (size_t i=0; i<cubes.size(); i++) {
			if (cubes[i]->getType() == LIBGENS_LIGHTFIELD_CUBE_NO_SPLIT) {
				cubes[i]->setValue(indices.size());
				for (size_t j=0; j<8; j++) {
					indices.push_back(cubes[i]->getCorner(j)->color->index);
				}
			}
			
			cubes[i]->write(file);
		}


		color_count = color_palette.size();
		color_address = file->getCurrentAddress();

		for (size_t i=0; i<color_palette.size(); i++) {
			color_palette[i]->write(file);
		}


		index_count = indices.size();
		index_address = file->getCurrentAddress();

		for (size_t i=0; i<indices.size(); i++) {
			file->writeInt32BE(&(indices[i]));
		}

		
		file->goToAddress(root_address);
		file->writeInt32BE(&cube_count);
		file->writeInt32BEA(&cube_address);
		file->writeInt32BE(&color_count);
		file->writeInt32BEA(&color_address);
		file->writeInt32BE(&index_count);
		file->writeInt32BEA(&index_address);

		file->goToEnd();
	}

	struct CubeParameter {
		LightFieldCube *cube; 
		AABB aabb;
		LightField *lightfield;
		unsigned int thread;
	};

	
	void *generateCube(void *data) {
		CubeParameter *object = (CubeParameter *) data;
		unsigned int thread = object->thread;
		LightField *lightfield = object->lightfield;
		lightfield->generateCubeProc(object);
		lightfield->setThreadStatus(thread, false);
		pthread_exit(0);
		return NULL;
	}

	pthread_mutex_t sampling_point_mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t area_mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t stack_mutex = PTHREAD_MUTEX_INITIALIZER;

	void LightField::generateCubeProc(CubeParameter *object) {
		LightFieldCube *cube = object->cube;
		AABB aabb = object->aabb;

		list<VRMapSample *> actual_samples=vrmap->getSampleListInAABB(aabb);

		float diffs[3];
		int current_winner=-1;
		float current_diff=0;

		diffs[0] = aabb.end.x - aabb.start.x;
		diffs[1] = aabb.end.y - aabb.start.y;
		diffs[2] = aabb.end.z - aabb.start.z;

		for (int axis=0; axis<=LIBGENS_MATH_AXIS_Z; axis++) {
			if ((diffs[axis] > current_diff) && (diffs[axis] > min_world_cube_size)) {
				current_diff   = diffs[axis];
				current_winner = axis;
			}
		}

		if ((actual_samples.size() <= sample_treshold) || (current_winner==-1)) {
			pthread_mutex_lock(&area_mutex);
			total_area_progress+=aabb.size();
			if ((((total_area_progress/total_area)*100.0 - (total_area_last_progress/total_area)*100.0)) >= 1.0f) {
				total_area_last_progress = total_area_progress;
				if (current_winner == -1) printf("Octree(%f %%): Minimum Size Reached\n", (total_area_progress/total_area)*100.0);
				else printf("Octree(%f %%): Samples %d\n", (total_area_progress/total_area)*100.0, actual_samples.size());
			}
			pthread_mutex_unlock(&area_mutex);
			

			// Define Lightfield Cube
			cube->setType(LIBGENS_LIGHTFIELD_CUBE_NO_SPLIT);

			pthread_mutex_lock(&sampling_point_mutex);
			for (size_t corner=0; corner<8; corner++) {
				cube->setCorner(createSamplingPoint(aabb.corner(corner)), corner);
			}
			pthread_mutex_unlock(&sampling_point_mutex);
		}
		else {
			cube->setType(current_winner);
			LightFieldCube *left  = new LightFieldCube();
			LightFieldCube *right = new LightFieldCube();

			cube->setLeft(left);
			cube->setRight(right);

			// Push other objects on the stack.
			CubeParameter *left_object=new CubeParameter;
			left_object->aabb = aabb.half(cube->getType(), LIBGENS_MATH_SIDE_LEFT);
			left_object->cube = left;
			left_object->lightfield = this;

			CubeParameter *right_object=new CubeParameter;
			right_object->aabb = aabb.half(cube->getType(), LIBGENS_MATH_SIDE_RIGHT);
			right_object->cube = right;
			right_object->lightfield = this;

			pushThreadStack(object->thread, left_object);
			pushThreadStack(object->thread, right_object);
		}

		delete object;
	}

	void LightField::generate(VRMap *vrmap_p, Color ambient_color, unsigned int sample_treshold_p, float sample_affect_distance_p, float min_world_cube_size_p, unsigned int threads) {
		sampling_points.clear();
		world_aabb = vrmap_p->getAABB();
		total_area = world_aabb.size();
		LightFieldCube *first_cube=new LightFieldCube();

		AABB aabb;
		while(!cube_stack.empty()) {
			cube_stack.pop();
		}

		CubeParameter *first_object=new CubeParameter;
		first_object->aabb = world_aabb;
		first_object->cube = first_cube;
		first_object->lightfield = this;

		vrmap = vrmap_p;
		sample_treshold = sample_treshold_p;
		sample_affect_distance = sample_affect_distance_p;
		min_world_cube_size = min_world_cube_size_p;

		// Setup a lookup grid for sampling point creation
		grid_size = vrmap->getGridSize();
		w=(int)(world_aabb.sizeX()/grid_size)+1;
		h=(int)(world_aabb.sizeY()/grid_size)+1;
		d=(int)(world_aabb.sizeZ()/grid_size)+1;
		grid.resize(w);
		for (int x=0; x<w; x++) {
			grid[x].resize(h);
			for (int y=0; y<h; y++) {
				grid[x][y].resize(d);
			}
		}

		// Create the first cube in the thread
		bool *first=new bool[threads];
		cube_stack.push(first_object);
		pthread_t *generation_threads = new pthread_t[threads];
		thread_status = new bool[threads];
		for (size_t i=0; i<threads; i++) {
			first[i] = true;
			thread_status[i] = false;
		}

		thread_stacks.resize(threads);

		// Setup scheduler to generate octree
		while(true) {
			bool done=true;
			for (size_t i=0; i<threads; i++) {
				if (!thread_status[i]) {
					if (!first[i]) {
						pthread_join(generation_threads[i],NULL);
					}

					while (!thread_stacks[i].empty()) {
						CubeParameter *object = thread_stacks[i].top();
						thread_stacks[i].pop();
						cube_stack.push(object);
					}

					if (!cube_stack.empty()) {
						CubeParameter *object = cube_stack.top();
						cube_stack.pop();

						setThreadStatus(i, true);
						object->thread = i;
						first[i] = false;

						pthread_create(&generation_threads[i], NULL, generateCube, object);
						done = false;
					}
				}
				else done = false;
			}

			if (done) break;
		}

		cube=first_cube;
		paintSamplingPoints(vrmap, ambient_color, sample_affect_distance);
	}


	void LightField::paintSamplingPoints(VRMap *vrmap, Color ambient_color, float sample_affect_distance) {
		Color color;
		unsigned char rgb[8][3]; 
		unsigned char flag=255;
		list<Vertex *> generation_vertices;

		size_t i=0;
		for (list<SamplingPoint *>::iterator it=sampling_points.begin(); it!=sampling_points.end(); it++) {
			list<VRMapSample *> samples=vrmap->getSampleListAroundPoint((*it)->point);
			list<VRMapSample *> sample_corners[8];

			for (size_t corner=0; corner<8; corner++) {
				sample_corners[corner].clear();
			}

			for (list<VRMapSample *>::iterator it2=samples.begin(); it2!=samples.end(); it2++) {
				size_t corner=(*it)->point.relativeCorner((*it2)->point);
				sample_corners[corner].push_back(*it2);
			}

			for (size_t corner=0; corner<8; corner++) {
				color = ambient_color;
				float total_mult=1.0f;
				float radius=sample_affect_distance;

				for (list<VRMapSample *>::iterator it2=samples.begin(); it2!=samples.end(); it2++) {
					Vector3 vect=((*it2)->point - (*it)->point);
					float distance=vect.length();
					if (distance < 0.0f) distance=0.0f;

					if (distance < radius) {
						float mult=1.0f-(distance/radius);

						if ((*it)->point.relativeCorner((*it2)->point) != corner) mult/=2;

						total_mult+=mult;
						color.r += (float)(*it2)->color.r / 255.0f * mult;
						color.g += (float)(*it2)->color.g / 255.0f * mult;
						color.b += (float)(*it2)->color.b / 255.0f * mult;
					}
				}

				color.r /= total_mult;
				color.g /= total_mult;
				color.b /= total_mult;

				rgb[corner][0] = (char)(color.r * 255.0f);
				rgb[corner][1] = (char)(color.g * 255.0f);
				rgb[corner][2] = (char)(color.b * 255.0f);

				if (corner == 0) printf("Painting sampling point %d of %d in corner %d: %d %d %d\n", i+1, sampling_points.size(), corner, rgb[corner][0], rgb[corner][1], rgb[corner][2]);
			}

			(*it)->color = createColorPoint(rgb, flag);
			i++;
		}
	}


	ColorPoint *LightField::createColorPoint(unsigned char rgb[8][3], unsigned char flag) {
		for (size_t i=0; i<color_palette.size(); i++) {
			bool result=true;

			for (size_t j=0; j<8; j++) {
				for (size_t k=0; k<3; k++) {
					if (color_palette[i]->rgb[j][k] != rgb[j][k]) {
						result=false;
						break;
					}
				}

				if (!result) break;
			}

			if (color_palette[i]->flag != flag) {
				result = false;
			}

			if (result) {
				return color_palette[i];
			}
		}

		ColorPoint *color=new ColorPoint();
		for (size_t j=0; j<8; j++) {
			for (size_t k=0; k<3; k++) {
				color->rgb[j][k] = rgb[j][k];
			}
		}
		color->flag = flag;
		color->index = color_palette.size();
		color_palette.push_back(color);
		return color;
	}
};