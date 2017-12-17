/**
 * @file heightmap.cpp
 * @brief 
 * @author Emmanuel RUFFIO (emmanuel.ruffio@gmail.com)
 * @date 2010-01-03
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include <o3d/engine/scene/scene.h>
#include <o3d/engine/utils/framemanager.h>
#include <o3d/engine/scene/sceneobjectmanager.h>
#include <o3d/engine/screenviewport.h>
#include <o3d/engine/viewportmanager.h>
#include <o3d/engine/hierarchy/node.h>
#include <o3d/engine/hierarchy/hierarchytree.h>
#include <o3d/engine/object/ftransform.h>
#include <o3d/engine/context.h>
#include <o3d/engine/renderer.h>
#include <o3d/engine/visibility/visibilitymanager.h>
#include <o3d/core/mouse.h>
#include <o3d/core/appwindow.h>
#include <o3d/core/main.h>
#include <o3d/core/dir.h>
#include <o3d/gui/gui.h>
#include <o3d/gui/truetypefont.h>
#include <o3d/gui/fontmanager.h>

#include <o3d/engine/landscape/heightmap/heightmapsplatting.h>

#ifdef _MSC_VER
#pragma comment(lib,"opengl32.lib")
#endif

//#define BEPO

#ifdef BEPO
#define LEFT KEY_U
#define RIGHT KEY_E

#define UP KEY_B
#define DOWN KEY_A

#define FORWARD KEY_P
#define BACKWARD KEY_I
#else
#define LEFT KEY_S
#define RIGHT KEY_F

#define UP KEY_A
#define DOWN KEY_Q

#define FORWARD KEY_E
#define BACKWARD KEY_D
#endif

using namespace o3d;

/**
 * @brief The HeightmapSample class
 * @date 2010-01-03
 * @author Emmanuel RUFFIO (emmanuel.ruffio@gmail.com)
 */
class HeightmapSample : public EvtHandler
{
private:

    AppWindow *m_appWindow;
    Renderer* m_glRenderer;
    Scene *m_scene;
    Gui *m_gui;

public:

    HeightmapSample(Dir basePath)
	{
        m_appWindow = new AppWindow;

        // OpenGL renderer
        m_glRenderer = new Renderer;

        m_appWindow->setTitle("Objective-3D Heightmap with deffered shading sample");
        m_appWindow->create(800, 600, AppWindow::COLOR_RGBA8, AppWindow::DEPTH_24_STENCIL_8, AppWindow::MSAA16X, False, True);

        m_glRenderer->create(m_appWindow);

        // create a scene and attach it to the window
        m_scene = new Scene(nullptr, basePath.getFullPathName(), m_glRenderer);
        m_scene->setSceneName("heigthmap");
        m_scene->defaultAttachment(m_appWindow);

        // new gui manager and attach it to the scene
        m_gui = new Gui(m_scene);
        m_scene->setGui(m_gui);
        m_gui->defaultAttachment(m_appWindow);

        m_appWindow->onUpdate.connect(this, &HeightmapSample::onSceneUpdate);
        m_appWindow->onDraw.connect(this, &HeightmapSample::onSceneDraw);
        m_appWindow->onClose.connect(this, &HeightmapSample::onClose);
        m_appWindow->onKey.connect(this, &HeightmapSample::onKey);
        m_appWindow->onMouseMotion.connect(this, &HeightmapSample::onMouseMotion);
        m_appWindow->onMouseButton.connect(this, &HeightmapSample::onMouseButton);
        m_appWindow->onDestroy.connect(this, &HeightmapSample::onDestroy);

        // getWindow()->grabMouse();
	}

    virtual ~HeightmapSample()
	{
	}

    AppWindow* getWindow() { return m_appWindow; }
    Scene* getScene() { return m_scene; }
    Gui* getGui() { return m_gui; }

    void onDestroy()
    {
        deletePtr(m_scene);
        deletePtr(m_glRenderer);

        this->getWindow()->logFps();

        // it is deleted by the application
        m_appWindow = nullptr;
    }

	void onSceneUpdate()
	{
		Keyboard * lpKeyboard = getWindow()->getInput().getKeyboard();

		const Float elapsed = getScene()->getFrameManager()->getFrameDuration();
		const Float speed = 10.f;

		Float cam_t_z=0.f, cam_t_y=0.f, cam_t_x=0.f;

        if (lpKeyboard->isKeyDown(FORWARD)) cam_t_z = speed*-1.f*elapsed;
        if (lpKeyboard->isKeyDown(BACKWARD)) cam_t_z = speed*1.f*elapsed;

        if (lpKeyboard->isKeyDown(LEFT)) cam_t_x = speed*-1.f*elapsed;
        if (lpKeyboard->isKeyDown(RIGHT)) cam_t_x = speed*1.f*elapsed;

        if (lpKeyboard->isKeyDown(DOWN)) cam_t_y = speed*-1.f*elapsed;
        if (lpKeyboard->isKeyDown(UP)) cam_t_y = speed*1.f*elapsed;

		SceneObject *lpCamera = getScene()->getSceneObjectManager()->searchName("CameraFPS");
		lpCamera->getNode()->getTransform()->translate(Vector3(cam_t_x,cam_t_y,cam_t_z));
	}

	void onSceneDraw()
	{
	}

    void onMouseMotion(Mouse* mouse)
	{
		Float elapsed = getScene()->getFrameManager()->getFrameDuration();

		SceneObject *lpCamera = getScene()->getSceneObjectManager()->searchName("CameraFPS");
		lpCamera->getNode()->getTransform()->rotate(Y, -mouse->getDeltaX() * elapsed);
		lpCamera->getNode()->getTransform()->rotate(X, -mouse->getDeltaY() * elapsed);
	}

    void onMouseButton(Mouse* mouse, ButtonEvent event)
	{
	}

    void onKey(Keyboard* keyboard, KeyEvent event)
	{
        if (event.isPressed() && (event.key() == KEY_ESCAPE)) {
			getWindow()->terminate();
        }

        if (event.isPressed() && (event.key() == KEY_F12)) {
            if (getWindow()->isMouseGrabbed()) {
                getWindow()->grabMouse(False);
                System::print("Ungrab mouse", "Change");
            } else {
                getWindow()->grabMouse(True);
                System::print("Grab mouse", "Change");
            }
        }
	}

	void onClose()
	{
		getWindow()->terminate();
	}

	static Int32 main()
	{
        // cleared log out file with new header
        Debug::instance()->setDefaultLog("heightmap.log");
        Debug::instance()->getDefaultLog().clearLog();

        Dir basePath("media");
        if (!basePath.exists()) {
            basePath.setPathName("../media");
            if (!basePath.exists()) {
                Application::message("Missing media content", "Error");
                return -1;
            }
        }

        HeightmapSample *lTerrainApp = new HeightmapSample(basePath);

		// Window initialisation
        lTerrainApp->getScene()->getContext()->setBackgroundColor(0.633f,0.792f,.914f,0.0f);

        TrueTypeFont * lpFont = lTerrainApp->getGui()->getFontManager()->addTrueTypeFont(basePath.makeFullFileName("gui/arial.ttf"));
		lpFont->setTextHeight(12);
        lpFont->setColor(Color(0.0f, 0.0f, 0.0f));

		// Camera initialisation
        Camera *lpFPSCamera = new Camera(lTerrainApp->getScene());
        lTerrainApp->getScene()->getViewPortManager()->addScreenViewPort(lpFPSCamera,0,0);
        lTerrainApp->getScene()->setDrawObject(Scene::DRAW_QUADTREE, False);
        lTerrainApp->getScene()->getVisibilityManager()->setGlobal(VisibilityManager::QUADTREE, 2, 512.0f);

		lpFPSCamera->setName("CameraFPS");
		lpFPSCamera->setZnear(0.25f);
		lpFPSCamera->setZfar(500.0f);
		lpFPSCamera->setFov(60.0f);
		lpFPSCamera->computePerspective();
		lpFPSCamera->disableVisibility();

        Node *lnode = lTerrainApp->getScene()->getHierarchyTree()->addNode(lpFPSCamera);
		FTransform *ftransform = new FTransform;
		ftransform->setPosition(Vector3(0.0f,2.f,0.0f));
		ftransform->rotate(Y, o3d::PI);
		lnode->addTransform(ftransform);

		// Terrain loading
        Image lHeightmap(basePath.makeFullFileName("terrain/heightmap/L3DT_Heightmap.jpg"));
        Image lNormalmap(basePath.makeFullFileName("terrain/heightmap/L3DT_Normal.jpg"));
        Image lColormap(basePath.makeFullFileName("terrain/heightmap/L3DT_Colormap.jpg"));
        Image lLightmap(basePath.makeFullFileName("terrain/heightmap/L3DT_Lightmap.jpg"));
        Image lNoise(basePath.makeFullFileName("terrain/heightmap/Noise.jpg"));

		// Flip those two images so that they will appear as they are shown in your OS
		Bool lRet = lHeightmap.hFlip();
		lRet = lColormap.hFlip();

        HeightmapSplatting * lpHeightmap = new HeightmapSplatting(lTerrainApp->getScene(), lpFPSCamera, HeightmapSplatting::OPT_NOISE);
		lpHeightmap->setUnits(Vector3(1.0f, 0.1f, 1.0f));
		lpHeightmap->setNoiseScale(2.0f);
		lpHeightmap->setHeightmap(lHeightmap, 0.0f);
		lpHeightmap->setNormalmap(lNormalmap);
		lpHeightmap->setColormap(lColormap);
	//	lpHeightmap->setLightmap(lLightmap);
		lpHeightmap->setNoise(lNoise);

        lTerrainApp->getScene()->getLandscape()->getTerrainManager().addTerrain(lpHeightmap);

        // Run the event loop
        Application::run();

        // Destroy any content
        deletePtr(lTerrainApp);

		return 0;
	}
};

O3D_NOCONSOLE_MAIN(HeightmapSample, O3D_DEFAULT_CLASS_SETTINGS)
