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

//main entry point for the app

#ifndef APP_HPP_
#define APP_HPP_

#include <xygine/StateStack.hpp>
#include <xygine/Resource.hpp>
#include <xygine/Score.hpp>
#include <xygine/MessageBus.hpp>
#include <xygine/Server.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Network/Packet.hpp>

#include <functional>

namespace xy
{   
    class App
    {
    public:

        struct VideoSettings final
        {
            sf::Int32 WindowStyle;
            sf::VideoMode VideoMode;
            bool VSync;
            std::vector<sf::VideoMode> AvailableVideoModes;

            VideoSettings()
                : WindowStyle(sf::Style::Close /*sf::Style::Fullscreen*/),
                VideoMode(1024, 768),
                VSync(true){}

            bool operator == (const VideoSettings& vs)
            {
                if (&vs == this) return true;
                return
                    (vs.VideoMode == this->VideoMode
                    && vs.VSync == this->VSync
                    && vs.WindowStyle == this->WindowStyle);
            }
        };

        struct AudioSettings final
        {
            bool muted = false;
            float volume = 1.f;
        };

        struct GameSettings final
        {
            Difficulty difficulty = Difficulty::Easy;
            bool controllerEnabled = true;
            std::array<char, 3u> playerInitials;
            GameSettings() : playerInitials({ { '-', '-', '-' } }){}
        };

        App();
        virtual ~App() = default;
        App(const App&) = delete;
        const App& operator = (const App&) = delete;

        void run();
        void pause();
        void resume();

        const AudioSettings& getAudioSettings() const;

        const VideoSettings& getVideoSettings() const;
        void applyVideoSettings(const VideoSettings&);

        const GameSettings& getGameSettings() const;

        sf::Font& getFont(const std::string& path);
        sf::Texture& getTexture(const std::string& path);
        MessageBus& getMessageBus();
        sf::TcpSocket& getSocket();

        bool connected() const;
        sf::Int16 getClientID() const;
        bool hosting() const;
        void setDestinationIP(const std::string&);

        void addScore(const std::string& name, float value);
        const std::vector<Scores::Item>& getScores() const;
        int getLastScoreIndex() const;

        void setPlayerInitials(const std::string&);

    protected:
        bool connect(const sf::IpAddress&, sf::Uint16);
        void disconnect();
        bool createLocalServer();

        sf::RenderWindow& getRenderWindow();

        virtual void handleEvent(const sf::Event&) = 0;
        virtual void handleMessage(const Message&) = 0;

        virtual void registerStates() = 0;
        virtual void updateApp(float dt) = 0;
        virtual void pauseApp(float dt) = 0;
        virtual void draw() = 0;

        virtual void finalise();

    private:

        struct SettingsFile final
        {
            int ident;
            int version;

            sf::VideoMode videoMode;
            sf::Int32 windowStyle;
            AudioSettings audioSettings;
            GameSettings gameSettings;
        };

        GameSettings m_gameSettings;
        AudioSettings m_audioSettings;

        VideoSettings m_videoSettings;
        sf::RenderWindow m_renderWindow;

        //StateStack m_stateStack;

        FontResource m_fontResource;
        TextureResource m_textureResource;

        MessageBus m_messageBus;

        Scores m_scores;
        Difficulty m_pendingDifficulty;

        std::unique_ptr<GameServer> m_server;
        sf::TcpSocket m_socket;
        bool m_connected;
        sf::Int16 m_clientID;
        std::string m_destinationIP;

        std::function<void(float)> update;

        void loadSettings();
        void saveSettings();

        void saveScreenshot();

        void handleEvents();
        void handleMessages();
    };
}
#endif //APP_HPP_