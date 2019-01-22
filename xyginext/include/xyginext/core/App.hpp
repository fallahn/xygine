/*********************************************************************
(c) Matt Marchant 2017 - 2019
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
\mainpage xyginext

A 2D Game Engine Framework built around SFML, xyginext implements a scene graph along with an Entity Component System.
The source for xyginext is released under the zlib license in the hope that it may be useful for other people - 
but offers no warranty that it is fit for any particular purpose, even those for which it was specifically written...

Explanations of systems and examples can be found on <a href="https://github.com/fallahn/xygine/wiki">the xygine wiki</a>.

The older revision of xygine can be found <a href="https://github.com/fallahn/xygine/tree/legacy">in its own branch</a>.

*/

#pragma once

#include "xyginext/core/StateStack.hpp"
#include "xyginext/core/MessageBus.hpp"
#include "xyginext/Config.hpp"
#include "xyginext/audio/Mixer.hpp"

#include <SFML/Graphics/RenderWindow.hpp>

#include <functional>

#ifdef XY_DEBUG
#define DPRINT(x, y) xy::App::printStat(x,y)
#else
#define DPRINT(x,y)
#endif //XY_DEBUG

#ifndef APP_NAME
#define APP_NAME "xygine_application"
#endif

/*!
\brief Main xyginext namespace
*/
namespace xy
{   
    class GuiClient;

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
        which are read and written to configuration file
        */
        struct VideoSettings final
        {
            sf::Int32 WindowStyle;
            sf::VideoMode VideoMode;
            sf::ContextSettings ContextSettings;
            bool VSync;
            sf::Uint32 FrameLimit;
            std::vector<sf::VideoMode> AvailableVideoModes;
            std::string Title;

            VideoSettings(sf::ContextSettings contextSettings = sf::ContextSettings())
                : WindowStyle(sf::Style::Close /*sf::Style::Fullscreen*/),
                VideoMode       (800, 600),
                ContextSettings (contextSettings),
                VSync           (true),
                FrameLimit      (0){}

            bool operator == (const VideoSettings& vs)
            {
                if (&vs == this) return true;
                return
                    (vs.VideoMode == this->VideoMode
                    && vs.ContextSettings.antialiasingLevel == this->ContextSettings.antialiasingLevel
                    && vs.ContextSettings.depthBits == this->ContextSettings.depthBits
                    && vs.VSync == this->VSync
                    && vs.WindowStyle == this->WindowStyle
                    && vs.FrameLimit == this->FrameLimit);
            }

            bool operator != (const VideoSettings& vs)
            {
                return !(*this == vs);
            }
        };

        /*!
        \brief Constructor.
        \param sf::ContextSettings. 
        Default context settings are supplied when inherting the App class,
        but provide the option to request a specific context if needed.
        */
        App(sf::ContextSettings contextSettings = sf::ContextSettings());
        virtual ~App() = default;
        App(const App&) = delete;
        App(App&&) = delete;
        App& operator = (const App&) = delete;
        App& operator = (App&&) = delete;

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
        \brief Returns a reference to a struct containing the current
        Video settings.
        */
        const VideoSettings& getVideoSettings() const;
        /*!
        \brief Applies a given set of video settings

        This is the preferred method for resizing the window,
        as SFML does not raise Resized events when calling
        Window::setSize(). This function ensures a message is
        posted to the message bus in such cases, so that active
        scenes and states can correctly re-calculate the render view.
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
        This should be a path to a 16x16px image on most platforms,
        although on macOS this icon should be larger as it appears 
        in the dock. Prefer this to setting the icon directly on the
        window object, as it will make sure to maintain the icon when
        video modes are modified
        */
        void setWindowIcon(const std::string&);

        /*!
        \brief Returns a reference to the current render window
        */
        static sf::RenderWindow* getRenderWindow();

        /*!
        \brief Prints the name/value pair to the stats window
        */
        static void printStat(const std::string&, const std::string&);

        /*!
        \brief Returns a reference to the active App instance
        */
        static App* getActiveInstance();

        /*!
        \brief Sets the application name.
        This is used when reading/writing config files such as window settings
        to the current user directory. A directory with this name is created
        to store the configurations files, and so should be unique to the application
        to prevent cross pollution of settings between xygine apps. This is set
        to "xygine_application" by default.
        */
        void setApplicationName(const std::string& name);

        /*!
        \brief Returns the current application name string.
        */
        const std::string& getApplicationName() const;

        /*!
        \brief Sets whether or not the mouse cursor should be visible.
        Prefer this rather than setting on the render window directly
        to prevent ImGUI settings overriding the window setting.
        */
        static void setMouseCursorVisible(bool);

        /*!
        \brief Returns whether or not the mouse cursor is set to be visible
        */
        static bool isMouseCursorVisible();

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
        If initialisation is successful then this should return true, else
        return false. Returning false will prevent the application running
        and cause it to attempt to perform a clean shutdown with finalise();
        */
        virtual bool initialise();

        /*!
        \brief Called when the application shuts down

        Optionally overridable this allows derived classes to tidy
        up any data when the program exits, such as clearing the state
        stack.
        */
        virtual void finalise();

    private:

        VideoSettings m_videoSettings;
        sf::RenderWindow m_renderWindow;
        sf::Image m_windowIcon;
        std::string m_applicationName;

        MessageBus m_messageBus;

        std::function<void(float)> update;
        std::function<void(const sf::Event&)> eventHandler;

        static bool m_mouseCursorVisible;

        void saveScreenshot();

        void handleEvents();
        void handleMessages();

        std::vector<std::pair<std::function<void()>, const GuiClient*>> m_guiWindows;

        static void addStatusControl(const std::function<void()>&, const GuiClient*);
        static void removeStatusControls(const GuiClient*);

        static void addConsoleTab(const std::string&, const std::function<void()>&, const GuiClient*);
        static void removeConsoleTab(const GuiClient*);

        static void addWindow(const std::function<void()>&, const GuiClient*);
        static void removeWindows(const GuiClient*);

        friend class GuiClient;

        void loadSettings();
        void saveSettings();
    };
}
