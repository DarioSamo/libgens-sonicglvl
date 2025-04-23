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

// Sonic Lost World .pac file (replacement for .ar)
#pragma once

#define LIBGENS_PAC_METAINFO_HEADER                    "PACPACK_METADATA"

#define LIBGENS_PAC_EXTENSION_ANM					   "anm"
#define LIBGENS_PAC_EXTENSION_ANM_FULL				   "anm:ResCharAnimScript"
#define LIBGENS_PAC_EXTENSION_LIGHT					   "light"
#define LIBGENS_PAC_EXTENSION_LIGHT_FULL			   "light:ResMirageLight"
#define LIBGENS_PAC_EXTENSION_LFT					   "lft"
#define LIBGENS_PAC_EXTENSION_LFT_FULL				   "lft:ResMirageLightField"
#define LIBGENS_PAC_EXTENSION_GISM                     "gism"
#define LIBGENS_PAC_EXTENSION_GISM_FULL                "gism:ResGismoConfig"
#define LIBGENS_PAC_EXTENSION_HHD                      "hhd"
#define LIBGENS_PAC_EXTENSION_HHD_FULL                 "hhd:ResCustomData"
#define LIBGENS_PAC_EXTENSION_PAC_PHY_HKX              "phy.hkx"
#define LIBGENS_PAC_EXTENSION_PAC_PHY_HKX_FULL         "phy.hkx:ResHavokMesh"
#define LIBGENS_PAC_EXTENSION_LUA		               "lua"
#define LIBGENS_PAC_EXTENSION_PAC_LUA_FULL             "lua:ResLuaData"
#define LIBGENS_PAC_EXTENSION_TTF                      "ttf"
#define LIBGENS_PAC_EXTENSION_TTF_FULL                 "ttf:ResTTFData"
#define LIBGENS_PAC_EXTENSION_OTF                      "otf"
#define LIBGENS_PAC_EXTENSION_OTF_FULL                 "otf:ResOTFData"
#define LIBGENS_PAC_EXTENSION_KERN                     "kern.bin"
#define LIBGENS_PAC_EXTENSION_KERN_FULL                "kern.bin:ResKerningData"
#define LIBGENS_PAC_EXTENSION_MATERIAL                 "material"
#define LIBGENS_PAC_EXTENSION_MATERIAL_FULL            "material:ResMirageMaterial"
#define LIBGENS_PAC_EXTENSION_TERRAIN_INSTANCEINFO      "terrain-instanceinfo"
#define LIBGENS_PAC_EXTENSION_TERRAIN_INSTANCEINFO_FULL "terrain-instanceinfo:ResMirageTerrainInstanceInfo"
#define LIBGENS_PAC_EXTENSION_MODEL_INSTANCEINFO       "model-instanceinfo"
#define LIBGENS_PAC_EXTENSION_MODEL_INSTANCEINFO_FULL  "model-instanceinfo:ResModelInstanceInfo"
#define LIBGENS_PAC_EXTENSION_TERRAIN_MODEL     	   "terrain-model"
#define LIBGENS_PAC_EXTENSION_TERRAIN_MODEL_FULL	   "terrain-model:ResMirageTerrainModel"
#define LIBGENS_PAC_EXTENSION_PAC_DEPEND               "pac.d"
#define LIBGENS_PAC_EXTENSION_PAC_DEPEND_FULL          "pac.d:ResPacDepend"
#define LIBGENS_PAC_EXTENSION_PAC_FXCOL                "fxcol.bin"
#define LIBGENS_PAC_EXTENSION_PAC_FXCOL_FULL           "fxcol.bin:ResFxColFile"
#define LIBGENS_PAC_EXTENSION_PAC_RAW                  ""
#define LIBGENS_PAC_EXTENSION_PAC_RAW_FULL             ":ResRawData"
#define LIBGENS_PAC_EXTENSION_PAC_MAT_ANIM             "mat-anim"
#define LIBGENS_PAC_EXTENSION_PAC_MAT_ANIM_FULL        "mat-anim:ResAnimMaterial"
#define LIBGENS_PAC_EXTENSION_PAC_ANM_HKX              "anm.hkx"
#define LIBGENS_PAC_EXTENSION_PAC_ANM_HKX_FULL         "anm.hkx:ResAnimSkeleton"
#define LIBGENS_PAC_EXTENSION_PAC_UV_ANIM              "uv-anim"
#define LIBGENS_PAC_EXTENSION_PAC_UV_ANIM_FULL         "uv-anim:ResAnimTexSrt"
#define LIBGENS_PAC_EXTENSION_PAC_VIS_ANIM             "vis-anim"
#define LIBGENS_PAC_EXTENSION_PAC_VIS_ANIM_FULL        "vis-anim:ResAnimVis"
#define LIBGENS_PAC_EXTENSION_PAC_EFFECT               "effect"
#define LIBGENS_PAC_EXTENSION_PAC_EFFECT_FULL          "effect:ResGrifEffect"
#define LIBGENS_PAC_EXTENSION_PAC_MODEL                "model"
#define LIBGENS_PAC_EXTENSION_PAC_MODEL_FULL           "model:ResModel"
#define LIBGENS_PAC_EXTENSION_PAC_SKL_HKX              "skl.hkx"
#define LIBGENS_PAC_EXTENSION_PAC_SKL_HKX_FULL         "skl.hkx:ResSkeleton"
#define LIBGENS_PAC_EXTENSION_PAC_DDS                  "dds"
#define LIBGENS_PAC_EXTENSION_PAC_DDS_FULL             "dds:ResTexture"
#define LIBGENS_PAC_EXTENSION_PAC_SVCOL                "svcol.bin"
#define LIBGENS_PAC_EXTENSION_PAC_SVCOL_FULL           "svcol.bin:ResSvCol"
#define LIBGENS_PAC_EXTENSION_PAC_SHADOW_MODEL         "shadow-model"
#define LIBGENS_PAC_EXTENSION_PAC_SHADOW_MODEL_FULL    "shadow-model:ResShadowModel"
#define LIBGENS_PAC_EXTENSION_PAC_SWIF                 "swif"
#define LIBGENS_PAC_EXTENSION_PAC_SWIF_FULL            "swif:ResSurfRideProject"
#define LIBGENS_PAC_EXTENSION_PAC_PATH2                "path2.bin"
#define LIBGENS_PAC_EXTENSION_PAC_PATH2_FULL           "path2.bin:ResSplinePath2"
#define LIBGENS_PAC_EXTENSION_PAC_PIXELSHADER		   "pixelshader"
#define LIBGENS_PAC_EXTENSION_PAC_PIXELSHADER_FULL	   "pixelshader:ResMiragePixelShader"
#define LIBGENS_PAC_EXTENSION_PAC_PIXELSHADERCODE	   "fpo"
#define LIBGENS_PAC_EXTENSION_PAC_PIXELSHADERCODE_FULL "fpo:ResMiragePixelShaderCode"
#define LIBGENS_PAC_EXTENSION_PAC_PIXELSHADERPARAMETER "psparam"
#define LIBGENS_PAC_EXTENSION_PAC_PIXELSHADERPARAMETER_FULL	   "psparam:ResMiragePixelShaderParameter"
#define LIBGENS_PAC_EXTENSION_PAC_SHADERLIST	       "shader-list"
#define LIBGENS_PAC_EXTENSION_PAC_SHADERLIST_FULL	   "shader-list:ResMirageShaderList"
#define LIBGENS_PAC_EXTENSION_PAC_VERTEXSHADER	       "vertexshader"
#define LIBGENS_PAC_EXTENSION_PAC_VERTEXSHADER_FULL	   "vertexshader:ResMirageVertexShader"
#define LIBGENS_PAC_EXTENSION_PAC_VERTEXSHADERCODE	   "vpo"
#define LIBGENS_PAC_EXTENSION_PAC_VERTEXSHADERCODE_FULL	  "vpo:ResMirageVertexShaderCode"
#define LIBGENS_PAC_EXTENSION_PAC_VERTEXSHADERPARAMETER	  "vsparam"
#define LIBGENS_PAC_EXTENSION_PAC_VERTEXSHADERPARAMETER_FULL "vsparam:ResMirageVertexShaderParameter"
#define LIBGENS_PAC_EXTENSION_PAC_SM4SHADERCONTAINER	     "gsh"
#define LIBGENS_PAC_EXTENSION_PAC_SM4SHADERCONTAINER_FULL	 "gsh:ResMirageResSM4ShaderContainer"
#define LIBGENS_PAC_EXTENSION_PAC_XTB2DATA	                 "xtb2"
#define LIBGENS_PAC_EXTENSION_PAC_XTB2DATA_FULL		         "xtb2:ResXTB2Data"

#define LIBGENS_PAC_SPLIT_BYTES_LIMIT                  0xA00000

namespace LibGens {
	class GensStringTable;

	class PacMetaString {
		public:
			string name;
			size_t address;
			size_t relative_address;
	};

	class PacFile {
		protected:
			string name;
			unsigned char *data;
			unsigned int data_size;
			size_t file_data_address;

			// Strings to be merged later into the PAC
			// Temporal workaround to cracking the affected formats
			vector<PacMetaString> meta_pac_extra_strings;
		public:
			PacFile();
			PacFile(string filename);
			// Generate a file based on pac names
			PacFile(vector<string> pac_names);
			// Generate a proxy
			PacFile(PacFile *pac_file);
			~PacFile();

			void read(File *file);
			void save(string filename);
			string getName();
			void setName(string v);
			bool scanForAddress(size_t address, File *file);
			void write(File *file, GensStringTable *string_table);
			void writeFixed(File *file);
			vector<string> getPacDependNames();
			unsigned int getDataSize();
			void hashInput(XXH3_state_t &hash_state);
	};

	class PacExtension {
		protected:
			string name;
			vector<PacFile *> files;
			size_t file_address;
		public:
			PacExtension();
			~PacExtension();
			void read(File *file);
			void extract(string folder, bool convert_textures=false, void (*callback)(string)=NULL);
			bool scanForAddress(size_t address, File *file);
			void setName(string v);
			string getName();
			void addFile(PacFile *file);
			size_t getInternalSize();
			void write(File *file, GensStringTable *string_table);
			void writeData(File *file, GensStringTable *string_table);
			void writeFixed(File *file);
			vector<PacFile *> getFiles();
			void proxyFiles();
			void deleteFiles();
			bool isEmpty();
			bool isSpecialExtension();
			void hashInput(XXH3_state_t& hash_state);
			list<string> getFileList();
	};

	class PacProxyEntry {
		protected:
			string name;
			string extension;
			unsigned int index;
		public:
			PacProxyEntry();
			void read(File *file);
			void write(File *file, GensStringTable *string_table);
			void setName(string v);
			void setExtension(string v);
			void setIndex(unsigned int v);
	};

	class PacPack {
		friend class PacPack;

		protected:
			vector<PacExtension *> extensions;
			vector<PacProxyEntry *> proxy_entries;
			string name;
		public:
			PacPack(string filename);
			PacPack();
			~PacPack();
			void addFile(string filename);
			void addFolder(string folder);
			void save(string filename);
			PacExtension *getExtension(string ext);
			PacExtension *getExtensionFull(string ext);
			void deleteExtension(string ext);
			string extensionToFull(string ext);
			vector<PacExtension *> getExtensions();
			void addProxy(PacProxyEntry *entry);
			void createExtensions();
			void cleanUnusedExtensions();
			void extract(string folder, bool convert_textures=false, void (*callback)(string)=NULL);
			void readFile(string filename);
			void scanForAddressesInsideFiles(File *file);
			void setName(string v);
			string getName();
			size_t getInternalSize();
			list<string> getFileList();
			void hashInput(XXH3_state_t& hash_state);
	};

	class PacSet {
		protected:
			vector<PacPack *> packs;
			string name;
			string folder;
		public:
			PacSet();
			PacSet(string filename);
			~PacSet();
			void addFolder(string target_folder);
			void splitPacks();
			void save(string filename);
			void openDependFile(PacFile *file);
			void extract(string target_folder, bool convert_textures=false, void (*callback)(string)=NULL);
			list<string> getFileList();
			XXH128_hash_t computeHash();
	};
};
