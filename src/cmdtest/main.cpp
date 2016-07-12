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

#include "LibGens.h"
#include "Light.h"
#include "GITextureGroup.h"
#include "FreeImage.h"

int main(int argc, char** argv) {
	LibGens::initialize();
	LibGens::Error::setLogging(true);
	FreeImage_Initialise();

	LibGens::GITextureGroup group;
	LibGens::File file("gia-296/atlasinfo", LIBGENS_FILE_READ_BINARY);
	group.readAtlasinfo(&file);
	list<LibGens::GITexture *> gi_textures = group.getTextures();

	for (list<LibGens::GITexture *>::iterator it = gi_textures.begin(); it != gi_textures.end(); it++) {
		string texture_name = (*it)->getName();
		string texture_filename = "gia-296/" + texture_name + ".dds";

		// Use FreeImage to convert the DDS to a png properly, since Qt doesn't load the alpha channels like it should.
		FIBITMAP *bitmap = NULL;
		FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(texture_filename.c_str());
		if (fif == FIF_UNKNOWN)
			fif = FreeImage_GetFIFFromFilename(texture_filename.c_str());

		if (fif != FIF_UNKNOWN) {
			bitmap = FreeImage_Load(fif, texture_filename.c_str());
		}

		// Load the converter texture and use it to extract information from there.
		if (bitmap) {
			unsigned int texture_width = FreeImage_GetWidth(bitmap);
			unsigned int texture_height = FreeImage_GetHeight(bitmap);
			(*it)->setWidth(texture_width);
			(*it)->setHeight(texture_height);

			list<LibGens::GISubtexture *> subtextures = (*it)->getSubtextures();
			for (list<LibGens::GISubtexture *>::iterator it2 = subtextures.begin(); it2 != subtextures.end(); it2++) {
				unsigned int subtexture_width = texture_width * (*it2)->getWidth();
				unsigned int subtexture_height = texture_height * (*it2)->getHeight();

				LibGens::GISubtexture *clone_subtexture = new LibGens::GISubtexture();
				clone_subtexture->setPixelWidth(subtexture_width);
				clone_subtexture->setPixelHeight(subtexture_height);
				clone_subtexture->setName((*it2)->getName());
				group.addSubtextureToOrganize(clone_subtexture);
			}

			FreeImage_Unload(bitmap);
		}
	}

	group.deleteTextures();
	group.organizeSubtextures(2048);
	gi_textures = group.getTextures();
	printf("Generated %d textures\n", gi_textures.size());
	for (list<LibGens::GITexture *>::iterator it = gi_textures.begin(); it != gi_textures.end(); it++) {
		printf("%s: %d %d\n", (*it)->getName().c_str(), (*it)->getWidth(), (*it)->getHeight());

		list<LibGens::GISubtexture *> gi_subtextures = (*it)->getSubtextures();
		for (list<LibGens::GISubtexture *>::iterator it2 = gi_subtextures.begin(); it2 != gi_subtextures.end(); it2++) {
			printf("%s: %f %f %f %f %d %d\n", (*it2)->getName().c_str(), (*it2)->getX(), (*it2)->getY(), (*it2)->getWidth(), (*it2)->getHeight(), (*it2)->getPixelWidth(), (*it2)->getPixelHeight());
		}
	}

	getchar();

    return 0;
}