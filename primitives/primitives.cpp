/**
 * @file primitives.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @author Emmanuel RUFFIO (emmanuel.ruffio@gmail.com)
 * @date 2008-01-01
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include <o3d/engine/scene/scene.h>
#include <o3d/engine/scene/sceneobjectmanager.h>
#include <o3d/core/keyboard.h>
#include <o3d/core/appwindow.h>
#include <o3d/core/application.h>
#include <o3d/core/main.h>
#include <o3d/core/diskdir.h>

#include <o3d/engine/viewport.h>
#include <o3d/engine/renderer.h>
#include <o3d/engine/context.h>
#include <o3d/engine/viewportmanager.h>
#include <o3d/engine/texture/texturemanager.h>
#include <o3d/engine/hierarchy/node.h>
#include <o3d/engine/hierarchy/hierarchytree.h>
#include <o3d/engine/object/primitive.h>
#include <o3d/engine/object/dome.h>
#include <o3d/engine/object/isosphere.h>
#include <o3d/engine/object/ftransform.h>
#include <o3d/engine/object/mtransform.h>
#include <o3d/engine/object/camera.h>
#include <o3d/engine/object/mesh.h>
#include <o3d/engine/utils/framemanager.h>
#include <o3d/engine/material/colormaterial.h>
#include <o3d/engine/material/ambientmaterial.h>
#include <o3d/engine/material/lambertmaterial.h>

#include <o3d/engine/object/primitivemanager.h>
#include <o3d/engine/shadow/shadowvolumeforward.h>

#include <o3d/geom/frustum.h>

#include <o3d/engine/visibility/visibilitymanager.h>

using namespace o3d;

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

/**
 * @brief The PrimitivesSample class
 * @date 2008-01-01
 * @author Emmanuel RUFFIO (emmanuel.ruffio@gmail.com)
 */
class PrimitivesSample : public EvtHandler
{
public:

    class Drawer : public ShadowVolumeForward
    {
    private:

        PrimitivesSample *m_sample;

    public:

        //! Constructor. Take a parent objets.
        Drawer(BaseObject *parent, PrimitivesSample *sample) :
            ShadowVolumeForward(parent),
            m_sample(sample)
        {

        }

        virtual void draw(ViewPort *viewPort)
        {
            // The camera modelview should be set before draw()
            if (getScene()->getActiveCamera() == nullptr) {
                return;
            }

            Context &context = *getScene()->getContext();
            Camera &camera = *getScene()->getActiveCamera();

            // Computes frustum just after camera put
            getScene()->getFrustum()->computeFrustum(context.projection().get(), camera.getModelviewMatrix());

            // Determines visible objects
            getScene()->getVisibilityManager()->processVisibility();

            //
            // Draw first pass with no lighting, in camera space
            //

            // draw the first pass with ambient
            DrawInfo drawInfo(DrawInfo::AMBIENT_PASS);

            context.disableStencilTest();
            context.setDefaultDepthFunc();

            // world objects
            getScene()->getVisibilityManager()->draw(drawInfo);

            m_sample->onSceneDraw();

            context.disableStencilTest();
            context.setDefaultStencilTestFunc();
            context.setDefaultDepthTest();
            context.setDefaultDepthWrite();
            context.setDefaultDepthFunc();

            // camera clear
            camera.clearCameraChanged();
        }
    };

private:

    AppWindow *m_appWindow;

    Renderer* m_glRenderer;

    Scene *m_scene;

    Cube* primitive;
    Cylinder* cylinder;
    Sphere* sphere;
    Cube* solidPrimitive;
    Cylinder* solidCylinder;
    Sphere* solidSphere;
    Mesh* texturedSphere;
    Surface* surface;
    Surface* solidSurface;
    IsoSphere* isoSphere;
    IsoSphere* solidIsoSphere;
    Dome* dome;
    Dome* solidDome;
    Mesh* texturedDome;

    GeometryData *geom;

    Int32 object;

public:

    PrimitivesSample(DiskDir basePath) :
            geom(nullptr),
            object(0)
	{
        m_appWindow = new AppWindow;

        // OpenGL renderer
        m_glRenderer = new Renderer;

        m_appWindow->setTitle("Objective-3D Primitives sample");
        m_appWindow->create(800, 600, AppWindow::COLOR_RGBA8, AppWindow::DEPTH_24_STENCIL_8, AppWindow::NO_MSAA, False, False);

        m_glRenderer->create(m_appWindow);

        // create a scene and attach it to the window
        m_scene = new Scene(nullptr, basePath.getFullPathName(), m_glRenderer);
        m_scene->setSceneName("primitives");
        m_scene->defaultAttachment(m_appWindow);

        primitive = new Cube(3.f,3, Primitive::WIRED_MODE);
        solidPrimitive = new Cube(3.f,3);
        cylinder = new Cylinder(3.f,0.f,1.5f,10,4, Primitive::WIRED_MODE);
        solidCylinder = new Cylinder(3.f,0.f,5.f,15,3);
        sphere = new Sphere(3, 10, 10, Primitive::WIRED_MODE);
        solidSphere = new Sphere(3, 10, 10);
        texturedSphere = new Mesh(getScene());
        surface = new Surface(3,3,3,5,Surface::SIMPLE_GRID);
        solidSurface = new Surface(3,3,3,1,Surface::ONE_SIDED);
        isoSphere = new IsoSphere(3.f,2, Primitive::WIRED_MODE | IsoSphere::HALF_SPHERE);
        solidIsoSphere = new IsoSphere(3.f,2);
        dome = new Dome(10.f,7,4, Primitive::WIRED_MODE);
        solidDome = new Dome(10.f,7,4);
        texturedDome = new Mesh(getScene());

		// Generation of meshes
        MeshData *lpSphereData = new MeshData(texturedSphere);
        GeometryData *lpSphereGeometry = new GeometryData(
                                             lpSphereData,
                                             Sphere(3, 50, 50, Primitive::GEN_TEX_COORDS | Sphere::FAST_CORRECTION));
        lpSphereData->setGeometry(lpSphereGeometry);
        lpSphereData->createGeometry();
        texturedSphere->setMeshData(lpSphereData);

        // Textured sphere
        Texture2D * lpEarthTexture = getScene()->getTextureManager()->addTexture2D("earth.jpg", True);

        texturedSphere->setNumMaterialProfiles(1);
        texturedSphere->getMaterialProfile(0).setNumTechniques(1);
        texturedSphere->getMaterialProfile(0).getTechnique(0).setNumPass(1);
        texturedSphere->getMaterialProfile(0).getTechnique(0).getPass(0).setMaterial(Material::AMBIENT, new ColorMaterial(getScene()));
        //texturedSphere->getMaterialProfile(0).getTechnique(0).getPass(0).setMaterial(Material::LIGHTING, new LambertMaterial(getScene()));
        texturedSphere->getMaterialProfile(0).getTechnique(0).getPass(0).setAmbient(Color(1.0f, 1.0f, 1.0f, 1.f));
        texturedSphere->getMaterialProfile(0).getTechnique(0).getPass(0).setDiffuse(Color(1.0f, 1.0f, 1.0f, 1.f));
        texturedSphere->getMaterialProfile(0).getTechnique(0).getPass(0).setDiffuseMap(lpEarthTexture);
        texturedSphere->getMaterialProfile(0).getTechnique(0).getPass(0).setMapWarp(MaterialPass::DIFFUSE_MAP, Texture::CLAMP);
        texturedSphere->getMaterialProfile(0).getTechnique(0).getPass(0).setSpecular(Color(0.0f, 0.0f, 0.0f, 1.f));
        texturedSphere->getMaterialProfile(0).getTechnique(0).getPass(0).setShine(1.f);
        texturedSphere->initMaterialProfiles();

        texturedSphere->disable();
        getScene()->getHierarchyTree()->addNode(texturedSphere);

		// Textured dome
        Texture2D * lpHemiTexture = getScene()->getTextureManager()->addTexture2D(basePath.makeFullFileName("terrain/hemispherical_2048.png"), True);

        MeshData * lpDomeData = new MeshData(texturedDome);
        Dome lLayerDome(Primitive::GEN_TEX_COORDS);
        lLayerDome.setRadius(610.f);
        lLayerDome.setHeight(7.f);
        lLayerDome.setSubDiv(4);
        //lLayerDome.setTextureCoordinatePolicy(Dome::TEX_PROJECTION);
        //lLayerDome.setTextureCoordinatePolicy(Dome::TEX_LATITUDE_LONGITUDE);
        lLayerDome.setTextureCoordinatePolicy(Dome::TEX_UNFOLD);
        lLayerDome.update();

        GeometryData * lpDomeGeometry = new GeometryData(lpDomeData, lLayerDome);
        lpDomeData->setGeometry(lpDomeGeometry);
        lpDomeData->createGeometry();
        texturedDome->setMeshData(lpDomeData);

        texturedDome->setNumMaterialProfiles(1);
        texturedDome->getMaterialProfile(0).setNumTechniques(1);
        texturedDome->getMaterialProfile(0).getTechnique(0).setNumPass(1);
        texturedDome->getMaterialProfile(0).getTechnique(0).getPass(0).setMaterial(Material::AMBIENT, new ColorMaterial(getScene()));
        //texturedDome->getMaterialProfile(0).getTechnique(0).getPass(0).setMaterial(Material::LIGHTING, new LambertMaterial(getScene()));
        texturedDome->getMaterialProfile(0).getTechnique(0).getPass(0).setAmbient(Color(1.0f, 1.0f, 1.0f, 1.f));
        texturedDome->getMaterialProfile(0).getTechnique(0).getPass(0).setDiffuse(Color(1.0f, 1.0f, 1.0f, 1.f));
        texturedDome->getMaterialProfile(0).getTechnique(0).getPass(0).setDiffuseMap(lpHemiTexture);
        //texturedDome->getMaterialProfile(0).getTechnique(0).getPass(0).setMapWarp(MaterialPass::DIFFUSE_MAP, Texture::CLAMP);
        texturedDome->getMaterialProfile(0).getTechnique(0).getPass(0).setSpecular(Color(0.0f, 0.0f, 0.0f, 1.f));
        texturedDome->getMaterialProfile(0).getTechnique(0).getPass(0).setShine(1.f);
        texturedDome->initMaterialProfiles();

        //texturedDome->disable();
        getScene()->getHierarchyTree()->addNode(texturedDome);

        // signals
        m_appWindow->onUpdate.connect(this, &PrimitivesSample::onSceneUpdate);
        m_appWindow->onDraw.connect(this, &PrimitivesSample::onSceneDraw);
        m_appWindow->onClose.connect(this, &PrimitivesSample::onClose);
        m_appWindow->onKey.connect(this, &PrimitivesSample::onKey);
        m_appWindow->onMouseMotion.connect(this, &PrimitivesSample::onMouseMotion);
        //m_appWindow->onMouseButton.connect(this, &PrimitivesSample::onMouseButton);
        m_appWindow->onDestroy.connect(this, &PrimitivesSample::onDestroy);

        //getWindow()->grabMouse();
        //getWindow()->getInput().getMouse()->setMouseSmoother(True);
        //getWindow()->getInput().getMouse()->setSmootherPeriod(0.01);
	}

    virtual ~PrimitivesSample()
    {
	}

    AppWindow* getWindow() { return m_appWindow; }

    Scene* getScene() { return m_scene; }

    void onDestroy()
    {
        deletePtr(geom);

        deletePtr(primitive);
        deletePtr(solidPrimitive);
        deletePtr(cylinder);
        deletePtr(solidCylinder);
        deletePtr(sphere);
        deletePtr(solidSphere);
        //deletePtr(texturedSphere);
        deletePtr(surface);
        deletePtr(solidSurface);
        deletePtr(isoSphere);
        deletePtr(solidIsoSphere);
        deletePtr(dome);
        deletePtr(solidDome);
        //deletePtr(texturedDome);

        deletePtr(m_scene);
        deletePtr(m_glRenderer);

        m_appWindow->logFps();

        // it is deleted by the application
        m_appWindow = nullptr;
    }

    void onClose()
    {
        getWindow()->terminate();
    }

    void onSceneUpdate()
	{
        Keyboard * lpKeyboard = m_appWindow->getInput().getKeyboard();

        Float elapsed = getScene()->getFrameManager()->getFrameDuration();

		const float speed = 10.f;

		Float cam_t_z=0.f, cam_t_y=0.f, cam_t_x=0.f;

        if (lpKeyboard->isKeyDown(FORWARD)) cam_t_z = speed*-1.f*elapsed;
        if (lpKeyboard->isKeyDown(BACKWARD)) cam_t_z = speed*1.f*elapsed;

        if (lpKeyboard->isKeyDown(LEFT)) cam_t_x = speed*-1.f*elapsed;
        if (lpKeyboard->isKeyDown(RIGHT)) cam_t_x = speed*1.f*elapsed;

        if (lpKeyboard->isKeyDown(DOWN)) cam_t_y = speed*-1.f*elapsed;
        if (lpKeyboard->isKeyDown(UP)) cam_t_y = speed*1.f*elapsed;

        Camera *lpCamera = (Camera*)getScene()->getSceneObjectManager()->searchName("CameraFPS");
        lpCamera->getNode()->getTransform()->translate(Vector3(cam_t_x,cam_t_y,cam_t_z));

        // here we are synchrone to mouse smoother update, then we can use the delta value
        if (getWindow()->getInput().getMouse()->isMouseSmoother()) {
            lpCamera->getNode()->getTransform()->rotate(Y,-getWindow()->getInput().getMouse()->getSmoothedDelta().x()*0.01f);
            lpCamera->getNode()->getTransform()->rotate(X,-getWindow()->getInput().getMouse()->getSmoothedDelta().y()*0.01f);
        }
	}

    void onKey(Keyboard* keyboard, KeyEvent event)
    {
        if (event.isPressed() && (event.key() == KEY_ESCAPE)) {
            getWindow()->terminate();
        }

        if (event.isPressed() && (event.key() == KEY_SPACE)) {
			object++;
            deletePtr(geom);

            if (object == 14) {
                object = 0;
            }

            switch (object) {
                case 0:
                    texturedDome->disable();
                    geom = new GeometryData(m_scene, *primitive);
                    break;
                case 1:
                    geom = new GeometryData(m_scene, *solidPrimitive);
                    break;
                case 2:
                    geom = new GeometryData(m_scene, *cylinder);
                    break;
                case 3:
                    geom = new GeometryData(m_scene, *solidCylinder);
                    break;
                case 4:
                    geom = new GeometryData(m_scene, *sphere);
                    break;
                case 5:
                    geom = new GeometryData(m_scene, *solidSphere);
                    break;
                case 6:
                    texturedSphere->enable();
                    break;
                case 7:
                    texturedSphere->disable();
                    geom = new GeometryData(m_scene, *surface);
                    break;
                case 8:
                    geom = new GeometryData(m_scene, *solidSurface);
                    break;
                case 9:
                    geom = new GeometryData(m_scene, *isoSphere);
                    break;
                case 10:
                    geom = new GeometryData(m_scene, *solidIsoSphere);
                    break;
                case 11:
                    geom = new GeometryData(m_scene, *dome);
                    break;
                case 12:
                    geom = new GeometryData(m_scene, *solidDome);
                    break;
                case 13:
                    texturedDome->enable();
                    break;
                default:
                    break;
            }

            if (geom) {
                SmartArrayFloat colorArray(geom->getNumVertices()*4);
                for (UInt32 i = 0; i < colorArray.getNumElt(); ++i) {
                    colorArray.getData()[i] = 1.0f;
                }

                geom->createElement(V_COLOR_ARRAY, colorArray);

                geom->create();
                geom->bindFaceArray(0);
            }
        }

        if (event.isPressed() && (event.key() == KEY_C)) {
            getScene()->getContext()->setDrawingMode(Context::DRAWING_WIREFRAME);
        }

        if (event.isPressed() && (event.key() == KEY_X)) {
            getScene()->getContext()->setDrawingMode(Context::DRAWING_FILLED);
        }
	}

    void onSceneDraw()
    {
        m_scene->getPrimitiveManager()->bind();
        m_scene->getPrimitiveManager()->drawLocalAxis();

        if (geom) {
            Camera *lpCamera = (Camera*)getScene()->getSceneObjectManager()->searchName("CameraFPS");
            getScene()->getContext()->modelView().set(lpCamera->getModelviewMatrix());

            m_scene->getPrimitiveManager()->setModelviewProjection();

            geom->attribute(V_VERTICES_ARRAY, V_VERTICES_ARRAY);
            geom->attribute(V_COLOR_ARRAY, V_COLOR_ARRAY);

            geom->draw();

            getScene()->getContext()->disableVertexAttribArray(V_VERTICES_ARRAY);
            getScene()->getContext()->disableVertexAttribArray(V_COLOR_ARRAY);
        }

        m_scene->getPrimitiveManager()->unbind();
	}

    void onMouseMotion(Mouse* mouse)
    {
        if (!mouse->isMouseSmoother()) {
            Camera *lpCamera = (Camera*)getScene()->getSceneObjectManager()->searchName("CameraFPS");
            lpCamera->getNode()->getTransform()->rotate(Y,-mouse->getDeltaX()*0.01f);
            lpCamera->getNode()->getTransform()->rotate(X,-mouse->getDeltaY()*0.01f);
        }
	}

    static Int32 main()
    {
        // cleared log out file with new header
        Debug::instance()->setDefaultLog("primitives.log");
        Debug::instance()->getDefaultLog().clearLog();
        Debug::instance()->getDefaultLog().writeHeaderLog();

        MemoryManager::instance()->enableLog(MemoryManager::MEM_RAM,128);
        MemoryManager::instance()->enableLog(MemoryManager::MEM_GFX);

        DiskDir basePath("media");
        if (!basePath.exists()) {
            basePath.setPathName("../media");
            if (!basePath.exists()) {
                Application::message("Missing media content", "Error");
                return -1;
            }
        }

        PrimitivesSample *primitivesApp = new PrimitivesSample(basePath);

        Camera *lpFPSCamera = new Camera(primitivesApp->getScene());
        primitivesApp->getScene()->getViewPortManager()->addScreenViewPort(
                    lpFPSCamera, new Drawer(primitivesApp->getScene(), primitivesApp), 0);

        lpFPSCamera->setName("CameraFPS");
        lpFPSCamera->setZnear(0.25f);
        lpFPSCamera->setZfar(10000.0f);
        lpFPSCamera->computePerspective();
        lpFPSCamera->disableVisibility();

        Node *cameraNode = primitivesApp->getScene()->getHierarchyTree()->addNode(lpFPSCamera);
        FTransform *ltransfrom = new FTransform;

        ltransfrom->translate(Vector3(0.0f,0.f,8.0f));

        cameraNode->addTransform(ltransfrom);

        primitivesApp->getScene()->getContext()->setBackgroundColor(0.633f,0.792f,.914f,0.0f);

        Application::run();

        deletePtr(primitivesApp);

        // write a footer banner in log out file
        Debug::instance()->getDefaultLog().writeFooterLog();

        return 0;
    }
};

O3D_NOCONSOLE_MAIN(PrimitivesSample, O3D_DEFAULT_CLASS_SETTINGS)
