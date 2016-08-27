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

#include <map>

#define LIBGENS_GI_TEXTURE_GROUP_ERROR_MESSAGE_NULL_FILE       "Trying to read GI group texture info data from unreferenced file."
#define LIBGENS_GI_TEXTURE_GROUP_ERROR_MESSAGE_WRITE_NULL_FILE "Trying to write GI group texture info data to an unreferenced file."
#define LIBGENS_GI_TEXTURE_GROUP_ATLASINFO_FILE                "atlasinfo"
#define LIBGENS_GI_TEXTURE_GROUP_INFO_FILE                     "gi-texture.gi-texture-group-info"
#define LIBGENS_GI_TEXTURE_GROUP_FOLDER_BEFORE                 "gia-"
#define LIBGENS_GI_TEXTURE_GROUP_FOLDER_AFTER                  ".ar"
#define LIBGENS_GI_TEXTURE_GROUP_SUBTEXTURE_LEVEL              "-level"

#define LIBGENS_GI_TEXTURE_GROUP_ROOT_TYPE                     2

#define LIBGENS_GI_TEXTURE_GROUP_QUALITY_LEVELS                3
#define LIBGENS_GI_TEXTURE_GROUP_LOWEST_QUALITY                2
#define LIBGENS_GI_TEXTURE_GROUP_HIGHEST_QUALITY               0

namespace LibGens {
	class GITexture;

	class GISubtexture {
		protected:
			float x, y, w, h;
			unsigned int pixel_w, pixel_h;
			string name;
			GITexture *parent;
			string full_path;
		public:
			GISubtexture();
			void read(File *file);
			void write(File *file);
			string getName();
			string &getPath();
			void setPath(string v);
			void setName(string v);
			void setParent(GITexture *v, string parent_folder, string parent_name);
			GITexture *getParent();
			float &getX();
			float &getY();
			float &getWidth();
			float &getHeight();
			void setX(float v);
			void setY(float v);
			void setWidth(float v);
			void setHeight(float v);
			void setPixelWidth(unsigned int v);
			void setPixelHeight(unsigned int v);
			unsigned int getPixelWidth();
			unsigned int getPixelHeight();
			unsigned int getBiggestPixelSize();
	};

	class GITextureTree {
	public:
		unsigned int x, y, w, h;
		GITextureTree *left;
		GITextureTree *right;
		GISubtexture *subtexture;

		GITextureTree();
		~GITextureTree();
		GITextureTree *insertSubtexture(GISubtexture *subtexture);
		void setSubtextures(unsigned int texture_width, unsigned int texture_height);
	};

	class GITexture {
		protected:
			list<GISubtexture *> subtextures;
			string texture_name;
			string folder;
			unsigned int width, height;
		public:
			GITexture();
			GITexture(string folder_p);
			~GITexture();
			void read(File *file);
			void write(File *file);
			GISubtexture *getTextureByInstance(string instance);
			string getFolder();
			string getName();
			void setName(string v);
			void addSubtexture(GISubtexture *v);
			list<GISubtexture *> getSubtextures();
			string getFilename();
			void setWidth(unsigned int v);
			void setHeight(unsigned int v);
			int getWidth();
			int getHeight();

			/** NOTE: Returns all textures that could not be fit properly. These textures are no longer owned by the texture, so you have to manage their deletion yourself. */
			list<GISubtexture *> organizeSubtextures(unsigned int max_texture_size);
	};

	class GITextureGroup {
		protected:
			unsigned int quality_level;
			list<GITexture *> textures;
			list<GISubtexture *> subtextures_to_organize;
			vector<unsigned int> instance_indices;
			string filename;
			string atlasinfo_filename;
			unsigned int folder_size;

			Vector3 center;
			float radius;
		public:
			GITextureGroup();
			~GITextureGroup();
			void read(File *file, string terrain_folder, string group_folder, vector<string> &global_instance_names);
			void write(File *file);
			void readAtlasinfo(File *file, string terrain_folder = "", vector<string> instance_names = vector<string>());
			void saveAtlasinfo(string atlasinfo_filename);
			void writeAtlasinfo(File *file);
			GISubtexture *getTextureByInstance(size_t instance_index, string instance, size_t quality_level_p, vector<GITextureGroup *> &groups);
			list<GITexture *> getTextures();
			void setQualityLevel(unsigned int v);
			unsigned int getQualityLevel();
			string getFilename();
			string getAtlasinfoFilename();
			bool hasInstanceIndex(size_t instance_index, vector<GITextureGroup *> &groups);
			void addInstanceIndex(unsigned int instance_index);
			void addSubtextureToOrganize(GISubtexture *subtexture);
			void addSubtextureToOrganize(GITextureGroup *clone_group, float downscale_factor, float minimum_texture_size);
			list<GISubtexture *> getSubtexturesToOrganize();
			size_t getInstanceIndexCount();
			void setCenter(Vector3 v);
			Vector3 getCenter();
			void setRadius(float v);
			float getRadius();
			void setFolderSize(unsigned int v);
			int getFolderSize();
			void organizeSubtextures(unsigned int max_texture_size);
			void fixIndices(std::map<int, int> index_map);
			void addTexture(GITexture *texture);
			void deleteTextures();
	};

	class GITextureGroupInfo {
		protected:
			vector<GITextureGroup *> groups;
			vector<string> instance_names;
			vector<Vector3> instance_centers;
			vector<float> instance_radius;
		public:
			GITextureGroupInfo();
			GITextureGroupInfo(string filename, string terrain_folder = "");
			void save(string filename);
			void read(File *file, string terrain_folder);
			void write(File *file);
			GITextureGroup *createGroup();
			GITextureGroup *getGroupByIndex(size_t index);
			int getGroupIndex(GITextureGroup *group);
			void addInstance(string name, Vector3 center, float radius);
			vector<string> getInstanceNames();
			GISubtexture *getTextureByInstance(string instance, size_t quality_level);
			vector<GITextureGroup *> getGroups();
			void clean();
			int getInstanceIndex(string instance_name);
			void sortGroupsByQualityLevel();
	};
};
