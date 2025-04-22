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

#define LIBGENS_SHADER_ERROR_MESSAGE_NULL_FILE        "Trying to read shader data from unreferenced file."
#define LIBGENS_SHADER_LIST_EXTENSION                 ".shader-list"
#define LIBGENS_VERTEX_SHADER_EXTENSION               ".vertexshader"
#define LIBGENS_PIXEL_SHADER_EXTENSION                ".pixelshader"
#define LIBGENS_VERTEX_SHADER_PARAMS_EXTENSION        ".vsparam"
#define LIBGENS_PIXEL_SHADER_PARAMS_EXTENSION         ".psparam"

namespace LibGens {
	class ShaderParam {
		protected:
			string name;
			unsigned char index_flag;
			unsigned char size_flag;
		public:
			ShaderParam();
			void read(File *file);

			string getName() {
				return name;
			}

			unsigned char getIndex() {
				return index_flag;
			}

			unsigned char getSize() {
				return size_flag;
			}
	};

	class ShaderParams {
		protected:
			vector<ShaderParam *> parameter_lists[5];
			string name;
		public:
			ShaderParams();
			ShaderParams(string filename);
			void read(File *file);

			string getName() {
				return name;
			}

			void setName(string v) {
				name = v;
			}

			vector<ShaderParam *> getParameterList(size_t index) {
				vector<ShaderParam *> empty;
				empty.clear();

				if (index > 4) return empty;
				else return parameter_lists[index];
			}
	};

	class Shader {
		protected:
			string shader_filename;
			vector<string> shader_parameter_filenames;
			string name;
			string extra;
		public:
			Shader();
			Shader(string filename);
			void read(File *file);

			string getName() {
				return name;
			}

			string getShaderFilename() {
				return shader_filename;
			}

			vector<string> getShaderParameterFilenames() {
				return shader_parameter_filenames;
			}

			void setName(string v) {
				name = v;
			}

			void setExtra(string v) {
				extra=v;
			}

			string getExtra() {
				return extra;
			}

			bool hasExtra() {
				return (extra.size() > 0);
			}
	};

	class VertexShaderSet {
		protected:
			unsigned int shader_flag;
			string rendering_mode;
			string vertex_shader_name;
		public:
			VertexShaderSet();
			void read(File *file);

			string getRenderingMode() {
				return rendering_mode;
			}

			string getVertexShaderName() {
				return vertex_shader_name;
			}

			bool check(bool const_tex_coord);
	};

	class ShaderSet {
		protected:
			unsigned int shader_flag;
			string rendering_mode;
			string pixel_shader_name;

			vector<VertexShaderSet *> vertex_shader_sets;
		public:
			ShaderSet();
			void read(File *file);

			string getRenderingMode() {
				return rendering_mode;
			}

			string getPixelShaderName() {
				return pixel_shader_name;
			}

			string getVertexShaderName(string rendering_mode="");

			bool check(bool const_tex_coord, bool no_gi, bool no_light);
	};

	class ShaderList {
		protected:
			vector<ShaderSet *> shader_sets;
			string name;
		public:
			ShaderList();
			ShaderList(string filename);
			void read(File *file);

			ShaderSet *getShaderSet(string rendering_mode="");

			string getName() {
				return name;
			}

			void setName(string v) {
				name = v;
			}
	};

	class ArPack;
	class ArFile;

	class ShaderLibrary {
		protected:
			list<ShaderList *> shader_lists;

			list<Shader *> vertex_shaders;
			list<Shader *> pixel_shaders;

			list<ShaderParams *> vertex_shader_params;
			list<ShaderParams *> pixel_shader_params;

			string folder;
			ArPack* ar_pack;
		public:
			ShaderLibrary(string folder_p);

			bool loadShaderArchive(const string& filename);

			ArFile* getFile(string filename);
			ArFile* getFileByIndex(size_t index);
			size_t getFileCount();
			ShaderList *getShaderList(string id);
			Shader *getVertexShader(string id);
			Shader *getPixelShader(string id);
			ShaderParams *getVertexShaderParams(string id);
			ShaderParams *getPixelShaderParams(string id);

			bool getMaterialShaders(string shader_list_name, Shader *&vertex_shader, Shader *&pixel_shader, bool no_light=true, bool no_gi=true, bool const_tex_coord=true);


	};
};
