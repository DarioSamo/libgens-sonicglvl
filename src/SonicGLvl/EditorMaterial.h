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

#ifndef EDITOR_MATERIAL_H_INCLUDED
#define EDITOR_MATERIAL_H_INCLUDED

class MaterialEditorPreviewListener : public Ogre::FrameListener, public Ogre::WindowEventListener, public OIS::KeyListener, public OIS::MouseListener {
	protected:
		EditorViewport *editor_viewport;
		Ogre::RenderWindow *mWindow;
		OIS::Mouse *mouse;
		OIS::Keyboard *keyboard;
		Ogre::AnimationState *animation_state;
	public:
		MaterialEditorPreviewListener() {
			animation_state = NULL;
		}

		void setEditorViewport(EditorViewport *v) {
			editor_viewport = v;
		}

		void setEditorWindow(Ogre::RenderWindow *v) {
			mWindow = v;
		}

		void setMouse(OIS::Mouse *v) {
			mouse = v;
		}

		void setKeyboard(OIS::Keyboard *v) {
			keyboard = v;
		}

		void setAnimationState(Ogre::AnimationState *v) {
			animation_state = v;
		}

		bool frameRenderingQueued(const Ogre::FrameEvent& evt) {
			keyboard->capture();
			mouse->capture();

			editor_viewport->update();

			if (animation_state) {
				animation_state->addTime(evt.timeSinceLastFrame);
			}

			return true;
		}

		

		bool keyPressed(const OIS::KeyEvent &arg) {
			editor_viewport->keyPressed(arg);
			return true;
		}

		bool keyReleased(const OIS::KeyEvent &arg) {
			editor_viewport->keyReleased(arg);
			return true;
		}

		bool mouseMoved(const OIS::MouseEvent &arg) {
			editor_viewport->mouseMoved(arg);
			return true;
		}

		bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id) {
			editor_viewport->mousePressed(arg, id);
			return true;
		}

		bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id) {
			editor_viewport->mouseReleased(arg, id);
			return true;
		}

		void windowResized(Ogre::RenderWindow* rw) {
			unsigned int width, height, depth;
			int left, top;
			rw->getMetrics(width, height, depth, left, top);

			const OIS::MouseState &ms = mouse->getMouseState();
			ms.width = width;
			ms.height = height;


			editor_viewport->resize(0, 0, 1, 1);
		}

		void windowClosed(Ogre::RenderWindow* rw) {
		}
};

#endif