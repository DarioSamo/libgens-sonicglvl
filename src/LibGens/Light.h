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

#define LIBGENS_LIGHT_ERROR_MESSAGE_NULL_FILE        "Trying to read light data from unreferenced file."
#define LIBGENS_LIGHT_ERROR_MESSAGE_WRITE_NULL_FILE  "Trying to write light data to an unreferenced file."
#define LIBGENS_LIGHT_ROOT_GENERATIONS               1
#define LIBGENS_LIGHT_LIST_ROOT_GENERATIONS          0
#define LIBGENS_LIGHT_EXTENSION                      ".light"
#define LIBGENS_LIGHT_LIST_FILENAME                  "light-list.light-list"

#define LIBGENS_LIGHT_TYPE_DIRECTIONAL               0

namespace LibGens {
	class Light {
		protected:
			unsigned int type;
			Vector3 position;
			Vector3 color;
			string name;

			unsigned int omni_attribute;
			float inner_range;
			float outer_range;
		public:
			Light() {
			}

			Light(string filename);
			void save(string filename);

			void read(File *file);
			void write(File *file);

			unsigned int getType() {
				return type;
			}

			Vector3 getPosition() {
				return position;
			}

			Vector3 getColor() {
				return color;
			}

			string getName() {
				return name;
			}

			void setName(string v) {
				name = v;
			}

			float getInnerRange() {
				return inner_range;
			}

			float getOuterRange() {
				return outer_range;
			}
	};

	class LightList {
		protected:
			vector<string> names;
			vector<Light *> lights;
			string folder;
		public:
			LightList() {
			}

			LightList(string filename);
			void save(string filename);

			Light *getLight(string name);
			vector<Light *> getOmniLights();

			void read(File *file);
			void write(File *file);
	};
};
