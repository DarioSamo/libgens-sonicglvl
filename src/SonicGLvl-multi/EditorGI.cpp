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

#include "stdafx.h"
#include "EditorGI.h"

void EditorGIListener::notifyRenderSingleObject(Ogre::Renderable *rend, const Ogre::Pass *pass, const Ogre::AutoParamDataSource *source, const Ogre::LightList *pLightList, bool suppressRenderStateChanges) {
	Ogre::Any any_ptr=rend->getUserObjectBindings().getUserAny();

	if (!any_ptr.isEmpty()) {
		EditorGIParameter *parameter=Ogre::any_cast<EditorGIParameter *>(any_ptr);

		if (parameter) {
			Ogre::Pass *edit_pass=const_cast<Ogre::Pass *>(pass);

			if (edit_pass) {
				if (edit_pass->hasVertexProgram()) {
					Ogre::GpuProgramParametersSharedPtr vp_parameters = edit_pass->getVertexProgramParameters();
					
					Ogre::TextureUnitState *state=edit_pass->getTextureUnitState(10);
					state->setTexture(parameter->texture_ptr);
					Ogre::Root::getSingleton().getRenderSystem()->_setTextureUnitSettings(10, *state);
					Ogre::Root::getSingleton().getRenderSystem()->bindGpuProgramParameters(Ogre::GPT_VERTEX_PROGRAM, vp_parameters, Ogre::GPV_ALL);
				}
			}
		}
	}
}


EditorGIParameter::EditorGIParameter(Ogre::TexturePtr ptr) {
	texture_ptr = ptr;
}

void EditorGIVisitor::visit(Ogre::Renderable *rend, Ogre::ushort lodIndex, bool isDebug, Ogre::Any *pAny) {
	rend->getUserObjectBindings().setUserAny(Ogre::Any(parameter));
}