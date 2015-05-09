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

#include "VRMap.h"

namespace LibGens {
	void VRMap::generateLookupGrid(float gsize, float unit_size) {
		if (!samples.size()) {
			Error::addMessage(Error::WARNING, LIBGENS_VRMAP_ERROR_MESSAGE_NO_SAMPLES);
			return;
		}

		grid_size=gsize;
		
		aabb.reset();
		for (list<VRMapSample *>::iterator it=samples.begin(); it!=samples.end(); it++) {
			aabb.addPoint((*it)->point);
		}
		aabb.expand(grid_size);
		
		w=(int)(aabb.sizeX()/grid_size)+1;
		h=(int)(aabb.sizeY()/grid_size)+1;
		d=(int)(aabb.sizeZ()/grid_size)+1;

		grid.resize(w);
		for (int x=0; x<w; x++) {
			grid[x].resize(h);
			for (int y=0; y<h; y++) {
				grid[x][y].resize(d);
			}
		}

		printf("Sorting %d samples into lookup grid.\n", samples.size());

		for (list<VRMapSample *>::iterator it=samples.begin(); it!=samples.end();) {
			int x=0;
			int y=0;
			int z=0;

			x = (int)(((*it)->point.x - aabb.start.x) / grid_size);
			y = (int)(((*it)->point.y - aabb.start.y) / grid_size);
			z = (int)(((*it)->point.z - aabb.start.z) / grid_size);

			
			bool can_add=true;

			for (size_t i=0; i<grid[x][y][z].size(); i++) {
				if (((*it)->point - grid[x][y][z][i]->point).length() < (unit_size)) {
					can_add = false;
					break;
				}
			}

			if (can_add) {
				grid[x][y][z].push_back(*it);
				++it;
			}
			else {
				delete *it;
				samples.erase(it++);
			}
		}
	}

	list<VRMapSample *> VRMap::getSampleListAroundPoint(Vector3 point) {
		list<VRMapSample *> results;

		int px=(int)((point.x - aabb.start.x) / grid_size);
		int py=(int)((point.y - aabb.start.y) / grid_size);
		int pz=(int)((point.z - aabb.start.z) / grid_size);

		int sx = px - 1;
		int sy = py - 1;
		int sz = pz - 1;
		int fx = px + 1;
		int fy = py + 1;
		int fz = pz + 1;

		if (sx < 0) sx = 0;
		if (sx > w-1) sx = w-1;
		if (fx < 0) fx = 0;
		if (fx > w-1) fx = w-1;

		if (sy < 0) sy = 0;
		if (sy > h-1) sy = h-1;
		if (fy < 0) fy = 0;
		if (fy > h-1) fy = h-1;

		if (sz < 0) sz = 0;
		if (sz > d-1) sz = d-1;
		if (fz < 0) fz = 0;
		if (fz > d-1) fz = d-1;

		for (int x=sx; x<=fx; x++) {
			for (int y=sy; y<=fy; y++) {
				for (int z=sz; z<=fz; z++) {
					for (size_t i=0; i<grid[x][y][z].size(); i++) {
						results.push_back(grid[x][y][z][i]);
					}
				}
			}
		}

		return results;
	}

	list<VRMapSample *> VRMap::getSampleListInAABB(AABB s_aabb) {
		list<VRMapSample *> results;

		int sx=(int)((s_aabb.start.x - aabb.start.x) / grid_size) - 1;
		int sy=(int)((s_aabb.start.y - aabb.start.y) / grid_size) - 1;
		int sz=(int)((s_aabb.start.z - aabb.start.z) / grid_size) - 1;
		int fx=(int)((s_aabb.end.x - aabb.start.x) / grid_size) + 1;
		int fy=(int)((s_aabb.end.y - aabb.start.y) / grid_size) + 1;
		int fz=(int)((s_aabb.end.z - aabb.start.z) / grid_size) + 1;

		if (sx < 0) sx = 0;
		if (sx > w-1) sx = w-1;
		if (fx < 0) fx = 0;
		if (fx > w-1) fx = w-1;

		if (sy < 0) sy = 0;
		if (sy > h-1) sy = h-1;
		if (fy < 0) fy = 0;
		if (fy > h-1) fy = h-1;

		if (sz < 0) sz = 0;
		if (sz > d-1) sz = d-1;
		if (fz < 0) fz = 0;
		if (fz > d-1) fz = d-1;

		for (int x=sx; x<=fx; x++) {
			for (int y=sy; y<=fy; y++) {
				for (int z=sz; z<=fz; z++) {
					for (size_t i=0; i<grid[x][y][z].size(); i++) {
						if (s_aabb.intersects(grid[x][y][z][i]->point)) {
							results.push_back(grid[x][y][z][i]);
						}
					}
				}
			}
		}

		return results;
	}
};