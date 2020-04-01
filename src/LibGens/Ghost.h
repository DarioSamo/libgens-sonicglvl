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

#define LIBGENS_GHOST_ERROR_MESSAGE_NULL_FILE        "Trying to read ghost data from unreferenced file."
#define LIBGENS_GHOST_ERROR_MESSAGE_WRITE_NULL_FILE  "Trying to write ghost data to an unreferenced file."
#define LIBGENS_GHOST_EXTENSION                      ".gst.bin"

namespace LibGens {
	class GhostNode;

	class Ghost {
		protected:
			vector<string> animation_names;
			vector<GhostNode *> ghost_nodes;
		public:
			Ghost(string filename);
			void read(File *file);
			void write(File *file);
			void save(string filename);
			void calculate(float time, Vector3 &position, Quaternion &rotation, string &animation_name, float &animation_frame, bool &animation_ball);
			float calculateDuration();
	};
};

