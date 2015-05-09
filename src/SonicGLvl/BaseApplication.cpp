//============================================================================================================================================
// Copied/Modified from the Ogre3D Wiki: http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Ogre+Wiki+Tutorial+Framework&structure=Development
//============================================================================================================================================

#include "BaseApplication.h"

WNDPROC globalWinProc = NULL;

BaseApplication::BaseApplication(void) : root(0), scene_manager(0), window(0), resources_config(Ogre::StringUtil::BLANK), 
	plugin_config(Ogre::StringUtil::BLANK), shut_down(false), input_manager(0), mouse(0), keyboard(0) {

}

BaseApplication::~BaseApplication(void) {
    Ogre::WindowEventUtilities::removeWindowEventListener(window, this);
    windowClosed(window);
    delete root;
}

bool BaseApplication::configure(void) {
    if(root->showConfigDialog()) {
        window = root->initialise(true, SONICGLVL_WINDOW_NAME);
        return true;
    }
    else {
        return false;
    }
}

void BaseApplication::createFrameListener(void) {
    OIS::ParamList pl;
    size_t windowHnd = 0;
    std::ostringstream windowHndStr;
    window->getCustomAttribute("WINDOW", &windowHnd);
    windowHndStr << windowHnd;
    pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
	pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND" )));
    pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
    pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
    pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")));
    input_manager = OIS::InputManager::createInputSystem( pl );
    keyboard = static_cast<OIS::Keyboard*>(input_manager->createInputObject(OIS::OISKeyboard, true));
    mouse = static_cast<OIS::Mouse*>(input_manager->createInputObject(OIS::OISMouse, true));
    mouse->setEventCallback(this);
    keyboard->setEventCallback(this);
    windowResized(window);
    Ogre::WindowEventUtilities::addWindowEventListener(window, this);
    root->addFrameListener(this);
}

void BaseApplication::destroyScene(void) {
}

void BaseApplication::setupResources(void) {
    Ogre::ConfigFile cf;
    cf.load(resources_config);

    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements()) {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i) {
            typeName = i->first;
            archName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName);
        }
    }
}

void BaseApplication::createResourceListener(void) {
}

void BaseApplication::loadResources(void) {
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

void BaseApplication::go(void) {
    resources_config = SONICGLVL_RESOURCES_NAME;
    plugin_config   = SONICGLVL_PLUGINS_NAME;

	char cCurrentPath[1024];
    _getcwd(cCurrentPath, sizeof(cCurrentPath));
    exe_path = ToString(cCurrentPath);

    if (!setup()) return;

	LibGens::initialize();
	LibGens::Error::setLogging(true);
	window->setDeactivateOnFocusChange(false);

    LONG iconID   = (LONG)LoadIcon( GetModuleHandle(0), MAKEINTRESOURCE(SONICGLVL_ICON) );
    SetClassLong( hwnd, GCL_HICON, iconID );

	while(true) {
		MSG  msg;
		while(PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE )) {
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
 
		if(window->isClosed()) break;
		if(!renderOneFrame()) break;
	}
    destroyScene();
}

bool BaseApplication::renderOneFrame() {
	return root->renderOneFrame();
}

bool BaseApplication::setup(void) {
	Ogre::LogManager * lm = new Ogre::LogManager();
    lm->createLog(SONICGLVL_LOG_NAME, true, false, false);

    root = new Ogre::Root(plugin_config);

    setupResources();

    bool carryOn = configure();
    if (!carryOn) return false;

    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
	Ogre::Animation::setDefaultInterpolationMode(Ogre::Animation::IM_LINEAR);
    Ogre::Animation::setDefaultRotationInterpolationMode(Ogre::Animation::RIM_LINEAR);

    createResourceListener();
    loadResources();
	window->getCustomAttribute("WINDOW", (void*) &hwnd);
	globalWinProc = (WNDPROC) SetWindowLong(hwnd, GWL_WNDPROC, (LONG) SubclassProc);

    createScene();
    createFrameListener();
    return true;
};


bool BaseApplication::frameRenderingQueued(const Ogre::FrameEvent& evt) {
    if(window->isClosed()) return false;
    if(shut_down) return false;

    keyboard->capture();
    mouse->capture();
    return true;
}

void BaseApplication::windowResized(Ogre::RenderWindow* rw) {
    unsigned int width, height, depth;
    int left, top;
    rw->getMetrics(width, height, depth, left, top);

    const OIS::MouseState &ms = mouse->getMouseState();
    ms.width = width;
    ms.height = height;

	screen_width = width;
	screen_height = height;
}

void BaseApplication::windowClosed(Ogre::RenderWindow* rw) {
    if(rw == window) {
        if(input_manager) {
            input_manager->destroyInputObject(mouse);
            input_manager->destroyInputObject(keyboard);

            OIS::InputManager::destroyInputSystem(input_manager);
            input_manager = 0;
        }
	}
}
