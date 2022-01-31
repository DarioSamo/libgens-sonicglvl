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
	class SampleChunkNode {
		protected:
			string name;
			unsigned int value;
			void *data;
			size_t data_address;
			vector<SampleChunkNode *> nodes;
		public:
			SampleChunkNode();
			SampleChunkNode(string name_p, unsigned int value_p);
			~SampleChunkNode();
			bool read(File *file);
			void write(File *file, bool last_node = false);
			string getName();
			unsigned int getValue();
			size_t getDataAddress();
			void setName(string v);
			void setValue(unsigned int v);
			void setData(void *data_p, int data_type);
			vector<SampleChunkNode *> getNodes();
			void addNode(SampleChunkNode *node);
			SampleChunkNode *newNode(string name = "", unsigned int value = 0);
			SampleChunkNode *find(string name, bool search_children = true);
	};
};