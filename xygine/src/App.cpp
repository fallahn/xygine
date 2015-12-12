/*********************************************************************
Matt Marchant 2014 - 2015
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

#include <xygine/App.hpp>
#include <xygine/Protocol.hpp>
#include <xygine/Util.hpp>
#include <xygine/Log.hpp>
#include <xygine/Reports.hpp>
#include <xygine/physics/World.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Network/IpAddress.hpp>

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
}

App::App()
    : m_videoSettings   (),
    m_renderWindow      (m_videoSettings.VideoMode, windowTitle, m_videoSettings.WindowStyle),
    m_pendingDifficulty (Difficulty::Easy),
    m_connected         (false),
    m_clientID          (-1),
    m_destinationIP     ("127.0.01")
{
    loadSettings();
    m_scores.load();

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

    update = std::bind(&App::updateApp, this, _1);
}

//public
void App::run()
{
    if (!sf::Shader::isAvailable())
    {
        Logger::log("Shaders reported as unavailable.", Logger::Type::Error, Logger::Output::File);
        return;
    }

    m_renderWindow.setMouseCursorVisible(false);

    sf::Clock fpsClock;

    frameClock.restart();
    while (m_renderWindow.isOpen())
    {
        float elapsedTime = frameClock.restart().asSeconds();
        timeSinceLastUpdate += elapsedTime;

        while (timeSinceLastUpdate > timePerFrame)
        {
            timeSinceLastUpdate -= timePerFrame;

            Physics::World::update(timePerFrame);

            handleEvents();
            handleMessages();
            update(timePerFrame);
        }
        draw();

        float fpsTime = 1.f / fpsClock.restart().asSeconds();
        REPORT("FPS", std::to_string(fpsTime));
    }
    m_messageBus.disable(); //prevents spamming with loads of entity quit messages

    saveSettings();
    m_scores.save();

    finalise();
}

void App::pause()
{
    update = std::bind(&App::pauseApp, this, _1);
    //TODO make this just block input, if game is networked
}

void App::resume()
{
    update = std::bind(&App::updateApp, this, _1);
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

sf::Font& App::getFont(const std::string& path)
{
    return m_fontResource.get(path);
}

sf::Texture& App::getTexture(const std::string& path)
{
    return m_textureResource.get(path);
}

MessageBus& App::getMessageBus()
{
    return m_messageBus;
}

sf::TcpSocket& App::getSocket()
{
    return m_socket;
}

bool App::connected() const
{
    return m_connected;
}

sf::Int16 App::getClientID() const
{
    return m_clientID;
}

bool App::hosting() const
{
    return (m_server != nullptr);
}

void App::setDestinationIP(const std::string& ip)
{
    m_destinationIP = ip;
}

void App::addScore(const std::string& name, float value)
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

//protected
bool App::connect(const sf::IpAddress& address, sf::Uint16 port)
{    
    m_socket.connect(address, port, sf::seconds(5.f));// == sf::TcpSocket::Done) return false;
    m_socket.setBlocking(false);

    sf::Clock timeoutClock;
    while (timeoutClock.getElapsedTime().asSeconds() < 5.f)
    {
        sf::Packet packet;
        if (m_socket.receive(packet) == sf::Socket::Done)
        {
            sf::Int32 packetType;
            packet >> packetType;
            if (packetType == Server::ClientID)
            {
                packet >> m_clientID;
                m_connected = true;
                LOG("CLIENT received client id " + std::to_string(m_clientID), Logger::Type::Info);
                break;
            }
        }
    }
    if (m_clientID == -1)
    {
        m_socket.disconnect();
        m_connected = false;
        LOG("CLIENT failed to receive valid client id", Logger::Type::Error);
    }

    return m_connected;
}

void App::disconnect()
{
    m_clientID = -1;
    m_socket.disconnect();
    m_connected = false;
    m_server = nullptr;
}

bool App::createLocalServer()
{
    m_server = std::make_unique<GameServer>();
    return connect({ "127.0.0.1" }, GameServer::getPort());
}

sf::RenderWindow& App::getRenderWindow()
{
    return m_renderWindow;
}

void App::finalise(){}

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
    std::time_t time = std::time(NULL);
    struct tm* timeInfo;

    timeInfo = std::localtime(&time);

    char buffer[40];
    std::string fileName;

    strftime(buffer, 40, "screenshot%d_%m_%y_%H_%M_%S.png", timeInfo);

    fileName.assign(buffer);

    sf::Image screenCap = m_renderWindow.capture();
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
            pause();
            continue;
        case sf::Event::GainedFocus:
            resume();
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
        handleEvent(evt);
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
        case Message::Type::PlayerMessage:
        {
            auto& msgData = msg.getData<Message::PlayerEvent>();
            if (msgData.action == Message::PlayerEvent::Spawned)
            {
                m_gameSettings.difficulty = m_pendingDifficulty;
            }
            break;
        }
        case Message::Type::UIMessage:
        {
            auto& msgData = msg.getData<Message::UIEvent>();
            switch (msgData.type)
            {
            case Message::UIEvent::RequestDifficultyChange:
                m_pendingDifficulty = msgData.difficulty;
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
        case Message::Type::NetworkMessage:
        {
            auto& msgData = msg.getData<Message::NetworkEvent>();
            switch (msgData.action)
            {
            case Message::NetworkEvent::RequestStartServer:
                /*m_server = std::make_unique<GameServer>();
                if (connect({ "127.0.0.1" }, GameServer::getPort()))
                {
                m_stateStack.pushState(msg.network.stateID);
                }*/
                break;
            case Message::NetworkEvent::RequestDisconnect:
                disconnect();
                break;
            case Message::NetworkEvent::RequestJoinServer:
                disconnect();
                if (connect({ m_destinationIP }, GameServer::getPort())) //TODO make port a variable
                {
                    auto msg = m_messageBus.post<Message::NetworkEvent>(Message::NetworkMessage);
                    msg->action = Message::NetworkEvent::ConnectSuccess;
                }
                else
                {
                    disconnect();

                    auto msg = m_messageBus.post<Message::NetworkEvent>(Message::NetworkMessage);
                    msg->action = Message::NetworkEvent::ConnectFail;

                    LOG("whoopsie", Logger::Type::Info);
                }
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

