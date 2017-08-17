/*********************************************************************
(c) Matt Marchant 2017
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

#include <xyginext/core/App.hpp>
#include <xyginext/core/Log.hpp>

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

#ifndef _DEBUG_
    std::string windowTitle("xyginext game (Release Build) - F1: console F2: Stats");
#else
    std::string windowTitle("xyginext game (Debug Build) - F1: console F2: Stats");
#endif //_DEBUG_

    sf::Clock frameClock;

    sf::RenderWindow* renderWindow = nullptr;

    bool running = false;

    sf::Color clearColour(0, 0, 0, 255);

#include "DefaultIcon.inl"
}

App::App()
    : m_videoSettings   (),
    m_renderWindow      (m_videoSettings.VideoMode, windowTitle, m_videoSettings.WindowStyle, m_videoSettings.ContextSettings)
{
    m_windowIcon.create(16u, 16u, defaultIcon);

    renderWindow = &m_renderWindow;

    m_renderWindow.setVerticalSyncEnabled(m_videoSettings.VSync);
    //m_renderWindow.setIcon(icon_width, icon_height, icon_arr);

    //store available modes and remove unusable
    m_videoSettings.AvailableVideoModes = sf::VideoMode::getFullscreenModes();
    m_videoSettings.AvailableVideoModes.erase(std::remove_if(m_videoSettings.AvailableVideoModes.begin(), m_videoSettings.AvailableVideoModes.end(),
        [](const sf::VideoMode& vm)
    {
        return (!vm.isValid() || vm.bitsPerPixel != 32);
    }), m_videoSettings.AvailableVideoModes.end());
    std::reverse(m_videoSettings.AvailableVideoModes.begin(), m_videoSettings.AvailableVideoModes.end());

    update = [this](float dt)
    {
        updateApp(dt);
    };
    eventHandler = std::bind(&App::handleEvent, this, _1);
}

//public
void App::run()
{
    if (!sf::Shader::isAvailable())
    {
        Logger::log("Shaders reported as unavailable.", Logger::Type::Error, Logger::Output::File);
        return;
    }

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
        }

        m_renderWindow.clear(clearColour);
        draw();
        m_renderWindow.display();
    }

    m_renderWindow.close();
    m_messageBus.disable(); //prevents spamming with loads of entity quit messages

    finalise();
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

const App::AudioSettings& App::getAudioSettings() const
{
    return m_audioSettings;
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
    m_renderWindow.create(settings.VideoMode, windowTitle, settings.WindowStyle, settings.ContextSettings);

    //check if the AA level is the same as requested
    auto newAA = m_renderWindow.getSettings().antialiasingLevel;
    if (oldAA != newAA)
    {
        Logger::log("Requested Anti-aliasing level not available, using level: " + std::to_string(newAA), Logger::Type::Warning, Logger::Output::All);
    }

    m_renderWindow.setVerticalSyncEnabled(settings.VSync);
    //m_renderWindow.setMouseCursorVisible(false);
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
    XY_ASSERT(renderWindow, "no valid window instance");

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
    windowTitle = title;
    m_renderWindow.setTitle(title);
}

void App::setWindowIcon(const std::string& path)
{
    if (m_windowIcon.loadFromFile(path))
    {
        auto size = m_windowIcon.getSize();
        XY_ASSERT(size.x == 16 && size.y == 16, "window icon must be 16x16 pixels");
        m_renderWindow.setIcon(size.x, size.y, m_windowIcon.getPixelsPtr());
    }
    else
    {
        xy::Logger::log("failed to open " + path, xy::Logger::Type::Error);
    }
}

//protected
sf::RenderWindow& App::getRenderWindow()
{
    return m_renderWindow;
}

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
        switch (evt.type)
        {
        case sf::Event::LostFocus:
            eventHandler = [](const sf::Event&) {};
            continue;
        case sf::Event::GainedFocus:
            eventHandler = std::bind(&App::handleEvent, this, _1);
            continue;
        case sf::Event::Closed:
            quit();
            return;
        default: break;
        }

        if (evt.type == sf::Event::KeyReleased)
        {
            switch (evt.key.code)
            {
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