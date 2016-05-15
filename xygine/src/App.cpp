/*********************************************************************
Matt Marchant 2014 - 2016
http://trederia.blogspot.com

xygine - Zlib license.

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

#include <xygine/detail/GLExtensions.hpp>

#include <xygine/App.hpp>
#include <xygine/util/Math.hpp>
#include <xygine/Log.hpp>
#include <xygine/Reports.hpp>
#include <xygine/physics/World.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Shader.hpp>

#include <algorithm>
#include <fstream>
#include <cstring>

using namespace std::placeholders;
using namespace xy;

namespace
{
    const float timePerFrame = 1.f / 60.f;
    float timeSinceLastUpdate = 0.f;

#ifndef _DEBUG_
    const std::string windowTitle("xygine game (Release Build)");
#else
    const std::string windowTitle("xygine game (Debug Build)");
#endif //_DEBUG_
    const std::string settingsFile("settings.set");

    int lastScoreIndex = 0;

    int settingsIdent = 0x45504f44;
    int settingsVersion = 1;

    sf::Clock frameClock;

    const sf::RenderWindow* renderWindow = nullptr;
}

App::App()
    : m_videoSettings   (),
    m_renderWindow(m_videoSettings.VideoMode, windowTitle, m_videoSettings.WindowStyle, sf::ContextSettings(0, 0, 0, 3, 2))/*,
    m_pendingDifficulty (Difficulty::Easy)*/
{
    loadSettings();
    m_scores.load();

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
        Physics::World::update(dt);
        updateApp(dt);
    };
    eventHandler = std::bind(&App::handleEvent, this, _1);

    if (ogl_LoadFunctions() == ogl_LOAD_FAILED)
    {
        Logger::log("Failed loading OpenGL extensions, MultiRenderTargets will be unavailable", Logger::Type::Error, Logger::Output::All);
    }
}

//public
void App::run()
{
    if (!sf::Shader::isAvailable())
    {
        Logger::log("Shaders reported as unavailable.", Logger::Type::Error, Logger::Output::File);
        return;
    }

    //m_renderWindow.setMouseCursorVisible(false);
    initialise();

#ifdef _DEBUG_
    sf::Clock fpsClock;
    sf::Clock fpsUpdateClock;
#endif //_DEBUG_

    frameClock.restart();
    while (m_renderWindow.isOpen())
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
        draw();
#ifdef _DEBUG_
        float fpsTime = 1.f / fpsClock.restart().asSeconds();
        if (fpsUpdateClock.getElapsedTime().asSeconds() > 0.25f)
        {
            REPORT("FPS", std::to_string(fpsTime));
            fpsUpdateClock.restart();
        }
#endif //_DEBUG_
    }
    m_messageBus.disable(); //prevents spamming with loads of entity quit messages

    saveSettings();
    m_scores.save();

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
        Physics::World::update(dt);
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

const App::GameSettings& App::getGameSettings() const
{
    return m_gameSettings;
}

void App::applyVideoSettings(const VideoSettings& settings)
{
    if (m_videoSettings == settings) return;

    auto availableModes = m_videoSettings.AvailableVideoModes;

    m_renderWindow.create(settings.VideoMode, windowTitle, settings.WindowStyle);
    m_renderWindow.setVerticalSyncEnabled(settings.VSync);
    m_renderWindow.setMouseCursorVisible(false);
    //TODO test validity and restore old settings if possible
    m_videoSettings = settings;
    m_videoSettings.AvailableVideoModes = availableModes;

    //m_renderWindow.setIcon(icon_width, icon_height, icon_arr);

    auto msg = m_messageBus.post<Message::UIEvent>(Message::UIMessage);
    msg->type = Message::UIEvent::ResizedWindow;
}

MessageBus& App::getMessageBus()
{
    return m_messageBus;
}

void App::addScore(const std::string& name, int value)
{
    lastScoreIndex = m_scores.add(name, value, m_gameSettings.difficulty);
}

int App::getLastScoreIndex() const
{
    return lastScoreIndex;
}

const std::vector<Scores::Item>& App::getScores() const
{
    return m_scores.getScores(m_gameSettings.difficulty);
}

void App::setPlayerInitials(const std::string& initials)
{
    std::string str(initials);
    while (str.size() < 3) str.push_back('-');
    if (str.size() > 3) str = str.substr(0, 3);

    std::strcpy(&m_gameSettings.playerInitials[0], str.c_str());
}

sf::Vector2f App::getMouseWorldPosition()
{
    XY_ASSERT(renderWindow, "no valid window instance");
    return renderWindow->mapPixelToCoords(sf::Mouse::getPosition(*renderWindow));
}

//protected
sf::RenderWindow& App::getRenderWindow()
{
    return m_renderWindow;
}

void App::initialise() {}

void App::finalise() {}

//private
void App::loadSettings()
{
    std::fstream file(settingsFile, std::ios::binary | std::ios::in);
    if (!file.good() || !file.is_open() || file.fail())
    {
        Logger::log("failed to open settings file for reading", Logger::Type::Warning, Logger::Output::All);
        Logger::log("file probably missing - new file will be created", Logger::Type::Warning, Logger::Output::All);
        file.close();
        return;
    }

    //check file size
    file.seekg(0, std::ios::end);
    int fileSize = static_cast<int>(file.tellg());
    file.seekg(0, std::ios::beg);

    if (fileSize < static_cast<int>(sizeof(SettingsFile)))
    {
        Logger::log("settings file not expected file size", Logger::Type::Error, Logger::Output::All);
        file.close();
        return;
    }

    SettingsFile settings;
    file.read((char*)&settings, sizeof(SettingsFile));
    file.close();
    
    if (settings.ident != settingsIdent || settings.version != settingsVersion)
    {
        Logger::log("settings file invalid or wrong version", Logger::Type::Error, Logger::Output::All);
        return;
    }

    VideoSettings  newVideoSettings;
    newVideoSettings.VideoMode = settings.videoMode;
    newVideoSettings.WindowStyle = settings.windowStyle;

    m_audioSettings = settings.audioSettings;
    m_audioSettings.volume = Util::Math::clamp(m_audioSettings.volume, 0.f, 1.f);

    m_gameSettings = settings.gameSettings;

    applyVideoSettings(newVideoSettings);
}

void App::saveSettings()
{
    std::fstream file(settingsFile, std::ios::binary | std::ios::out);
    if (!file.good() || !file.is_open() || file.fail())
    {
        Logger::log("failed to open settings file for writing", Logger::Type::Error, Logger::Output::All);
        file.close();
        return;
    }

    SettingsFile settings;
    settings.ident = settingsIdent;
    settings.version = settingsVersion;
    settings.videoMode = m_videoSettings.VideoMode;
    settings.windowStyle = m_videoSettings.WindowStyle;

    settings.audioSettings = m_audioSettings;
    settings.gameSettings = m_gameSettings;

    file.write((char*)&settings, sizeof(SettingsFile));
    file.close();
}

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
            m_renderWindow.close();
            return;
        default: break;
        }

#ifdef _DEBUG_
        if (evt.type == sf::Event::KeyPressed)
        {
            switch (evt.key.code)
            {
            case sf::Keyboard::Escape:
                m_renderWindow.close();
                break;
            default: break;
            }
        }
#endif //_DEBUG_

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
    REPORT("Messages This Frame", std::to_string(m_messageBus.pendingMessageCount()));
    while (!m_messageBus.empty())
    {
        auto msg = m_messageBus.poll();

        switch (msg.id)
        {
        case Message::Type::UIMessage:
        {
            auto& msgData = msg.getData<Message::UIEvent>();
            switch (msgData.type)
            {
            case Message::UIEvent::RequestDifficultyChange:
                m_gameSettings.difficulty = msgData.difficulty;
                break;
            case Message::UIEvent::RequestAudioMute:
                m_audioSettings.muted = true;
                break;
            case Message::UIEvent::RequestAudioUnmute:
                m_audioSettings.muted = false;
                break;
            case Message::UIEvent::RequestVolumeChange:
                m_audioSettings.volume = msgData.value;
                break;
            case Message::UIEvent::RequestControllerEnable:
                m_gameSettings.controllerEnabled = true;
                break;
            case Message::UIEvent::RequestControllerDisable:
                m_gameSettings.controllerEnabled = false;
                break;
            default: break;
            }
            break;
        }
        default: break;
        }

        handleMessage(msg);
    } 
}

