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

/*!
\mainpage XYGINEXT

A 2D Game Engine Framework built around SFML, xygineXT implements a scene graph along with an Entity Component System.
The source for xygineXT is released under the zlib license in the hope that it may be useful for other people - 
but offers no warranty that it is fit for any particular purpose, even those for which it was specifically written...

*/

#ifndef XY_APP_HPP_
#define XY_APP_HPP_

#include <xyginext/core/StateStack.hpp>
#include <xyginext/core/MessageBus.hpp>
#include <xyginext/Config.hpp>
#include <xyginext/audio/Mixer.hpp>

#include <SFML/Graphics/RenderWindow.hpp>

#include <functional>

#ifdef _DEBUG_
#define DPRINT(x, y) xy::App::printStat(x,y)
#else
#define DPRINT(x,y)
#endif //_DEBUG_

#ifndef APP_NAME
#define APP_NAME "xygine_application"
#endif

/*!
\brief Main xyginext namespace
*/
namespace xy
{   
    /*!
    \brief Abstract base class for new games or applications

    When creating a new game or application this class provides
    management for basic proeprties such as video settings and
    event handling. All events and system messages are despatched
    from here so that they are available across the entire application.
    */
    class XY_EXPORT_API App
    {
    public:

        /*!
        \brief VideoSettings struct

        Contains the current video settings for the application
        which a read and written to configuration file
        */
        struct VideoSettings final
        {
            sf::Int32 WindowStyle;
            sf::VideoMode VideoMode;
            sf::ContextSettings ContextSettings;
            bool VSync;
            std::vector<sf::VideoMode> AvailableVideoModes;
            std::string Title;

            VideoSettings()
                : WindowStyle(sf::Style::Close /*sf::Style::Fullscreen*/),
                VideoMode(800, 600),
                ContextSettings(),
                VSync(true){}

            bool operator == (const VideoSettings& vs)
            {
                if (&vs == this) return true;
                return
                    (vs.VideoMode == this->VideoMode
                    && vs.ContextSettings.antialiasingLevel == this->ContextSettings.antialiasingLevel
                    && vs.VSync == this->VSync
                    && vs.WindowStyle == this->WindowStyle);
            }
        };
        /*!
        \brief Audio settings struct

        Contains the audio settings which are read / written to the settings file
        */
        struct AudioSettings final
        {
            float volume = 1.f;
            std::array<float, AudioMixer::MaxChannels> channelVolumes{};
        };


        /*!
        \brief Constructor.
        \param sf::ContextSettings. 
        Default context settings are supplied when inherting the App class,
        but provide the option to request a specific context if needed.
        For example when creating a game which uses the Model component
        (or other 3D features) a context with OpenGL version 3.2 or 
        higher is needed, as well as a depth buffer.
        */
        App();
        virtual ~App() = default;
        App(const App&) = delete;
        const App& operator = (const App&) = delete;

        /*!
        \brief Starts the application

        This should be called once from within main()
        This will start the application and enter the game loop
        */
        void run();
        /*!
        \brief Pauses the app's logic update function
        */
        void pause();
        /*!
        \brief Resumes the app's update function, if it is paused
        */
        void resume();
        /*!
        \brief Returns a reference to the struct containing
        the current audio settings.

        Audio settings properties are updated via the messaging
        system
        \see MessageBus
        */
        const AudioSettings& getAudioSettings() const;
        /*!
        \brief Returns a reference to a struct containing the current
        Video settings.
        */
        const VideoSettings& getVideoSettings() const;
        /*!
        \brief Applies a given set of video settings

        Video settings can be updated va the messaging system
        or by providing this function with a struct containing
        the desired settings. If requested settings are invalid
        for any reason the settings will not be applied and
        a message will be printed when in debug mode.
        */
        void applyVideoSettings(const VideoSettings&);

        /*!
        \brief Returns a reference to the message bus
        \see MessageBus
        */
        MessageBus& getMessageBus();

        /*!
        \brief Quits the application
        */
        static void quit();

        /*!
        \brief Sets the clear colour for the render window.
        Defaults to sf::Color::Black
        */
        static void setClearColour(sf::Color);

        /*!
        \brief Gets the current clear colour of the app render window
        */
        static sf::Color getClearColour();

        /*!
        \brief Sets the window title.
        Prefer this when setting the window title over setting it directly
        as it will ensure the title is maintained when switching video modes
        */
        void setWindowTitle(const std::string&);

        /*!
        \brief Sets the window icon.
        This should be a path to a 16x16px image. Prefer this to
        setting the icon directly on the window object, as it will
        make sure to maintain the icon when video modes are modified
        */
        void setWindowIcon(const std::string&);

        /*!
        \brief Returns a reference to the current render window
        */
        static sf::RenderWindow& getRenderWindow();

        /*!
        \brief Prints the name/value pair to the stats window
        */
        static void printStat(const std::string&, const std::string&);

        /*!
        \brief Returns a reference to the active App instance
        */
        static App& getActiveInstance();

    protected:
        /*!
        \brief Function for despatching all window events

        This should be implemented in the derived game or application
        to allow passing down any received SFML events throughout the
        program. Usually this would be passing the event to a state stack
        */
        virtual void handleEvent(const sf::Event&) = 0;
        /*!
        \brief Function for despatching messages received by the message bus

        This should be implemented by any derived game or application
        to allow custom or system messages to be handled by xygine objects
        */
        virtual void handleMessage(const Message&) = 0;
        /*!
        \brief Registers a custom state with a state stack

        This should be implements by derived games or applications
        so that custom states can be registered with the xygine
        state stack instance.
        \see State
        */
        virtual void registerStates() = 0;
        /*!
        \brief Updates derived applications with the elapsed frame time
        during the game loop

        Logic updates should be performed here by any game objects such
        as the state stack. The frame time is fixed at 1/60 second
        */
        virtual void updateApp(float dt) = 0;
        /*!
        \brief Draws to the render window.
        All drawing operations should be started from here. clear() and
        display() are called automatically by xygine and need to be manually
        used in derived applications.
        */
        virtual void draw() = 0;

        /*!
        \brief Called when the application is first run.

        Optionally overridable this should be used when performing
        operations which may throw exceptions such as loading configuration
        data, which should not be performed in the constructor of a derived
        application.
        */
        virtual void initialise();

        /*!
        \brief Called when the application shuts down

        Optionally overridable this allows derived classes to tidy
        up any data when the program exits, such as clearing the state
        stack.
        */
        virtual void finalise();

    private:

        AudioSettings m_audioSettings;

        VideoSettings m_videoSettings;
        sf::RenderWindow m_renderWindow;
        sf::Image m_windowIcon;

        MessageBus m_messageBus;

        std::function<void(float)> update;
        std::function<void(const sf::Event&)> eventHandler;

        void saveScreenshot();

        void handleEvents();
        void handleMessages();

        bool m_showStats;
        std::vector<std::string> m_debugLines;
        void doImgui();

        void loadSettings();
        void saveSettings();
    };
}
#endif //XY_APP_HPP_
