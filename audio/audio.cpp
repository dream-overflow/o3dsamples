/**
 * @file audio.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2004-01-01
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include <o3d/core/memorymanager.h>
#include <o3d/core/keyboard.h>
#include <o3d/core/appwindow.h>
#include <o3d/engine/utils/framemanager.h>
#include <o3d/engine/context.h>
#include <o3d/engine/hierarchy/node.h>
#include <o3d/engine/hierarchy/hierarchytree.h>
#include <o3d/engine/screenviewport.h>
#include <o3d/engine/viewportmanager.h>
#include <o3d/engine/scene/scene.h>
#include <o3d/engine/scene/sceneobjectmanager.h>

#include <o3d/audio/sndlistener.h>
#include <o3d/audio/sndsource.h>
#include <o3d/audio/sndbuffermanager.h>
#include <o3d/audio/audio.h>
#include <o3d/audio/audiorenderer.h>

#include <o3d/engine/object/primitive.h>
#include <o3d/engine/object/ftransform.h>
#include <o3d/engine/object/mtransform.h>
#include <o3d/engine/object/camera.h>
#include <o3d/engine/renderer.h>

#include <o3d/core/diskfileinfo.h>
#include <o3d/core/main.h>

using namespace o3d;

/**
 * @brief The SoundSample class. Main entry of the sound sample.
 * @date 2004-01-01
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 */
class SoundSample : public EvtHandler
{
private:

    AppWindow *m_appWindow;

    Renderer* m_glRenderer;
    AudioRenderer* m_alRenderer;

    Scene *m_scene;
    Audio *m_audio;

public:

    SoundSample()
	{
        m_appWindow = new AppWindow;

        // OpenGL renderer
        m_glRenderer = new Renderer;

        // OpenAL renderer
        m_alRenderer = new AudioRenderer;

        m_appWindow->setTitle("Objective-3D Audio sample");
        m_appWindow->create(800, 600, AppWindow::COLOR_RGBA8, AppWindow::DEPTH_24_STENCIL_8, AppWindow::NO_MSAA, False, False);

        m_glRenderer->create(m_appWindow);

        // create a scene and attach it to the window
        m_scene = new Scene(nullptr, "../media", m_glRenderer);
        m_scene->setSceneName("audio");
        m_scene->defaultAttachment(m_appWindow);

        // define our audio controller
        m_audio = new Audio(m_scene, "../media", m_alRenderer);
        m_scene->setAudio(m_audio);

		// We listen synchronously to each update event coming from the main window.
		// The first parameter is an helper macro that take :
		// - Object class name to listen
		// - Instance of the object to listen
		// - Name of the signal to listen
		// - Instance of the receiver (in our case this)
		// - Method called on an event
        m_appWindow->onUpdate.connect(this, &SoundSample::onSceneUpdate);
        m_appWindow->onKey.connect(this, &SoundSample::onKey);
        m_appWindow->onDestroy.connect(this, &SoundSample::onDestroy);

		// Notice that update and draw events of the window are
		// thrown by two timers. And that it is possible to change easily these timings.
	}

	virtual ~SoundSample()
    {
	}

    AppWindow* getWindow() { return m_appWindow; }

    Scene* getScene() { return m_scene; }

    Audio* getAudio() { return m_audio; }

    void onDestroy()
    {
        deletePtr(m_scene);
        m_audio = nullptr;

        deletePtr(m_glRenderer);
        deletePtr(m_alRenderer);

        this->getWindow()->logFps();

        // it is deleted by the application
        m_appWindow = nullptr;
    }

	// Method called on main window update
	void onSceneUpdate()
	{
		// Get the keyboard object from the input manager of the main window
        Keyboard * lpKeyboard = getWindow()->getInput().getKeyboard();

		// Get the time (in ms) elapsed since the last update
        Float elapsed = getScene()->getFrameManager()->getFrameDuration();
        Float angle = 0.f;
        Float x = 0.f, z = 0.f;

		// Inc/dec a rotation angle depending of the left and right key states.
		// The angle speed stay the same by using the elapsed time as factor.
        if (lpKeyboard->isKeyDown(KEY_LEFT)) angle = 1.f*elapsed;
        if (lpKeyboard->isKeyDown(KEY_RIGHT)) angle = -1.f*elapsed;

        if (lpKeyboard->isKeyDown(KEY_E)) z = -5.f*elapsed;
        if (lpKeyboard->isKeyDown(KEY_D)) z = 5.f*elapsed;

        if (lpKeyboard->isKeyDown(KEY_S)) x = -5.f*elapsed;
        if (lpKeyboard->isKeyDown(KEY_F)) x = 5.f*elapsed;

		// Search into the scene object manager our node that containing the camera.
        SceneObject *camera = getScene()->getSceneObjectManager()->searchName("Camera");
		BaseNode *node = camera->getNode();

		// Rotate on the Y axis
		if (angle != 0.f)
			node->getTransform()->rotate(Y, angle*3.f);

		if (x != 0.f || z != 0.f)
			node->getTransform()->translate(Vector3(x, 0.f, z));
	}

    void onKey(Keyboard* keyboard, KeyEvent event)
	{
		if (event.isPressed() && (event.key() ==  KEY_ESCAPE))
            getWindow()->terminate();
	}

    static Int32 main()
	{
        // cleared log out file with new header
        Debug::instance()->setDefaultLog("audio.log");
        Debug::instance()->getDefaultLog().clearLog();
        Debug::instance()->getDefaultLog().writeHeaderLog();

		// We want to log memory allocation higher than 128 bytes.
		MemoryManager::instance()->enableLog(MemoryManager::MEM_RAM,128);
		// And we want to log too, any allocation onto the VRAM, such as texture creation
		// or deletion, vbo, fbo.
		MemoryManager::instance()->enableLog(MemoryManager::MEM_GFX);
		// And log sound allocation
		MemoryManager::instance()->enableLog(MemoryManager::MEM_SFX);

		// Our application object
        SoundSample *myApp = new SoundSample;

		// Create a camera into the scene. You notice that we always need a parent
		// to build an object. We simply set directly the scene as its parent.
		// We can too add our camera into the getSceneObjectManager() of the scene,
		// but it is useless in this example.
        Camera *lpCamera = new Camera(myApp->getScene());

		// A viewport is a part of the screen. Each viewport might be attached to a camera.
		// We can overload the drawing callback, and define a displaying priority.
		// Because the viewport manager provide the method to create and add a viewport
		// we don't need to set its parent. In this case its parent is the viewport manager.
        ScreenViewPort *pViewPort = myApp->getScene()->getViewPortManager()->addScreenViewPort(lpCamera,0,0);

		// Set a unique name to our camera. It should be unique to retrieve it by its name
		// into the hierarchy tree or using the scene object manager.
		lpCamera->setName("Camera");

		// Define Z clipping plane
		lpCamera->setZnear(0.25f);
		lpCamera->setZfar(10000.0f);

		// Compute the projection matrix of the camera as a perspective projection.
		lpCamera->computePerspective();
		// We don't want to see it
		lpCamera->disableVisibility();

		// Create a new node to contain our new camera
        Node *cameraNode = myApp->getScene()->getHierarchyTree()->addNode(lpCamera);
		// We also need a first view person transformation on this node
		FTransform *ltransfrom = new FTransform;
		cameraNode->addTransform(ltransfrom);

		// Initial position at origin {x,y,z} with Y+ up screen and Z+ comes to you
		ltransfrom->translate(Vector3(0.f,-0.1f,0.f));

		// Use this line to enable asynchronous sound loading
        myApp->getAudio()->getBufferManager()->enableAsynchronous();

		// Insert a 3d sound source
        if (myApp->getAudio()->getBufferManager()->isResourceExists("walk.wav"))
		{
			// Set 5 seconds (samples duration is 3 seconds) of decodeMaxDuration because we want this sample fully decoded (not streamed).
            SndBuffer *sndBuffer = myApp->getAudio()->getBufferManager()->addSndBuffer("walk.wav", 5.f);
            OmniSource *sndSource = new OmniSource(myApp->getScene());

			// play only a single buffer
			sndSource->setUniqueBuffer(sndBuffer);

			// in loop
			sndSource->enableLooping();

			// auto play the source when ready
            sndSource->setAutoPlay(True);

			// add it to the scene root
            Node *sndSourceNode = myApp->getScene()->getHierarchyTree()->addNode(sndSource);
		}
		else
		{
			O3D_WARNING("missing media/sounds/walk.wav");
		}

		// Insert an ambient sound source (like a music)
        if (myApp->getAudio()->getBufferManager()->isResourceExists("music.ogg"))
		{
			// This sample will be streamed because it is greater than the default decodeMaxDuration of 2 seconds.
            SndBuffer *sndBuffer = myApp->getAudio()->getBufferManager()->addSndBuffer("music.ogg");
            OmniSource *music = new OmniSource(myApp->getScene());

			// play only a single buffer
			music->setUniqueBuffer(sndBuffer);

			// always relative to the listener, position and orientation never affect it
            music->setRelative(True);

			// auto play the source when ready
            music->setAutoPlay(True);

			// in loop
			music->enableLooping();

			// add it to the scene root
            Node *sndSourceNode = myApp->getScene()->getHierarchyTree()->addNode(music);
		}
		else
		{
			O3D_WARNING("missing media/sounds/music.ogg");
		}

		// Setup a sound listener in the same node as the camera
        SndListener *listener = new SndListener(myApp->getScene());
		cameraNode->addSonLast(listener);

		// and set it as current active listener
        myApp->getAudio()->setActiveListener(listener);

		// Change the clear color
        myApp->getScene()->getContext()->setBackgroundColor(0.633f,0.792f,.914f,0.0f);

        myApp->getScene()->setDrawObject(Scene::DRAW_SND_SOURCE_OMNI, True);
        myApp->getScene()->setDrawObject(Scene::DRAW_LOCAL_AXIS, True);
        myApp->getScene()->setDrawObject(Scene::DRAW_BOUNDING_VOLUME, True);
        myApp->getScene()->setDrawObject(Scene::DRAW_SND_LISTENER, True);

        // Run the event loop
        Application::run();

		// Destroy any content
        deletePtr(myApp);

        // write a footer banner in log out file
        Debug::instance()->getDefaultLog().writeFooterLog();

		return 0;
	}
};

O3D_CONSOLE_MAIN(SoundSample, O3D_DEFAULT_CLASS_SETTINGS)

