/**
 * @file gui.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2015-12-29
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include <o3d/engine/scene/scene.h>
#include <o3d/engine/scene/sceneobjectmanager.h>
#include <o3d/core/keyboard.h>
#include <o3d/core/appwindow.h>
#include <o3d/core/application.h>
#include <o3d/core/main.h>
#include <o3d/core/diskfileinfo.h>

#include <o3d/engine/viewport.h>
#include <o3d/engine/renderer.h>
#include <o3d/engine/context.h>
#include <o3d/engine/object/camera.h>
#include <o3d/engine/utils/framemanager.h>

#include <o3d/gui/gui.h>
#include <o3d/gui/thememanager.h>
#include <o3d/gui/widgetmanager.h>
#include <o3d/gui/widgets/button.h>
#include <o3d/gui/widgets/combobox.h>
#include <o3d/gui/widgets/checkbox.h>
#include <o3d/gui/widgets/editbox.h>
#include <o3d/gui/widgets/listbox.h>
#include <o3d/gui/widgets/menuwidget.h>
#include <o3d/gui/widgets/radiobutton.h>
#include <o3d/gui/widgets/scrollbar.h>
#include <o3d/gui/widgets/statictext.h>
#include <o3d/gui/widgets/tabbedpane.h>
#include <o3d/gui/widgets/tabbedwidget.h>
#include <o3d/gui/widgets/toolbar.h>
#include <o3d/gui/widgets/toolbutton.h>
#include <o3d/gui/widgets/tooltip.h>

#include <functional>

using namespace o3d;

/**
 * @brief The GuiSample class
 * @date 2015-12-29
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 */
class GuiSample : public EvtHandler
{
private:

    AppWindow *m_appWindow;

    Renderer* m_glRenderer;

    Scene *m_scene;
    Gui *m_gui;

public:

    GuiSample()
	{
        String basePath("../media/");

        m_appWindow = new AppWindow;

        // OpenGL renderer
        m_glRenderer = new Renderer;

        m_appWindow->setTitle("Objective-3D GUI sample");
        m_appWindow->create(800, 600, AppWindow::COLOR_RGBA8, AppWindow::DEPTH_24_STENCIL_8, AppWindow::NO_MSAA, False, True);
        m_appWindow->setMinSize(Vector2i(320, 240));

        m_glRenderer->create(m_appWindow);
        m_glRenderer->setDebug();

        // create a scene and attach it to the window
        m_scene = new Scene(nullptr, basePath, m_glRenderer);
        m_scene->setSceneName("gui");
        m_scene->defaultAttachment(m_appWindow);

        // event
        m_appWindow->onUpdate.connect(this, &GuiSample::onSceneUpdate);
        m_appWindow->onDraw.connect(this, &GuiSample::onSceneDraw);
        m_appWindow->onKey.connect(this, &GuiSample::onKey);
        m_appWindow->onClose.connect(this, &GuiSample::onClose);
        m_appWindow->onDestroy.connect(this, &GuiSample::onDestroy);

        //m_appWindow->grabMouse();
        //m_appWindow->grabKeyboard();

        m_scene->getContext()->setBackgroundColor(0.633f,0.792f,.914f,0.0f);

        m_gui = new Gui(m_scene);
        m_scene->setGui(m_gui);

        // connect event to gui contoller
        m_gui->defaultAttachment(m_appWindow);

        // Application icon
        DiskFileInfo iconFile(basePath + '/' +  "icon.bmp");
        if (iconFile.isExist())
            m_appWindow->setIcon("../media/icon.bmp");

        m_scene->setGlobalAmbient(Color(0.8f, 0.8f, 0.8f, 1.0f));

        // load the gui theme
        Theme *theme = m_gui->getThemeManager()->addTheme(basePath + "gui/WindowsLook.xml");

        // and set it as the default theme to use
        m_gui->getWidgetManager()->setDefaultTheme(theme);

        // now add a set of gui window
        new Window1(m_gui->getWidgetManager());
        //new Window2(m_gui->getWidgetManager());
    }

    virtual ~GuiSample()
	{
	}

    void onDestroy()
    {
        deletePtr(m_scene);
        deletePtr(m_glRenderer);

        this->getWindow()->logFps();

        // it is deleted by the application
        m_appWindow = nullptr;
    }

    class Window1 : public Window
    {
    public:

        Window1(WidgetManager *wm) :
            Window(wm,
                   "Window 1",
                   Vector2i(50, 50),
                   Vector2i(200, 220)),
            m_wm(wm)
        {
            wm->addWindowForegound(this);

            m_checkBox1 = new CheckBox(getLayout(), "Align me at right");
            m_checkBox1->setHorizontalAlign(Widget::LEFT_ALIGN);
            getLayout()->addWidget(m_checkBox1);

            m_checkBox1->onCheckBoxChanged.connect(this, [this] (CheckBox::CheckBoxState state) {
                if (state == CheckBox::CHECKED)
                    m_checkBox1->setHorizontalAlign(Widget::RIGHT_ALIGN);
                else
                    m_checkBox1->setHorizontalAlign(Widget::LEFT_ALIGN);

                // update the layout because this modification impact the widgets positions
                layout();
            });

            m_button1 = new Button(getLayout(), "Fit size to content");
            getLayout()->addWidget(m_button1);

            m_button1->onButtonPressed.connect(this, [this] () {
                m_staticText1->setText("Size updated !");
                m_staticText1->fit();

                // update the layout because the size of the static text has changed
                fit();
                layout();
            });

            RadioButton *radio1 = new RadioButton(getLayout(), nullptr, "Choose me...");
            getLayout()->addWidget(radio1);

            RadioButton *radio2 = new RadioButton(getLayout(), radio1, "Or me...");
            getLayout()->addWidget(radio2);

            m_staticText1 = new StaticText(getLayout(), "This is a static text.");
            getLayout()->addWidget(m_staticText1);

            m_editBox1 = new EditBox(getLayout(), Widget::DEFAULT_POS, Vector2i(150, 24));
            getLayout()->addWidget(m_editBox1);
            m_editBox1->setPlaceholder("this is an edit box !");

            m_editBox2 = new EditBox(getLayout(), Widget::DEFAULT_POS, Vector2i(150, 24), EditBox::EDITBOX_HIDDEN_CHAR);
            getLayout()->addWidget(m_editBox2);
            m_editBox2->setPlaceholder("enter a password !");

            // initial layout of the window
            layout();
        }

    private:

        WidgetManager *m_wm;
        CheckBox *m_checkBox1;
        Button *m_button1;
        StaticText *m_staticText1;
        EditBox *m_editBox1;
        EditBox *m_editBox2;
    };

    void addWindow2()
    {
        Window *window = new Window(m_gui->getWidgetManager(),
                                    "Window 2",
                                    Vector2i(250, 50),
                                    Vector2i(400, 350));

        m_gui->getWidgetManager()->addWindowForegound(window);
    }

    AppWindow* getWindow() { return m_appWindow; }

    Scene* getScene() { return m_scene; }

    Gui* getGui() { return m_gui; }

    void onClose()
    {
        getWindow()->terminate();
    }

    void onSceneUpdate()
	{
        Keyboard * lpKeyboard = m_appWindow->getInput().getKeyboard();
        Float elapsed = getScene()->getFrameManager()->getFrameDuration();
	}

    void onKey(Keyboard* keyboard, KeyEvent event)
    {
        if (event.isPressed() && (event.key() == KEY_ESCAPE))
            getWindow()->terminate();
	}

    void onSceneDraw()
    {
	}

    void onMouseMotion(Mouse* mouse)
    {
	}

    static Int32 main()
    {
        // cleared log out file with new header
        Debug::instance()->setDefaultLog("gui.log");
        Debug::instance()->getDefaultLog().clearLog();
        Debug::instance()->getDefaultLog().writeHeaderLog();

        MemoryManager::instance()->enableLog(MemoryManager::MEM_RAM, 128);
        MemoryManager::instance()->enableLog(MemoryManager::MEM_GFX);

        GuiSample *guiApp = new GuiSample;

        Application::run();

        deletePtr(guiApp);

        // write a footer banner in log out file
        Debug::instance()->getDefaultLog().writeFooterLog();

        return 0;
    }
};

O3D_NOCONSOLE_MAIN(GuiSample, O3D_DEFAULT_CLASS_SETTINGS)

