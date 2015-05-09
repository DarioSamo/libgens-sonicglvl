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

#include "Havok.h"
#include "HavokEnviroment.h"
#include "HavokPhysicsCache.h"
#include "HavokSkeletonCache.h"
#include "HavokAnimationCache.h"

namespace LibGens {
	void HavokEnviroment::addFolder(string folder) {
		bool found = false;
		for (list<string>::iterator it=search_paths.begin(); it!=search_paths.end(); it++) {
			if ((*it) == folder) {
				found = true;
				break;
			}
		}

		if (!found) {
			search_paths.push_back(folder);
		}
	}

	HavokEnviroment::HavokEnviroment(int bufferSize) {
		hkMemoryRouter * memoryRouter = hkMemoryInitUtil::initDefault( hkMallocAllocator::m_defaultMallocAllocator, hkMemorySystem::FrameInfo(bufferSize));
		hkBaseSystem::init(memoryRouter, HavokErrorReport);
	}

	
	HavokPhysicsCache *HavokEnviroment::getPhysics(string physics_name) {
		for (list<HavokPhysicsCache *>::iterator it=physics_cache.begin(); it!=physics_cache.end(); it++) {
			if ((*it)->getName() == physics_name) {
				return (*it);
			}
		}

		for (list<string>::iterator it=search_paths.begin(); it!=search_paths.end(); it++) {
			string filename=(*it) + physics_name + LIBGENS_HAVOK_PHYSICS_EXTENSION;

			if (File::check(filename)) {
				hkSerializeUtil::ErrorDetails loadError;
				hkResource *data = hkSerializeUtil::load(filename.c_str(), &loadError);
				if (!(loadError.id == loadError.ERRORID_NONE && data != NULL)) {
					Error::addMessage(Error::FILE_NOT_FOUND, "Couldn't load " + filename + " Havok file. Reason: " + ToString(loadError.defaultMessage.cString()));
					continue;
				}

				hkRootLevelContainer *container = data->getContents<hkRootLevelContainer>();
				if (container) {
					hkpPhysicsData *physics = container->findObject<hkpPhysicsData>();

					if (physics) {
						HavokPhysicsCache *physics_cache_entry = new HavokPhysicsCache(data, filename, physics_name, physics);
						physics_cache.push_back(physics_cache_entry);
						return physics_cache_entry;
					}
				}
			}
		}

		return NULL;
	}



	HavokSkeletonCache *HavokEnviroment::getSkeleton(string skeleton_name) {
		for (list<HavokSkeletonCache *>::iterator it=skeleton_cache.begin(); it!=skeleton_cache.end(); it++) {
			if ((*it)->getName() == skeleton_name) {
				return (*it);
			}
		}

		for (list<string>::iterator it=search_paths.begin(); it!=search_paths.end(); it++) {
			string filename=(*it) + skeleton_name + LIBGENS_HAVOK_SKELETON_EXTENSION;

			if (File::check(filename)) {
				hkSerializeUtil::ErrorDetails loadError;
				hkResource *data = hkSerializeUtil::load(filename.c_str(), &loadError);
				if (!(loadError.id == loadError.ERRORID_NONE && data != NULL)) {
					Error::addMessage(Error::FILE_NOT_FOUND, "Couldn't load " + filename + " Havok file. Reason: " + ToString(loadError.defaultMessage.cString()));
					continue;
				}

				hkRootLevelContainer *container = data->getContents<hkRootLevelContainer>();
				if (container) {
					hkaAnimationContainer *animations = container->findObject<hkaAnimationContainer>();

					if (animations) {
						for (int i=0; i<animations->m_skeletons.getSize(); i++) {
							hkaSkeleton *skeleton=animations->m_skeletons[i];

							if (skeleton) {
								HavokSkeletonCache *skeleton_cache_entry = new HavokSkeletonCache(data, filename, skeleton_name, skeleton);
								skeleton_cache.push_back(skeleton_cache_entry);
								return skeleton_cache_entry;
							}
						}
					}
				}
			}
		}

		return NULL;
	}

	
	HavokAnimationCache *HavokEnviroment::getAnimation(string animation_name) {
		for (list<HavokAnimationCache *>::iterator it=animation_cache.begin(); it!=animation_cache.end(); it++) {
			if ((*it)->getName() == animation_name) {
				return (*it);
			}
		}

		for (list<string>::iterator it=search_paths.begin(); it!=search_paths.end(); it++) {
			string filename=(*it) + animation_name + LIBGENS_HAVOK_ANIMATION_EXTENSION;

			if (File::check(filename)) {
				hkSerializeUtil::ErrorDetails loadError;
				hkResource *data = hkSerializeUtil::load(filename.c_str(), &loadError);
				if (!(loadError.id == loadError.ERRORID_NONE && data != NULL)) {
					Error::addMessage(Error::FILE_NOT_FOUND, "Couldn't load " + filename + " Havok file. Reason: " + ToString(loadError.defaultMessage.cString()));
					continue;
				}

				hkRootLevelContainer *container = data->getContents<hkRootLevelContainer>();
				if (container) {
					hkaAnimationContainer *animations = container->findObject<hkaAnimationContainer>();

					if (animations) {
						for (int i=0; i<animations->m_bindings.getSize(); i++) {
							hkaAnimationBinding *animation_binding=animations->m_bindings[i];
							if (animation_binding) {
								HavokAnimationCache *animation_cache_entry = new HavokAnimationCache(data, filename, animation_name, animation_binding, animation_binding->m_animation);
								animation_cache.push_back(animation_cache_entry);
								return animation_cache_entry;
							}
						}
					}
				}
			}
		}

		return NULL;
	}

	bool HavokEnviroment::deletePhysicsEntry(string physics_name) {
		for (list<HavokPhysicsCache *>::iterator it=physics_cache.begin(); it!=physics_cache.end(); it++) {
			if ((*it)->getName() == physics_name) {
				delete (*it);
				physics_cache.erase(it);
				return true;
			}
		}

		return false;
	}
};