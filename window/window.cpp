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
 * @brief The WindowSample class.
 * @date 2004-01-01
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 */
class WindowSample : public EvtHandler
{
private:

    AppWindow *m_appWindow;

    Renderer* m_glRenderer;

    Scene *m_scene;

public:

    WindowSample(Dir &basePath) :
        m_camera(nullptr)
	{
        m_appWindow = new AppWindow;

        // OpenGL renderer
        m_glRenderer = new Renderer;

        m_appWindow->setTitle("Objective-3D Window sample");
        m_appWindow->create(800, 600, AppWindow::COLOR_RGBA8, AppWindow::DEPTH_24_STENCIL_8, AppWindow::NO_MSAA, False, False);

        File iconFile(basePath.makeFullFileName("icon.bmp"));
        if (iconFile.exists()) {
            m_appWindow->setIcon(iconFile.getFullFileName());
        }

        // Resize the window to an available fullscreen resolution (@see Video class).
        // m_appWindow->setFullScreen(True);

        m_glRenderer->create(m_appWindow);

        // create a scene and attach it to the window
        m_scene = new Scene(nullptr, basePath.getFullPathName(), m_glRenderer);
        m_scene->setSceneName("window");
        m_scene->defaultAttachment(m_appWindow);

        m_appWindow->onClose.connect(this, &WindowSample::onClose);
        m_appWindow->onDraw.connect(this, &WindowSample::onDraw);
        m_appWindow->onDestroy.connect(this, &WindowSample::onDestroy);

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

        m_scene->getContext()->setBackgroundColor(Color(1.0f,0,0,1));
        // Lock the mouse position
        //  m_appWindow->getInput().getMouse()->setGrab(False);
	}

    virtual ~WindowSample()
    {
        if (m_appWindow) {
            onDestroy();
        }
    }

    AppWindow* getWindow() { return m_appWindow; }

    Scene* getScene() { return m_scene; }

    void onDestroy()
    {
        if (!m_appWindow) {
            return;
        }

        this->getWindow()->logFps();

        deletePtr(m_scene);
        deletePtr(m_glRenderer);

        // it is deleted by the application
        m_appWindow = nullptr;
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

class MyActivity : public Activity
{
public:

    static Int32 main()
    {
        MemoryManager::instance()->enableLog(MemoryManager::MEM_RAM,128);
        MemoryManager::instance()->enableLog(MemoryManager::MEM_GFX);

        Application::setActivity(new MyActivity);

        Application::start();
        Application::run();
        Application::stop();

        return 0;
    }

    virtual Int32 onStart() override
    {
        Dir basePath("media");
        if (!basePath.exists()) {
            basePath = Dir("../media");
            if (!basePath.exists()) {
                Application::message("Missing media content", "Error");
                return -1;
            }
        }

        m_app = new WindowSample(basePath);

        return 0;
    }

    virtual Int32 onStop() override
    {
        deletePtr(m_app);
        return 0;
    }

    virtual Int32 onPause() override
    {
        // m_app->pause();
        return 0;
    }

    virtual Int32 onResume() override
    {
        // m_app->resume();
        return 0;
    }

    virtual Int32 onSave() override
    {
        // m_app->save();
        return 0;
    }

private:

    WindowSample *m_app;
};

// We Call our application
O3D_NOCONSOLE_MAIN(MyActivity, O3D_DEFAULT_CLASS_SETTINGS)
