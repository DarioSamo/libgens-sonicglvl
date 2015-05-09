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

#define LIBGENS_MATERIAL_ERROR_MESSAGE_NULL_FILE        "Trying to read material data from unreferenced file."
#define LIBGENS_MATERIAL_ERROR_MESSAGE_WRITE_NULL_FILE  "Trying to write material data to an unreferenced file."
#define LIBGENS_MATERIAL_EXTENSION                      ".material"
#define LIBGENS_TEXTURE_FILE_EXTENSION                  ".dds"

#define LIBGENS_MATERIAL_TEXTURE_UNIT_DIFFUSE           "diffuse"
#define LIBGENS_MATERIAL_TEXTURE_UNIT_OPACITY           "opacity"
#define LIBGENS_MATERIAL_TEXTURE_UNIT_SPECULAR          "specular"
#define LIBGENS_MATERIAL_TEXTURE_UNIT_GLOSS             "gloss"
#define LIBGENS_MATERIAL_TEXTURE_UNIT_NORMAL            "normal"
#define LIBGENS_MATERIAL_TEXTURE_UNIT_DISPLACEMENT      "displacement"
#define LIBGENS_MATERIAL_TEXTURE_UNIT_REFLECTION        "reflection"

#define LIBGENS_MATERIAL_ROOT_GENERATIONS               3

namespace LibGens {
	class Texture;
	class Parameter;

	class Material {
		protected:
			string name;
			string folder;
			vector<Texture *> textures;
			vector<Parameter *> parameters;
			string shader;
			string sub_shader;
			string extra;
			string gi_extra;
			unsigned char material_flag;
			bool no_culling;
			bool color_blend;
		public:
			Material();
			Material(string filename);
			void read(File *file);
			void write(File *file);
			void save(string filename);
			void setShader(string v);
			string getShader();
			Texture *getTextureByUnit(string unit, size_t offset_count=0);
			Parameter *getParameterByName(string unit);
			Parameter *getParameterByIndex(size_t i);
			vector<Parameter *> getParameters();
			vector<Texture *> getTextureUnits();
			void addTextureUnit(Texture *texture);
			void addParameter(Parameter *parameter);
			string getName();
			string getFolder();
			void setExtra(string v);
			string getExtra();
			bool hasExtra();
			void setExtraGI(string v);
			string getExtraGI();
			bool hasExtraGI();
			bool hasNoCulling();
			bool hasColorBlend();
	};
};

