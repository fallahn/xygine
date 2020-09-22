/*********************************************************************
(c) Matt Marchant 2017 - 2020
http://trederia.blogspot.com

xygineXT - Zlib license.

This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#include "xyginext/core/App.hpp"
#include "xyginext/core/Log.hpp"
#include "xyginext/core/Console.hpp"
#include "xyginext/core/ConfigFile.hpp"
#include "xyginext/core/FileSystem.hpp"
#include "xyginext/detail/Operators.hpp"
#include "xyginext/gui/GuiClient.hpp"

#include "xyginext/gui/imgui.h"
#include "xyginext/gui/imgui-SFML.h"
#include "../imgui/imgui_internal.h"

#include "../detail/GLCheck.hpp"
#ifdef _MSC_VER
#ifdef XY_DEBUG
//prints callstack
//note this can be enabled for linux/mac
//I just don't know which libraries need
//to be linked
#include "../detail/ust.hpp"
#include <signal.h>
#endif
#endif

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <algorithm>
#include <fstream>
#include <cstring>
#include <array>

using namespace std::placeholders;
using namespace xy;

namespace
{
    const float timePerFrame = 1.f / 60.f;
    float timeSinceLastUpdate = 0.f;

#ifndef XY_DEBUG
    const std::string windowTitle("xyginext game (Release Build) - F1: Open Console");
#else
    const std::string windowTitle("xyginext game (Debug Build) - F1: Open Console");
#ifdef _MSC_VER
    void winAbort(int)
    {
        Logger::log(Logger::Type::Error, Logger::Output::All) << "Call Stack:\n" << ust::generate() << std::endl;
    }
#endif
#endif //XY_DEBUG

    sf::Clock frameClock;

    sf::RenderWindow* renderWindow = nullptr;

    bool running = false;

    sf::Color clearColour(0, 0, 0, 255);

    App* appInstance = nullptr;

    const std::string settingsFile("settings.cfg");

#include "DefaultIcon.inl"

    void setImguiStyle(ImGuiStyle* dst)
    {
        ImGuiStyle& st = dst ? *dst : ImGui::GetStyle();
        st.FrameBorderSize = 1.0f;
        st.FramePadding = ImVec2(4.0f, 4.0f);
        st.ItemSpacing = ImVec2(8.0f, 4.0f);
        st.WindowBorderSize = 1.0f;
        st.TabBorderSize = 1.0f;
        st.WindowRounding = 1.0f;
        st.ChildRounding = 1.0f;
        st.FrameRounding = 1.0f;
        st.ScrollbarRounding = 1.0f;
        st.GrabRounding = 1.0f;
        st.TabRounding = 1.0f;

        // Setup style
        ImVec4* colors = st.Colors;
        colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 1.00f, 0.95f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.40f, 0.50f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.12f, 0.22f, 0.78f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.95f, 0.95f, 1.00f, 0.00f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.10f, 0.94f);
        colors[ImGuiCol_Border] = ImVec4(0.50f, 0.50f, 0.59f, 0.46f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.10f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.25f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.22f, 0.26f, 0.40f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.16f, 0.16f, 0.19f, 0.53f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.04f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.05f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.02f, 0.51f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.03f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.33f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.43f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.48f, 0.48f, 0.50f, 1.00f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.79f, 0.79f, 0.79f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.48f, 0.47f, 0.47f, 0.91f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.55f, 0.55f, 0.62f);
        colors[ImGuiCol_Button] = ImVec4(0.42f, 0.42f, 0.50f, 0.63f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.60f, 0.60f, 0.68f, 0.63f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.20f, 0.26f, 0.63f);
        colors[ImGuiCol_Header] = ImVec4(0.54f, 0.54f, 0.68f, 0.58f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.64f, 0.65f, 0.70f, 0.80f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.25f, 0.28f, 0.80f);
        colors[ImGuiCol_Separator] = ImVec4(0.58f, 0.58f, 0.62f, 0.50f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.64f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.81f, 0.81f, 0.81f, 0.64f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.87f, 0.87f, 0.87f, 0.53f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.87f, 0.87f, 0.87f, 0.74f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.87f, 0.87f, 0.87f, 0.74f);
        colors[ImGuiCol_Tab] = ImVec4(0.01f, 0.01f, 0.03f, 0.86f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.29f, 0.29f, 0.33f, 1.00f);
        colors[ImGuiCol_TabActive] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.02f, 0.02f, 0.04f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.19f, 0.19f, 0.21f, 1.00f);
        //colors[ImGuiCol_DockingPreview] = ImVec4(0.38f, 0.48f, 0.60f, 1.00f);
        //colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.68f, 0.68f, 0.68f, 1.00f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.77f, 0.33f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.87f, 0.55f, 0.08f, 1.00f);
        colors[ImGuiCol_TextSelectedBg] = ImVec4(0.47f, 0.60f, 0.76f, 0.47f);
        colors[ImGuiCol_DragDropTarget] = ImVec4(0.58f, 0.58f, 0.58f, 0.90f);
        colors[ImGuiCol_NavHighlight] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.20f, 0.20f, 0.24f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.24f, 0.35f);
    }
}

bool App::m_mouseCursorVisible = true;

App::App(sf::ContextSettings contextSettings)
    : m_videoSettings   (contextSettings),
    m_renderWindow      (m_videoSettings.VideoMode, windowTitle, m_videoSettings.WindowStyle, m_videoSettings.ContextSettings),
    m_applicationName   (APP_NAME)
{
    renderWindow = &m_renderWindow;

    m_renderWindow.setVerticalSyncEnabled(m_videoSettings.VSync);

    //tiny icon looks awful in the dock
#ifndef __APPLE__
    m_windowIcon.create(16u, 16u, defaultIcon);
    m_renderWindow.setIcon(16, 16, m_windowIcon.getPixelsPtr());
#endif

    //store available modes and remove unusable
    m_videoSettings.AvailableVideoModes = sf::VideoMode::getFullscreenModes();
    m_videoSettings.AvailableVideoModes.erase(std::remove_if(m_videoSettings.AvailableVideoModes.begin(), m_videoSettings.AvailableVideoModes.end(),
        [](const sf::VideoMode& vm)
    {
        return (!vm.isValid() || vm.bitsPerPixel != 32);
    }), m_videoSettings.AvailableVideoModes.end());
    std::reverse(m_videoSettings.AvailableVideoModes.begin(), m_videoSettings.AvailableVideoModes.end());

    m_videoSettings.Title = windowTitle;

    /*update = [this](float dt)
    {
        updateApp(dt);
    };*/
    eventHandler = std::bind(&App::handleEvent, this, _1);

    appInstance = this;

    if (!gladLoadGL())
    {
        Logger::log("Something went wrong loading OpenGL. Particles may be unavailable", Logger::Type::Error, Logger::Output::All);
    }

#ifdef XY_DEBUG
#ifdef _MSC_VER
    //register custom abort which prints the call stack
    signal(SIGABRT, &winAbort);
#endif
#endif
}

//public
void App::run()
{
    if (!sf::Shader::isAvailable())
    {
        Logger::log("Shaders reported as unavailable.", Logger::Type::Error, Logger::Output::File);
        return;
    }

    //if we find a settings file apply those settings
    loadSettings();

    ImGui::SFML::Init(m_renderWindow);
    //TODO load custom styles from config
    setImguiStyle(&ImGui::GetStyle());
    Console::init();
    running = initialise();

    frameClock.restart();
    while (running)
    {
        float elapsedTime = frameClock.restart().asSeconds();
        timeSinceLastUpdate += elapsedTime;
        
        while (timeSinceLastUpdate > timePerFrame)
        {
            timeSinceLastUpdate -= timePerFrame;
            
            handleEvents();
            handleMessages();

            updateApp(timePerFrame);
            
            appInstance->m_renderWindow.setMouseCursorVisible(m_mouseCursorVisible || Console::isVisible());
        }
        
        ImGui::SFML::Update(m_renderWindow, sf::seconds(elapsedTime));
        
        // Do imgui stuff (Console and any client windows)
        Console::draw();
        for (auto& f : m_guiWindows) f.first();
        
        //m_renderWindow.clear(clearColour);
        if (m_renderWindow.setActive(true))
        {
            glCheck(glClearColor(clearColour.r / 255.f, clearColour.g / 255.f, clearColour.b / 255.f, clearColour.a / 255.f));
            glCheck(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        }
        draw();       
        ImGui::SFML::Render(m_renderWindow);
        m_renderWindow.display();
    }

    m_messageBus.disable(); //prevents spamming with loads of entity quit messages
    
    finalise();
    Console::finalise();  
    ImGui::SFML::Shutdown();

    saveSettings();
    m_renderWindow.close();
}

void App::pause()
{
    //update = [](float) {};
}

void App::resume()
{
    /*update = [this](float dt)
    {
        updateApp(dt);
    };
    frameClock.restart();
    timeSinceLastUpdate = 0.f;*/
}

const App::VideoSettings& App::getVideoSettings() const
{
    return m_videoSettings;
}

void App::applyVideoSettings(const VideoSettings& settings) 
{
    if (m_videoSettings == settings) return;

    auto availableModes = m_videoSettings.AvailableVideoModes;

    auto oldAA = settings.ContextSettings.antialiasingLevel;
    if (settings.WindowStyle != m_videoSettings.WindowStyle
        || settings.ContextSettings != m_videoSettings.ContextSettings
        || settings.VideoMode != m_videoSettings.VideoMode)
    {
        m_renderWindow.create(settings.VideoMode, settings.Title, settings.WindowStyle, settings.ContextSettings);
    /*}
    else
    {*/
        //m_renderWindow.setSize({ settings.VideoMode.width, settings.VideoMode.height });
        m_renderWindow.setTitle(settings.Title);

        sf::Vector2u windowPos = { sf::VideoMode::getDesktopMode().width / 2, sf::VideoMode::getDesktopMode().height / 2 };
        windowPos.x -= settings.VideoMode.width / 2;
        windowPos.y -= settings.VideoMode.height / 2;
        m_renderWindow.setPosition({ static_cast<sf::Int32>(windowPos.x), static_cast<sf::Int32>(windowPos.y) });
    }

    auto* msg = m_messageBus.post<Message::WindowEvent>(Message::WindowMessage);
    msg->type = Message::WindowEvent::Resized;
    msg->width = settings.VideoMode.width;
    msg->height = settings.VideoMode.height;

    //check if the AA level is the same as requested
    auto newAA = m_renderWindow.getSettings().antialiasingLevel;
    if (oldAA != newAA)
    {
        Logger::log("Requested Anti-aliasing level not available, using level: " + std::to_string(newAA), Logger::Type::Warning, Logger::Output::All);
    }

    m_renderWindow.setVerticalSyncEnabled(settings.VSync);
    //only set frame limiter if not vSync
    if (settings.FrameLimit && !settings.VSync)
    {
        m_renderWindow.setFramerateLimit(settings.FrameLimit);
    }
    else
    {
        m_renderWindow.setFramerateLimit(0);
    }

    //TODO test validity and restore old settings if possible
    m_videoSettings = settings;
    m_videoSettings.ContextSettings.antialiasingLevel = newAA; //so it's correct if requested
    m_videoSettings.AvailableVideoModes = availableModes;

    if (m_windowIcon.getPixelsPtr())
    {
        auto size = m_windowIcon.getSize();
        m_renderWindow.setIcon(size.x, size.y, m_windowIcon.getPixelsPtr());
    }
}

MessageBus& App::getMessageBus()
{
    return m_messageBus;
}

void App::quit()
{
    //XY_ASSERT(renderWindow, "no valid window instance");

    //renderWindow->close();
    running = false;
}

void App::setClearColour(sf::Color colour)
{
    clearColour = colour;
}

sf::Color App::getClearColour()
{
    return clearColour;
}

void App::setWindowTitle(const std::string& title)
{
    m_videoSettings.Title = title;
    m_renderWindow.setTitle(title);
}

void App::setWindowIcon(const std::string& path)
{
    if (m_windowIcon.loadFromFile(xy::FileSystem::getResourcePath() + path))
    {
        auto size = m_windowIcon.getSize();
        //XY_ASSERT(size.x == 16 && size.y == 16, "window icon must be 16x16 pixels");
        m_renderWindow.setIcon(size.x, size.y, m_windowIcon.getPixelsPtr());
    }
    else
    {
        xy::Logger::log("failed to open " + path, xy::Logger::Type::Error);
    }
}

sf::RenderWindow* App::getRenderWindow()
{
    //XY_ASSERT(renderWindow, "Window not created");
    return renderWindow;
}

void App::printStat(const std::string& name, const std::string& value)
{
    Console::printStat(name,value);
}

App* App::getActiveInstance()
{
    //XY_ASSERT(appInstance, "No active app instance");
    return appInstance;
}

void App::setApplicationName(const std::string& name)
{
    XY_ASSERT(!name.empty(), "must be a valid name");
    m_applicationName = name;
}

const std::string& App::getApplicationName() const
{
    return m_applicationName;
}

void App::setMouseCursorVisible(bool visible)
{
    m_mouseCursorVisible = visible;
}

bool App::isMouseCursorVisible()
{
    return m_mouseCursorVisible || Console::isVisible();
}

//protected
bool App::initialise() { return true; }

void App::finalise() {}

//private
void App::saveScreenshot()
{
    std::time_t time = std::time(nullptr);
    struct tm* timeInfo;
    timeInfo = std::localtime(&time);

    std::array<char, 40u> buffer;
    std::string fileName;

    strftime(buffer.data(), 40, "screenshot%d_%m_%y_%H_%M_%S.png", timeInfo);

    fileName.assign(buffer.data());

    sf::Texture t;
    t.create(m_renderWindow.getSize().x, m_renderWindow.getSize().y);
    t.update(m_renderWindow);
    sf::Image screenCap = t.copyToImage();
    if (!screenCap.saveToFile(fileName)) Logger::log("failed to save " + fileName, Logger::Type::Error, Logger::Output::File);
}

void App::handleEvents()
{
    sf::Event evt;

    while (m_renderWindow.pollEvent(evt))
    {
        ImGui::SFML::ProcessEvent(evt);
        if (evt.type == sf::Event::KeyReleased)
        {
            switch (evt.key.code)
            {
            case sf::Keyboard::F1:
                Console::show();
                if (!Console::isVisible())
                {
                    saveSettings();
                }
                break;
            case sf::Keyboard::F5:
                saveScreenshot();
                break;
            default:break;
            }
        }

        switch (evt.type)
        {
        case sf::Event::LostFocus:
            eventHandler = [](const sf::Event&) {};

            {
                auto* msg = m_messageBus.post<Message::WindowEvent>(Message::WindowMessage);
                msg->type = Message::WindowEvent::LostFocus;
                msg->width = m_renderWindow.getSize().x;
                msg->height = m_renderWindow.getSize().y;
            }
            continue;
        case sf::Event::GainedFocus:
            eventHandler = std::bind(&App::handleEvent, this, _1);
            frameClock.restart(); //prevent dumps of HUGE dt
            {
                auto* msg = m_messageBus.post<Message::WindowEvent>(Message::WindowMessage);
                msg->type = Message::WindowEvent::GainedFocus;
                msg->width = m_renderWindow.getSize().x;
                msg->height = m_renderWindow.getSize().y;
            }

            continue;
        case sf::Event::Closed:
            quit();
            return;
        case sf::Event::Resized:
        {
            auto* msg = m_messageBus.post<Message::WindowEvent>(Message::WindowMessage);
            msg->type = Message::WindowEvent::Resized;
            msg->width = evt.size.width;
            msg->height = evt.size.height;
        }
            break;
        default: break;
        }
        
        eventHandler(evt);
    }   
}

void App::handleMessages()
{
    while (!m_messageBus.empty())
    {
        auto msg = m_messageBus.poll();

        handleMessage(msg);
    } 
}

void App::addStatusControl(const std::function<void()>& func, const GuiClient* c)
{
    Console::addStatusControl(func, c);
}

void App::removeStatusControls(const GuiClient* c)
{
    Console::removeStatusControls(c);
}

void App::addConsoleTab(const std::string& name, const std::function<void()>& func, const GuiClient* c)
{
    Console::addConsoleTab(name, func, c);
}

void App::removeConsoleTab(const GuiClient* c)
{
    Console::removeConsoleTab(c);
}

void App::addWindow(const std::function<void()>& func, const GuiClient* c)
{
    XY_ASSERT(appInstance, "App not properly instanciated!");

    appInstance->m_guiWindows.push_back(std::make_pair(func, c));
}

void App::removeWindows(const GuiClient* c)
{
    XY_ASSERT(appInstance, "App not properly instanciated!");

    appInstance->m_guiWindows.erase(
        std::remove_if(std::begin(appInstance->m_guiWindows), std::end(appInstance->m_guiWindows),
            [c](const std::pair<std::function<void()>, const GuiClient*>& pair)
    {
        return pair.second == c;
    }), std::end(appInstance->m_guiWindows));
}

void App::loadSettings()
{
    ConfigFile settings;
    if (settings.loadFromFile(FileSystem::getConfigDirectory(m_applicationName) + settingsFile))
    {
        auto objects = settings.getObjects();
        auto vObj = std::find_if(objects.begin(), objects.end(),
            [](const ConfigObject& o)
        {
            return o.getName() == "video";
        });

        if (vObj != objects.end())
        {
            const auto& properties = vObj->getProperties();
            VideoSettings vSettings;
            for (const auto& p : properties)
            {
                if (p.getName() == "resolution")
                {
                    auto res = static_cast<sf::Vector2u>(p.getValue<sf::Vector2f>());
                    vSettings.VideoMode.width = res.x;
                    vSettings.VideoMode.height = res.y;
                }
                else if (p.getName() == "vsync")
                {
                    vSettings.VSync = p.getValue<bool>();
                }
                else if (p.getName() == "fullscreen")
                {
                    vSettings.WindowStyle = (p.getValue<bool>()) ? sf::Style::Fullscreen : sf::Style::Close;
                }
                else if (p.getName() == "framelimit")
                {
                    vSettings.FrameLimit = p.getValue<sf::Int32>();
                }
            }

            vSettings.ContextSettings = m_videoSettings.ContextSettings; //not stored in the settings file so we use what was passed to the ctor
            vSettings.Title = windowTitle;
            applyVideoSettings(vSettings);
        }
        

        //load audio settings and apply to mixer / master vol
        auto aObj = std::find_if(objects.begin(), objects.end(),
            [](const ConfigObject& o)
        {
            return o.getName() == "audio";
        });

        if (aObj != objects.end())
        {
            const auto& properties = aObj->getProperties();
            for (const auto& p : properties)
            {
                if (p.getName() == "master")
                {
                    AudioMixer::setMasterVolume(p.getValue<float>());
                }
                else
                {
                    auto name = p.getName();
                    auto found = name.find("channel");
                    if (found != std::string::npos)
                    {
                        auto ident = name.substr(found + 7);
                        try
                        {
                            auto channel = std::stoi(ident);
                            AudioMixer::setVolume(p.getValue<float>(), channel);
                        }
                        catch (...)
                        {
                            continue;
                        }
                    }
                }
            }
        }
    }
}

void App::saveSettings()
{
    ConfigFile settings("settings");

    auto* vObj = settings.addObject("video");
    vObj->addProperty("resolution", std::to_string(m_videoSettings.VideoMode.width) + "," + std::to_string(m_videoSettings.VideoMode.height));
    vObj->addProperty("vsync", m_videoSettings.VSync ? "true" : "false");
    vObj->addProperty("fullscreen", (m_videoSettings.WindowStyle & sf::Style::Fullscreen) ? "true" : "false");
    vObj->addProperty("framelimit", std::to_string(m_videoSettings.FrameLimit));
    
    auto* aObj = settings.addObject("audio");
    aObj->addProperty("master", std::to_string(AudioMixer::getMasterVolume()));
    for (auto i = 0u; i < AudioMixer::MaxChannels; ++i)
    {
        aObj->addProperty("channel" + std::to_string(i), std::to_string(AudioMixer::getVolume(i)));
    }
    
    settings.save(FileSystem::getConfigDirectory(m_applicationName) + settingsFile);
}
