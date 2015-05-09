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

#include "BIXF.h"

namespace LibGens {
	const size_t BIXFConverter::IDTableSize=104;

	const string BIXFConverter::IDTable[] = {
		"X",
		"Y",
		"Z",
		"R",
		"G",
		"B",
		"A",
		"U",
		"V",
		"Id",
		"AddressMode",
		"AlphaScroll",
		"AlphaScrollRandom",
		"AlphaScrollSpeed",
		"Animation",
		"BlendMode",
		"ChildEmitter",
		"ChildEmitterTime",
		"Color",
		"ColorScroll",
		"ColorScrollRandom",
		"ColorScrollSpeed",
		"Deceleration",
		"DecelerationRandom",
		"Direction",
		"DirectionRandom",
		"DirectionType",
		"Effect",
		"EmissionDirectionType",
		"EmissionInterval",
		"EmitCondition",
		"Emitter",
		"EndAngle",
		"EndTime",
		"ExternalAccel",
		"ExternalAccelRandom",
		"Flags",
		"GravitationalAccel",
		"Height",
		"InParam",
		"InterpolationType",
		"Key",
		"Latitude",
		"LifeTime",
		"LocusHistorySize",
		"LocusHistorySizeRandom",
		"Longitude",
		"LoopStartTime",
		"LoopEndTime",
		"Material",
		"MaxCount",
		"Mesh",
		"MeshName",
		"Name",
		"OutParam",
		"Parameter",
		"Particle",
		"ParticlePerEmission",
		"PivotPosition",
		"PointCount",
		"Radius",
		"RandomFlags",
		"RandomRange",
		"ReboundPlaneY",
		"ReflectionCoeff",
		"ReflectionCoeffRandom",
		"RepeatType",
		"Rotation",
		"RotationAdd",
		"RotationAddRandom",
		"RotationRandom",
		"Scaling",
		"SecondaryAlphaScroll",
		"SecondaryAlphaScrollRandom",
		"SecondaryAlphaScrollSpeed",
		"SecondaryBlend",
		"SecondaryBlendMode",
		"SecondaryColorScroll",
		"SecondaryColorScrollRandom",
		"SecondaryColorScrollSpeed",
		"SecondaryTexture",
		"Shader",
		"Size",
		"SizeRandom",
		"Speed",
		"SpeedRandom",
		"Split",
		"StartAngle",
		"StartTime",
		"Texture",
		"TextureIndex",
		"Time",
		"Translation",
		"Type",
		"UvChangeInterval",
		"UvIndex",
		"UvIndexType",
		"Value",
		"ZOffset",
		"EmitterTranslationEffectRatio",
		"FollowEmitterTranslationRatio",
		"FollowEmitterTranslationYRatio",
		"UvIndexStart",
		"UvIndexEnd"
	};


	const size_t BIXFConverter::IDTableEventSize=80;

	const string BIXFConverter::IDTableEvent[] = {
		"Scene",
		"Name",
		"Pause",
		"Movie",
		"Stage",
		"Offset",
		"Camera",
		"Light",
		"Shadow",
		"Bloom",
		"Object",
		"Model",
		"Skeleton",
		"Animation",
		"UVAnimation",
		"PatternAnimation",
		"MaterialAnimation",
		"VisibilityAnimation",
		"MorphAnimation",
		"Id",
		"File",
		"Enable",
		"Tx",
		"Ty",
		"Tz",
		"Ry",
		"Set",
		"Cut",
		"NearClip",
		"FarClip",
		"Depth",
		"Global",
		"Bias",
		"SlopeBias",
		"MapSize",
		"MapBoxWidth",
		"MapBoxHeight",
		"MapBoxNear",
		"MapBoxFar",
		"Vertical",
		"Threshold",
		"Scale",
		"Focus",
		"FocusRange",
		"RangeNear",
		"RangeFar",
		"Subset",
		"Sound",
		"CueSheet",
		"Cue",
		"Mode",
		"BossName",
		"Start",
		"Length",
		"Restore",
		"Height1",
		"Height2",
		"Kind",
		"Localize",
		"Hide",
		"Wait",
		"Before",
		"After",
		"FadeIn",
		"FadeOut",
		"Color",
		"CutFade",
		"OutLength",
		"InLength",
		"OutWait",
		"InWait",
		"EdgeEmission",
		"Mix",
		"HideAutoDraw",
		"HideSetParticle",
		"Fog",
		"Density",
		"SubCueSheet",
		"SubCue",
		"BgmStart"
	};


	const size_t BIXFConverter::ValueTableSize=81;

	const string BIXFConverter::ValueTable[] = {
		"Box",
		"Cylinder",
		"Polygon",
		"Sphere",
		"Time",
		"MovingDistance",
		"ParentAxis",
		"Billboard",
		"XAxis",
		"YAxis",
		"ZAxis",
		"YRotationOnly",
		"Inward",
		"Outward",
		"Particle",
		"ParticleVelocity",
		"Deflection",
		"Layered",
		"Quad",
		"Mesh",
		"Locus",
		"Line",
		"TopLeft",
		"TopCenter",
		"TopRight",
		"MiddleLeft",
		"MiddleCenter",
		"MiddleRight",
		"BottomLeft",
		"BottomCenter",
		"BottomRight",
		"DirectionalAngle",
		"DirectionalAngleBillboard",
		"EmittedEmitterAxis",
		"EmitterAxis",
		"EmitterDirection",
		"Fixed",
		"InitialRandom",
		"InitialRandomReverseOrder",
		"InitialRandomSequentialOrder",
		"RandomOrder",
		"ReverseOrder",
		"SequentialOrder",
		"User",
		"Tx",
		"Ty",
		"Tz",
		"Rx",
		"Ry",
		"Rz",
		"Sx",
		"Sy",
		"Sz",
		"SAll",
		"ColorR",
		"ColorG",
		"ColorB",
		"ColorA",
		"UScroll",
		"VScroll",
		"UScrollAlpha",
		"VScrollAlpha",
		"SecondaryUScroll",
		"SecondaryVScroll",
		"SecondaryUScrollAlpha",
		"SecondaryVScrollAlpha",
		"EmissionInterval",
		"ParticlePerEmission",
		"Constant",
		"Hermite",
		"Linear",
		"Add",
		"Multiply",
		"Opaque",
		"PunchThrough",
		"Subtract",
		"Typical",
		"UseMaterial",
		"Zero",
		"Clamp",
		"Wrap"
	};


	string BIXFConverter::nodeIDtoString(unsigned char id, int mode_flag) {
		string value="Node #" + ToString((unsigned int) id);

		if (mode_flag == LIBGENS_BIXF_MODE_PARTICLES) {
			if (id < IDTableSize) value = IDTable[id];
		}

		if (mode_flag == LIBGENS_BIXF_MODE_EVENT) {
			if (id < IDTableEventSize) value = IDTableEvent[id];
		}

		return value;
	}

	string BIXFConverter::valueIDtoString(unsigned char id, int mode_flag) {
		string value="Value #" + ToString((unsigned int) id);

		if (mode_flag == LIBGENS_BIXF_MODE_PARTICLES) {
			if (id < ValueTableSize) value = ValueTable[id];
		}

		if (mode_flag == LIBGENS_BIXF_MODE_EVENT) {
		}

		return value;
	}

	bool BIXFConverter::isOnNodeIDTable(string v, unsigned char &id, int mode_flag) {
		if (mode_flag == LIBGENS_BIXF_MODE_PARTICLES) {
			for (size_t i=0; i<IDTableSize; i++) {
				if (IDTable[i] == v) {
					id = i;
					return true;
				}
			}
		}

		if (mode_flag == LIBGENS_BIXF_MODE_EVENT) {
			for (size_t i=0; i<IDTableEventSize; i++) {
				if (IDTableEvent[i] == v) {
					id = i;
					return true;
				}
			}
		}

		return false;
	}

	bool BIXFConverter::isOnValueIDTable(string v, unsigned char &id) {
		for (size_t i=0; i<ValueTableSize; i++) {
			if (ValueTable[i] == v) {
				id = i;
				return true;
			}
		}

		return false;
	}

	void BIXFConverter::convertToXML(string source, string dest, int mode_flag) {
		File file(source, LIBGENS_FILE_READ_BINARY);

		TiXmlDocument doc;
		TiXmlDeclaration *decl = new TiXmlDeclaration( "1.0", "", "" );
		doc.LinkEndChild( decl );

		if (file.valid()) {
			unsigned int first_section_size=0;
			file.goToAddress(8);
			file.readInt32(&first_section_size);

			unsigned int string_count=0;
			file.goToAddress(16);
			file.readInt32(&string_count);


			file.goToAddress(23+first_section_size);

			string *table=new string[string_count];

			for (size_t c=0; c<string_count; c++) {
				string entry="";
				file.readString(&entry);
				table[c] = entry;
			}

			file.goToAddress(20);

			TiXmlElement *parentElem=NULL;
			TiXmlElement *currentElem=NULL;
			string current_parameter="";

			//first_section_size = 26;

			for (size_t c=0; c<first_section_size; c++) {
				unsigned char byte=0;
				file.readUChar(&byte);
				
				if (byte==LIBGENS_BIXF_GO_TO_PARENT) {
					currentElem = parentElem;
					if (currentElem) {
						parentElem = (currentElem->Parent()? currentElem->Parent()->ToElement(): NULL);
					}
				}
				else if (byte==LIBGENS_BIXF_NEW_PARAMETER) {
					c++;
					file.readUChar(&byte);
					current_parameter = table[byte];
				}
				else if (byte==LIBGENS_BIXF_NEW_PARAMETER_TABLE) {
					c++;
					file.readUChar(&byte);
					current_parameter = nodeIDtoString(byte, mode_flag);
				}
				else if (byte==LIBGENS_BIXF_NEW_VALUE) {
					c++;
					file.readUChar(&byte);
					currentElem->SetAttribute(current_parameter, table[byte]);
				}
				else if (byte==LIBGENS_BIXF_NEW_VALUE_TABLE) {
					c++;
					file.readUChar(&byte);
					currentElem->SetAttribute(current_parameter, valueIDtoString(byte, mode_flag));
				}
				else if (byte==LIBGENS_BIXF_NEW_VALUE_BOOL) {
					c++;
					file.readUChar(&byte);
					currentElem->SetAttribute(current_parameter, (byte ? "true" : "false"));
				}
				else if (byte==LIBGENS_BIXF_NEW_VALUE_INT) {
					int value=0;
					file.readInt32(&value);
					currentElem->SetAttribute(current_parameter, ToString(value));

					c+=4;
				}
				else if (byte==LIBGENS_BIXF_NEW_VALUE_UINT) {
					unsigned int value=0;
					file.readInt32(&value);
					currentElem->SetAttribute(current_parameter, ToString(value));

					c+=4;
				}
				else if (byte==LIBGENS_BIXF_NEW_VALUE_FLOAT) {
					float value=0;
					file.readFloat32(&value);
					currentElem->SetAttribute(current_parameter, ToString(value));

					c+=4;
				}
				// New Node
				else if (byte==LIBGENS_BIXF_NEW_NODE) {
					c++;
					file.readUChar(&byte);

					parentElem = currentElem;
					currentElem = new TiXmlElement(table[byte]);
					if (parentElem) {
						parentElem->LinkEndChild(currentElem);
					}
					else doc.LinkEndChild(currentElem);
				}
				else if (byte==LIBGENS_BIXF_NEW_NODE_TABLE) {
					c++;
					file.readUChar(&byte);

					parentElem = currentElem;
					currentElem = new TiXmlElement(nodeIDtoString(byte, mode_flag));
					if (parentElem) {
						parentElem->LinkEndChild(currentElem);
					}
					else doc.LinkEndChild(currentElem);
				}
				else {
					printf("Stuck at %d\n", file.getCurrentAddress()-1);
					getchar();
				}
			}

			file.close();
		}

		doc.SaveFile(dest);
	}


	void BIXFConverter::convertToBIXF(TiXmlElement *pElem, vector<string> &string_table, vector<unsigned char> &data, int mode_flag) {
		// Add Node Declaration
		unsigned char table_id=0;

		if (isOnNodeIDTable(pElem->ValueStr(), table_id, mode_flag)) {
			data.push_back(LIBGENS_BIXF_NEW_NODE_TABLE);
			data.push_back(table_id);
		}
		else {
			data.push_back(LIBGENS_BIXF_NEW_NODE);
			data.push_back(createBIXFstring(pElem->ValueStr(), string_table));
		}
		
		// Query Attributes
		TiXmlAttribute* pAttrib=pElem->FirstAttribute();
		for (pAttrib; pAttrib; pAttrib=pAttrib->Next()) {
			if (isOnNodeIDTable(pAttrib->Name(), table_id, mode_flag)) {
				data.push_back(LIBGENS_BIXF_NEW_PARAMETER_TABLE);
				data.push_back(table_id);
			}
			else {
				data.push_back(LIBGENS_BIXF_NEW_PARAMETER);
				data.push_back(createBIXFstring(pAttrib->Name(), string_table));
			}


			if (isOnValueIDTable(pAttrib->Value(), table_id)) {
				data.push_back(LIBGENS_BIXF_NEW_VALUE_TABLE);
				data.push_back(table_id);
			}
			else {
				if (ToString(pAttrib->Value()) == "true") {
					data.push_back(LIBGENS_BIXF_NEW_VALUE_BOOL);
					data.push_back(1);
				}
				else if (ToString(pAttrib->Value()) == "false") {
					data.push_back(LIBGENS_BIXF_NEW_VALUE_BOOL);
					data.push_back(0);
				}
				else {
					data.push_back(LIBGENS_BIXF_NEW_VALUE);
					data.push_back(createBIXFstring(pAttrib->Value(), string_table));
				}
			}
		}

		// Traverse Children and call method recursively
		TiXmlElement* pElem_i;
		pElem_i=pElem->FirstChildElement();
		for(pElem_i; pElem_i; pElem_i=pElem_i->NextSiblingElement()) {
			convertToBIXF(pElem_i, string_table, data, mode_flag);
		}

		// Close this node and go to parent
		data.push_back(LIBGENS_BIXF_GO_TO_PARENT);
	}

	void BIXFConverter::convertToBIXF(string source, string dest, int mode_flag) {
		File file(dest, LIBGENS_FILE_WRITE_BINARY);
		vector<string> string_table;
		vector<unsigned char> data;

		
		if (file.valid()) {
			TiXmlDocument doc(source);
			doc.LoadFile();

			TiXmlHandle hDoc(&doc);
			TiXmlElement* pElem;
			TiXmlHandle hRoot(0);

			pElem=hDoc.FirstChildElement().Element();
			for(pElem; pElem; pElem=pElem->NextSiblingElement()) {
				convertToBIXF(pElem, string_table, data, mode_flag);
			}

			// Write Binary Data
			unsigned char header=0x01;
			file.writeString("BIXF");
			file.writeUChar(&header);
			file.fixPadding(20);


			unsigned int total_data_size=data.size();
			unsigned int total_string_size=0;
			unsigned int total_string_count=string_table.size();


			for (size_t c=0; c<data.size(); c++) {
				file.writeUChar(&(data[c]));
			}

			// Apparently it needs three zeros at the end of the data
			unsigned char zero=0;
			file.writeUChar(&zero);
			file.writeUChar(&zero);
			file.writeUChar(&zero);
			total_string_size += 3;

			for (size_t c=0; c<string_table.size(); c++) {
				file.writeString(&(string_table[c]));
				total_string_size += string_table[c].size() + 1;
			}

			file.goToAddress(8);
			file.writeInt32(&total_data_size);
			file.writeInt32(&total_string_size);
			file.writeInt32(&total_string_count);

			file.close();
		}
	}


	unsigned char BIXFConverter::createBIXFstring(string value, vector<string> &string_table) {
		for (size_t c=0; c<string_table.size(); c++) {
			if (string_table[c] == value) {
				return c;
			}
		}

		string_table.push_back(value);
		return string_table.size()-1;
	}
};