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

#define LIBGENS_LEVEL_START_ID_GENERATION            100
#define LIBGENS_LEVEL_END_ID_GENERATION              4294967296

#define LIBGENS_LEVEL_CAMERA_VIEW_FORWARD            "Forward"
#define LIBGENS_LEVEL_CAMERA_VIEW_SIDE               "Side"
#define LIBGENS_LEVEL_DATA_STAGE                     "Stage.stg.xml"
#define LIBGENS_LEVEL_DATA_TERRAIN                   "Terrain.stg.xml"
#define LIBGENS_LEVEL_DATA_SCENE_EFFECT              "SceneEffect.prm.xml"

#define LIBGENS_LEVEL_STAGE_GUIDE_PATH               "StageGuidePath"
#define LIBGENS_LEVEL_XML_PATH                       "Path"
#define LIBGENS_LEVEL_XML_CONTAINER                  "Container"
#define LIBGENS_LEVEL_XML_TERRAIN                    "Terrain"
#define LIBGENS_LEVEL_XML_TERRAIN_INFO               "TerrainInfoFile"
#define LIBGENS_LEVEL_XML_RIGID_BODY                 "RigidBodyContainer"
#define LIBGENS_LEVEL_XML_COLLISION_RENDER           "IsCollisionRender"
#define LIBGENS_LEVEL_XML_LIGHT                      "Light"
#define LIBGENS_LEVEL_XML_DATA_NAME                  "DataName"
#define LIBGENS_LEVEL_XML_SKY                        "Sky"
#define LIBGENS_LEVEL_XML_MODEL                      "Model"

#define LIBGENS_LEVEL_XML_DEFAULT                    "Default"
#define LIBGENS_LEVEL_XML_BASIC                      "Basic"
#define LIBGENS_LEVEL_XML_SKY_FOLLOW_UP_RATIO_Y      "CFxSceneRenderer::m_skyFollowUpRatioY"
#define LIBGENS_LEVEL_XML_SKY_INTENSITY_SCALE        "CFxSceneRenderer::m_skyIntensityScale"

#define LIBGENS_LEVEL_XML_LIGHT_SCATTERING           "LightScattering"
#define LIBGENS_LEVEL_XML_CATEGORY                   "Category"
#define LIBGENS_LEVEL_XML_COMMON                     "Common"
#define LIBGENS_LEVEL_XML_FOG                        "Fog"
#define LIBGENS_LEVEL_XML_PARAM                      "Param"

#define LIBGENS_LEVEL_XML_COLOR_X                    "ms_Color.x"
#define LIBGENS_LEVEL_XML_COLOR_Y                    "ms_Color.y"
#define LIBGENS_LEVEL_XML_COLOR_Z                    "ms_Color.z"
#define LIBGENS_LEVEL_XML_RAY_MIE_RAY2_MIE2_X    	 "ms_Ray_Mie_Ray2_Mie2.x"
#define LIBGENS_LEVEL_XML_RAY_MIE_RAY2_MIE2_Y    	 "ms_Ray_Mie_Ray2_Mie2.y"
#define LIBGENS_LEVEL_XML_FAR_NEAR_SCALE_X       	 "ms_FarNearScale.x"
#define LIBGENS_LEVEL_XML_FAR_NEAR_SCALE_Y       	 "ms_FarNearScale.y"
#define LIBGENS_LEVEL_XML_FAR_NEAR_SCALE_Z       	 "ms_FarNearScale.z"
#define LIBGENS_LEVEL_XML_FAR_NEAR_SCALE_W       	 "ms_FarNearScale.w"


#define LIBGENS_LEVEL_XML_SONIC                      "Sonic"
#define LIBGENS_LEVEL_XML_EVIL                       "Evil"
#define LIBGENS_LEVEL_XML_YAW                        "Yaw"
#define LIBGENS_LEVEL_XML_START                      "Start"
#define LIBGENS_LEVEL_XML_MODE                       "Mode"
#define LIBGENS_LEVEL_XML_SPEED                      "Speed"
#define LIBGENS_LEVEL_XML_TIME                       "Time"
#define LIBGENS_LEVEL_XML_DEAD_HEIGHT                "DeadHeight"
#define LIBGENS_LEVEL_XML_CAMERA_VIEW                "CameraView"
#define LIBGENS_LEVEL_XML_IS_SIDE_VIEW               "IsSideView"

#define LIBGENS_LEVEL_XML_SET_DATA                   "SetData"
#define LIBGENS_LEVEL_XML_LAYER                      "Layer"
#define LIBGENS_LEVEL_XML_INDEX                      "Index"
#define LIBGENS_LEVEL_XML_NAME                       "Name"
#define LIBGENS_LEVEL_XML_FILENAME                   "FileName"
#define LIBGENS_LEVEL_XML_COLOR                      "Color"
#define LIBGENS_LEVEL_XML_IS_GAME_ACTIVE             "IsGameActive"

#define LIBGENS_LEVEL_HAVOK_EXTENSION                ".phy.hkx"

#define LIBGENS_LEVEL_GAME_GENERATIONS               0
#define LIBGENS_LEVEL_GAME_UNLEASHED                 1

#define LIBGENS_LEVEL_GAME_STRING_GENERATIONS        "Generations"
#define LIBGENS_LEVEL_GAME_STRING_UNLEASHED          "Unleashed"
#define LIBGENS_LEVEL_GAME_STRING_LOST_WORLD         "LostWorld"

namespace LibGens {
	class LevelCollisionEntry {
		public:
			string name;
			bool rendering;

			LevelCollisionEntry() {
				name = "";
				rendering = false;
			}
	};

	class LevelSetEntry {
		public:
			string name;
			string color;
			string filename;
			size_t index;
			bool active;

			LevelSetEntry() {
				name = "";
				color = "";
				filename = "";
				index = 0;
				active = false;
			}
	};

	class SceneEffect {
		public:
			Color light_scattering_ray_mie_ray2_mie2;
			Color light_scattering_far_near_scale;
			Color light_scattering_color;

			float sky_follow_up_ratio_y;
			float sky_intensity_scale;
	};

	class Object;
	class ObjectSet;
	class ObjectLibrary;
	class Path;

	class Level {
		protected:
			list<ObjectSet *> sets;
			list<Path *> paths;
			string slot;
			string folder;
			string terrain_info_file;
			vector<string> skybox_names;
			string direct_light_name;
			size_t game_mode;

			string spawn_type;
			float spawn_yaw;
			float spawn_dead_height;
			string spawn_camera_view;
			Vector3 spawn_position;
			string spawn_mode;
			float spawn_speed;
			float spawn_time;

			string bgm_container;
			string bgm_name;

			SceneEffect scene_effect;

			list<LevelCollisionEntry *> collision_entries;
			list<LevelSetEntry *> set_entries;
		public:
			Level();
			Level(string folder_p, string game_name=LIBGENS_LEVEL_GAME_STRING_GENERATIONS);
			void setName(string nm);
			size_t getGameMode();
			void setGameMode(size_t v);
			void setGameMode(string v);
			string getName();
			string getTerrainInfo();
			string getDirectLight();
			string getSkybox();
			string getFolder();
			void addSet(ObjectSet *set);
			void removeSet(ObjectSet *set);
			list<ObjectSet *> getSets();
			list<Path *> getPaths();
			Object *getObjectByID(size_t id);
			list<LevelCollisionEntry *> getCollisionEntries();
			LevelCollisionEntry *getCollisionEntry(string collision_filename);
			void getObjectsByName(string name, list<Object *> &total_list);
			void loadPath(TiXmlElement *root);
			void loadStage();
			void loadSets();
			void loadTerrain();
			void loadSceneEffect();
			void loadSpawn(TiXmlElement *root);
			void saveSpawn();
			SceneEffect &getSceneEffect();
			ObjectSet *getSet(string name);
			size_t newObjectID();
			void learnFromLibrary(ObjectLibrary *library);
	};
};
