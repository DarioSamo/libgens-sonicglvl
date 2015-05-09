//============================================================================================================================================
// Copied/Modified from the Ogre3D Wiki: http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Ogre+Wiki+Tutorial+Framework&structure=Development
//============================================================================================================================================

#include "Common.h"

#ifndef BASE_APPLICATION_H_INCLUDED
#define BASE_APPLICATION_H_INCLUDED

#define SONICGLVL_WINDOW_NAME         "SonicGLvl v0.9.0"
#define SONICGLVL_LOG_NAME            "sonicglvl.log"
#define SONICGLVL_PLUGINS_NAME        "plugins.cfg"
#define SONICGLVL_RESOURCES_NAME      "resources.cfg"

class BaseApplication : public Ogre::FrameListener, public Ogre::WindowEventListener, public OIS::KeyListener, public OIS::MouseListener, OgreBites::SdkTrayListener
{
public:
    BaseApplication(void);
    virtual ~BaseApplication(void);

    virtual void go(void);

	HWND getHwnd() {
		return hwnd;
	}
protected:
	Ogre::Root *root;
    Ogre::SceneManager* scene_manager;
    Ogre::RenderWindow* window;
    Ogre::String resources_config;
    Ogre::String plugin_config;
    bool shut_down;
    OIS::InputManager* input_manager;
    OIS::Mouse*    mouse;
    OIS::Keyboard* keyboard;
	HWND hwnd;
	string exe_path;
	int screen_width;
	int screen_height;

    virtual bool setup();
    virtual bool configure(void);
    virtual void createFrameListener(void);
    virtual void createScene(void) = 0;
    virtual void destroyScene(void);
    virtual void setupResources(void);
    virtual void createResourceListener(void);
    virtual void loadResources(void);
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
    virtual bool keyPressed(const OIS::KeyEvent &arg) = 0;
    virtual bool keyReleased(const OIS::KeyEvent &arg) = 0;
    virtual bool mouseMoved(const OIS::MouseEvent &arg) = 0;
    virtual bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id) = 0;
    virtual bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id) = 0;
    virtual void windowResized(Ogre::RenderWindow* rw);
    virtual void windowClosed(Ogre::RenderWindow* rw);
	virtual bool renderOneFrame();

};

LRESULT APIENTRY SubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif