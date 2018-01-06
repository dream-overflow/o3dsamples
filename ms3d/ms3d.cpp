/**
 * @file ms3d.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2004-01-01
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include <o3d/core/memorymanager.h>

#include <o3d/core/keyboard.h>
#include <o3d/core/appwindow.h>
#include <o3d/core/main.h>
#include <o3d/core/display.h>
#include <o3d/core/dir.h>
#include <o3d/core/file.h>

#include <o3d/engine/utils/framemanager.h>
#include <o3d/engine/utils/ms3d.h>

#include <o3d/engine/context.h>
#include <o3d/engine/picking.h>

#include <o3d/engine/hierarchy/node.h>
#include <o3d/engine/hierarchy/hierarchytree.h>

#include <o3d/engine/feedbackviewport.h>
#include <o3d/engine/screenviewport.h>
#include <o3d/engine/viewportmanager.h>

#include <o3d/engine/scene/scene.h>
#include <o3d/engine/deferred/deferreddrawer.h>

#include <o3d/engine/object/skin.h>
#include <o3d/engine/object/meshdatamanager.h>

#include <o3d/engine/texture/texturemanager.h>
#include <o3d/engine/material/lambertmaterial.h>
#include <o3d/engine/material/ambientmaterial.h>
#include <o3d/engine/material/pickingmaterial.h>

#include <o3d/engine/scene/sceneobjectmanager.h>

#include <o3d/engine/animation/animationplayermanager.h>
#include <o3d/engine/animation/animationmanager.h>

#include <o3d/engine/effect/specialeffectsmanager.h>
#include <o3d/engine/effect/skybox.h>
#include <o3d/engine/effect/lenseffect.h>

#include <o3d/engine/object/primitive.h>
#include <o3d/engine/object/ftransform.h>
#include <o3d/engine/object/mtransform.h>
#include <o3d/engine/object/camera.h>
#include <o3d/engine/object/light.h>
#include <o3d/engine/lodstrategy.h>
#include <o3d/engine/renderer.h>

#include <o3d/core/localfile.h>
#include <o3d/core/wintools.h>

#include <o3d/gui/thememanager.h>
#include <o3d/gui/widgetmanager.h>
#include <o3d/gui/gui.h>

#include <o3d/physic/rigidbody.h>
#include <o3d/physic/gravityforce.h>
#include <o3d/physic/forcemanager.h>
#include <o3d/physic/physicentitymanager.h>

#define LIGHT1
#define LIGHT2
#define LIGHT3
#define LIGHT4
#define SYMBOLIC

using namespace o3d;

class KeyMapping
{
public:

    enum Keys
    {
        LEFT = 0,
        RIGHT,
        UP,
        DOWN = 3,
        FORWARD,
        BACKWARD
    };

    inline VKey left() const { return m_keys[LEFT]; }
    inline VKey right() const { return m_keys[RIGHT]; }
    inline VKey up() const { return m_keys[UP]; }
    inline VKey down() const { return m_keys[DOWN]; }
    inline VKey forward() const { return m_keys[FORWARD]; }
    inline VKey backward() const { return m_keys[BACKWARD]; }

    inline VKey key(UInt32 c) { return m_keys[c]; }

    inline UInt32 type() { return m_type; }

protected:

    UInt32 m_type;

    VKey m_keys[255];
};

struct KeyMapAzerty : public KeyMapping
{
public:

    KeyMapAzerty()
    {
        m_type = 0;

        m_keys[LEFT] = KEY_S;
        m_keys[RIGHT] = KEY_F;

        m_keys[UP] = KEY_A;
        m_keys[DOWN] = KEY_Q;

        m_keys[FORWARD] = KEY_E;
        m_keys[BACKWARD] = KEY_D;
    }
};

struct KeyMapQwerty : public KeyMapping
{
public:

    KeyMapQwerty()
    {
        m_type = 1;

        m_keys[LEFT] = KEY_S;
        m_keys[RIGHT] = KEY_F;

        m_keys[UP] = KEY_Q;
        m_keys[DOWN] = KEY_W;

        m_keys[FORWARD] = KEY_E;
        m_keys[BACKWARD] = KEY_D;
    }
};

struct KeyMapBepo : public KeyMapping
{
public:

    KeyMapBepo()
    {
        m_type = 2;

        m_keys[LEFT] = KEY_U;
        m_keys[RIGHT] = KEY_E;

        m_keys[UP] = KEY_B;
        m_keys[DOWN] = KEY_A;

        m_keys[FORWARD] = KEY_P;
        m_keys[BACKWARD] = KEY_I;
    }
};

/**
 * @brief The Ms3dSample class.
 * @date 2004-01-01
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 */
class Ms3dSample : public EvtHandler
{
private:

    AppWindow *m_appWindow;
    Renderer* m_glRenderer;
    Scene *m_scene;
    Gui *m_gui;
    AutoPtr<KeyMapping> m_keys;

public:

    Ms3dSample(Dir &basePath)
	{
        m_keys = new KeyMapAzerty;

        // Create a new window
        m_appWindow = new AppWindow;

        // OpenGL renderer
        m_glRenderer = new Renderer;

        m_appWindow->setTitle("Objective-3D Ms3d sample");
        m_appWindow->create(800, 600, AppWindow::COLOR_RGBA8, AppWindow::DEPTH_24_STENCIL_8, AppWindow::MSAA4X, False, True);

        // @todo init debug mode crash on Windows
        m_glRenderer->create(m_appWindow); //, True);
        // m_glRenderer->setDebug();
        // m_glRenderer->setVSyncMode(Renderer::VSYNC_YES);

        // create a scene and attach it to the window
        m_scene = new Scene(nullptr, basePath.getFullPathName(), m_glRenderer);
        m_scene->setSceneName("ms3d");
        m_scene->defaultAttachment(m_appWindow);

        // new gui manager and attach it to the scene
        m_gui = new Gui(m_scene);
        m_scene->setGui(m_gui);
        m_gui->defaultAttachment(m_appWindow);

        //getWindow()->grabMouse();
        //getWindow()->grabKeyboard();
        getWindow()->setMinSize(Vector2i(320, 240));
        //getWindow()->resize(1680, 1050);
        //getWindow()->setFullScreen(True);
        //getWindow()->setFullScreen(False);

		// We listen synchronously to each update event coming from the main window.
		// The first parameter is an helper macro that take :
		// - Object class name to listen
		// - Instance of the object to listen
		// - Name of the signal to listen
		// - Instance of the receiver (in our case this)
		// - Method called on an event
        m_appWindow->onUpdate.connect(this, &Ms3dSample::onSceneUpdate);
        m_appWindow->onDraw.connect(this, &Ms3dSample::onSceneDraw);
        m_appWindow->onClose.connect(this, &Ms3dSample::onClose);
        m_appWindow->onKey.connect(this, &Ms3dSample::onKey);
        m_appWindow->onMouseMotion.connect(this, &Ms3dSample::onMouseMotion);
        m_appWindow->onMouseButton.connect(this, &Ms3dSample::onMouseButton);
        m_appWindow->onTouchScreenMotion.connect(this, &Ms3dSample::onTouchScreenMotion);
        m_appWindow->onTouchScreenChange.connect(this, &Ms3dSample::onTouchScreenChange);
        m_appWindow->onDestroy.connect(this, &Ms3dSample::onDestroy);

		// Notice that update and draw event of the window are thrown by two timers.
		// And that it is possible to change easily these timings.

        File iconFile(basePath.makeFullFileName("icon.bmp"));
        if (iconFile.exists()) {
            getWindow()->setIcon(iconFile.getFullFileName());
        }

        getScene()->setGlobalAmbient(Color(0.8f, 0.8f, 0.8f, 1.0f));

        // Create a camera into the scene. You notice that we always need a parent
        // to build an object. We simply set directly the scene as its parent.
        // We can too add our camera into the getSceneObjectManager() of the scene,
        // but it is useless in this example.
        Camera *lpCamera = new Camera(getScene());

        // A viewport is a part of the screen. Each viewport might be attached to a camera.
        // We can overload the drawing callback, and define a displaying priority.
        // Because the viewport manager provide the method to create and add a viewport
        // we don't need to set its parent. In this case its parent is the viewport manager.
        ScreenViewPort *pViewPort = getScene()->getViewPortManager()->addScreenViewPort(
                                        lpCamera,
                                        nullptr,
                                        0);

        Texture2D *lpTexture = new Texture2D(getScene());
        lpTexture->create(False, 800, 600, PF_RGBA_U8);

        FeedbackViewPort *pFbViewPort = getScene()->getViewPortManager()->addFeedbackViewPort(
                    lpCamera,
                    new DeferredDrawer(getScene()),
                    lpTexture,
                    0);

        // default to deferred or forward
        pFbViewPort->disable();
        // pViewPort->disable();

        GBuffer *gbuffer = new GBuffer(pFbViewPort);
        gbuffer->create(800, 600, 1);
        ((DeferredDrawer*)(pFbViewPort)->getSceneDrawer())->setGBuffer(gbuffer);

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
        Node *cameraNode = getScene()->getHierarchyTree()->addNode(lpCamera);
        // We also need a first view person transformation on this node
        FTransform *ltransfrom = new FTransform;
        cameraNode->addTransform(ltransfrom);

        // Initial position at (5,50,120) {x,y,z} with Y+ up screen and Z+ comes to you
        ltransfrom->translate(Vector3(0.0f, 50.f, 120.0f));

        // Change the clear color
        getScene()->getContext()->setBackgroundColor(0.633f, 0.792f, 0.914f, 0.0f);

        // Set this parameter to True if you want to visualize the bones
        getScene()->setDrawObject(Scene::DRAW_BONES, True);
        // Set the parameter to True if you want to visualize the bounding volumes
        getScene()->setDrawObject(Scene::DRAW_BOUNDING_VOLUME, True);
        // Set the parameter to False if you want to disable the rendering step of any skinned meshes
        getScene()->setDrawObject(Scene::DRAW_SKIN, True);
    #ifndef SYMBOLIC
        getScene()->hideAllSymbolicObject();
    #endif

        //
        // light 1
        //

        #ifdef LIGHT1
        // create a spot light
        Light *light1 = new Light(getScene(), Light::SPOT_LIGHT);
        light1->setName("light1");
        light1->setAmbient(0.0f ,0.0f, 0.0f, 1.f);
        //light1->setAmbient(0.2f, 0.2f, 0.2f, 1.f);
        light1->setDiffuse(0.0f, 1.0f, 0.0f, 1.f);
        light1->setSpecular(0.4f, 0.4f, 0.4f, 1.f);
        light1->setAttenuation(1.f, 0.0f, 0.0002f);
        light1->setExponent(0.3f);
        light1->setCutOff(15.f);

        // Create a new node to contain our new light
        Node *lightNode1 = getScene()->getHierarchyTree()->addNode(light1);
        // We also need a first view person transformation on this node
        MTransform *lightTransform1 = new MTransform;
        lightNode1->addTransform(lightTransform1);

        // Initial position at (50,50,50) {x,y,z} with Y+ up screen and Z+ comes to you
        lightTransform1->setPosition(Vector3(50.0f, 50.0f, 50.0f));
        lightTransform1->setDirectionZ(Vector3(-1.0f, -1.0f, -1.f));
        #endif

        //
        // light 2
        //

        #ifdef LIGHT2
        // create a second spot light
        Light *light2 = new Light(getScene(), Light::SPOT_LIGHT);
        light2->setName("light2");
        light2->setAmbient(0.0f, 0.0f, 0.0f, 1.f);
        //light2->setAmbient(0.2f, 0.2f, 0.2f, 1.f);
        light2->setDiffuse(1.0f, 0.0f, 0.0f, 1.f);
        light2->setSpecular(0.4f, 0.4f, 0.4f, 1.f);
        light2->setAttenuation(1.f, 0.0f, 0.0002f);
        light2->setExponent(0.3f);
        light2->setCutOff(15.f);

        // Create a new node to contain our new light
        Node *lightNode2 = getScene()->getHierarchyTree()->addNode(light2);
        // We also need a first view person transformation on this node
        MTransform *lightTransform2 = new MTransform;
        lightNode2->addTransform(lightTransform2);

        // Initial position at (50,50,50) {x,y,z} with Y+ up screen and Z+ comes to you
        lightTransform2->setPosition(Vector3(-50.0f, 50.f, 50.0f));
        lightTransform2->setDirectionZ(Vector3(1.0f, -1.0f, -1.f));
        #endif

        //
        // light 3
        //

        #ifdef LIGHT3
        // create a third point light
        Light *light3 = new Light(getScene(), Light::POINT_LIGHT);
        light3->setName("light3");
        light3->setAmbient(0.0f, 0.0f, 0.0f, 1.f);
        light3->setDiffuse(0.2f ,0.2f, 1.0f, 1.f);
        light3->setSpecular(0.4f ,0.4f, 0.4f, 1.f);
        light3->setAttenuation(1.f, 0.0001f, 0.00005f);

        // Create a new node to contain our new light
        Node *lightNode3 = getScene()->getHierarchyTree()->addNode(light3);
        // We also need a first view person transformation on this node
        MTransform *lightTransform3 = new MTransform;
        lightNode3->addTransform(lightTransform3);

        // Initial position at (50,50,50) {x,y,z} with Y+ up screen and Z+ comes to you
        lightTransform3->setPosition(Vector3(0.0f, 200.f, 0.0f));
        //lightTransform3->setDirectionZ(Vector3(0.0f, -1.0f, 0.f));
        #endif

        //
        // light 4
        //

        #ifdef LIGHT4
        // create a directionnal light
        Light *light4 = new Light(getScene(), Light::DIRECTIONAL_LIGHT);
        light4->setName("light4");
        light4->setAmbient(0.0f ,0.0f, 0.0f, 1.f);
        light4->setDiffuse(0.1f, 0.1f, 0.1f, 1.f);
        light4->setSpecular(0.5f, 0.5f, 0.5f, 1.f);

        // Create a new node to contain our new light
        Node *lightNode4 = getScene()->getHierarchyTree()->addNode(light4);
        // We also need a first view person transformation on this node
        MTransform *lightTransform4 = new MTransform;
        lightNode4->addTransform(lightTransform4);

        // Y- down screen and Z+ comes to you
        lightTransform4->setDirectionZ(Vector3(0, -1.0f, 0.0f));
        #endif

        //
        // plane ground
        //

        // Add a simple plane for simulate a ground to project shadow on
        Surface surface(1000, 1000, 4, 4, Surface::FILLED_MODE | Surface::ALTERNATE_TRIANGLE);
        Mesh *meshSurface = new Mesh(getScene());
        meshSurface->setName("plane");
        MeshData *meshData = new MeshData(getScene());

        GeometryData *surfaceGeometry = new GeometryData(meshData, surface);
        surfaceGeometry->genNormals();
        //surfaceGeometry->genTangentSpace();

        meshData->setGeometry(surfaceGeometry);
        meshData->computeBounding(GeometryData::BOUNDING_BOX);
        meshData->createGeometry();

        meshSurface->setMeshData(meshData);

        meshSurface->setNumMaterialProfiles(1);
        meshSurface->getMaterialProfile(0).setNumTechniques(1);
        meshSurface->getMaterialProfile(0).getTechnique(0).setNumPass(1);
        meshSurface->getMaterialProfile(0).getTechnique(0).getPass(0).setMaterial(Material::AMBIENT, new AmbientMaterial(getScene()));
        meshSurface->getMaterialProfile(0).getTechnique(0).getPass(0).setMaterial(Material::LIGHTING, new LambertMaterial(getScene()));
        meshSurface->getMaterialProfile(0).getTechnique(0).getPass(0).setMaterial(Material::PICKING, new PickingMaterial(getScene()));
        meshSurface->getMaterialProfile(0).getTechnique(0).getPass(0).setMaterial(Material::DEFERRED, new LambertMaterial(getScene()));
        meshSurface->getMaterialProfile(0).getTechnique(0).getPass(0).setAmbient(Color(0.0f, 0.0f, 0.0f, 1.f));
        meshSurface->getMaterialProfile(0).getTechnique(0).getPass(0).setDiffuse(Color(1.0f, 1.0f, 1.0f, 1.f));
        meshSurface->getMaterialProfile(0).getTechnique(0).getPass(0).setSpecular(Color(0.0f, 0.0f, 0.0f, 1.f));
        meshSurface->getMaterialProfile(0).getTechnique(0).getPass(0).setShine(1.f);
        meshSurface->initMaterialProfiles();

        Node *surfaceNode = getScene()->getHierarchyTree()->addNode(meshSurface);

        // a bit of physic to the plane to compute collisions
        RigidBody *surfaceRigidBody = new RigidBody(surfaceNode);
        getScene()->getPhysicEntityManager()->addElement(surfaceRigidBody);

        //
        // cube or sphere object
        //

        // Add a simple box or a sphere
    //	Cube cube(50, 1);
    //	Cylinder cube(25, 25, 50, 16, 2);
        Sphere cube(25, 16, 16);
        Mesh *meshCube = new Mesh(getScene());
        meshCube->setName("shadowCaster");
        meshData = new MeshData(getScene());

        GeometryData *cubeGeometry = new GeometryData(meshData, cube);
        cubeGeometry->genNormals();
        //cubeGeometry->genTangentSpace();

        meshData->setGeometry(cubeGeometry);
        meshData->computeBounding(GeometryData::BOUNDING_SPHERE);
        meshData->createGeometry();

        meshCube->setMeshData(meshData);

        std::vector<Float> lodLevels;
        lodLevels.push_back(0.f);
        lodLevels.push_back(100.f);

        meshCube->setNumMaterialProfiles(1);
        meshCube->getMaterialProfile(0).setLodStrategy(new LodStrategy());
        meshCube->getMaterialProfile(0).setLodLevels(lodLevels);
        meshCube->getMaterialProfile(0).setNumTechniques(2);

        meshCube->getMaterialProfile(0).getTechnique(0).setNumPass(1);
        meshCube->getMaterialProfile(0).getTechnique(0).setLodIndex(0);
        meshCube->getMaterialProfile(0).getTechnique(0).getPass(0).setMaterial(Material::AMBIENT, new AmbientMaterial(getScene()));
        meshCube->getMaterialProfile(0).getTechnique(0).getPass(0).setMaterial(Material::LIGHTING, new LambertMaterial(getScene()));
        meshCube->getMaterialProfile(0).getTechnique(0).getPass(0).setMaterial(Material::PICKING, new PickingMaterial(getScene()));
        meshCube->getMaterialProfile(0).getTechnique(0).getPass(0).setMaterial(Material::DEFERRED, new LambertMaterial(getScene()));
        meshCube->getMaterialProfile(0).getTechnique(0).getPass(0).setAmbient(Color(0.3f, 0.3f, 0.3f, 1.f));
        meshCube->getMaterialProfile(0).getTechnique(0).getPass(0).setDiffuse(Color(1.0f, 1.0f, 1.0f, 1.f));
        meshCube->getMaterialProfile(0).getTechnique(0).getPass(0).setSpecular(Color(0.5f, 0.5f, 0.5f, 1.f));
        meshCube->getMaterialProfile(0).getTechnique(0).getPass(0).setShine(1000.f);

        meshCube->getMaterialProfile(0).getTechnique(1).setNumPass(1);
        meshCube->getMaterialProfile(0).getTechnique(1).setLodIndex(1);
        meshCube->getMaterialProfile(0).getTechnique(1).getPass(0).setMaterial(Material::AMBIENT, new AmbientMaterial(getScene()));
        meshCube->getMaterialProfile(0).getTechnique(1).getPass(0).setMaterial(Material::LIGHTING, new LambertMaterial(getScene()));
        meshCube->getMaterialProfile(0).getTechnique(1).getPass(0).setMaterial(Material::PICKING, new PickingMaterial(getScene()));
        meshCube->getMaterialProfile(0).getTechnique(1).getPass(0).setMaterial(Material::DEFERRED, new LambertMaterial(getScene()));
        meshCube->getMaterialProfile(0).getTechnique(1).getPass(0).setAmbient(Color(0.3f, 0.3f, 0.3f, 1.f));
        meshCube->getMaterialProfile(0).getTechnique(1).getPass(0).setDiffuse(Color(1.0f, 0.0f, 0.0f, 1.f));
        meshCube->getMaterialProfile(0).getTechnique(1).getPass(0).setSpecular(Color(0.5f, 0.5f, 0.5f, 1.f));
        meshCube->getMaterialProfile(0).getTechnique(1).getPass(0).setShine(1000.f);

        meshCube->initMaterialProfiles();

        meshCube->enableShadowCast();

        Node *cubeNode = getScene()->getHierarchyTree()->addNode(meshCube);
        cubeNode->addTransform(new MTransform());
        cubeNode->getTransform()->setPosition(Vector3(0.f, 135.f, 0.f));
        cubeNode->getTransform()->rotate(Y,3.14f/4);

        // Import an MS3D animated mesh
        //getScene()->importScene(basePath.makeFullFileName("models/Sample ms3d.o3dsc"), nullptr);

        // Use the next line to enable asynchronous texture loading
        //getScene()->getTextureManager()->enableAsynchronous();

        //
        // import the dwarf1.ms3d
        //

        Ms3dSettings settings;
        Ms3dResult result;
        settings.setResultContainer(&result);
        settings.setBoundingVolumeGen(GeometryData::BOUNDING_FAST);
        Ms3d::import(getScene(), basePath.makeFullFileName("models/dwarf1.ms3d"), settings);

        // We change the duration of the animation to 22 seconds
        result.getAnimation()->setDuration(22.f);
        // And set the frame rate to 30f/s
        result.getAnimationPlayer()->setFramePerSec(30);

        result.getAnimation()->addAnimRange("walk", 2, 14);
        result.getAnimation()->addAnimRange("run", 16, 26);
        result.getAnimation()->addAnimRange("jump", 28, 40);
        result.getAnimation()->addAnimRange("jumpSpot", 42, 54);
        result.getAnimation()->addAnimRange("crouchDown", 56, 59);
        result.getAnimation()->addAnimRange("stayCrouchedLoop", 60, 69);
        result.getAnimation()->addAnimRange("getUp", 70, 74);
        result.getAnimation()->addAnimRange("battleIdle1", 75, 88);
        result.getAnimation()->addAnimRange("battleIdle2", 90, 110);
        result.getAnimation()->addAnimRange("attack1SwipeAxe", 112, 126, 126);  // this animation cannot be broken before the end
        result.getAnimation()->addAnimRange("attack2Jump", 128, 142);
        result.getAnimation()->addAnimRange("attack3Spin360", 144, 160);
        result.getAnimation()->addAnimRange("attack4Swipes", 162, 180);
        result.getAnimation()->addAnimRange("attack5Stab", 182, 192);
        result.getAnimation()->addAnimRange("block", 194, 210);
        result.getAnimation()->addAnimRange("die1Forwards", 212, 227);
        result.getAnimation()->addAnimRange("die2Backwards", 230, 251);
        result.getAnimation()->addAnimRange("nodYes", 253, 272);
        result.getAnimation()->addAnimRange("shakeHeadNo", 274, 290);
        result.getAnimation()->addAnimRange("idle1", 292, 325);
        result.getAnimation()->addAnimRange("idle2", 327, 360);

        // finally we need to setup animation range for the tracks
        result.getAnimation()->computeAnimRange();

        // and start with player the idle1 animation in loop.
        result.getAnimationPlayer()->playAnimRange("idle1");

        Rigging *rigging = o3d::dynamicCast<Rigging*>(result.getMesh());
        if (rigging) {
            //rigging->setCPUMode();
            rigging->enablePicking();
//			rigging->enableShadowCast();
        }

        // a bit of physic to the dwarf
        RigidBody *dwarfRigidBody = new RigidBody(result.getRootNode());
        getScene()->getPhysicEntityManager()->addElement(dwarfRigidBody);

        dwarfRigidBody->setUpMassSphere(1.0f, 5.f);

        GravityForce *gravityForce = new GravityForce(getScene(), Vector3(0.f, -98.1f, 0.f));
        //getScene()->getPhysicEntityManager()->getForceManager().addElement(gravityForce);
        ForceManager *dwarfForceManager = new ForceManager(dwarfRigidBody);
        dwarfRigidBody->setForceManager(dwarfForceManager);
        dwarfRigidBody->getForceManager()->addElement(gravityForce);

        // define the specular for each material
        UInt32 numProfiles = result.getMesh()->getNumMaterialProfiles();
        for (UInt32 i = 0; i < numProfiles; ++i) {
            result.getMesh()->getMaterialProfile(i).setSpecular(Color(0.8f, 0.8f, 0.8f, 1.f));
            result.getMesh()->getMaterialProfile(i).setShine(100.f);
        }

        // animation control set to the dwarf
        setAnimationPlayer(result.getAnimationPlayer());

        //
        // import the monster.ms3d
        //

        Ms3d::import(getScene(), basePath.makeFullFileName("models/monster.ms3d"), settings);

        result.getAnimation()->addAnimRange("walk", 0, 120, 30);
        result.getAnimation()->addAnimRange("run", 150, 210, 190);
        result.getAnimation()->addAnimRange("attack01", 250, 333);
        result.getAnimation()->addAnimRange("attack02", 320, 400);
        result.getAnimation()->addAnimRange("death01", 390, 418);
        result.getAnimation()->addAnimRange("growl", 478, 500);
        result.getAnimation()->addAnimRange("death02", 500, 550);
        result.getAnimation()->addAnimRange("death03", 565, 650);

        result.getAnimation()->computeAnimRange();
        result.getAnimationPlayer()->playAnimRange("attack02");

        // And set the frame rate to 30f/s
        result.getAnimationPlayer()->setFramePerSec(30);

        rigging = dynamicCast<Rigging*>(result.getMesh());
        if (rigging) {
            rigging->enablePicking();

            MTransform *mtransform = new MTransform(rigging->getNode());
            rigging->getNode()->addTransform(mtransform);

            mtransform->translate(Vector3(60.f, 0, 45));
        }

        // define the specular for each material
        UInt32 numMaterials = result.getMesh()->getNumMaterialProfiles();
        for (UInt32 i = 0; i < numMaterials; ++i) {
            MaterialProfile &material = result.getMesh()->getMaterialProfile(i);
            //material.setAmbient(Color(0.5f, 0.5f, 0.5f, 1.f));
            material.setDiffuse(Color(0.8f, 0.8f, 0.8f, 1.f));
            material.setSpecular(Color(0.8f, 0.8f, 0.8f, 1.f));
            material.setShine(100.f);
        }

        // setAnimationPlayer(result->getAnimationPlayer());

        // Enable the color picking mode.
        getScene()->getPicking()->setMode(Picking::COLOR);

        // This camera is used to compute some unprojection (useful for GetPointerPos or GetHitPos).
        getScene()->getPicking()->setCamera(lpCamera);

        // We need a mouse look to pick on the screen, so simply load a GUI theme
        Theme *theme = getGui()->getThemeManager()->addTheme(basePath.makeFullFileName("gui/revolutioning.xml"));

        // and set it as the default theme to use
        getGui()->getWidgetManager()->setDefaultTheme(theme);

        //
        // finally, why not to add a simple skybox ?
        //

        SkyBox *skyBox = new SkyBox(getScene());
        skyBox->setName("skyBox");
        skyBox->create(
                2048.f,
                "sky01_xp.jpg",
                "sky01_xn.jpg",
                "sky01_yp.jpg",
                "", // no Y down
                "sky01_zp.jpg",
                "sky01_zn.jpg",
                True,
                Texture::TRILINEAR_ANISOTROPIC,
                4.f);
        getScene()->getSpecialEffectsManager()->addSpecialEffects(skyBox);

        //
        // and a marvelous lens flare ?
        //

        LensFlareModel lensFlareModel;

        lensFlareModel.setSizeX(10.0f);
        lensFlareModel.setSizeY(10.0f);
        lensFlareModel.setMaxDistance(100.0f);
        lensFlareModel.setMinDistance(0.0f);
        lensFlareModel.setMaxFadeRange(30.0f);
        lensFlareModel.setMinFadeRange(10.0f);
        lensFlareModel.setFadeInPersistence(0.1f);
        lensFlareModel.setFadeOutPersistence(0.2f);
        lensFlareModel.setSimpleOcclusion(False);

        // flare0
        lensFlareModel.addFlare(getScene()->getTextureManager()->addTexture2D("Flare5.bmp", True),0,0,0);
        lensFlareModel.getFlare(0)->color.set(0.8f, 0.6f, 0.2f, 0.6f);
        lensFlareModel.getFlare(0)->halfSizeX = 7.0f;
        lensFlareModel.getFlare(0)->halfSizeY = 7.0f;
        lensFlareModel.getFlare(0)->position = 0.7f;
        lensFlareModel.getFlare(0)->attenuationRange = 1.0f;

        // flare1
        lensFlareModel.addFlare(getScene()->getTextureManager()->addTexture2D("Flare1.bmp", True),0,0,0);
        lensFlareModel.getFlare(1)->color.set(0.2f, 0.6f, 1.0f, 1.0f);
        lensFlareModel.getFlare(1)->halfSizeX = 5.0f;
        lensFlareModel.getFlare(1)->halfSizeY = 5.0f;
        lensFlareModel.getFlare(1)->position = 0.5f;
        lensFlareModel.getFlare(1)->attenuationRange = 1.0f;

        // flare2
        lensFlareModel.addFlare(getScene()->getTextureManager()->addTexture2D("Flare6.bmp", True),0,0,0);
        lensFlareModel.getFlare(2)->color.set(0.5f, 0.8f, 0.2f, 0.9f);
        lensFlareModel.getFlare(2)->halfSizeX = 10.0f;
        lensFlareModel.getFlare(2)->halfSizeY = 10.0f;
        lensFlareModel.getFlare(2)->position = 0.4f;

        // flare3
        lensFlareModel.addFlare(getScene()->getTextureManager()->addTexture2D("Flare2.bmp", True),0,0,0);
        lensFlareModel.getFlare(3)->color.set(0.9f, 0.4f, 0.1f, 1.0f);
        lensFlareModel.getFlare(3)->halfSizeX = 5.0f;
        lensFlareModel.getFlare(3)->halfSizeY = 5.0f;
        lensFlareModel.getFlare(3)->position = 0.25f;
        lensFlareModel.getFlare(3)->attenuationRange = 1.0f;

        // flare4
        lensFlareModel.addFlare(getScene()->getTextureManager()->addTexture2D("Flare2.bmp", True),0,0,0);
        lensFlareModel.getFlare(4)->color.set(1.0f, 1.0f, 0.1f, 1.0f);
        lensFlareModel.getFlare(4)->halfSizeX = 5.0f;
        lensFlareModel.getFlare(4)->halfSizeY = 5.0f;
        lensFlareModel.getFlare(4)->position = 0.12f;
        lensFlareModel.getFlare(4)->attenuationRange = 1.0f;

        // flare5
        lensFlareModel.addFlare(getScene()->getTextureManager()->addTexture2D("Flare2.bmp", True),0,0,0);
        lensFlareModel.getFlare(5)->color.set(1.0f, 0.7f, 0.1f, 1.0f);
        lensFlareModel.getFlare(5)->halfSizeX = 4.0f;
        lensFlareModel.getFlare(5)->halfSizeY = 4.0f;
        lensFlareModel.getFlare(5)->position = 0.05f;
        lensFlareModel.getFlare(5)->attenuationRange = 1.0f;

        // flare6
        lensFlareModel.addFlare(getScene()->getTextureManager()->addTexture2D("Flare4.bmp", True),0,0,0);
        lensFlareModel.getFlare(6)->color.set(1.0f, 0.5f, 0.1f, 0.4f);
        lensFlareModel.getFlare(6)->halfSizeX = 7.5f;
        lensFlareModel.getFlare(6)->halfSizeY = 7.5f;
        lensFlareModel.getFlare(6)->position = -0.2f;
        lensFlareModel.getFlare(6)->attenuationRange = 1.0f;

        // flare7
        lensFlareModel.addFlare(getScene()->getTextureManager()->addTexture2D("Flare2.bmp", True),0,0,0);
        lensFlareModel.getFlare(7)->color.set(0.0f, 0.5f, 1.0f, 1.0f);
        lensFlareModel.getFlare(7)->halfSizeX = 4.5f;
        lensFlareModel.getFlare(7)->halfSizeY = 4.5f;
        lensFlareModel.getFlare(7)->position = -0.4f;
        lensFlareModel.getFlare(7)->attenuationRange = 1.0f;

        // flare8
        lensFlareModel.addFlare(getScene()->getTextureManager()->addTexture2D("Flare5.bmp", True),0,0,0);
        lensFlareModel.getFlare(8)->color.set(1.0f, 1.0f, 0.0f, 1.0f);
        lensFlareModel.getFlare(8)->halfSizeX = 3.0f;
        lensFlareModel.getFlare(8)->halfSizeY = 3.0f;
        lensFlareModel.getFlare(8)->position = -0.58f;
        lensFlareModel.getFlare(8)->attenuationRange = 1.0f;

        // flare9
        lensFlareModel.addFlare(getScene()->getTextureManager()->addTexture2D("Flare2.bmp", True),0,0,0);
        lensFlareModel.getFlare(9)->color.set(1.0f, 0.5f, 0.5f, 1.0f);
        lensFlareModel.getFlare(9)->halfSizeX = 7.5f;
        lensFlareModel.getFlare(9)->halfSizeY = 7.5f;
        lensFlareModel.getFlare(9)->position = -0.9f;
        lensFlareModel.getFlare(9)->attenuationRange = 1.0f;

        // glow0
        lensFlareModel.addGlow(getScene()->getTextureManager()->addTexture2D("Flare1.bmp", True),0,0,0);
        lensFlareModel.getGlow(0)->color.set(0.9f,0.9f,0.32f,1.0f);
        lensFlareModel.getGlow(0)->halfSizeX = 35.0f;
        lensFlareModel.getGlow(0)->halfSizeY = 35.0f;
        lensFlareModel.getGlow(0)->attenuationRange = 1.0f;
        lensFlareModel.getGlow(0)->minIntensity = 0.5f;
        lensFlareModel.getGlow(0)->isBehindEffect = True;

        // glow1
        lensFlareModel.addGlow(getScene()->getTextureManager()->addTexture2D("Flare1.bmp", True),0,0,0);
        lensFlareModel.getGlow(1)->color.set(0.85f,0.85f,0.3f,0.5f);
        lensFlareModel.getGlow(1)->halfSizeX = 40.0f;
        lensFlareModel.getGlow(1)->halfSizeY = 40.0f;
        lensFlareModel.getGlow(1)->attenuationRange = 1.0f;
        lensFlareModel.getGlow(1)->isBehindEffect = False;

        // glow2
        lensFlareModel.addGlow(getScene()->getTextureManager()->addTexture2D("Shine7.bmp", True),0,0,0);
        lensFlareModel.getGlow(2)->color.set(1.0f,0.85f,0.32f,0.4f);
        lensFlareModel.getGlow(2)->halfSizeX = 90.f;
        lensFlareModel.getGlow(2)->halfSizeY = 90.f;
        lensFlareModel.getGlow(2)->attenuationRange = 1.0f;
        lensFlareModel.getGlow(2)->isBehindEffect = False;

        // glow3
        lensFlareModel.addGlow(getScene()->getTextureManager()->addTexture2D("Flare1.bmp", True),0,0,0);
        lensFlareModel.getGlow(3)->color.set(0.95f,0.95f,0.32f,0.7f);
        lensFlareModel.getGlow(3)->halfSizeX = 150.0f;
        lensFlareModel.getGlow(3)->halfSizeY = 100.0f;
        lensFlareModel.getGlow(3)->attenuationRange = 0.5f;
        lensFlareModel.getGlow(3)->isBehindEffect = False;

        LensEffect *lensEffect = new LensEffect(getScene(), lensFlareModel, True);
        lensEffect->setName("sunLensFlare");
        getScene()->getSpecialEffectsManager()->addSpecialEffects(lensEffect);

        lensEffect->setDirection(Vector3(0.f,0.5f,-1.0f));

        //getScene()->exportScene(basePath.makeFullFileName("models"), SceneIO());
	}

	virtual ~Ms3dSample()
	{
        if (m_appWindow) {
            onDestroy();
        }
	}

    void onDestroy()
    {
        if (!m_appWindow) {
            return;
        }

        deletePtr(m_scene);
        deletePtr(m_glRenderer);

        this->getWindow()->logFps();

        // it is deleted by the application
        m_appWindow = nullptr;
    }

    AppWindow* getWindow() { return m_appWindow; }
    Scene* getScene() { return m_scene; }
    Gui* getGui() { return m_gui; }

	// Method called on main window update
	void onSceneUpdate()
	{
		// Get the time (in ms) elapsed since the last update
		Float elapsed = getScene()->getFrameManager()->getFrameDuration();

		// move the camera using ESDFQA
		BaseNode *cameraNode = getScene()->getSceneObjectManager()->searchName("Camera")->getNode();
        if (cameraNode) {
            cameraNode->getTransform()->translate(m_camVelocity*elapsed);
		}

		// Search into the scene object manager our node that containing the animated mesh.
		// When loading an MS3D file the node have the name of the file without its extension.
		// This is not a very good way, because it need to search a string into a hash map.
		// Notice the usage of o3d::dynamicCast that take the pointer of the object and the type
		// to cast. It's like a dynamic_cast operator with RTTI, but it doesn't use the C++ RTTI.
		Node *dwarf = dynamicCast<Node*>(getScene()->getSceneObjectManager()->searchName("dwarf1"));
		//Node *dwarf = o3d::dynamicCast<Node*>(getScene()->getSceneObjectManager()->searchName("monster"));
        if (dwarf) {
            Float run = o3d::abs(dwarf->getRigidBody()->getSpeed().x() + dwarf->getRigidBody()->getSpeed().z());

            if ((run >= 0.1f) && (m_animationPlayer->getAnimRangeName() == "idle1")) {
                m_animationPlayer->playAnimRange("walk");
            } else if ((run <= 0.1f) && (m_animationPlayer->getAnimRangeName() == "walk")) {
                m_animationPlayer->playAnimRange("idle1");
            }

			// We want to apply the rotation to this node, but by default there is no
			// transformation, so we create ones if necessary.
            if (!dwarf->getTransform()) {
				// Its a simple matrix transform using a {Position,Quaternion,Scale} uplet
				// that is transformed into a 4x4 matrix.
				MTransform *transform = new MTransform;
				dwarf->addTransform(transform);
			}

            // simulate the plane collision
            if (dwarf->getRigidBody()->getPosition().y() < 0.0f) {
                Vector3 pos = dwarf->getRigidBody()->getPosition();
                Vector3 speed = dwarf->getRigidBody()->getSpeed();
                pos.y() = 0.0f;
                speed.y() = 0.0f;
                dwarf->getRigidBody()->setSpeed(speed);
                dwarf->getRigidBody()->setPosition(pos);
            }

			// Rotate on the Y axis
            if (m_dwarfRotVelocity.y() != 0.f) {
                dwarf->getTransform()->rotate(Y, m_dwarfRotVelocity.y()*elapsed);
            }

            // Rotate on the X axis
            if (m_dwarfRotVelocity.x() != 0.f) {
                dwarf->getTransform()->rotate(X, m_dwarfRotVelocity.x()*elapsed);
            }
		}
	}

	void onSceneDraw()
	{
		// Check for a hit
        if (getScene()->getPicking()->getSingleHit()) {
			// Dynamic cast to scene object, or you can use two static_cast.
			// Casts are tested using pointer and reference.
			SceneObject *object = o3d::dynamicCast<SceneObject*>(getScene()->getPicking()->getSingleHit());
			//SceneObject &objectRef = o3d::dynamicCast<SceneObject&>(*getScene()->getPicking()->getSingleHit());

			// And clear all listed hits.
			getScene()->getPicking()->clearPickedList();

			// Draw the name of the picked object.
			Vector3 vec = getScene()->getPicking()->getHitPos();
			System::print(object->getName() + String::print(" at %f %f %f", vec.x(), vec.y(), vec.z()), "ms3d");
		}
	}

    void onMouseMotion(Mouse* mouse)
	{
		Float elapsed = getScene()->getFrameManager()->getFrameDuration();

		// camera rotation
        if (mouse->isRightDown()) {
			Mouse *mouse = getWindow()->getInput().getMouse();

			BaseNode *cameraNode = getScene()->getSceneObjectManager()->searchName("Camera")->getNode();
            if (cameraNode) {
				cameraNode->getTransform()->rotate(Y, -mouse->getDeltaX() * elapsed);
				cameraNode->getTransform()->rotate(X, -mouse->getDeltaY() * elapsed);
			}
		}
	}

    void onMouseButton(Mouse* mouse, ButtonEvent event)
	{
        if (event.isPressed() && (event.button() == Mouse::LEFT)) {
			// Process to a picking a next draw pass.
			// The mouse Y coordinate should be inverted because Y+ is on top of the screen for OpenGL.
			getScene()->getPicking()->postPickingEvent(
					mouse->getMappedPosition().x(),
					getScene()->getViewPortManager()->getReshapeHeight() - mouse->getMappedPosition().y());
		}
	}

    void toggleDrawMode()
    {
        if (getScene()->getViewPortManager()->getViewPort(1)->getActivity()) {
            getScene()->getViewPortManager()->getViewPort(1)->disable();
            getScene()->getViewPortManager()->getViewPort(2)->enable();
            System::print("Switch to Deferred Shading", "Change");
        } else {
            getScene()->getViewPortManager()->getViewPort(2)->disable();
            getScene()->getViewPortManager()->getViewPort(1)->enable();
            System::print("Switch to Shadow Volume Forward Renderering", "Change");
        }
    }

    void onKey(Keyboard* keyboard, KeyEvent event)
	{
        if (event.isPressed() && (event.key() == KEY_F1)) {
            if (m_keys->type() == 0) {
                m_keys = new KeyMapQwerty;
                System::print("Switch to QWERTY", "Change");
            } else if (m_keys->type() == 1) {
                m_keys = new KeyMapBepo;
                System::print("Switch to BEPO", "Change");
            } else if (m_keys->type() == 2) {
                m_keys = new KeyMapAzerty;
                System::print("Switch to AZERTY", "Change");
            }
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

        if (event.isPressed() && (event.key() == KEY_F10)) {
            if (getScene()->getContext()->getDrawingMode() == Context::DRAWING_FILLED) {
                getScene()->getContext()->setDrawingMode(Context::DRAWING_WIREFRAME);
                System::print("Wired mode", "Change");
            } else {
                getScene()->getContext()->setDrawingMode(Context::DRAWING_FILLED);
                System::print("Filled mode", "Change");
            }
        }

        // rotate player

        // Inc/dec a rotation angle depending of the left and right key states.
        // The angle speed stay the same by using the elapsed time as factor.
        m_dwarfRotVelocity.y() = 0;
        m_dwarfRotVelocity.y() += event.action(KEY_LEFT, -1.f, 0.f, 0.f);
        m_dwarfRotVelocity.y() += event.action(KEY_RIGHT, 1.f, -0.f, 0.f);

        //m_dwarfPosVelocity.z() += event.action(KEY_UP, 10000.f, -10000.f, 0.f);
        //m_dwarfPosVelocity.z() += event.action(KEY_DOWN, -10000.f, 10000.f, 0.f);

        Node *dwarf = dynamicCast<Node*>(getScene()->getSceneObjectManager()->searchName("dwarf1"));
        if (dwarf) {
            // simulate the plane collision
            if (dwarf->getRigidBody()->getPosition().y() < 0.0f) {
                Vector3 pos = dwarf->getRigidBody()->getPosition();
                Vector3 speed = dwarf->getRigidBody()->getSpeed();
                pos.y() = 0.0f;
                speed.y() = 0.0f;
                dwarf->getRigidBody()->setSpeed(speed);
                dwarf->getRigidBody()->setPosition(pos);
            }

            Float dwarfImpulse = 0, dwarfJumpImpulse = 0;
            System::print("", dwarf->getRigidBody()->getP());
            if (o3d::abs(dwarf->getRigidBody()->getP().y()) <= 0.01f) {
                if (event.isPressed() && !event.isRepeat()) {
                    if (event.key() == KEY_UP) {
                        dwarfImpulse = -40000.f;
                    } else if (event.key() == KEY_DOWN) {
                        dwarfImpulse = 40000.f;
                    } else if (event.key() == KEY_SPACE) {
                        dwarfJumpImpulse = 70000.f;
                    }
                } else if (event.isReleased()) {
                    if ((event.key() == KEY_UP) || (event.key() == KEY_DOWN)) {
                        dwarf->getRigidBody()->setP(Vector3());
                        m_dwarfPosVelocity.z() = 0;
                    }
                }
            }

            static Float rotY = 0.f;
            Float oldRotY = rotY;
            rotY += m_dwarfRotVelocity.y();

            if (rotY != oldRotY) {
                dwarfImpulse = m_dwarfPosVelocity.z();
            }

            Quaternion rot;
            rot.fromAxisAngle3(Vector3(0.f, 1.f, 0.f), -rotY);

            dwarf->getRigidBody()->setRotation(rot);

            if (o3d::abs(dwarfJumpImpulse) > o3d::Limits<Float>::epsilon()) {
                dwarf->getRigidBody()->addForceImpulse(Vector3(0.f, dwarfJumpImpulse, 0.f));
            }

            if ((o3d::abs(dwarfImpulse) > o3d::Limits<Float>::epsilon()) || (rotY != oldRotY)) {
                Matrix3 rotMat;
                rotMat.rotateY(rotY);

                Vector3 impulse = rotMat * Vector3(0.f, 0.f, dwarfImpulse);
                dwarf->getRigidBody()->setP(Vector3(0.f, dwarf->getRigidBody()->getP().y(), 0.f));
                dwarf->getRigidBody()->addForceImpulse(impulse);

                m_dwarfPosVelocity.z() = dwarfImpulse;
            }
        }

        // translate camera
        const Float speed = 200.f;

        m_camVelocity.x() += event.action( m_keys->left(), -speed, -min(speed, m_camVelocity.x()), 0.f);
        m_camVelocity.x() += event.action( m_keys->right(), speed, -max(-speed, m_camVelocity.x()), 0.f);

        m_camVelocity.y() += event.action( m_keys->down(), -speed, -min(speed, m_camVelocity.y()), 0.f);
        m_camVelocity.y() += event.action( m_keys->up(), speed, -max(-speed, m_camVelocity.y()), 0.f);

        m_camVelocity.z() += event.action( m_keys->forward(), -speed, -min(speed, m_camVelocity.z()), 0.f);
        m_camVelocity.z() += event.action( m_keys->backward(), speed, -max(-speed, m_camVelocity.z()), 0.f);
/*
        m_velocity.x() += event.action( m_keys->left(), -speed, -min(-speed, m_velocity.x()), 0.f);
        m_velocity.x() += event.action( m_keys->right(), speed, -max(-speed, m_velocity.x()), 0.f);

        m_velocity.y() += event.action( m_keys->down(), -speed, -min(speed, m_velocity.y()), 0.f);
        m_velocity.y() += event.action( m_keys->up(), speed, -max(-speed, m_velocity.y()), 0.f);

        m_velocity.z() += event.action( m_keys->forward(), -speed, -min(speed, m_velocity.z()), 0.f);
        m_velocity.z() += event.action( m_keys->backward(), speed, -max(-speed, m_velocity.z()), 0.f);
*/
        if (m_camVelocity.length() > speed) {
            m_camVelocity *= speed / m_camVelocity.length();
        }

        if (event.isPressed() && (event.key() == KEY_F3)) {
            toggleDrawMode();
        }

        if (event.isPressed() && (event.character() == KEY_1)) {
            getScene()->getSceneObjectManager()->searchName("light1")->toggleActivity();
            System::print("Toggle light1", "Change");
        }
        if (event.isPressed() && (event.character() == KEY_2)) {
            getScene()->getSceneObjectManager()->searchName("light2")->toggleActivity();
            System::print("Toggle light2", "Change");
        }
        if (event.isPressed() && (event.character() == KEY_3)) {
            getScene()->getSceneObjectManager()->searchName("light3")->toggleActivity();
            System::print("Toggle light3", "Change");
        }
        if (event.isPressed() && (event.character() == KEY_4)) {
            getScene()->getSceneObjectManager()->searchName("light4")->toggleActivity();
            System::print("Toggle light4", "Change");
        }

        if (event.isPressed() && (event.key() == KEY_SPACE)) {
  			m_animationPlayer->togglePlayPause();
            System::print("Toggle player play/pause", "Change");
        }

        if (event.isPressed() && (event.key() == KEY_J)) {
			m_animationPlayer->enqueueAnimRange("attack1SwipeAxe", AnimationPlayer::MODE_CONTINUE);
			m_animationPlayer->enqueueAnimRange("idle1", AnimationPlayer::MODE_LOOP);
		}

        if (event.isPressed() && (event.key() == KEY_F2)) {
            FeedbackViewPort *vp = o3d::dynamicCast<FeedbackViewPort*>(getScene()->getViewPortManager()->getViewPort(2));
			Image im;
			const UInt8 *d = vp->mapData();
			im.loadBuffer(vp->getDataWidth(), vp->getDataHeight(), vp->getDataSize(), vp->getPixelFormat(), d);
            im.save("feedback.png", Image::PNG);
            im.save("feedback.jpg", Image::JPEG);
			vp->unmapData();
            System::print("Take a screenshot using the feeback viewport", "Action");
		}

        if (event.isPressed() && (event.key() == KEY_ESCAPE)) {
            System::print("Terminate", "Action");
			getWindow()->terminate();
        }
	}

    void onTouchScreenMotion(TouchScreen* touch)
    {
        if (touch->isSize()) {
            Float z = -touch->getDeltaSize() * 0.01;

            Camera *lpCamera = (Camera*)getScene()->getSceneObjectManager()->searchName("Camera");
            lpCamera->getNode()->getTransform()->translate(Vector3(0, 0, z));
        } else {
            Camera *lpCamera = (Camera*)getScene()->getSceneObjectManager()->searchName("Camera");
            lpCamera->getNode()->getTransform()->rotate(Y,-touch->getDeltaX()*0.005f);
            lpCamera->getNode()->getTransform()->rotate(X,-touch->getDeltaY()*0.005f);
        }
    }

    void onTouchScreenChange(TouchScreen* touch, TouchScreenEvent event)
    {
        // attack on tap
        if (touch->isTap()) {
            m_animationPlayer->enqueueAnimRange("attack1SwipeAxe", AnimationPlayer::MODE_CONTINUE);
            m_animationPlayer->enqueueAnimRange("idle1", AnimationPlayer::MODE_LOOP);
        }

        // jump on double tap
        if (touch->isDoubleTap()) {
            Node *dwarf = dynamicCast<Node*>(getScene()->getSceneObjectManager()->searchName("dwarf1"));
            if (dwarf) {
                // simulate the plane collision
                if (dwarf->getRigidBody()->getPosition().y() < 0.0f) {
                    Vector3 pos = dwarf->getRigidBody()->getPosition();
                    Vector3 speed = dwarf->getRigidBody()->getSpeed();
                    pos.y() = 0.0f;
                    speed.y() = 0.0f;
                    dwarf->getRigidBody()->setSpeed(speed);
                    dwarf->getRigidBody()->setPosition(pos);
                }

                Float dwarfImpulse = 0, dwarfJumpImpulse = 0;
                System::print("", dwarf->getRigidBody()->getP());
                if (o3d::abs(dwarf->getRigidBody()->getP().y()) <= 0.01f) {
                    dwarfJumpImpulse = 70000.f;
                }

                static Float rotY = 0.f;
                Float oldRotY = rotY;
                rotY += m_dwarfRotVelocity.y();

                if (rotY != oldRotY) {
                    dwarfImpulse = m_dwarfPosVelocity.z();
                }

                Quaternion rot;
                rot.fromAxisAngle3(Vector3(0.f, 1.f, 0.f), -rotY);

                dwarf->getRigidBody()->setRotation(rot);

                if (o3d::abs(dwarfJumpImpulse) > o3d::Limits<Float>::epsilon()) {
                    dwarf->getRigidBody()->addForceImpulse(Vector3(0.f, dwarfJumpImpulse, 0.f));
                }

                if ((o3d::abs(dwarfImpulse) > o3d::Limits<Float>::epsilon()) || (rotY != oldRotY)) {
                    Matrix3 rotMat;
                    rotMat.rotateY(rotY);

                    Vector3 impulse = rotMat * Vector3(0.f, 0.f, dwarfImpulse);
                    dwarf->getRigidBody()->setP(Vector3(0.f, dwarf->getRigidBody()->getP().y(), 0.f));
                    dwarf->getRigidBody()->addForceImpulse(impulse);

                    m_dwarfPosVelocity.z() = dwarfImpulse;
                }
            }
        }

        // jump on long tap
        if (touch->isLongTap()) {
            toggleDrawMode();
        }
    }

	void onClose()
	{
        System::print("Terminate", "Action");
		getWindow()->terminate();
	}

	//! Set animation player.
	void setAnimationPlayer(AnimationPlayer *player)
	{
		m_animationPlayer = player;
	}

private:

	AnimationPlayer *m_animationPlayer;

    Vector3 m_camVelocity;

    Vector3 m_dwarfRotVelocity;
    Vector3 m_dwarfPosVelocity;
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

        m_app = new Ms3dSample(basePath);

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

    Ms3dSample *m_app;
};

//O3D_NOCONSOLE_MAIN(MyActivity, O3D_DEFAULT_CLASS_SETTINGS)
O3D_CONSOLE_MAIN(MyActivity, O3D_DEFAULT_CLASS_SETTINGS)
