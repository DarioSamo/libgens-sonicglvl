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

#define LIBGENS_MATERIAL_ROOT_UNLEASHED                 1
#define LIBGENS_MATERIAL_ROOT_GENERATIONS               3

namespace LibGens {
	class Texture;
	class Parameter;
	class SampleChunkProperty;

	class Material {
		protected:
			string name;
			string folder;
			vector<Texture *> textures;
			vector<Parameter *> parameters;
			vector<SampleChunkProperty *> properties;
			string shader;
			string sub_shader;
			string extra;
			string gi_extra;
			string layer;
			unsigned char material_flag;
			bool no_culling;
			bool color_blend;
		public:
			static const string LayerOpaq;
			static const string LayerTrans;
			static const string LayerPunch;

			Material();
			Material(string filename);
			~Material();
			void read(File *file);
			void readRootNodeGenerations(File *file);
			void readRootNodeUnleashed(File *file);
			void readRootNodeLostWorld(File *file);
			void write(File *file);
			void writeRootNodeGenerations(File *file);
			void writeRootNodeUnleashed(File *file);
			void writeRootNodeLostWorld(File *file);
			void save(string filename, int root_type = LIBGENS_MATERIAL_ROOT_GENERATIONS);
			void setShader(string v);
			string getShader();
			void setLayer(string v);
			string getLayer();
			Texture *getTextureByUnit(string unit, size_t offset_count=0);
			Texture* Material::getTextureByIndex(size_t i);
			Parameter *getParameterByName(string unit);
			Parameter *getParameterByIndex(size_t i);
			vector<Parameter *> getParameters();
			vector<Texture *> getTextureUnits();
			void removeParameter(string name);
			void removeParameterByIndex(size_t i);
			void removeAllParameters();
			void removeTextureUnit(string unit);
			void removeTextureUnitByIndex(size_t i);
			int getTextureUnitsSize();
			void addTextureUnit(Texture *texture);
			void addParameter(Parameter *parameter);
			void setParameter(string parameter_name, Color color);
			void setName(string v);
			string getName();
			string getFolder();
			void setExtra(string v);
			string getExtra();
			bool hasExtra();
			void setExtraGI(string v);
			string getExtraGI();
			bool hasExtraGI();
			void setNoCulling(bool v);
			bool hasNoCulling();
			void setColorBlend(bool v);
			bool hasColorBlend();
			void setPropertyValue(string name, unsigned int value);
	};
};

