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

#include "Common.h"

#ifndef SONICGLVL_CONFIGURATION_H_INCLUDED
#define SONICGLVL_CONFIGURATION_H_INCLUDED

#define SONICGLVL_CONFIGURATION_ERROR_FILE                  "No valid configuration file found: "
#define SONICGLVL_CONFIGURATION_ERROR_FILE_ROOT             "Configuration file doesn't have a valid root."
#define SONICGLVL_CONFIGURATION_FILENAME_ATTRIBUTE          "filename"
#define SONICGLVL_CONFIGURATION_VALUE_ATTRIBUTE             "value"
#define SONICGLVL_CONFIGURATION_OBJECT_PRODUCTION_PATH      "ObjectProductionPath"
#define SONICGLVL_CONFIGURATION_TERRAIN_VIEW_DISTANCE       "TerrainViewDistance"
#define SONICGLVL_CONFIGURATION_TERRAIN_CAMERA_OFFSET       "TerrainCameraOffset"
#define SONICGLVL_CONFIGURATION_GI_LEVEL_1_QUALITY_OFFSET   "GILevel1QualityOffset"
#define SONICGLVL_CONFIGURATION_GI_LEVEL_2_QUALITY_OFFSET   "GILevel2QualityOffset"
#define SONICGLVL_CONFIGURATION_TERRAIN_UPDATE_INTERVAL_S   "TerrainUpdateIntervalSeconds"
#define SONICGLVL_CONFIGURATION_VISIBILITY_FLAGS			"VisibilityFlags"


class EditorConfiguration {
	protected:
		string object_production_path;
		float terrain_view_distance;
		float terrain_camera_offset;
		float gi_level_1_quality_offset;
		float gi_level_2_quality_offset;
		float terrain_update_interval;
		unsigned int visibility_flags;
	public:
		EditorConfiguration();

		void load(string filename);
		void save(string filename);

		string getObjectProductionPath() {
			return object_production_path;
		}

		float getTerrainCameraOffset() {
			return terrain_camera_offset;
		}

		float getTerrainViewDistance() {
			return terrain_view_distance;
		}

		float getGILevel1QualityOffset() {
			return gi_level_1_quality_offset;
		}

		float getGILevel2QualityOffset() {
			return gi_level_2_quality_offset;
		}

		float getTerrainUpdateInterval() {
			return terrain_update_interval;
		}

		unsigned int checkVisibilityFlag(unsigned int flag) {
			return visibility_flags & flag;
		}

		void toggleVisibilityFlag(unsigned int flag) {
			visibility_flags = visibility_flags ^ flag;
		}
};

#endif