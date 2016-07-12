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

#define LIBGENS_TERRAIN_GROUP_ERROR_MESSAGE_NULL_FILE       "Trying to read terrain group data from unreferenced file."
#define LIBGENS_TERRAIN_GROUP_ERROR_MESSAGE_WRITE_NULL_FILE "Trying to write terrain group data to an unreferenced file."
#define LIBGENS_TERRAIN_INSTANCE_EXTENSION                  ".terrain-instanceinfo"
#define LIBGENS_TERRAIN_MODEL_EXTENSION                     ".terrain-model"
#define LIBGENS_TERRAIN_GROUP_ROOT_GENERATIONS              1

namespace LibGens {
	class TerrainGroupInfo;
	class TerrainInstance;
	class Model;

	class TerrainGroup {
		friend class TerrainGroupInfo;

		protected:
			vector< vector<TerrainInstance *> > instances;
			vector<Vector3> instance_centers;
			vector<float> instance_radius;
			vector<Model *> models;
			Vector3 center;
			float radius;
			string filename; 
			string terrain_folder;
			string name;
			bool loaded;
			float current_distance;
			int subset_id;
		public:
			TerrainGroup() {
				loaded = false;
				current_distance = 0;
			}

			TerrainGroup(string group_filename, string filename_p, string terrain_folder_p);
			void save(string filename_p);
			void savePack(string filename_p);

			void load();

			void read(File *file, string terrain_folder);
			void write(File *file);

			vector<Model *> getModels() {
				return models;
			}

			list<TerrainInstance *> getInstances();
			vector< vector<TerrainInstance *> > getInstanceVectors();
			vector<float> getInstanceRadiuses();
			vector<Vector3> getInstanceCenters();
			bool checkDistance(Vector3 position_to_check, float extra_range);
			float getDistance(Vector3 position_to_check);
			void unload();

			void setCenter(Vector3 center_p) {
				center = center_p;
			}

			void setRadius(float radius_p) {
				radius = radius_p;
			}

			void addModel(Model *model) {
				if (model) models.push_back(model);
			}

			void addFakeModel(std::string name);

			void setSubsetID(int v);
			int getSubsetID();

			void addInstances(vector<TerrainInstance *> instances_p) {
				instances.push_back(instances_p);
			}

			bool isLoaded() {
				return loaded;
			}

			float getCurrentDistance() {
				return current_distance;
			}

			float getRadius() {
				return radius;
			}

			void setName(string v) {
				name = v;
			}

			string getName() {
				return name;
			}

			void buildSpheres();

			~TerrainGroup();
	};

	class TerrainGroupInfo {
		protected:
			string filename;
			unsigned int folder_size;
			unsigned int subset_id;
			Vector3 center;
			float radius;
			vector<Vector3> instance_centers;
			vector<float> instance_radius;
		public:
			TerrainGroupInfo() {
				
			}

			TerrainGroupInfo(TerrainGroup *group);

			void read(File *file);
			void write(File *file);

			void setFolderSize(unsigned int v);
			unsigned int getFolderSize();

			string getName() {
				return filename;
			}

			Vector3 getCenter() {
				return center;
			}

			float getRadius() {
				return radius;
			}
	};
};
