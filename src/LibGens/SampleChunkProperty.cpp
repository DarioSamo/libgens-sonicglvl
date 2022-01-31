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

#include "SampleChunkProperty.h"
#include "SampleChunkNode.h"

namespace LibGens {
	SampleChunkProperty::SampleChunkProperty(string name_p, unsigned int value_p) {
		name = name_p;
		value = value_p;
	}

	SampleChunkProperty::SampleChunkProperty(SampleChunkNode *sample_chunk_node_p) {
		name = sample_chunk_node_p->getName();
		value = sample_chunk_node_p->getValue();
	}

	string SampleChunkProperty::getName() {
		return name;
	}

	unsigned int SampleChunkProperty::getValue() {
		return value;
	}

	void SampleChunkProperty::setName(string v) {
		name = v;
	}

	void SampleChunkProperty::setValue(unsigned int v) {
		value = v;
	}

	SampleChunkNode *SampleChunkProperty::toSampleChunkNode() {
		return new SampleChunkNode( name, value );
	}
}
