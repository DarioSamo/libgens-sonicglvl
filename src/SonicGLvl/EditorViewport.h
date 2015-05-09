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

#ifndef EDITOR_VIEWPORT_H_INCLUDED
#define EDITOR_VIEWPORT_H_INCLUDED

#define EDITOR_VIEWPORT_FAR_CLIP_DEFAULT   9000

class EditorViewport {
	protected:
		Ogre::Camera *camera;
		Ogre::Camera *camera_overlay;
		Ogre::Viewport *viewport;
		Ogre::Viewport *viewport_overlay;
		Ogre::ColourValue bg_color;
		Ogre::RaySceneQuery *ray_scene_query;
		Ogre::RaySceneQuery *ray_scene_query_overlay;
		Ogre::Entity *current_entity;
		bool moving;
		bool rotating;
		bool zooming;

		bool panning_left;
		bool panning_right;
		bool panning_up;
		bool panning_down;

		float panning_multiplier;
		float rotation_multiplier;
		float zooming_multiplier;
		Ogre::uint32 query_flags;
	public:
		EditorViewport(Ogre::SceneManager *scene_manager, Ogre::SceneManager *axis_scene_manager, Ogre::RenderWindow *window, string camera_name, int zOrder=0, float left=0.0f, float top=0.0f, float width=1.0f, float height=1.0f);

		bool keyPressed(const OIS::KeyEvent &arg);
		bool keyReleased(const OIS::KeyEvent &arg);
		bool mouseMoved(const OIS::MouseEvent &arg);
		bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
		bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);

		void setPanningMultiplier(float v) {
			panning_multiplier = v;
		}

		void setRotationMultiplier(float v) {
			rotation_multiplier = v;

		}
		void setZoomingMultiplier(float v) {
			zooming_multiplier = v;
		}

		void resize(float left=0.0f, float top=0.0f, float width=1.0f, float height=1.0f);

		void getEntityInformation(Ogre::Entity *entity, size_t &vertex_count, Ogre::Vector3* &vertices, size_t &index_count, unsigned long* &indices, 
								  const Ogre::Vector3 &position, const Ogre::Quaternion &orient, const Ogre::Vector3 &scale);

		void convertMouseToLocalScreen(float &mouse_x, float &mouse_y);
		bool isMouseInLocalScreen(const OIS::MouseEvent &arg);
		
		Ogre::Entity *raycast(float raycast_x, float raycast_y, Ogre::RaySceneQuery *query, Ogre::Vector3 *output_point=NULL, Ogre::uint32 flags=0);
		bool raycastPlacement(float raycast_x, float raycast_y, float placement_distance=5.0f, Ogre::Vector3 *output_point=NULL, Ogre::uint32 flags=0);
		Ogre::Entity *raycastEntity(float raycast_x, float raycast_y, Ogre::uint32 flags=0);

		Ogre::Entity *getCurrentEntity() {
			return current_entity;
		}

		Ogre::Camera *getCamera() {
			return camera;
		}

		Ogre::Camera *getCameraOverlay() {
			return camera_overlay;
		}

		void setQueryFlags(Ogre::uint32 v) {
			query_flags = v;
		}

		Ogre::uint32 getQueryFlags() {
			return query_flags;
		}

		void update();

		void focusOnPoint(Ogre::Vector3 point, Ogre::Real distance=10.0f, Ogre::Vector3 direction=Ogre::Vector3(1, -1, -1));

		void setTechnique(string technique) {
			viewport->setMaterialScheme(technique);
			viewport_overlay->setMaterialScheme(technique);
		}
};

#endif