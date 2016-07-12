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

class EditorGIListener : public Ogre::RenderObjectListener {
	public:
		EditorGIListener() {
		}

		void notifyRenderSingleObject(Ogre::Renderable *rend, const Ogre::Pass *pass, const Ogre::AutoParamDataSource *source, const Ogre::LightList *pLightList, bool suppressRenderStateChanges);
};


class EditorGIParameter {
	public:
		Ogre::TexturePtr texture_ptr;
		EditorGIParameter(Ogre::TexturePtr ptr);
};

class EditorGIVisitor : public Ogre::Renderable::Visitor {
	protected:
		EditorGIParameter *parameter;
	public:
		EditorGIVisitor(EditorGIParameter *p) {
			parameter = p;
		}

		void visit(Ogre::Renderable *rend, Ogre::ushort lodIndex, bool isDebug, Ogre::Any *pAny = 0);
};
