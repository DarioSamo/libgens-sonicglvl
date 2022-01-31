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
	enum VertexElementData {
		VECTOR3=2761657,
		VECTOR3_360=2761104,
		VECTOR3_FORCES = 2761095,
		VECTOR2=2892709,
		VECTOR2_HALF=2892639,
		INDICESB=1712774,
		INDICES=1713030,
		VECTOR4=1713062,
		VECTOR4_CHAR=1712262
	};

	enum VertexElementID {	
		POSITION=0,
		BONE_WEIGHTS=1,
		BONE_INDICES=2,
		NORMAL=3,
		UV=5,
		TANGENT=6,
		BINORMAL=7,
		RGBA=10
	};

	// Vertex Format Table declarations
	class VertexFormatElement {
		protected:
			unsigned int offset;
			VertexElementData data;
			VertexElementID id;
			unsigned short index;
		public:
			VertexFormatElement();
			VertexFormatElement(unsigned int offset_p, VertexElementData data_p, VertexElementID id_p, size_t index_p);
			unsigned int getOffset();
			void setOffset(unsigned int offset_p);
			VertexElementData getData();
			void setData(VertexElementData data_p);
			VertexElementID getID();
			void setID(VertexElementID id_p);
			unsigned short getIndex();
			void setIndex(unsigned short index_p);
	};

	class VertexFormat {
		friend class VertexFormat;

		protected:
			list<VertexFormatElement> elements;
			unsigned int size;
		public:
			VertexFormat();
			VertexFormat(VertexFormat *clone);
			VertexFormat(unsigned int type);
			void addElement(VertexFormatElement vfe);
			list<VertexFormatElement> getElements();
			void read(File *file);
			void write(File *file);
			void setSize(unsigned int v);
			unsigned int getSize();
			void fixForPC();

	};
};