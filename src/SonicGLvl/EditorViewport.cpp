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

//==================================================================================================================================
// Raycasting/Entity Information code has been copied/modified from: http://www.ogre3d.org/tikiwiki/Raycasting+to+the+polygon+level
//==================================================================================================================================

#include "EditorApplication.h"

EditorViewport::EditorViewport(Ogre::SceneManager *scene_manager, Ogre::SceneManager *axis_scene_manager, Ogre::RenderWindow *window, string camera_name, int zOrder, float left, float top, float width, float height) :
	moving(false),
	rotating(false),
	zooming(false),
	panning_left(false),
	panning_right(false),
	panning_up(false),
	panning_down(false),
	panning_multiplier(0.1),
	rotation_multiplier(0.1),
	zooming_multiplier(0.5)
{
	if (!scene_manager) {
		return;
	}

	if (!window) {
		return;
	}

	bg_color = Ogre::ColourValue(0.4, 0.4, 0.4);

	camera = scene_manager->createCamera(camera_name);
    camera->setPosition(Ogre::Vector3(0,1,0));
    camera->lookAt(Ogre::Vector3(10,1,0));
    camera->setNearClipDistance(1);
	camera->setUseRenderingDistance(true);
	camera->setFarClipDistance(EDITOR_VIEWPORT_FAR_CLIP_DEFAULT);

	viewport = window->addViewport(camera, zOrder, left, top, width, height);
    viewport->setBackgroundColour(bg_color);
    camera->setAspectRatio(Ogre::Real(viewport->getActualWidth()) / Ogre::Real(viewport->getActualHeight()));    

	camera_overlay = axis_scene_manager->createCamera(camera_name);
    camera_overlay->setPosition(Ogre::Vector3(0,1,0));
    camera_overlay->lookAt(Ogre::Vector3(10,1,0));
    camera_overlay->setNearClipDistance(1);
	camera_overlay->setUseRenderingDistance(true);
	camera_overlay->setFarClipDistance(EDITOR_VIEWPORT_FAR_CLIP_DEFAULT);
	
	viewport_overlay = window->addViewport(camera_overlay, zOrder+1, left, top, width, height);
    viewport_overlay->setBackgroundColour(Ogre::ColourValue(0,0,0));
	camera_overlay->setAspectRatio(Ogre::Real(viewport_overlay->getActualWidth()) / Ogre::Real(viewport_overlay->getActualHeight()));    
	viewport_overlay->setClearEveryFrame(true, Ogre::FBT_DEPTH);
	viewport_overlay->setSkiesEnabled(false);


	ray_scene_query          =  scene_manager->createRayQuery(Ogre::Ray());
	ray_scene_query_overlay  =  axis_scene_manager->createRayQuery(Ogre::Ray());

	current_entity = NULL;
}


void EditorViewport::resize(float left, float top, float width, float height) {
	viewport->setDimensions(left, top, width, height);
    camera->setAspectRatio(Ogre::Real(viewport->getActualWidth()) / Ogre::Real(viewport->getActualHeight()));    

	viewport_overlay->setDimensions(left, top, width, height);
	camera_overlay->setAspectRatio(Ogre::Real(viewport_overlay->getActualWidth()) / Ogre::Real(viewport_overlay->getActualHeight()));
}



bool EditorViewport::keyPressed(const OIS::KeyEvent &arg) {
	if(arg.key == OIS::KC_LMENU) {
		rotating = true;
	}

	if(arg.key == OIS::KC_LCONTROL) {
		zooming = true;
	}

	if(arg.key == OIS::KC_LEFT) {
		panning_left = true;
	}

	if(arg.key == OIS::KC_RIGHT) {
		panning_right = true;
	}

	if(arg.key == OIS::KC_UP) {
		panning_up = true;
	}

	if(arg.key == OIS::KC_DOWN) {
		panning_down = true;
	}

    return true;
}


bool EditorViewport::keyReleased(const OIS::KeyEvent &arg) {
	if(arg.key == OIS::KC_LMENU) {
		rotating = false;
	}

	if(arg.key == OIS::KC_LCONTROL) {
		zooming = false;
	}

	if(arg.key == OIS::KC_LEFT) {
		panning_left = false;
	}

	if(arg.key == OIS::KC_RIGHT) {
		panning_right = false;
	}

	if(arg.key == OIS::KC_UP) {
		panning_up = false;
	}

	if(arg.key == OIS::KC_DOWN) {
		panning_down = false;
	}

    return true;
}


void EditorViewport::getEntityInformation(Ogre::Entity *entity, size_t &vertex_count, Ogre::Vector3* &vertices, size_t &index_count, 
										  unsigned long* &indices, const Ogre::Vector3 &position, const Ogre::Quaternion &orient, const Ogre::Vector3 &scale) {
    bool added_shared = false;
    size_t current_offset = 0;
    size_t shared_offset = 0;
    size_t next_offset = 0;
    size_t index_offset = 0;
    vertex_count = index_count = 0;
	Ogre::MeshPtr mesh = entity->getMesh();
    for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i) {
        Ogre::SubMesh* submesh = mesh->getSubMesh(i);
        if(submesh->useSharedVertices) {
            if(!added_shared) {
                vertex_count += mesh->sharedVertexData->vertexCount;
                added_shared = true;
            }
        }
        else {
			vertex_count += submesh->vertexData->vertexCount;
        }
        index_count += submesh->indexData->indexCount;
    }
 
    vertices = new Ogre::Vector3[vertex_count];
    indices = new unsigned long[index_count];
    added_shared = false;
    for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i) {
        Ogre::SubMesh* submesh = mesh->getSubMesh(i);
		Ogre::VertexData* vertex_data;
        vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;

        if((!submesh->useSharedVertices)||(submesh->useSharedVertices && !added_shared)) {
            if(submesh->useSharedVertices) {
                added_shared = true;
                shared_offset = current_offset;
            }
            const Ogre::VertexElement* posElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
            Ogre::HardwareVertexBufferSharedPtr vbuf = vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());
            unsigned char* vertex = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
            float* pReal;
            for( size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize()) {
                posElem->baseVertexPointerToElement(vertex, &pReal);
                Ogre::Vector3 pt(pReal[0], pReal[1], pReal[2]);
                vertices[current_offset + j] = (orient * (pt * scale)) + position;
            }
            vbuf->unlock();
            next_offset += vertex_data->vertexCount;
        }
 
        Ogre::IndexData* index_data = submesh->indexData;
        size_t numTris = index_data->indexCount / 3;
        Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;
        bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);
        unsigned long*  pLong = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
        unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);
        size_t offset = (submesh->useSharedVertices)? shared_offset : current_offset;
        size_t index_start = index_data->indexStart;
        size_t last_index = numTris*3 + index_start;
        if (use32bitindexes) {
            for (size_t k = index_start; k < last_index; ++k) {
                indices[index_offset++] = pLong[k] + static_cast<unsigned long>( offset );
            }
		}
        else {
            for (size_t k = index_start; k < last_index; ++k) {
                indices[ index_offset++ ] = static_cast<unsigned long>( pShort[k] ) + static_cast<unsigned long>( offset );
            }
		}
 
        ibuf->unlock();
        current_offset = next_offset;
    }
}

void EditorViewport::convertMouseToLocalScreen(float &mouse_x, float &mouse_y) {
	float left=viewport->getLeft();
	float top=viewport->getTop();
	float width=viewport->getWidth();
	float height=viewport->getHeight();

	mouse_x -= left;
	mouse_y -= top;

	mouse_x /= width;
	mouse_y /= height;
}


bool EditorViewport::isMouseInLocalScreen(const OIS::MouseEvent &arg) {
	float mouse_x=arg.state.X.abs/float(arg.state.width);
	float mouse_y=arg.state.Y.abs/float(arg.state.height);
	convertMouseToLocalScreen(mouse_x, mouse_y);

	if ((mouse_x > 0.0f) && (mouse_x < 1.0f) && (mouse_y > 0.0f) && (mouse_y < 1.0f)) {
		return true;
	}

	return false;
}


Ogre::Entity *EditorViewport::raycast(float raycast_x, float raycast_y, Ogre::RaySceneQuery *query, Ogre::Vector3 *output_point, Ogre::uint32 flags) {
	Ogre::Ray ray = camera->getCameraToViewportRay(raycast_x, raycast_y);
	if (flags) query->setQueryMask(flags);
	query->setRay(ray);
	query->setSortByDistance(true);
 
	Ogre::RaySceneQueryResult &result = query->execute();
	Ogre::RaySceneQueryResult::iterator iter = result.begin();

    Ogre::Real closest_distance = -1.0f;
    Ogre::Vector3 closest_result;
    Ogre::RaySceneQueryResult &query_result = query->getLastResults();

	Ogre::Entity *result_entity = NULL;

    for (size_t qr_idx = 0; qr_idx < query_result.size(); qr_idx++) {
        if ((closest_distance >= 0.0f) && (closest_distance < query_result[qr_idx].distance)) break;
 
        if ((query_result[qr_idx].movable != NULL) && (query_result[qr_idx].movable->getMovableType().compare("Entity") == 0)) {
            Ogre::Entity *pentity = static_cast<Ogre::Entity*>(query_result[qr_idx].movable); 

			if (!pentity->getVisible()) {
				continue;
			}

            size_t vertex_count;
            size_t index_count;
            Ogre::Vector3 *vertices;
            unsigned long *indices;
 
			getEntityInformation(pentity, vertex_count, vertices, index_count, indices,             
                              pentity->getParentNode()->_getDerivedPosition(),
                              pentity->getParentNode()->_getDerivedOrientation(),
                              pentity->getParentNode()->_getDerivedScale());
 

            bool new_closest_found = false;
            for (int i = 0; i < static_cast<int>(index_count); i += 3) {
                std::pair<bool, Ogre::Real> hit = Ogre::Math::intersects(ray, vertices[indices[i]], vertices[indices[i+1]], vertices[indices[i+2]], true, false);
 
                if (hit.first) {
					if ((closest_distance < 0.0f) || (hit.second < closest_distance)) {
                        closest_distance = hit.second;
                        new_closest_found = true;
                    }
                }
            }
 
            delete[] vertices;
            delete[] indices;
 
            if (new_closest_found) {
                if (output_point) *output_point = ray.getPoint(closest_distance);    
				result_entity = pentity;
				//return pentity;
            }
        }       
    }

	return result_entity;
}


bool EditorViewport::raycastPlacement(float raycast_x, float raycast_y, float placement_distance, Ogre::Vector3 *output_point, Ogre::uint32 flags) {
	if (raycast(raycast_x, raycast_y, ray_scene_query, output_point, flags)) {
		return true;
	}

	if (output_point) {
		Ogre::Ray ray = camera->getCameraToViewportRay(raycast_x, raycast_y);
		*output_point = ray.getPoint(placement_distance);    
	}
	return false;
}


Ogre::Entity *EditorViewport::raycastEntity(float raycast_x, float raycast_y, Ogre::uint32 flags) {
	return raycast(raycast_x, raycast_y, ray_scene_query, NULL, flags);
}


void EditorViewport::update() {
	Ogre::Vector3 camera_panning_movement(0,0,0);

	if (panning_left)  camera_panning_movement.x = -panning_multiplier;
	if (panning_right) camera_panning_movement.x = panning_multiplier;
	if (panning_down)  camera_panning_movement.y = -panning_multiplier;
	if (panning_up)    camera_panning_movement.y = panning_multiplier;

	if (panning_left || panning_right || panning_down || panning_up) {
		camera->moveRelative(camera_panning_movement);
		camera_overlay->moveRelative(camera_panning_movement);
	}
}


bool EditorViewport::mouseMoved(const OIS::MouseEvent &arg) {
	if (moving) {
		float mouse_movement_x=arg.state.X.rel;
		float mouse_movement_y=arg.state.Y.rel;

		// Movement Priority
		// Zooming > Rotation > Panning

		if (zooming) {
			camera->moveRelative(Ogre::Vector3(0, 0, mouse_movement_y*zooming_multiplier));
			camera_overlay->moveRelative(Ogre::Vector3(0, 0, mouse_movement_y*zooming_multiplier));
		}
		else if (rotating) {
			camera->yaw(Ogre::Degree(-mouse_movement_x*rotation_multiplier));
			camera->pitch(Ogre::Degree(-mouse_movement_y*rotation_multiplier));

			camera_overlay->yaw(Ogre::Degree(-mouse_movement_x*rotation_multiplier));
			camera_overlay->pitch(Ogre::Degree(-mouse_movement_y*rotation_multiplier));
		}
		else {
			camera->moveRelative(Ogre::Vector3(-mouse_movement_x*panning_multiplier, mouse_movement_y*panning_multiplier, 0));
			camera_overlay->moveRelative(Ogre::Vector3(-mouse_movement_x*panning_multiplier, mouse_movement_y*panning_multiplier, 0));
		}
	}

	// Raycast to 3D overlay
	float mouse_x=arg.state.X.abs/float(arg.state.width);
	float mouse_y=arg.state.Y.abs/float(arg.state.height);

	convertMouseToLocalScreen(mouse_x, mouse_y);

	current_entity = raycast(mouse_x, mouse_y, ray_scene_query_overlay, NULL, EDITOR_NODE_AXIS);

	// Raycast to 3D scene if it didn't hit anything in the overlay
	if (!current_entity) {
		current_entity = raycast(mouse_x, mouse_y, ray_scene_query, NULL, query_flags);
	}
    return true;
}


bool EditorViewport::mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id) {
	if (id == OIS::MB_Middle) moving = true;
    return true;
}


bool EditorViewport::mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id) {
	if (id == OIS::MB_Middle) moving = false;
    return true;
}


void EditorViewport::focusOnPoint(Ogre::Vector3 point, Ogre::Real distance, Ogre::Vector3 direction) {
	direction.normalise();
	Ogre::Vector3 new_position = point - (direction * distance);

	camera->setPosition(new_position);
	camera->setDirection(direction);
	
	camera_overlay->setPosition(new_position);
	camera_overlay->setDirection(direction);
}