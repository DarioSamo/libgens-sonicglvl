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

namespace LibGens {
	class Bone {
		protected:
			Matrix4 matrix;
			string name;
			unsigned int parent_index;
		public:
			Bone();
			void readDescription(File *file);
			void readMatrix(File *file);
			void writeDescription(File *file);
			void writeMatrix(File *file);
			string getName();
			unsigned int getParentIndex();
			Matrix4 getMatrix();
	};
};