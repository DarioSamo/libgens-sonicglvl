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

#define LIBGENS_BITMAP_EXTENSION  ".png"

namespace LibGens {
	class Bitmap {
		protected:
			string name;
			ALLEGRO_BITMAP *bitmap;
		public:
			Bitmap(string id, string bitmap_folder);
			string getName();
			Color pickColor(Vector2 uv);

			static list<Bitmap *> loaded_bitmaps;
			static Bitmap *getBitmap(string id, string bitmap_folder);
			static void deleteBitmap(Bitmap *bitmap);
	};
};

