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

#include "EditorApplication.h"


void EditorApplication::copySelection() {
	if(OpenClipboard(hwnd)) {
		// Create and populate XML
		TiXmlDocument doc;

		TiXmlElement *root=new TiXmlElement(LIBGENS_OBJECT_SET_ROOT);

		// Retrieve Object pointers from Object Nodes
		for (list<EditorNode *>::iterator it=selected_nodes.begin(); it!=selected_nodes.end(); it++) {
			if ((*it)->getType() == EDITOR_NODE_OBJECT) {
				ObjectNode *object_node = static_cast<ObjectNode *>(*it);
				LibGens::Object *object = object_node->getObject();

				if (object) {
					object->writeXML(root);
				}
			}
		}

		doc.LinkEndChild(root);

		// Set up XML Printer
		TiXmlPrinter printer;
		printer.SetIndent("  ");
		doc.Accept(&printer);

		// Clipboard Routine
		HGLOBAL clipbuffer;
		char * buffer;
		EmptyClipboard();
		clipbuffer = GlobalAlloc(GMEM_ZEROINIT, printer.Size()+1);
		buffer = (char*)GlobalLock(clipbuffer);
		strcpy(buffer, printer.CStr());
		GlobalUnlock(clipbuffer);
		SetClipboardData(CF_TEXT, clipbuffer);
		CloseClipboard();
	}
}

void EditorApplication::pasteSelection() {
	char *buffer;
	if(OpenClipboard(hwnd)) {
		buffer = (char*)GetClipboardData(CF_TEXT);

		TiXmlDocument doc;
		doc.Parse((const char*) buffer, 0, TIXML_ENCODING_UTF8);

		TiXmlHandle hDoc(&doc);
		TiXmlElement* pElem;
		TiXmlHandle hRoot(0);

		pElem=hDoc.FirstChildElement().Element();
		if (!pElem) {
			return;
		}

		// Check if a root node named "SetObject" exists, and skip to its child root if it does
		if (pElem->ValueStr() == LIBGENS_OBJECT_SET_ROOT) {
			pElem=pElem->FirstChildElement();
		}
		
		list<LibGens::Object *> paste_objects;
		for(pElem; pElem; pElem=pElem->NextSiblingElement()) {
			if (pElem->ValueStr() != LIBGENS_OBJECT_SET_LAYER_DEFINE) {
				LibGens::Object *obj=new LibGens::Object(pElem->ValueStr());

				obj->readXML(pElem);
				obj->learnFromLibrary(library);

				paste_objects.push_back(obj);
			}
		}

		overrideObjectsPalettePreview(paste_objects);
	}

	CloseClipboard(); 
}

void EditorApplication::openLevelGUI() {
	char *filename = (char *) malloc(1024);
	strcpy(filename, "");

	OPENFILENAME    ofn;
    memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize     = sizeof(ofn);
	ofn.lpstrFilter     = "Level File(#level.ar.00)\0#*.ar.00\0";
	ofn.nFilterIndex    = 1;
	ofn.lpstrFile       = filename;
    ofn.nMaxFile        = 1024;
    ofn.lpstrTitle      = "Choose the Level Data file";
    ofn.Flags           = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST |
                          OFN_LONGNAMES     | OFN_EXPLORER |
                          OFN_HIDEREADONLY  | OFN_ENABLESIZING;

    if(GetOpenFileName(&ofn)) {
		chdir(exe_path.c_str());
		openLevel(ToString(filename));
	}

	chdir(exe_path.c_str());
    free(filename);
}


void EditorApplication::openLostWorldLevelGUI() {
	char *directory = (char *) malloc(1024);
	strcpy(directory, "enter in the directory and press Save");

	OPENFILENAME    ofdn;
	memset(&ofdn, 0, sizeof(ofdn));
	ofdn.lStructSize     = sizeof(ofdn);
	ofdn.lpstrFilter     = "*\0";
	ofdn.nFilterIndex    = 1;
	ofdn.lpstrFile       = directory;
	ofdn.nMaxFile        = 1024;
	ofdn.lpstrTitle      = "Choose the Sonic Lost World level Directory";
	ofdn.Flags           = OFN_PATHMUSTEXIST |
						OFN_LONGNAMES     | OFN_EXPLORER |
						OFN_HIDEREADONLY  | OFN_ENABLESIZING;

	if (GetSaveFileName(&ofdn)) {
		int last_slash=0;
        for (size_t i=0; i<strlen(directory); i++) {
			if (directory[i] == '\\') last_slash = i;
        }
        directory[last_slash] = '\0';
		chdir(exe_path.c_str());

		openLostWorldLevel(ToString(directory));
	}
	free(directory);

	chdir(exe_path.c_str());
}

void EditorApplication::exportSceneFBXGUI() {
	char *filename = (char *) malloc(1024);
	strcpy(filename, "");

	OPENFILENAME    ofn;
    memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize     = sizeof(ofn);
	ofn.lpstrFilter     = "FBX File(*.fbx)\0*.fbx\0";
	ofn.nFilterIndex    = 1;
	ofn.lpstrFile       = filename;
    ofn.nMaxFile        = 1024;
    ofn.lpstrTitle      = "Save FBX file to...";
    ofn.Flags           = OFN_PATHMUSTEXIST |
                          OFN_LONGNAMES     | OFN_EXPLORER |
                          OFN_HIDEREADONLY  | OFN_ENABLESIZING;

    if(GetOpenFileName(&ofn)) {
		chdir(exe_path.c_str());
		exportSceneFBX(ToString(filename));
	}

	chdir(exe_path.c_str());
    free(filename);
}

void EditorApplication::importLevelTerrainFBXGUI() {
	char *filename = (char *) malloc(1024);
	strcpy(filename, "");

	OPENFILENAME    ofn;
    memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize     = sizeof(ofn);
	ofn.lpstrFilter     = "FBX(.fbx)\0*.fbx\0";
	ofn.nFilterIndex    = 1;
	ofn.lpstrFile       = filename;
    ofn.nMaxFile        = 1024;
    ofn.lpstrTitle      = "Choose the FBX file";
    ofn.Flags           = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST |
                          OFN_LONGNAMES     | OFN_EXPLORER |
                          OFN_HIDEREADONLY  | OFN_ENABLESIZING;

    if(GetOpenFileName(&ofn)) {
		chdir(exe_path.c_str());
		importLevelTerrainFBX(ToString(filename));
	}

	chdir(exe_path.c_str());
    free(filename);
}

void EditorApplication::saveLevelDataGUI() {
	if (current_level) saveLevelData(current_level_filename);
}

void EditorApplication::saveLevelTerrainGUI() {
	if (terrain_streamer) {
		if (CONFIRM_MSG("To do any terrain operations you have to wait for the terrain streamer to finish first. Do you want load all the terrain?") == IDYES) {
			terrain_streamer->forceLoad();
		}
		return;
	}

	if (current_level) saveLevelTerrain();
}