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

#include "EditorApplication.h"
#include <d3d9.h>

namespace Ogre {
	class __declspec(dllimport) D3D9RenderSystem {
		public:
			static IDirect3DDevice9 *getActiveD3D9Device();
	};

	class __declspec(dllimport) D3D9Texture {
		public:
			IDirect3DBaseTexture9 *getTexture();
	};
}

void GlobalIlluminationListener::notifyRenderSingleObject(Ogre::Renderable *rend, const Ogre::Pass *pass, const Ogre::AutoParamDataSource *source, const Ogre::LightList *pLightList, bool suppressRenderStateChanges) {
	if (pass_to_ignore == pass) return;

	const Ogre::Any& any_ptr=rend->getUserObjectBindings().getUserAny();

	if (!any_ptr.isEmpty() && any_ptr.getType() == typeid(GlobalIlluminationParameter *)) {
		GlobalIlluminationParameter *parameter = any_ptr.get<GlobalIlluminationParameter *>();
		Ogre::D3D9RenderSystem::getActiveD3D9Device()->SetTexture(10, reinterpret_cast<Ogre::D3D9Texture *>(parameter->texture_ptr.get())->getTexture());
	}
}


GlobalIlluminationParameter::GlobalIlluminationParameter(LibGens::GISubtexture *gi) {
	gi_texture = gi;
	texture_ptr = Ogre::TextureManager::getSingletonPtr()->getByName(gi_texture->getPath());

	if (texture_ptr.isNull()) {
		texture_ptr = Ogre::TextureManager::getSingletonPtr()->load(gi_texture->getPath(), GENERAL_MESH_GROUP);
	}
}


void GlobalIlluminationAssignVisitor::visit(Ogre::Renderable *rend, Ogre::ushort lodIndex, bool isDebug, Ogre::Any *pAny) {
	rend->getUserObjectBindings().setUserAny(Ogre::Any(parameter));
	rend->setCustomParameter(0, Ogre::Vector4(parameter->gi_texture->getWidth(), parameter->gi_texture->getHeight(), parameter->gi_texture->getX(), parameter->gi_texture->getY()));
}