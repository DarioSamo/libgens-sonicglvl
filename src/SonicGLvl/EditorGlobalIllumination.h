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

#include "Common.h"

#ifndef EDITOR_GLOBAL_ILLUMINATION_H_INCLUDED
#define EDITOR_GLOBAL_ILLUMINATION_H_INCLUDED

class GlobalIlluminationListener : public Ogre::RenderObjectListener {
	protected:
		Ogre::Pass *pass_to_ignore;
	public:
		GlobalIlluminationListener() {
			pass_to_ignore = NULL;
		}

		void notifyRenderSingleObject(Ogre::Renderable *rend, const Ogre::Pass *pass, const Ogre::AutoParamDataSource *source, const Ogre::LightList *pLightList, bool suppressRenderStateChanges);

		void setPassToIgnore(Ogre::Pass *pass) {
			pass_to_ignore = pass;
		}
};


class GlobalIlluminationParameter {
	public:
		LibGens::GISubtexture *gi_texture;
		Ogre::TexturePtr texture_ptr;

		GlobalIlluminationParameter(LibGens::GISubtexture *gi);
};

class GlobalIlluminationAssignVisitor : public Ogre::Renderable::Visitor {
	protected:
		GlobalIlluminationParameter *parameter;
	public:
		GlobalIlluminationAssignVisitor(GlobalIlluminationParameter *p) {
			parameter = p;
		}

		void visit(Ogre::Renderable *rend, Ogre::ushort lodIndex, bool isDebug, Ogre::Any *pAny = 0);
};

#endif