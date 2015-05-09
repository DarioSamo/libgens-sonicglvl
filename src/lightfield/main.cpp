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
#include "Terrain.h"
#include "VRMap.h"
#include "LightField.h"
#include "minini/minini.h"

int main(int argc, char** argv) {
	char temp[1024]="";
	char ini_file[]="lightfield.ini";

	ini_gets("Main", "TerrainFile", "", temp, 512, ini_file);
	string terrain_file=ToString(temp);
	ini_gets("Main", "Resources", "", temp, 512, ini_file);
	string resources=ToString(temp);
	ini_gets("Main", "Stage", "", temp, 512, ini_file);
	string stage=ToString(temp);
	ini_gets("Main", "GIA", "", temp, 512, ini_file);
	string gia=ToString(temp);

	float sample_unit_distance=ini_getf("Main", "SampleUnitDistance", 0.75f, ini_file);
	float saturation_multiplier=ini_getf("Main", "SaturationMultiplier", 1.0f, ini_file);
	float lookup_grid_size=ini_getf("Main", "LookupGridSize", 5.0f, ini_file);

	LibGens::initialize();
	LibGens::Error::setLogging(true);

	LibGens::Terrain terrain(terrain_file, resources, stage, gia);
	printf("Done loading Terrain\n");

	// Sample Unit Distance, Saturation Multiplier
	LibGens::VRMap *vrmap=terrain.generateVRMap(sample_unit_distance, saturation_multiplier);
	printf("VR Map setup...\n");

	vrmap->generateLookupGrid(lookup_grid_size, sample_unit_distance);
	printf("Done generating VR Map of %d samples covering %f units of area.\n", vrmap->getSampleList().size(), vrmap->getAABB().size());

	LibGens::LightField lightfield;

	float ambient_color_r=ini_getf("Main", "AmbientColorR", 1.0f, ini_file);
	float ambient_color_g=ini_getf("Main", "AmbientColorG", 1.0f, ini_file);
	float ambient_color_b=ini_getf("Main", "AmbientColorB", 1.0f, ini_file);
	float ambient_color_a=ini_getf("Main", "AmbientColorA", 1.0f, ini_file);
	int sample_min_count=ini_getl("Main", "SampleMinCount", 2, ini_file);
	float sample_blend_distance=ini_getf("Main", "SampleBlendDistance", 8.0f, ini_file);
	float min_octree_cube_size=ini_getf("Main", "MinOctreeCubeSize", 3.0f, ini_file);
	int cpu_threads=ini_getl("Main", "Threads", 1, ini_file);

	// Ambient Color, Sample Min Count, Sample Blend Distance, Min Octree Cube Size
	lightfield.generate(vrmap, LibGens::Color(ambient_color_r, ambient_color_g, ambient_color_b, ambient_color_a), sample_min_count, sample_blend_distance, min_octree_cube_size, cpu_threads);
	printf("Done generating lightfield.\n");
	lightfield.save("light-field.lft");
	printf("Saved lightfield. Press Enter to exit.\n");
    getchar();
    return 0;
}