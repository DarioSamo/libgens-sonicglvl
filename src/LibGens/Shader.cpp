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

#include "Shader.h"

namespace LibGens {
	ShaderParam::ShaderParam() {
	}

	void ShaderParam::read(File *file) {
		size_t name_address=0;
		file->readInt32BEA(&name_address);
		file->readUChar(&index_flag);
		file->readUChar(&size_flag);

		file->goToAddress(name_address);
		file->readString(&name);

		//printf("   Parameter %s - Index: %d Size: %d\n", name.c_str(), (int)index_flag, (int)size_flag);
	}

	
	ShaderParams::ShaderParams() {
	}

	ShaderParams::ShaderParams(string filename) {
		File file(filename, LIBGENS_FILE_READ_BINARY);

		name = filename;
		size_t sep = name.find_last_of("\\/");
		if (sep != std::string::npos) {
			name = name.substr(sep + 1, name.size() - sep - 1);
		}
	
		size_t dot = name.find_last_of(".");
		if (dot != string::npos) {
			name = name.substr(0, dot);
		}

		if (file.valid()) {
			file.readHeader();
			read(&file);
			file.close();
		}
	}
	
	void ShaderParams::read(File *file) {
		size_t header_address=file->getCurrentAddress();

		for (size_t slot=0; slot<5; slot++) {
			file->goToAddress(header_address + slot*8);

			unsigned int parameters_count=0;
			size_t parameters_table_address=0;
			file->readInt32BE(&parameters_count);
			file->readInt32BEA(&parameters_table_address);

			//printf("Reading parameter list #%d with %d elements.\n", slot, parameters_count);
			
			for (size_t i=0; i<parameters_count; i++) {
				file->goToAddress(parameters_table_address + i*4);

				size_t address=0;
				file->readInt32BEA(&address);
				file->goToAddress(address);

				ShaderParam *shader_param=new ShaderParam();
				shader_param->read(file);
				parameter_lists[slot].push_back(shader_param);
			}
		}
	}


	
	Shader::Shader() {
		extra = "";
	}

	Shader::Shader(string filename) {
		File file(filename, LIBGENS_FILE_READ_BINARY);

		extra = "";

		name = filename;
		size_t sep = name.find_last_of("\\/");
		if (sep != std::string::npos) {
			name = name.substr(sep + 1, name.size() - sep - 1);
		}
	
		size_t dot = name.find_last_of(".");
		if (dot != string::npos) {
			name = name.substr(0, dot);
		}

		if (file.valid()) {
			file.readHeader();
			read(&file);
			file.close();
		}
	}
	
	void Shader::read(File *file) {
		size_t shader_filename_address=0;
		unsigned int shader_parameter_count=0;
		size_t shader_parameter_table_address=0;

		file->readInt32BEA(&shader_filename_address);
		file->readInt32BE(&shader_parameter_count);
		file->readInt32BEA(&shader_parameter_table_address);

		file->goToAddress(shader_filename_address);
		file->readString(&shader_filename);

		//printf("Shader that uses the %s file and has %d parameter files.\n", shader_filename.c_str(), shader_parameter_count);

		for (size_t i=0; i<shader_parameter_count; i++) {
			file->goToAddress(shader_parameter_table_address + i*4);

			size_t address=0;
			file->readInt32BEA(&address);
			file->goToAddress(address);

			string shader_parameter="";
			file->readString(&shader_parameter);
			shader_parameter_filenames.push_back(shader_parameter);

			//printf("   Parameter file: %s\n", shader_parameter.c_str());
		}
	}


	VertexShaderSet::VertexShaderSet() {
	}

	void VertexShaderSet::read(File *file) {
		size_t rendering_mode_address=0;
		size_t vertex_shader_name_address=0;

		file->readInt32BE(&shader_flag);
		file->readInt32BEA(&rendering_mode_address);
		file->readInt32BEA(&vertex_shader_name_address);

		file->goToAddress(rendering_mode_address);
		file->readString(&rendering_mode);

		file->goToAddress(vertex_shader_name_address);
		file->readString(&vertex_shader_name);

		//printf("  Vertex Shader Set %s (Mode: %s, Flag: %d)\n", vertex_shader_name.c_str(), rendering_mode.c_str(), shader_flag);
	}


	ShaderSet::ShaderSet() {
	}

	void ShaderSet::read(File *file) {
		size_t rendering_mode_address=0;
		size_t pixel_shader_name_address=0;
		unsigned int vertex_shader_sets_count=0;
		size_t vertex_shader_set_table_address=0;

		file->readInt32BE(&shader_flag);
		file->readInt32BEA(&rendering_mode_address);
		file->readInt32BEA(&pixel_shader_name_address);
		file->readInt32BE(&vertex_shader_sets_count);
		file->readInt32BEA(&vertex_shader_set_table_address);

		file->goToAddress(rendering_mode_address);
		file->readString(&rendering_mode);

		file->goToAddress(pixel_shader_name_address);
		file->readString(&pixel_shader_name);

		//printf("Pixel Shader Set %s (Mode: %s, Flag: %d) with %d Vertex Shader sets.\n", pixel_shader_name.c_str(), rendering_mode.c_str(), shader_flag, vertex_shader_sets_count);

		for (size_t i=0; i<vertex_shader_sets_count; i++) {
			file->goToAddress(vertex_shader_set_table_address + i*4);

			size_t address=0;
			file->readInt32BEA(&address);
			file->goToAddress(address);

			VertexShaderSet *vertex_shader_set = new VertexShaderSet();
			vertex_shader_set->read(file);
			vertex_shader_sets.push_back(vertex_shader_set);
		}
	}

	string ShaderSet::getVertexShaderName(string rendering_mode) {
		for (size_t i=0; i<vertex_shader_sets.size(); i++) {
			if (!rendering_mode.size()) {
				return vertex_shader_sets[i]->getVertexShaderName();
			}
			else {
				if (rendering_mode == vertex_shader_sets[i]->getRenderingMode()) {
					return vertex_shader_sets[i]->getVertexShaderName();
				}
			}
		}

		return "";
	}


	ShaderList::ShaderList() {
	}

	ShaderList::ShaderList(string filename) {
		File file(filename, LIBGENS_FILE_READ_BINARY);

		name = filename;
		size_t sep = name.find_last_of("\\/");
		if (sep != std::string::npos) {
			name = name.substr(sep + 1, name.size() - sep - 1);
		}
	
		size_t dot = name.find_last_of(".");
		if (dot != string::npos) {
			name = name.substr(0, dot);
		}

		if (file.valid()) {
			file.readHeader();
			read(&file);
			file.close();
		}
	}
	
	void ShaderList::read(File *file) {
		unsigned int shader_set_count=0;
		size_t shader_set_table_address=0;

		file->readInt32BE(&shader_set_count);
		file->readInt32BEA(&shader_set_table_address);

		//printf("Found %d Pixel Shader Sets in shader list.\n", shader_set_count);

		for (size_t i=0; i<shader_set_count; i++) {
			file->goToAddress(shader_set_table_address + i*4);

			size_t address=0;
			file->readInt32BEA(&address);
			file->goToAddress(address);

			ShaderSet *shader_set = new ShaderSet();
			shader_set->read(file);
			shader_sets.push_back(shader_set);
		}
	}

	ShaderSet *ShaderList::getShaderSet(string rendering_mode) {
		for (size_t i=0; i<shader_sets.size(); i++) {
			if (!rendering_mode.size()) {
				return shader_sets[i];
			}
			else {
				if (rendering_mode == shader_sets[i]->getRenderingMode()) {
					return shader_sets[i];
				}
			}
		}

		return NULL;
	}


	ShaderList *ShaderLibrary::getShaderList(string id) {
		for (list<ShaderList *>::iterator it=shader_lists.begin(); it!=shader_lists.end(); it++) {
			if ((*it)->getName() == id) {
				return *it;
			}
		}
		
		string new_filename = folder + id + LIBGENS_SHADER_LIST_EXTENSION;
		if (File::check(new_filename)) {
			ShaderList *shader_list=new ShaderList(new_filename);
			shader_lists.push_back(shader_list);
			return shader_list;
		}
		return NULL;
	}

	Shader *ShaderLibrary::getVertexShader(string id) {
		for (list<Shader *>::iterator it=vertex_shaders.begin(); it!=vertex_shaders.end(); it++) {
			if ((*it)->getName() == id) {
				return *it;
			}
		}
		
		Shader *vertex_shader=new Shader(folder + id + LIBGENS_VERTEX_SHADER_EXTENSION);
		vertex_shaders.push_back(vertex_shader);
		return vertex_shader;
	}

	Shader *ShaderLibrary::getPixelShader(string id) {
		for (list<Shader *>::iterator it=pixel_shaders.begin(); it!=pixel_shaders.end(); it++) {
			if ((*it)->getName() == id) {
				return *it;
			}
		}
		
		Shader *pixel_shader=new Shader(folder + id + LIBGENS_PIXEL_SHADER_EXTENSION);
		pixel_shaders.push_back(pixel_shader);
		return pixel_shader;
	}

	ShaderParams *ShaderLibrary::getVertexShaderParams(string id) {
		for (list<ShaderParams *>::iterator it=vertex_shader_params.begin(); it!=vertex_shader_params.end(); it++) {
			if ((*it)->getName() == id) {
				return *it;
			}
		}
		
		ShaderParams *vertex_shader_param=new ShaderParams(folder + id + LIBGENS_VERTEX_SHADER_PARAMS_EXTENSION);
		vertex_shader_params.push_back(vertex_shader_param);
		return vertex_shader_param;
	}

	ShaderParams *ShaderLibrary::getPixelShaderParams(string id) {
		for (list<ShaderParams *>::iterator it=pixel_shader_params.begin(); it!=pixel_shader_params.end(); it++) {
			if ((*it)->getName() == id) {
				return *it;
			}
		}
		
		ShaderParams *pixel_shader_param=new ShaderParams(folder + id + LIBGENS_PIXEL_SHADER_PARAMS_EXTENSION);
		pixel_shader_params.push_back(pixel_shader_param);
		return pixel_shader_param;
	}

	bool ShaderLibrary::getMaterialShaders(string shader_list_name, Shader *&vertex_shader, Shader *&pixel_shader, bool no_light, bool no_gi, bool const_tex_coord) {
		ShaderList *shader_list=getShaderList(shader_list_name);
		if (shader_list) {
			ShaderSet *shader_set=shader_list->getShaderSet();
			
			if (shader_set) {
				// Pixel Shader
				string pixel_shader_name=shader_set->getPixelShaderName();
				if (no_light) pixel_shader_name += "_NoLight";
				if (no_gi) pixel_shader_name += "_NoGI";
				if (const_tex_coord) pixel_shader_name += "_ConstTexCoord";
				pixel_shader = getPixelShader(pixel_shader_name);

				// Vertex Shader
				string vertex_shader_name = shader_set->getVertexShaderName();

				if (const_tex_coord) vertex_shader_name += "_ConstTexCoord";

				if (vertex_shader_name.size()) {
					vertex_shader = getVertexShader(vertex_shader_name);
				}

				return true;
			}
		}

		return false;
	}
};