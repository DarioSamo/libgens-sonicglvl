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

#include "LibGens.h"
#include "EditorViewerGrid.h"

const Ogre::String EditorViewerGrid::GridLineXName = "GridLineX";
const Ogre::String EditorViewerGrid::GridLineZName = "GridLineZ";

EditorViewerGrid::EditorViewerGrid(Ogre::SceneManager *scene_manager) {
    scene_node = scene_manager->getRootSceneNode()->createChildSceneNode();

    float line_width = 0.005f;
    float grid_size_f = 0.2f;
    int grid_size_x = 25;
    int grid_size_y = 25;
    float grid_length_x = (grid_size_x * grid_size_f) / 2;
    float grid_length_y = (grid_size_y * grid_size_f) / 2;

    for (int i = 0; i <= grid_size_x; i++) {
        Ogre::ManualObject *manual = static_cast<Ogre::ManualObject*>(scene_manager->createMovableObject(GridLineXName + ToString(i + 1), Ogre::ManualObjectFactory::FACTORY_TYPE_NAME));
        manual->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_TRIANGLE_STRIP);

        manual->position(-line_width, line_width, -grid_length_x);
        manual->position(-line_width, line_width, grid_length_x);
        manual->position(0.0, line_width, -grid_length_x);
        manual->position(0.0, line_width, grid_length_x);

        manual->end();

        Ogre::SceneNode *grid_node = scene_node->createChildSceneNode();
        grid_node->setPosition(-grid_length_x + i*grid_size_f, 0.0, 0.0);
        grid_node->attachObject(manual);
    }

    for (int i = 0; i <= grid_size_y; i++) {
        Ogre::ManualObject *manual = static_cast<Ogre::ManualObject*>(scene_manager->createMovableObject(GridLineZName + ToString(i + 1), Ogre::ManualObjectFactory::FACTORY_TYPE_NAME));
        manual->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_TRIANGLE_STRIP);

        manual->position(-grid_length_y, 0.0, 0.0);
        manual->position(grid_length_y, 0.0, 0.0);
        manual->position(-grid_length_y, 0.0, -line_width);
        manual->position(grid_length_y, 0.0, -line_width);

        manual->end();

        Ogre::SceneNode *grid_node = scene_node->createChildSceneNode();
        grid_node->setPosition(0.0, 0.0, -grid_length_y + i*grid_size_f);
        grid_node->attachObject(manual);
    }
}

EditorViewerGrid::~EditorViewerGrid() {

}
