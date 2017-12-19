/**
 * @file window.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2004-01-01
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include <o3d/core/appwindow.h>

#include <o3d/core/dir.h>
#include <o3d/core/file.h>
#include <o3d/image/perlinnoise2d.h>

#include <o3d/engine/context.h>
#include <o3d/engine/scene/scene.h>
#include <o3d/engine/object/camera.h>
#include <o3d/engine/object/mtransform.h>
#include <o3d/engine/hierarchy/hierarchytree.h>
#include <o3d/engine/object/primitivemanager.h>
#include <o3d/engine/viewport.h>
#include <o3d/engine/renderer.h>
#include <o3d/core/display.h>
#include <o3d/core/filemanager.h>
#include <o3d/core/main.h>

using namespace o3d;

/**
 * @brief The SoundSample class.
 * @date 2004-01-01
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 */
class Window : public EvtHandler
{
private:

    AppWindow *m_appWindow;

    Renderer* m_glRenderer;

    Scene *m_scene;

public:

	static Int32 main()
    {
    //	{
        //		PerlinNoise2d lPerlin;
        //		Image lPicture;
        //		Bool lRet = lPerlin.ToPicture(lPicture);
        //		lPicture.save("PerlinGen.jpg", Image::Jpeg);
    //	}

        Dir basePath("media");
        if (!basePath.exists()) {
            basePath = Dir("../media");
            if (!basePath.exists()) {
            O3D_MESSAGE("00002222");
                Application::message("Missing media content", "Error");
                return -1;
            }
        }
O3D_MESSAGE("00002");
        Window *apps = new Window(basePath);
O3D_MESSAGE("00003");

        File iconFile(basePath.makeFullFileName("icon.bmp"));
        if (iconFile.exists()) {
            apps->getWindow()->setIcon(iconFile.getFullFileName());
        }
O3D_MESSAGE("00004");
        apps->getScene()->getContext()->setBackgroundColor(Color(1.0f,0,0,1));
		// Unlock the mouse position
        apps->getWindow()->getInput().getMouse()->setGrab(False);
O3D_MESSAGE("00005");
        // Run the event loop
        Application::run();
        O3D_MESSAGE("00006");
        // Destroy any content
        deletePtr(apps);
		return 0;
	}

    Window(Dir &basePath) :
        m_camera(nullptr)
	{
        O3D_MESSAGE("000");
        m_appWindow = new AppWindow;
O3D_MESSAGE("001");
        // OpenGL renderer
        m_glRenderer = new Renderer;
O3D_MESSAGE("002");
        m_appWindow->setTitle("Objective-3D Window sample");
        m_appWindow->create(800, 600, AppWindow::COLOR_RGBA8, AppWindow::DEPTH_24_STENCIL_8, AppWindow::NO_MSAA, False, False);
O3D_MESSAGE("003");
        // Resize the window to an available fullscreen resolution (@see Video class).
        // m_appWindow->setFullScreen(True);
O3D_MESSAGE("004");
        m_glRenderer->create(m_appWindow);
O3D_MESSAGE("005");
        // create a scene and attach it to the window
        m_scene = new Scene(nullptr, basePath.getFullPathName(), m_glRenderer);
        m_scene->setSceneName("window");
        m_scene->defaultAttachment(m_appWindow);
O3D_MESSAGE("006");
		m_appWindow->onClose.connect(this, &Window::onClose);
		m_appWindow->onDraw.connect(this, &Window::onDraw);
        m_appWindow->onDestroy.connect(this, &Window::onDestroy);
O3D_MESSAGE("007");
		// create a simple camera
		m_camera = new Camera(getScene());
		Node *node = getScene()->getHierarchyTree()->addNode(m_camera);

		m_camera->setFov(60.f);
		m_camera->setRatio(800.f / 600.f);
		m_camera->setZnear(1.f);
		m_camera->setZfar(1000.f);
		m_camera->computePerspective();

		MTransform *transform = new MTransform;
		node->addTransform(transform);
		transform->setPosition(Vector3(0.f, 0.f, 1.f));
	}

    virtual ~Window()
    {
    }

    AppWindow* getWindow() { return m_appWindow; }

    Scene* getScene() { return m_scene; }

    void onDestroy()
    {
        this->getWindow()->logFps();

        // it is deleted by the application
        m_appWindow = nullptr;

        deletePtr(m_scene);
        deletePtr(m_glRenderer);
    }

	// draw a triangle
	void onDraw()
	{
		PrimitiveAccess access = getScene()->getPrimitiveManager()->access();

		getScene()->getContext()->projection().set(m_camera->getProjectionMatrix());
		getScene()->getContext()->modelView().set(m_camera->getModelviewMatrix());

		access->setModelviewProjection();

		access->setColor(Color(1.f,1.f,1.f));
		access->beginDraw(P_TRIANGLES);
			access->addVertex(Vector3(0.f, 1.f, -1.f), Color(1.f, 0.f, 0.f));
			access->addVertex(Vector3(-1.f, -1.f, -1.f), Color(0.f, 1.f, 0.f));
			access->addVertex(Vector3(1.f, -1.f, -1.f), Color(0.f, 0.f, 1.f));
		access->endDraw();
	}

	// to terminate our application with the close button of the window
	void onClose()
	{
		getWindow()->terminate();
	}

private:

	Camera *m_camera;
};

// We Call our application
O3D_NOCONSOLE_MAIN(Window, O3D_DEFAULT_CLASS_SETTINGS)
