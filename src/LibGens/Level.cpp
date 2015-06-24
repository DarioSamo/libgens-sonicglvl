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

#define _USE_MATH_DEFINES
#include "Level.h"
#include "Object.h"
#include "ObjectSet.h"
#include "ObjectElement.h"
#include "LostWorldObjectSet.h"
#include "Path.h"
#include <math.h>

namespace LibGens {
	Level::Level() {
	}

	Level::Level(string folder_p, string game_name) {
		folder = folder_p;
		terrain_info_file = "terrain";
		direct_light_name = "";

		spawn_yaw         = 0.0f;
		spawn_dead_height = -3000.0f;
		spawn_camera_view = LIBGENS_LEVEL_CAMERA_VIEW_FORWARD;
		spawn_position    = Vector3(0.0f, 0.0f, 0.0f);
		spawn_mode        = "Stand";
		spawn_speed       = 0.0f;
		spawn_time        = 0.0f;

		game_mode = LIBGENS_LEVEL_GAME_GENERATIONS;

		if (game_name == LIBGENS_LEVEL_GAME_STRING_UNLEASHED) {
			game_mode = LIBGENS_LEVEL_GAME_UNLEASHED;
		}

		loadStage();
		loadSceneEffect();
		loadSets();
		loadTerrain();
	}

	Level* Level::LostWorldLevel(string folder_p, ObjectLibrary *library)
	{
		// Static method
		Level *level = new Level();
		level->folder = folder_p;
		level->game_mode = LIBGENS_LEVEL_GAME_LOST_WORLD;

		// Get slot name - folder is "<slot>/data/" so remove "data" and fetch <slot>
		level->slot = folder_p.substr(0, folder_p.size() - 6);
		level->slot = LibGens::File::nameFromFilename(level->slot);

		// Fill in SceneEffect with default values from ghz200
		level->scene_effect.light_scattering_color = Color(0.11f, 0.35f, 0.760001f);
		level->scene_effect.light_scattering_far_near_scale = Color(3200.f, 380.f, 1.2f, 114.f);
		level->scene_effect.light_scattering_ray_mie_ray2_mie2.r = 0.291f;
		level->scene_effect.light_scattering_ray_mie_ray2_mie2.g = 0.96f;
		level->scene_effect.sky_follow_up_ratio_y = 1.0f;
		level->scene_effect.sky_intensity_scale = 1.3f;

		// Load sets from cache folder
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind;
		hFind = FindFirstFile((folder_p + "*.orc").c_str(), &FindFileData);
		if (hFind == INVALID_HANDLE_VALUE) {} 
		else {
			do {
				const char *name=FindFileData.cFileName;
				if (name[0]=='.') continue;

				string new_filename = folder_p + ToString(name);
				LostWorldObjectSet *object_set = new LibGens::LostWorldObjectSet(new_filename, library);
				level->addSet(object_set);
			} while (FindNextFile(hFind, &FindFileData) != 0);
			FindClose(hFind);
		}

		// Fix local transform on sets
		for (list<ObjectSet*>::iterator it = level->sets.begin(); it != level->sets.end(); it++) {
			LostWorldObjectSet *lw_set = static_cast<LostWorldObjectSet*>(*it);
			lw_set->fixTransform(level);
		}

		// Set collision entries (doesn't actually load, presumably because of big endian hkx)
		string collision_filename = folder_p + level->slot + "_col.phy.hkx";
		LevelCollisionEntry *entry = new LevelCollisionEntry();
		entry->name = collision_filename;
		entry->rendering = true;
		level->collision_entries.push_back(entry);
		
		// Load paths
		Path *path = new Path();
		string path_filename = folder_p + level->slot + "_path.path2.bin";
		path->readPath2(path_filename);
		level->paths.push_back(path);

		// Load config
		string lua_config = folder_p + level->slot + "_config.lua";

		File config(lua_config, "r");
		if (config.valid())
			level->loadLuaConfig(&config);

		return level;
	}
	
	void Level::loadStage() {
		TiXmlDocument doc(folder + LIBGENS_LEVEL_DATA_STAGE);
		if (!doc.LoadFile()) {
			return;
		}

		TiXmlHandle hDoc(&doc);
		TiXmlElement* pElem;
		TiXmlHandle hRoot(0);

		pElem=hDoc.FirstChildElement().Element();
		if (!pElem) {
			return;
		}

		pElem=pElem->FirstChildElement();
		for(pElem; pElem; pElem=pElem->NextSiblingElement()) {
			string element_name=pElem->ValueStr();

			if (element_name == LIBGENS_LEVEL_XML_SONIC) {
				loadSpawn(pElem);
			}

			if (element_name == LIBGENS_LEVEL_XML_PATH) {
				loadPath(pElem);
			}
			
			if (element_name == LIBGENS_LEVEL_XML_SET_DATA) {
				TiXmlElement* pElem_i=pElem->FirstChildElement();
				for(pElem_i; pElem_i; pElem_i=pElem_i->NextSiblingElement()) {
					if (pElem_i->ValueStr()==LIBGENS_LEVEL_XML_LAYER) {
						LevelSetEntry *level_set_entry = new LevelSetEntry();
						
						TiXmlElement* pElem_l=pElem_i->FirstChildElement();
						for(pElem_l; pElem_l; pElem_l=pElem_l->NextSiblingElement()) {
							char *text_ptr=(char *) pElem_l->GetText();
							size_t index=0;

							if (text_ptr) {
								string layer_attribute=ToString(text_ptr);

								if (pElem_l->ValueStr()==LIBGENS_LEVEL_XML_INDEX) {
									FromString<size_t>(index, layer_attribute, std::dec);
									level_set_entry->index = index;
								}

								if (pElem_l->ValueStr()==LIBGENS_LEVEL_XML_NAME) {
									level_set_entry->name = layer_attribute;
								}

								if (pElem_l->ValueStr()==LIBGENS_LEVEL_XML_FILENAME) {
									level_set_entry->filename = layer_attribute;
								}

								if (pElem_l->ValueStr()==LIBGENS_LEVEL_XML_COLOR) {
									level_set_entry->color = layer_attribute;
								}

								if (pElem_l->ValueStr()==LIBGENS_LEVEL_XML_IS_GAME_ACTIVE) {
									level_set_entry->active = (layer_attribute == LIBGENS_OBJECT_ELEMENT_BOOL_TRUE);
								}
							}
						}

						set_entries.push_back(level_set_entry);
					}
				}
			}
		}
	}

	void Level::loadSpawn(TiXmlElement *root) {
		for (TiXmlElement *pElem=root->FirstChildElement(); pElem; pElem=pElem->NextSiblingElement()) {
			string element_name=pElem->ValueStr();
			char *text_ptr=(char *) pElem->GetText();
			float value_f = 0.0f;
			
			if (element_name == LIBGENS_OBJECT_ELEMENT_POSITION) {
				spawn_position.readXML(pElem);
			}

			if (element_name == LIBGENS_LEVEL_XML_YAW) {
				FromString<float>(value_f, ToString(text_ptr), std::dec);
				spawn_yaw = value_f;
			}

			if (element_name == LIBGENS_LEVEL_XML_START) {
				TiXmlElement* pElem_i=pElem->FirstChildElement();
				for(pElem_i; pElem_i; pElem_i=pElem_i->NextSiblingElement()) {
					text_ptr=(char *) pElem_i->GetText();

					if (text_ptr) {
						if (pElem_i->ValueStr() == LIBGENS_LEVEL_XML_MODE) {
							spawn_mode = ToString(text_ptr);
						}

						if (pElem_i->ValueStr() == LIBGENS_LEVEL_XML_SPEED) {
							FromString<float>(value_f, ToString(text_ptr), std::dec);
							spawn_speed = value_f;
						}

						if (pElem_i->ValueStr() == LIBGENS_LEVEL_XML_TIME) {
							FromString<float>(value_f, ToString(text_ptr), std::dec);
							spawn_time = value_f;
						}
					}
				}
			}

			if (element_name == LIBGENS_LEVEL_XML_DEAD_HEIGHT) {
				FromString<float>(value_f, ToString(text_ptr), std::dec);
				spawn_dead_height = value_f;
			}

			if (element_name == LIBGENS_LEVEL_XML_CAMERA_VIEW) {
				spawn_camera_view = ToString(text_ptr);
			}
		}
	}

	void Level::loadLuaConfig(File *file) {
		string line;
		string currentParamGroup;
		string currentParamType;

		while (!file->endOfFile()) {
			file->readLine(&line);

			unsigned int numWhitespace = 0;
			for (; numWhitespace < line.size() && isspace(line[numWhitespace]); numWhitespace++) {}
			line = line.substr(numWhitespace, line.size() - numWhitespace);

			size_t pre_eq = line.find_first_of("=");
			size_t post_eq = line.find_last_of("=");

			if (pre_eq == string::npos) continue;
			pre_eq -= 1;
			post_eq += 2;

			string name = line.substr(0, pre_eq);
			string parm = line.substr(post_eq, line.size() - post_eq);

			if (name == LIBGENS_LEVEL_LUA_NAME)
			{
				size_t nameStart = parm.find_first_of("\"");
				size_t nameEnd = parm.find_last_of("\"");
				currentParamGroup = parm.substr(nameStart+1, nameEnd-nameStart-1);
			}

			if (name == LIBGENS_LEVEL_LUA_PARAMS)
			{
				currentParamType = LIBGENS_LEVEL_LUA_PARAMS;
			}

			if (name == LIBGENS_LEVEL_LUA_PARAMS_FAR)
			{
				currentParamType = LIBGENS_LEVEL_LUA_PARAMS_FAR;
			}

			if (currentParamGroup == LIBGENS_LEVEL_LUA_SCENE)
			{
				if (name == LIBGENS_LEVEL_LUA_SKY_INTENSITY_SCALE)
					sscanf(parm.c_str(), "%f,", &(scene_effect.sky_intensity_scale));

				if (name == LIBGENS_LEVEL_LUA_SKY_FOLLOWUP_RATIO_Y)
					sscanf(parm.c_str(), "%f,", &(scene_effect.sky_follow_up_ratio_y));
			}

			if ((currentParamGroup == LIBGENS_LEVEL_LUA_LIGHT_SCATTERING) && (currentParamType == LIBGENS_LEVEL_LUA_PARAMS)) {
				if (name == LIBGENS_LEVEL_LUA_COLOR) {
					sscanf(parm.c_str(), "{ %f, %f, %f, %f },",
						&(scene_effect.light_scattering_color.r),
						&(scene_effect.light_scattering_color.g),
						&(scene_effect.light_scattering_color.b),
						&(scene_effect.light_scattering_color.a)
						);
				}

				if (name == LIBGENS_LEVEL_LUA_RAYLEIGH)
					sscanf(parm.c_str(), "%f,", &(scene_effect.light_scattering_ray_mie_ray2_mie2.r));

				if (name == LIBGENS_LEVEL_LUA_MIE)
					sscanf(parm.c_str(), "%f,", &(scene_effect.light_scattering_ray_mie_ray2_mie2.g));

				if (name == LIBGENS_LEVEL_LUA_FAR)
					sscanf(parm.c_str(), "%f,", &(scene_effect.light_scattering_far_near_scale.r));

				if (name == LIBGENS_LEVEL_LUA_NEAR)
					sscanf(parm.c_str(), "%f,", &(scene_effect.light_scattering_far_near_scale.g));

			}
		}
	}

	void Level::loadPath(TiXmlElement *root) {
		for(TiXmlElement *pElem=root->FirstChildElement(); pElem; pElem=pElem->NextSiblingElement()) {
			string element_name=pElem->ValueStr();
			char *text_ptr=(char *) pElem->GetText();
			
			if (element_name == LIBGENS_LEVEL_XML_CONTAINER) {
				if (text_ptr) {
					Path *path = new Path(folder + ToString(text_ptr) + LIBGENS_PATH_FULL_GENERATIONS_EXTENSION);
					paths.push_back(path);
				}
			}
		}
	}

	void Level::loadSets() {
		// Generations loads all setdata_* files into the game
		if (game_mode == LIBGENS_LEVEL_GAME_GENERATIONS) {
			WIN32_FIND_DATA FindFileData;
			HANDLE hFind;
			hFind = FindFirstFile((folder+"setdata_*.set.xml").c_str(), &FindFileData);
			if (hFind == INVALID_HANDLE_VALUE) {} 
			else {
				do {
					const char *name=FindFileData.cFileName;
					if (name[0]=='.') continue;

					string new_filename=folder+ToString(name);
					ObjectSet *object_set=new LibGens::ObjectSet(new_filename);
					addSet(object_set);
				} while (FindNextFile(hFind, &FindFileData) != 0);
				FindClose(hFind);
			}
		}
		// Unleashed uses the set files that were loaded from Stage.stg.xml
		else if (game_mode == LIBGENS_LEVEL_GAME_UNLEASHED) {
			for (list<LevelSetEntry *>::iterator it=set_entries.begin(); it!=set_entries.end(); it++) {
				if (!(*it)->active) continue;

				string new_filename=folder+(*it)->filename;

				ObjectSet *object_set=new LibGens::ObjectSet(new_filename);
				object_set->setName((*it)->name);
				addSet(object_set);
			}
		}
	}


	void Level::loadSceneEffect() {
		string scene_effect_filename = folder + LIBGENS_LEVEL_DATA_SCENE_EFFECT;

		TiXmlDocument doc(scene_effect_filename);
		if (!doc.LoadFile()) {
			return;
		}

		TiXmlHandle hDoc(&doc);
		TiXmlElement* pElem;
		TiXmlHandle hRoot(0);

		pElem=hDoc.FirstChildElement().Element();
		if (!pElem) {
			return;
		}

		pElem=pElem->FirstChildElement();
		for(pElem; pElem; pElem=pElem->NextSiblingElement()) {
			// Light Scattering

			if (pElem->ValueStr()==LIBGENS_LEVEL_XML_LIGHT_SCATTERING) {
				TiXmlElement* pElem_i=pElem->FirstChildElement();
				for(pElem_i; pElem_i; pElem_i=pElem_i->NextSiblingElement()) {
					if (pElem_i->ValueStr()==LIBGENS_LEVEL_XML_CATEGORY) {
						TiXmlElement* pElem_j=pElem_i->FirstChildElement();
						for(pElem_j; pElem_j; pElem_j=pElem_j->NextSiblingElement()) {
							// Common Fog LightScattering
							TiXmlElement* pElem_k=pElem_j->FirstChildElement();
							for(pElem_k; pElem_k; pElem_k=pElem_k->NextSiblingElement()) {
								// Param
								TiXmlElement* pElem_l=pElem_k->FirstChildElement();
								for(pElem_l; pElem_l; pElem_l=pElem_l->NextSiblingElement()) {
									char *text_ptr=(char *) pElem_l->GetText();
									if (text_ptr) {
										string value_text = ToString(text_ptr);

										if (pElem_l->ValueStr()==LIBGENS_LEVEL_XML_COLOR_X) {
											FromString<float>(scene_effect.light_scattering_color.r, value_text, std::dec);
										}

										if (pElem_l->ValueStr()==LIBGENS_LEVEL_XML_COLOR_Y) {
											FromString<float>(scene_effect.light_scattering_color.g, value_text, std::dec);
										}

										if (pElem_l->ValueStr()==LIBGENS_LEVEL_XML_COLOR_Z) {
											FromString<float>(scene_effect.light_scattering_color.b, value_text, std::dec);
										}

										if (pElem_l->ValueStr()==LIBGENS_LEVEL_XML_RAY_MIE_RAY2_MIE2_X) {
											FromString<float>(scene_effect.light_scattering_ray_mie_ray2_mie2.r, value_text, std::dec);
										}

										if (pElem_l->ValueStr()==LIBGENS_LEVEL_XML_RAY_MIE_RAY2_MIE2_Y) {
											FromString<float>(scene_effect.light_scattering_ray_mie_ray2_mie2.g, value_text, std::dec);
										}

										if (pElem_l->ValueStr()==LIBGENS_LEVEL_XML_FAR_NEAR_SCALE_X) {
											FromString<float>(scene_effect.light_scattering_far_near_scale.r, value_text, std::dec);
										}

										if (pElem_l->ValueStr()==LIBGENS_LEVEL_XML_FAR_NEAR_SCALE_Y) {
											FromString<float>(scene_effect.light_scattering_far_near_scale.g, value_text, std::dec);
										}

										if (pElem_l->ValueStr()==LIBGENS_LEVEL_XML_FAR_NEAR_SCALE_Z) {
											FromString<float>(scene_effect.light_scattering_far_near_scale.b, value_text, std::dec);
										}

										if (pElem_l->ValueStr()==LIBGENS_LEVEL_XML_FAR_NEAR_SCALE_W) {
											FromString<float>(scene_effect.light_scattering_far_near_scale.a, value_text, std::dec);
										}
									}
								}
							}
						}	
					}
				}
			}


			// Sky Parameters

			if (pElem->ValueStr()==LIBGENS_LEVEL_XML_DEFAULT) {
				TiXmlElement* pElem_i=pElem->FirstChildElement();
				for(pElem_i; pElem_i; pElem_i=pElem_i->NextSiblingElement()) {
					if (pElem_i->ValueStr()==LIBGENS_LEVEL_XML_CATEGORY) {
						TiXmlElement* pElem_j=pElem_i->FirstChildElement();
						for(pElem_j; pElem_j; pElem_j=pElem_j->NextSiblingElement()) {
							// Basic Blb
							TiXmlElement* pElem_k=pElem_j->FirstChildElement();
							for(pElem_k; pElem_k; pElem_k=pElem_k->NextSiblingElement()) {
								// Param
								TiXmlElement* pElem_l=pElem_k->FirstChildElement();
								for(pElem_l; pElem_l; pElem_l=pElem_l->NextSiblingElement()) {
									char *text_ptr=(char *) pElem_l->GetText();
									if (text_ptr) {
										string value_text = ToString(text_ptr);

										if (pElem_l->ValueStr()==LIBGENS_LEVEL_XML_SKY_FOLLOW_UP_RATIO_Y) {
											FromString<float>(scene_effect.sky_follow_up_ratio_y, value_text, std::dec);
										}

										if (pElem_l->ValueStr()==LIBGENS_LEVEL_XML_SKY_INTENSITY_SCALE) {
											FromString<float>(scene_effect.sky_intensity_scale, value_text, std::dec);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}


	void Level::loadTerrain() {
		string terrain_stg_filename = folder + LIBGENS_LEVEL_DATA_TERRAIN;

		// Re-open Stage.stg.xml if it's an Unleashed level
		if (game_mode == LIBGENS_LEVEL_GAME_UNLEASHED) {
			terrain_stg_filename = folder + LIBGENS_LEVEL_DATA_STAGE;
		}

		TiXmlDocument doc(terrain_stg_filename);
		if (!doc.LoadFile()) {
			return;
		}

		TiXmlHandle hDoc(&doc);
		TiXmlElement* pElem;
		TiXmlHandle hRoot(0);

		pElem=hDoc.FirstChildElement().Element();
		if (!pElem) {
			return;
		}

		pElem=pElem->FirstChildElement();
		for(pElem; pElem; pElem=pElem->NextSiblingElement()) {
			if (pElem->ValueStr()==LIBGENS_LEVEL_XML_TERRAIN) {
				TiXmlElement* pElem_i=pElem->FirstChildElement();
				LevelCollisionEntry *entry=NULL;

				for(pElem_i; pElem_i; pElem_i=pElem_i->NextSiblingElement()) {
					/*
					if (pElem_i->ValueStr()==LIBGENS_LEVEL_XML_TERRAIN_INFO) {
						char *text_ptr=(char *) pElem_i->GetText();
						if (text_ptr) {
							terrain_info_file = ToString(text_ptr);

							// WTF SONIC TEAM WHY IS THIS AN OPTION
							if (terrain_info_file == "*") {
								terrain_info_file = "terrain";
							}
						}
					}
					*/

					if (pElem_i->ValueStr()==LIBGENS_LEVEL_XML_RIGID_BODY) {
						char *text_ptr=(char *) pElem_i->GetText();
						if (text_ptr) {
							entry = new LevelCollisionEntry();
							entry->name = ToString(text_ptr);
						}
					}

					if (pElem_i->ValueStr()==LIBGENS_LEVEL_XML_COLLISION_RENDER) {
						char *text_ptr=(char *) pElem_i->GetText();
						if (text_ptr && entry) {
							string text=ToString(text_ptr);
							entry->rendering = (text==LIBGENS_OBJECT_ELEMENT_BOOL_TRUE);
						}
					}
				}

				if (entry) collision_entries.push_back(entry);
			}

			if (pElem->ValueStr()==LIBGENS_LEVEL_XML_LIGHT) {
				TiXmlElement* pElem_i=pElem->FirstChildElement();
				for(pElem_i; pElem_i; pElem_i=pElem_i->NextSiblingElement()) {
					if (pElem_i->ValueStr()==LIBGENS_LEVEL_XML_DATA_NAME) {
						char *text_ptr=(char *) pElem_i->GetText();
						if (text_ptr) {
							direct_light_name = ToString(text_ptr);
						}
					}
				}
			}

			if (pElem->ValueStr()==LIBGENS_LEVEL_XML_SKY) {
				TiXmlElement* pElem_i=pElem->FirstChildElement();
				for(pElem_i; pElem_i; pElem_i=pElem_i->NextSiblingElement()) {
					if (pElem_i->ValueStr()==LIBGENS_LEVEL_XML_MODEL) {
						char *text_ptr=(char *) pElem_i->GetText();
						if (text_ptr) {
							skybox_names.push_back(ToString(text_ptr));
						}
					}
				}
			}
		}
	}


	void Level::saveSpawn() {
		string filename = folder + LIBGENS_LEVEL_DATA_STAGE;

		TiXmlDocument doc(filename);
		if (!doc.LoadFile()) {
			return;
		}

		TiXmlHandle hDoc(&doc);
		TiXmlElement* pElem;
		TiXmlElement* pElem_i;
		TiXmlHandle hRoot(0);

		pElem=hDoc.FirstChildElement().Element();
		if (!pElem) {
			return;
		}

		// Update the current spawn point with an active Sonic Spawn object if available
		list<Object *> spawn_objects;
		getObjectsByName(LIBGENS_SPAWN_POINT_OBJECT_NAME, spawn_objects);

		for (list<Object *>::iterator it=spawn_objects.begin(); it!=spawn_objects.end(); it++) {
			ObjectElement *element = (*it)->getElement(LIBGENS_SPAWN_POINT_OBJECT_FLAG);

			if (element) {
				ObjectElementBool *element_bool = static_cast<LibGens::ObjectElementBool *>(element);

				if (element_bool->value) {
					ObjectElementString *element_mode = static_cast<LibGens::ObjectElementString *>((*it)->getElement(LIBGENS_LEVEL_XML_MODE));
					ObjectElementString *element_view = static_cast<LibGens::ObjectElementString *>((*it)->getElement(LIBGENS_LEVEL_XML_CAMERA_VIEW));
					ObjectElementFloat  *element_speed = static_cast<LibGens::ObjectElementFloat *>((*it)->getElement(LIBGENS_LEVEL_XML_SPEED));
					ObjectElementFloat  *element_time = static_cast<LibGens::ObjectElementFloat *>((*it)->getElement(LIBGENS_LEVEL_XML_TIME));

					spawn_yaw         = (*it)->getRotation().getYawDegrees();
					spawn_position    = (*it)->getPosition();
					spawn_mode        = (element_mode  ? element_mode->value  : "Stand");
					spawn_speed       = (element_speed ? element_speed->value : 0.0f);
					spawn_time        = (element_time  ? element_time->value  : 0.0f);
					spawn_camera_view = (element_view ? element_view->value : "Forward");
					break;
				}
			}
		}

		// Search and delete the first appearance of the Sonic Spawn Point Segment
		pElem_i=pElem->FirstChildElement();
		for(pElem_i; pElem_i; pElem_i=pElem_i->NextSiblingElement()) {
			if (pElem_i->ValueStr() == LIBGENS_LEVEL_XML_SONIC) {
				pElem->RemoveChild(pElem_i);
				break;
			}
		}

		// Re-create the Sonic Spawn Point Segment and insert it at the start

		TiXmlElement newSpawnNode(LIBGENS_LEVEL_XML_SONIC);
		pElem_i = pElem->InsertBeforeChild(pElem->FirstChildElement(), newSpawnNode)->ToElement();

		TiXmlElement* positionRoot=new TiXmlElement(LIBGENS_OBJECT_ELEMENT_POSITION);
		spawn_position.writeXML(positionRoot);
		pElem_i->LinkEndChild(positionRoot);

		TiXmlElement* yawRoot=new TiXmlElement(LIBGENS_LEVEL_XML_YAW);
		TiXmlText* yawValue=new TiXmlText(ToString(spawn_yaw));
		yawRoot->LinkEndChild(yawValue);
		pElem_i->LinkEndChild(yawRoot);

		TiXmlElement* startRoot=new TiXmlElement(LIBGENS_LEVEL_XML_START);
		{
			TiXmlElement* modeRoot=new TiXmlElement(LIBGENS_LEVEL_XML_MODE);
			TiXmlText* modeValue=new TiXmlText(spawn_mode);
			modeRoot->LinkEndChild(modeValue);
			startRoot->LinkEndChild(modeRoot);

			TiXmlElement* speedRoot=new TiXmlElement(LIBGENS_LEVEL_XML_SPEED);
			TiXmlText* speedValue=new TiXmlText(ToString(spawn_speed));
			speedRoot->LinkEndChild(speedValue);
			startRoot->LinkEndChild(speedRoot);

			TiXmlElement* timeRoot=new TiXmlElement(LIBGENS_LEVEL_XML_TIME);
			TiXmlText* timeValue=new TiXmlText(ToString(spawn_time));
			timeRoot->LinkEndChild(timeValue);
			startRoot->LinkEndChild(timeRoot);
		}
		pElem_i->LinkEndChild(startRoot);

		TiXmlElement* deadHeightRoot=new TiXmlElement(LIBGENS_LEVEL_XML_DEAD_HEIGHT);
		TiXmlText* deadHeightValue=new TiXmlText(ToString(spawn_dead_height));
		deadHeightRoot->LinkEndChild(deadHeightValue);
		pElem_i->LinkEndChild(deadHeightRoot);

		TiXmlElement* cameraViewRoot=new TiXmlElement(LIBGENS_LEVEL_XML_CAMERA_VIEW);
		TiXmlText* cameraViewValue=new TiXmlText(spawn_camera_view);
		cameraViewRoot->LinkEndChild(cameraViewValue);
		pElem_i->LinkEndChild(cameraViewRoot);

		// Save the changes
		if (!doc.SaveFile(filename)) {
			Error::addMessage(Error::FILE_NOT_FOUND, LIBGENS_OBJECT_H_ERROR_WRITE_SET_XML + filename);
		}
	}


	ObjectSet *Level::getSet(string name) {
		for (list<ObjectSet *>::iterator it=sets.begin(); it!=sets.end(); it++) {
			if ((*it)->getName() == name) {
				return *it;
			}
		}

		return NULL;
	}


	Object *Level::getObjectByID(size_t id) {
		for (list<ObjectSet *>::iterator it=sets.begin(); it!=sets.end(); it++) {
			Object *obj=(*it)->getByID(id);

			if (obj) {
				return obj;
			}
		}

		return NULL;
	}

	size_t Level::newObjectID() {
		size_t id;

		for (id=LIBGENS_LEVEL_START_ID_GENERATION; id<LIBGENS_LEVEL_END_ID_GENERATION; id++) {
			Object *object=getObjectByID(id);
			if (!object) break;
		}

		return id;
	}

	void Level::learnFromLibrary(ObjectLibrary *library) {
		if (!library) return;

		for (list<ObjectSet *>::iterator it=sets.begin(); it!=sets.end(); it++) {
			(*it)->learnFromLibrary(library);
		}
	}

	void Level::getObjectsByName(string name, list<Object *> &total_list) {
		for (list<ObjectSet *>::iterator it=sets.begin(); it!=sets.end(); it++) {
			(*it)->getObjectsByName(name, total_list);
		}
	}

	LevelCollisionEntry *Level::getCollisionEntry(string collision_filename) {
		for (list<LibGens::LevelCollisionEntry *>::iterator it=collision_entries.begin(); it!=collision_entries.end(); it++) {
			if (collision_filename == (*it)->name) {
				return *it;
			}
		}

		return NULL;
	}

	SceneEffect &Level::getSceneEffect() {
		return scene_effect;
	}

	void Level::setName(string nm) {
		slot=nm;
	}

	size_t Level::getGameMode() {
		return game_mode;
	}

	void Level::setGameMode(size_t v) {
		game_mode = v;
	}

	void Level::setGameMode(string v) {
		game_mode = LIBGENS_LEVEL_GAME_GENERATIONS;
		if (v == LIBGENS_LEVEL_GAME_STRING_UNLEASHED) {
			game_mode = LIBGENS_LEVEL_GAME_UNLEASHED;
		}
	}

	string Level::getName() {
		return slot;
	}

	string Level::getTerrainInfo() {
		return terrain_info_file;
	}

	string Level::getDirectLight() {
		return direct_light_name;
	}

	string Level::getSkybox() {
		if (skybox_names.size()) {
			return skybox_names[0];
		}

		return "";
	}

	void Level::addSet(ObjectSet *set) {
		sets.push_back(set);
	}

	list<ObjectSet *> Level::getSets() {
		return sets;
	}

	list<Path *> Level::getPaths() {
		return paths;
	}

	list<LevelCollisionEntry *> Level::getCollisionEntries() {
		return collision_entries;
	}
}