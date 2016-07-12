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

#include "Bitmap.h"

namespace LibGens {
	list<Bitmap *> Bitmap::loaded_bitmaps;

	Bitmap::Bitmap(string id, string terrain_folder) {
		string filename=terrain_folder + id + LIBGENS_BITMAP_EXTENSION;
		File file(filename, LIBGENS_FILE_READ_BINARY);
		name=id;

		if (file.valid()) {
			file.close();

			bitmap=al_load_bitmap(filename.c_str());
		}
	}

	Color Bitmap::pickColor(Vector2 uv) {
		if (!bitmap) return Color();

		int w=al_get_bitmap_width(bitmap);
		int h=al_get_bitmap_height(bitmap);

		int x=uv.x*w;
		int y=uv.y*h;

		ALLEGRO_COLOR c=al_get_pixel(bitmap, x, y);
		Color out;
		al_unmap_rgba_f(c, &out.r, &out.g, &out.b, &out.a);
		return out;
	}

	string Bitmap::getName() {
		return name;
	}

	Bitmap *Bitmap::getBitmap(string id, string terrain_folder) {
		for (list<Bitmap *>::iterator it=loaded_bitmaps.begin(); it!=loaded_bitmaps.end(); it++) {
			if ((*it)->getName() == id) {
				return *it;
			}
		}

		Bitmap *mat=new Bitmap(id, terrain_folder);
		loaded_bitmaps.push_back(mat);
		return mat;
	}

	void Bitmap::deleteBitmap(Bitmap *bitmap) {
		for (list<Bitmap *>::iterator it=loaded_bitmaps.begin(); it!=loaded_bitmaps.end(); it++) {
			if ((*it) == bitmap) {
				loaded_bitmaps.erase(it);
				delete *it;
				return;
			}
		}
	}
};