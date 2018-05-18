 /*********************************************************************
(c) Matt Marchant 2017 - 2018
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
#include "xyginext/core/Editor.hpp"
#include "xyginext/core/Log.hpp"
#include "xyginext/core/Console.hpp"
#include "xyginext/core/ConfigFile.hpp"
#include "xyginext/core/FileSystem.hpp"
#include "xyginext/detail/Operators.hpp"
#include "xyginext/gui/GuiClient.hpp"

#include "../imgui/imgui-SFML.h"

#include "../detail/GLCheck.hpp"

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>

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
#endif //XY_DEBUG

    sf::Clock frameClock;

    sf::RenderTarget* renderTarget = nullptr;

    bool running = false;

    sf::Color clearColour(0, 0, 0, 255);

    App* appInstance = nullptr;

    const std::string settingsFile("settings.cfg");

#include "DefaultIcon.inl"
}

bool App::m_mouseCursorVisible = true;

App::App(sf::ContextSettings contextSettings, const std::string& name)
    : m_videoSettings   (contextSettings),
    m_renderWindow(m_videoSettings.VideoMode, windowTitle, m_videoSettings.WindowStyle, m_videoSettings.ContextSettings),
    m_applicationName   (name)
{
    m_windowIcon.create(16u, 16u, defaultIcon);

    renderTarget = &m_renderWindow;

    // At this point we can safely assume the target will be a renderwindow.
    // Perhaps in future add the ability to initialise with a rendertexture
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

    update = [this](float dt)
    {
        updateApp(dt);
    };
    eventHandler = std::bind(&App::handleEvent, this, _1);

    appInstance = this;

    if (!gladLoadGL())
    {
        Logger::log("Something went wrong loading OpenGL. Particles may be unavailable", Logger::Type::Error, Logger::Output::All);
    }
    
    ImGui::SFML::Init(m_renderWindow);
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
    
    Console::init();
    
    #ifdef XY_EDITOR
        Editor::init();
    #endif
    
    initialise();

    running = true;
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

            update(timePerFrame);
            
            if (Editor::isEnabled())
            {
                Editor::update(timePerFrame);
            }
            
        }
        
        ImGui::SFML::Update(m_renderWindow, sf::seconds(elapsedTime));
        
        for (auto& f : m_guiWindows) f.first();
        
        if (m_renderWindow.setActive(true))
        {
            glCheck(glClearColor(clearColour.r / 255.f, clearColour.g / 255.f, clearColour.b / 255.f, clearColour.a / 255.f));
            glCheck(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        }
        
        // Draw the editor, if enabled
        if (Editor::isEnabled())
        {
            // make sure to update rendertarget too
            renderTarget->clear();
            draw();
            Editor::draw();
        }
        else
        {
            draw();
        }
        
        ImGui::SFML::Render(m_renderWindow);
        
        m_renderWindow.display();
    }

    m_messageBus.disable(); //prevents spamming with loads of entity quit messages

    finalise();
    Editor::shutdown();
    Console::finalise();  
    ImGui::SFML::Shutdown();

    saveSettings();
    m_renderWindow.close();
}

void App::pause()
{
    update = [](float) {};
}

void App::resume()
{
    update = [this](float dt)
    {
        updateApp(dt);
    };
    frameClock.restart();
    timeSinceLastUpdate = 0.f;
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
        || settings.ContextSettings != m_videoSettings.ContextSettings)
    {
        m_renderWindow.create(settings.VideoMode, settings.Title, settings.WindowStyle, settings.ContextSettings);
    }
    else
    {
        m_renderWindow.setSize({ settings.VideoMode.width, settings.VideoMode.height });
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
    //only set frame limiter is not vSync
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

sf::RenderTarget* App::getRenderTarget()
{
    //XY_ASSERT(renderWindow, "Window not created");
    return renderTarget;
}

void App::setRenderTarget(sf::RenderTarget* rt)
{
    renderTarget = rt;
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
    return m_mouseCursorVisible || Editor::isEnabled();
}

//protected
void App::initialise() {}

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
        if (Editor::handleEvent(evt))
        {
            // Editor has taken the event
            continue;
        }
        
        ImGui::SFML::ProcessEvent(evt);

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
        
        if (evt.type == sf::Event::KeyReleased)
        {
            switch (evt.key.code)
            {
            case sf::Keyboard::F1:
                Editor::toggle();
                break;
            case sf::Keyboard::F5:
                saveScreenshot();
                break;
            default:break;
            }
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
