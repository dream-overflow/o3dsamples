/**
 * @file pclodterrain.cpp
 * @brief 
 * @author Emmanuel RUFFIO (emmanuel.ruffio@gmail.com)
 * @date 2008-01-01
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include <o3d/engine/landscape/pclod/terrain.h>
#include <o3d/engine/landscape/pclod/configs.h>

#include <o3d/engine/texture/texture2d.h>
#include <o3d/engine/sky/skyscattering.h>
#include <o3d/engine/sky/skyobject.h>
#include <o3d/engine/sky/cloudlayerperlin.h>
#include <o3d/engine/object/worldlabel.h>
#include <o3d/engine/object/primitivemanager.h>
#include <o3d/engine/object/camera.h>
#include <o3d/engine/object/light.h>
#include <o3d/engine/object/ftransform.h>
#include <o3d/engine/object/mtransform.h>

#include <o3d/engine/utils/framemanager.h>
#include <o3d/engine/scene/sceneobjectmanager.h>
#include <o3d/engine/screenviewport.h>
#include <o3d/engine/viewportmanager.h>
#include <o3d/engine/hierarchy/hierarchytree.h>
#include <o3d/engine/matrix.h>
#include <o3d/engine/context.h>

#include <o3d/engine/scene/scene.h>
#include <o3d/engine/renderer.h>

#include <o3d/gui/truetypefont.h>
#include <o3d/gui/fontmanager.h>
#include <o3d/gui/gui.h>

#include <o3d/image/perlinnoise2d.h>

#include <o3d/core/mouse.h>
#include <o3d/core/appwindow.h>
#include <o3d/core/objects.h>
#include <o3d/core/main.h>
#include <o3d/core/diskdir.h>

#include <cstdlib>

using namespace o3d;

Light *lpLight1 = nullptr;
Light *lpLight2 = nullptr;
Light *lpLight3 = nullptr;
Light *lpLight4 = nullptr;

SkyScattering * lpSky = nullptr;
//SkyTexture * lpSky = nullptr;
TrueTypeFont * lpFont = nullptr;

#ifdef _MSC_VER
#pragma comment(lib, "opengl32.lib")
#endif

/**
 * @brief The HeightmapSample class
 * @date 2008-01-01
 * @author Emmanuel RUFFIO (emmanuel.ruffio@gmail.com)
 */
class TerrainSample : public EvtHandler
{
private:

    AppWindow *m_appWindow;
    Renderer* m_glRenderer;
    Scene *m_scene;
    Gui *m_gui;

	WorldLabel * m_pLabel;

	Float m_time;

public:

    TerrainSample(DiskDir basePath)
	{
        m_appWindow = new AppWindow;

        // OpenGL renderer
        m_glRenderer = new Renderer;

        m_appWindow->setTitle("Objective-3D Progressive Mesh LOD terrain sample");
        m_appWindow->create(1024, 600, AppWindow::COLOR_RGBA8, AppWindow::DEPTH_24_STENCIL_8, AppWindow::MSAA8X, False, False);

        m_glRenderer->create(m_appWindow);

        // create a scene and attach it to the window
        m_scene = new Scene(nullptr, basePath.getFullPathName(), m_glRenderer);
        m_scene->setSceneName("pclodterrain");
        m_scene->defaultAttachment(m_appWindow);

        // new gui manager and attach it to the scene
        m_gui = new Gui(m_scene);
        m_scene->setGui(m_gui);
        m_gui->defaultAttachment(m_appWindow);

        m_appWindow->onUpdate.connect(this, &TerrainSample::onSceneUpdate);
        m_appWindow->onDraw.connect(this, &TerrainSample::onSceneDraw);
        m_appWindow->onClose.connect(this, &TerrainSample::onClose);
        m_appWindow->onKey.connect(this, &TerrainSample::onKey);
        m_appWindow->onMouseMotion.connect(this, &TerrainSample::onMouseMotion);
        m_appWindow->onMouseButton.connect(this, &TerrainSample::onMouseButton);
        m_appWindow->onDestroy.connect(this, &TerrainSample::onDestroy);

		m_time = 0.001f*System::getMsTime();

		//getWindow()->grabMouse();
	}

    virtual ~TerrainSample()
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

		Float elapsed = getScene()->getFrameManager()->getFrameDuration();

		const float speed = 10.f;

		Float cam_t_z=0.f, cam_t_y=0.f, cam_t_x=0.f;

        if (lpKeyboard->isKeyDown(KEY_E)) cam_t_z = speed*-1.f*elapsed;
        if (lpKeyboard->isKeyDown(KEY_D)) cam_t_z = speed*1.f*elapsed;

        if (lpKeyboard->isKeyDown(KEY_S)) cam_t_x = speed*-1.f*elapsed;
        if (lpKeyboard->isKeyDown(KEY_F)) cam_t_x = speed*1.f*elapsed;

        if (lpKeyboard->isKeyDown(KEY_A)) cam_t_y = speed*-1.f*elapsed;
        if (lpKeyboard->isKeyDown(KEY_Q)) cam_t_y = speed*1.f*elapsed;

		SceneObject *lpCamera = getScene()->getSceneObjectManager()->searchName("CameraFPS");
		lpCamera->getNode()->getTransform()->translate(Vector3(cam_t_x,cam_t_y,cam_t_z));

		static int lCounter = 0;

        if ((++lCounter % 5) == 0) {
			Vector3 lDirection1(0.0f, -0.4f, 2.0f*cos(lCounter/400.0f));
			lDirection1.normalize();

			Vector3 lDirection2(2.0f*cos(lCounter/200.0f), -0.7f, 0.0f);
			lDirection2.normalize();

			lpLight1->getNode()->getTransform()->setDirectionZ(lDirection1);
			lpLight2->getNode()->getTransform()->setDirectionZ(lDirection2);
		}

		m_time = 0.001f*System::getMsTime();

        if (lpSky != nullptr) {
			lpSky->setTime(m_time);
        }
	}

	void onSceneDraw()
    {
		Int32 lViewPort[4];
		getScene()->getContext()->getViewPort(lViewPort);

		Vector2f lViewPortf(Float(lViewPort[2] - 1), Float(lViewPort[3] - 1));

		Matrix4 l2DProjection, lProjection;
		l2DProjection.buildOrtho(0.0f, lViewPortf[X], lViewPortf[Y], 0.0f, -1.0f, 1.0f);

		lProjection = getScene()->getContext()->projection().get();
		getScene()->getContext()->projection().set(l2DProjection);
		getScene()->getContext()->modelView().push();
		getScene()->getContext()->modelView().identity();

		const Vector2f lBounds(lpSky->getCurrentTimeBounds());
		Float lCoef = 0.0f;

        if (lBounds[1] != lBounds[0]) {
			lCoef = Float((lpSky->getTime() - lBounds[0])/(lBounds[1] - lBounds[0]));
        } else {
			lCoef = 0.0f;
        }

		// primitive draw
		{
			PrimitiveAccess primitive = getScene()->getPrimitiveManager()->access();

			getScene()->getContext()->modelView().translate(Vector3(0.2f, 0.2f, 0.0f));

			getScene()->getContext()->setDepthFunc(COMP_LEQUAL);
			getScene()->getContext()->setCullingMode(CULLING_NONE);

			primitive->setColor(1,1,1,1);
			primitive->beginDraw(P_TRIANGLE_STRIP);
				primitive->addVertex(100.0f, 10.0f, 0);
				primitive->addVertex(lViewPortf[X] - 100.0f, 10.0f, 0);
				primitive->addVertex(100.0f, 60.0f, 0);
				primitive->addVertex(lViewPortf[X] - 100.0f, 60.0f, 0);
			primitive->endDraw();

			primitive->setColor(0,0,0,1);
			primitive->beginDraw(P_LINE_LOOP);
				primitive->addVertex(100.0f, 10.0f, 0);
				primitive->addVertex(lViewPortf[X] - 100.0f, 10.0f, 0);
				primitive->addVertex(lViewPortf[X] - 100.0f, 60.0f, 0);
				primitive->addVertex(100.0f, 60.0f, 0);
			primitive->endDraw();

            if (lpSky->isForecast()) {
				primitive->setColor(1,0,0,1);
				primitive->beginDraw(P_TRIANGLE_STRIP);
					primitive->addVertex(110.0f, 24.0f, 0);
					primitive->addVertex(110.0f + (lViewPortf[X] - 220.0f) * lCoef, 24.0f, 0);
					primitive->addVertex(110.0f, 36.0f, 0);
					primitive->addVertex(110.0f + (lViewPortf[X] - 220.0f) * lCoef, 36.0f, 0);
				primitive->endDraw();

				primitive->setColor(0,0,0,1);
				primitive->beginDraw(P_LINE_LOOP);
					primitive->addVertex(110.0f, 24.0f, 0);
					primitive->addVertex(lViewPortf[X] - 110.0f, 24.0f, 0);
					primitive->addVertex(lViewPortf[X] - 110.0f, 36.0f, 0);
					primitive->addVertex(110.0f, 36.0f,0);
				primitive->endDraw();
			}
		}

		String lText;
		lText = String::print("Time = %.2f    Delta = %.2f", lpSky->getTime(), lpSky->getRequestedTime() - lpSky->getTime());
		lpFont->write(Vector2i((lViewPort[2]-lpFont->sizeOf(lText))/2, 22), lText);

        if (lpSky->isForecast()) {
			lText = String::print("%.2f", lBounds[0]);
			lpFont->write(Vector2i(110, 52), lText);

			lText = String::print("%.2f", lBounds[1]);
			lpFont->write(Vector2i(lViewPort[2] - 110 - lpFont->sizeOf(lText), 52), lText);
		}

		getScene()->getContext()->setDefaultDepthFunc();
		getScene()->getContext()->setDefaultCullingMode();

		getScene()->getContext()->modelView().pop();
		getScene()->getContext()->projection().set(lProjection);
	}

    void onMouseMotion(Mouse* mouse)
	{
		Float elapsed = getScene()->getFrameManager()->getFrameDuration();

		SceneObject *lpCamera = getScene()->getSceneObjectManager()->searchName("CameraFPS");
        if (lpCamera) {
			lpCamera->getNode()->getTransform()->rotate(Y, -mouse->getDeltaX() * elapsed);
			lpCamera->getNode()->getTransform()->rotate(X, -mouse->getDeltaY() * elapsed);
		}
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
        Debug::instance()->setDefaultLog("pclodterrainMain.log");
        Debug::instance()->getDefaultLog().clearLog();

        // MemoryManager::Instance()->enableLog(MemoryManager::MemoryCentral, 128);
        // MemoryManager::Instance()->enableLog(MemoryManager::MemoryGraphic);

        DiskDir basePath("media");
        if (!basePath.exists()) {
            basePath.setPathName("../media");
            if (!basePath.exists()) {
                Application::message("Missing media content", "Error");
                return -1;
            }
        }

        TerrainSample *lTerrainApp = new TerrainSample(basePath);

        Camera *lpFPSCamera = new Camera(lTerrainApp->getScene());
        lTerrainApp->getScene()->getViewPortManager()->addScreenViewPort(lpFPSCamera,0,0);

		lpFPSCamera->setName("CameraFPS");
		lpFPSCamera->setZnear(1.0f);
		lpFPSCamera->setZfar(2000.0f);
		lpFPSCamera->setFov(60.0f);
		lpFPSCamera->computePerspective();
		lpFPSCamera->disableVisibility();

        Node *lnode = lTerrainApp->getScene()->getHierarchyTree()->addNode(lpFPSCamera);
		FTransform *ftransform = new FTransform;
		ftransform->setPosition(Vector3(0.0f,10.f,0.0f));
		ftransform->rotate(Y, o3d::PI);
		lnode->addTransform(ftransform);

        lTerrainApp->getScene()->getContext()->setBackgroundColor(0.633f,0.792f,.914f,0.0f);

        PCLODTerrain *pTerrain = new PCLODTerrain(lTerrainApp->getScene(), lpFPSCamera);
        lTerrainApp->getScene()->getLandscape()->getTerrainManager().addTerrain(pTerrain);

		//String headerFile = basePath + String("TerrainTerragen_LightmapTest.hclm");
		//String headerFile = basePath + String("TerrainTerragen1zone_WithMaterials.hclm");
		//MAUVAIS FORMAT String headerFile = basePath + String("TerrainTerragen_WithMaterials.hclm");

		//String headerFile = basePath + String("Terrain_Tower.hclm");
        String headerFile = basePath.makeFullFileName("terrain/TerrainTerragen_64.hclm");
        //String headerFile = basePath.makeFileName("terrain/TerrainTerragen_LightmapTest.hclm");
        String dataDir = basePath.makeFullPathName("terrain");
        String materialDir = basePath.makeFullPathName("terrain/Materials");
        String colormapDir = basePath.makeFullPathName("terrain/Colormaps");

        Image noise(basePath.makeFullFileName("terrain/noise.jpg"));

		pTerrain->getCurrentConfigs().setColormapPolicy(PCLODConfigs::COLORMAP_AUTO);
		pTerrain->getCurrentConfigs().setColormapPrecision(2);
		pTerrain->getCurrentConfigs().setDistanceOnlyMaterial(10.0f);
		pTerrain->getCurrentConfigs().setDistanceOnlyColormap(25.0f);
		pTerrain->getCurrentConfigs().setViewDistance(40);
        pTerrain->getCurrentConfigs().enableAsynchRefresh(True);
		pTerrain->getCurrentConfigs().setRefreshFrequency(10);
		pTerrain->getCurrentConfigs().setColormapStaticNoise(noise);
        pTerrain->getCurrentConfigs().enableColormapStaticNoise(noise.isValid());
		pTerrain->getCurrentConfigs().setColormapStaticNoiseFactor(0.1f);
		pTerrain->getCurrentConfigs().enableFrustumCulling(True);
		pTerrain->getCurrentConfigs().enableFrontToBack(True);
		pTerrain->getCurrentConfigs().setFrontToBackMinViewMove(10.0f);
		pTerrain->getCurrentConfigs().setFrontToBackRefreshPeriodicity(100);
		pTerrain->getCurrentConfigs().setLightMinAngleVariation(0.5f * 3.14159f/180.0f);
        pTerrain->getCurrentConfigs().setText2D(lTerrainApp->getGui()->getFontManager()->addTrueTypeFont(basePath.makeFullFileName("gui/arial.ttf")));
		pTerrain->getCurrentConfigs().enableDebugLabel(True);

		pTerrain->getCurrentConfigs().enableLightmapLod(False);
		pTerrain->getCurrentConfigs().setLightmapPoint(0.0f, 0);
		pTerrain->getCurrentConfigs().setLightmapPoint(50.0f, -1);
		pTerrain->getCurrentConfigs().setLightmapPoint(100.0f, -2);
		pTerrain->getCurrentConfigs().setLightmapPoint(200.0f, -3);

		pTerrain->getCurrentConfigs().enableWireFrame(False);
		pTerrain->getCurrentConfigs().enableLightning(True);
		pTerrain->getCurrentConfigs().enableSelfShadowing(True);

		pTerrain->load(headerFile, dataDir, materialDir, colormapDir);
		pTerrain->init(Vector3(0.0, 10.0, 0.0));
		pTerrain->getCurrentConfigs().showMessage(True);

		//
		// light 1
		//

        lpLight1 = new Light(lTerrainApp->getScene(), Light::DIRECTIONAL_LIGHT);
		lpLight1->setAmbient(0.0f, 0.0f, 0.0f, 1.0f);
		lpLight1->setDiffuse(1.2f, 1.2f, 1.2f, 1.0f);
		lpLight1->setSpecular(0.0f, 0.0f, 0.0f, 0.0f);

        lnode = lTerrainApp->getScene()->getHierarchyTree()->addNode(lpLight1);

		MTransform *ltransform = new MTransform;
		ltransform->setPosition(Vector3(4.0f, 10.0f, 4.0f)); // only used to have a symbolic here
		ltransform->setDirectionZ(Vector3(0.0f, -0.7f, 0.0f));

		lnode->addTransform(ltransform);

		//
		// light 2
		//

        lpLight2 = new Light(lTerrainApp->getScene(), Light::DIRECTIONAL_LIGHT);
		lpLight2->setAmbient(0.0f, 0.0f, 0.0f, 0.0f);
		lpLight2->setDiffuse(0.5f, 0.5f, 0.5f, 1.0f);
		lpLight2->setSpecular(0.0f, 0.0f, 0.0f, 0.0f);

		ltransform = new MTransform;
		ltransform->setPosition(Vector3(10.f, 10.f, 10.f));
		ltransform->setDirectionZ(Vector3(-0.714f, -0.5f, 0.0f));

        lnode = lTerrainApp->getScene()->getHierarchyTree()->addNode(lpLight2);
		lnode->addTransform(ltransform);

		//
		// light 3
		//

        lpLight3 = new Light(lTerrainApp->getScene(), Light::SPOT_LIGHT);
		lpLight3->setAmbient(0.0f, 0.0f, 0.0f, 0.0f);
		lpLight3->setDiffuse(1.0f, 0.0f, 0.0f, 1.0f);
		lpLight3->setSpecular(0.0f, 0.0f, 0.0f, 0.0f);

		ltransform = new MTransform;
		ltransform->setPosition(Vector3(15.f, 10.f, 15.f));
		ltransform->setDirectionZ(Vector3(0.0f, -0.5f, 0.5f));

        lnode = lTerrainApp->getScene()->getHierarchyTree()->addNode(lpLight3);
		lnode->addTransform(ltransform);

		lpLight3->setCutOff(28.f);
		lpLight3->setExponent(1.0f);
		lpLight3->setConstantAttenuation(1.0f);
		lpLight3->setLinearAttenuation(0.0f);
		lpLight3->setQuadraticAttenuation(0.0f);

		//
		// light 4
		//

        lpLight4 = new Light(lTerrainApp->getScene(), Light::SPOT_LIGHT);
		lpLight4->setAmbient(0.0f, 0.0f, 0.0f, 0.0f);
		lpLight4->setDiffuse(0.0f, 0.0f, 1.0f, 1.0f);
		lpLight4->setSpecular(0.0f, 0.0f, 0.0f, 0.0f);

		ltransform = new MTransform;
		ltransform->setPosition(Vector3(15.f, 10.f, 15.f));
		ltransform->setDirectionZ(Vector3(0.0f, -0.5f, 0.3f));

        lnode = lTerrainApp->getScene()->getHierarchyTree()->addNode(lpLight4);
		lnode->addTransform(ltransform);

		lpLight4->setCutOff(28.f);
		lpLight4->setExponent(1.0f);
		lpLight4->setConstantAttenuation(1.0f);
		lpLight4->setLinearAttenuation(0.0f);
		lpLight4->setQuadraticAttenuation(0.0f);

		//
		// debug info
		//

        lTerrainApp->getScene()->setDrawObject(Scene::DRAW_DIRECTIONAL_LIGHT, True);
        lTerrainApp->getScene()->setDrawObject(Scene::DRAW_SPOT_LIGHT, True);
        //lTerrainApp->getScene()->setDrawObject(Scene::GLOBAL_BOUNDING, True);
        lTerrainApp->getScene()->setDrawObject(Scene::DRAW_QUADTREE, False);
        lTerrainApp->getScene()->setDrawObject(Scene::DRAW_DEBUG, True);

		//
		// add lights to terrain
		//

		TerrainBase::LightInfos lLightInfos;
		lLightInfos.policy = TerrainBase::LIGHT_POLICY_PER_VERTEX;
		lLightInfos.type = TerrainBase::LIGHT_TYPE_STATIC;
		lLightInfos.update = TerrainBase::LIGHT_UPDATE_AUTO;

		lLightInfos.pLight = lpLight1;
		pTerrain->addLight(lLightInfos);

		lLightInfos.pLight = lpLight3;
		pTerrain->addLight(lLightInfos);

		lLightInfos.pLight = lpLight4;
		pTerrain->addLight(lLightInfos);

		lLightInfos.pLight = lpLight2;
		pTerrain->addLight(lLightInfos);

		//
		// sky object
		//

		lpSky = new SkyScattering(pTerrain);
		lpSky->setPlanetRadius(6400.0f);
		lpSky->setAtmosphereThickness(100.0f);
		lpSky->setDomePrecision(4);
		lpSky->setMoleculePhaseFunctionCoefficients(Vector2f(1.75f, 0.25f));
		lpSky->setIntegrationStepFactor(1.1f);
		lpSky->setIntegrationStepIndex(5);
		lpSky->enableForecast(True);
		lpSky->setTimeStep(30.0f);
		lpSky->enableAsync(True);
		lpSky->enableColorInterpolation(False);
		lpSky->setTime(12.5);

		//
		// sun
		//

		SkyObject * lpSun = new SkyObject(lpSky);
    //	lpSun->setPosition(Vector3(0.0f, -0.25f, 150E9f)); // a night position
		lpSun->setPosition(Vector3(0.0f, 0.7f, 150E9f));   // a day position
		lpSun->setApparentAngle(Vector2f(2.0f*o3d::toRadian(0.53f), 2.0f*o3d::toRadian(0.53f)));
		lpSun->setIntensity(Vector3(200.0f, 220.0f, 250.0f));
		lpSun->setWaveLength(Vector3(650.0e-9f, 610.0e-9f, 475.0e-9f));

		Texture2D * lpSunTexture = new Texture2D(lpSun, basePath.makeFullFileName("terrain/sun.png"));
		lpSunTexture->create(True);
		lpSun->setTexture(lpSunTexture);
		lpSky->addObject(lpSun);

		//
		// moon
		//

		SkyObject * lpMoon = new SkyObject(lpSky);
        //lpMoon->setPosition(Vector3(1.0f, 0.7f, 384E6f));  // visible moon
		lpMoon->setApparentAngle(Vector2f(3.0f*o3d::toRadian(0.53f), 3.0f*o3d::toRadian(0.53f)));
		lpMoon->setIntensity(Vector3(400E-6f, 440E-6f, 500E-6f));
		lpMoon->setWaveLength(Vector3(650.0e-9f, 610.0e-9f, 475.0e-9f));

        Texture2D * lpMoonTexture = new Texture2D(lpMoon, basePath.makeFullFileName("terrain/moon256.png"));
		lpMoonTexture->create(True);
		lpMoon->setTexture(lpMoonTexture);
		lpSky->addObject(lpMoon);

		//
		// clouds
		//
		//                 radius		cloud altitude
		Dome lLayerDome(6400.0f,			10.f,			4, Primitive::GEN_TEX_COORDS);
	//	lLayerDome.setTextureCoordinatePolicy(Dome::TEX_PROJECTION);
    //	lLayerDome.setTextureCoordinatePolicy(Dome::TEX_LATITUDE_LONGITUDE);
        lLayerDome.setTextureCoordinatePolicy(Dome::TEX_UNFOLD);

		// The cloud layer is generated
		CloudLayerPerlin * lpCloudLayer = new CloudLayerPerlin(
			lpSky,
			CloudLayerPerlin::CLOUD_SHADING | CloudLayerPerlin::CLOUD_SHADOWING | CloudLayerPerlin::CLOUD_OCCLUSION);

		lpCloudLayer->setDome(lLayerDome);
		lpCloudLayer->setContrast(20);
		lpCloudLayer->setCoveringRate(0.45f);
		lpCloudLayer->setScale(1.0f);
		lpCloudLayer->setAverageSize(1);
        //lpCloudLayer->setLightDirection(Vector3(1.0f, 0.0f, 0.0f));
		Vector3 lSunDirection = lpSun->getCartesianPosition();
		lSunDirection.normalize();

		lpCloudLayer->setLightDirection(lSunDirection); // Direction of the sun specified above
		lpCloudLayer->setCloudColor(Vector3(1.0f, 1.0f, 1.0f));
		lpCloudLayer->setNoiseParameters(Vector4(13.0f, 43.0f, 101.0f, 0.5f));
		lpCloudLayer->setIntensityParameters(Vector4(0.005f, 0.5f, 1.0f, 0.0f));
		lpCloudLayer->setColorParameters(Vector4(0.6f, 3.0f, 0.1f, 0.0f));
		lpCloudLayer->setLightParameters(Vector4(0.6f, 0.4f, -3.0f, 0.5f));
		lpCloudLayer->setVelocity(Vector2f(0.005f, 0.001f));

		// The perlin cloud texture is generated (optional)
		PerlinNoise2d lNoise;
		lNoise.setAmplitudes(PerlinNoise2d::geometricSequence(5, 0.5f, 1.0f));
		lNoise.setFrequencies(PerlinNoise2d::geometricSequence(5, 2, 4));
		lNoise.setBoundaryPolicy(PerlinNoise2d::BOUNDARY_REPEAT);
		lNoise.setOctaveGenerationPolicy(PerlinNoise2d::OCTAVE_POSITIVE);
		lNoise.setSize(128);
		lNoise.setRandomSeed(15);

		lpCloudLayer->setPerlin(lNoise);

		Image lPerlinPict;
		lNoise.toImage(lPerlinPict);
		lPerlinPict.save("PerlinCloud.jpg", Image::JPEG);

		// The noise texture is retrieved, its random seed is set
		lNoise = lpCloudLayer->getNoise();
		lNoise.setRandomSeed(10);

		lpCloudLayer->setNoise(lNoise);

		Image lPerlinPict2;
		lpCloudLayer->getNoise().toImage(lPerlinPict2);
		lPerlinPict2.save("PerlinNoise.jpg", Image::JPEG);

		// Finally, the layer is added to the sky
		lpSky->addCloudLayer(lpCloudLayer);

	/*
		lpSky = new SkyTexture(pTerrain);
		lpSky->setDomePrecision(3);

		Texture2D * lpTexture = new Texture2D(lpSky, basePath + "hemispherical_2048.png");
        lpTexture->setWarp(TextureRepeat);
		lpTexture->create(True);

		lpSky->setTexture(lpTexture);
	*/
		pTerrain->setSky(lpSky);

        lpFont = lTerrainApp->getGui()->getFontManager()->addTrueTypeFont(basePath.makeFullFileName("gui/arial.ttf"));
		lpFont->setTextHeight(12);
        lpFont->setColor(Color(0.0f, 0.0f, 0.0f));

		lpSky->init();

        // Run the event loop
        Application::run();

        // Destroy any content
        deletePtr(lTerrainApp);

		return 0;
	}
};

O3D_NOCONSOLE_MAIN(TerrainSample, O3D_DEFAULT_CLASS_SETTINGS)
