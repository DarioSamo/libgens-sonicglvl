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

#pragma once

#define LIBGENS_INSTANCE_MTI_ERROR_MESSAGE_NULL_FILE        "Trying to read instance brush data from unreferenced file."
#define LIBGENS_INSTANCE_MTI_ERROR_MESSAGE_WRITE_NULL_FILE  "Trying to write instance brush data to an unreferenced file."
#define LIBGENS_INSTANCE_MTI_EXTENSION                      ".mti"

#define LIBGENS_INSTANCE_MTI_COUNT_ADDRESS               8
#define LIBGENS_INSTANCE_MTI_TABLE_ADDRESS               28
#define LIBGENS_INSTANCE_MTI_NODE_SIZE                   24
#define LIBGENS_INSTANCE_MTI_HEADER                      0x2049544D
#define LIBGENS_INSTANCE_MTI_ROOT_TYPE                   1
#define LIBGENS_INSTANCE_MTI_HEADER_NULL_SIZE            12
#define LIBGENS_INSTANCE_MTI_HEADER_NODE_ADDRESS         32

#define LIBGENS_INSTANCE_MTI_NODE_HEADER                 0xFF

namespace LibGens {
	class InstanceBrushNode {
		protected:
			Vector3 position;
			unsigned char index;
			float rot_x;
			float rot_y;
			float rot_z;
			Color color; 
		public:
			InstanceBrushNode();
			void read(File *file);
			void write(File *file);
	};

	class InstanceBrush {
		protected:
			list<InstanceBrushNode *> nodes;
		public:
			InstanceBrush(string filename);
			void read(File *file);
			void save(string filename);
			void write(File *file);
	};
};
