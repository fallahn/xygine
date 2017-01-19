/*********************************************************************
© Matt Marchant 2014 - 2017
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

//abstract base class for game states

#ifndef XY_STATE_HPP_
#define XY_STATE_HPP_

#include <xygine/Config.hpp>

#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/System/Thread.hpp>
#include <SFML/System/Clock.hpp>

#include <memory>
#include <atomic>

namespace sf
{
    class Event;
    class RenderWindow;
}

namespace xy
{
    using StateID = sf::Int32;
    
    class StateStack;
    class App;
    class Message;

    /*!
    \brief Abstract base class for xygine states

    States are used in conjunction with the StateStack to encapsulate the
    data required to display different states such as the main menu, pause
    screen, or game state
    */
    class XY_EXPORT_API State
    {
    public:
        using Ptr = std::unique_ptr<State>;

        /*!
        \brief Contains information about the current application context

        The default view contains a correctly letterbox view with respect
        to the current window resolution. This is useful when setting the
        initial view of a Scene instance.

        A state's context can be retreived at any time with getContext()
        so that the App properties or RenderWindow can easily be accessed
        */
        struct XY_EXPORT_API Context
        {
            Context(sf::RenderWindow& renderWindow, App& app);
            sf::RenderWindow& renderWindow;
            App& appInstance;
            sf::View defaultView;
        };

        /*!
        \brief Constructor
        \param stateStack State stack to which this state should be added
        \param context The app's current context
        */
        State(StateStack& stateStack, Context context);
        virtual ~State() = default;
        State(const State&) = delete;
        const State& operator =(const State&) = delete;

        /*!
        \brief Receives window events from the state stack to which the state belongs
        \returns true if events should continue to be passed up the stack, else false.
        For example when displaying a pause state over the top of a game state input
        events should be consumed so that they do not affect the running game state
        underneath
        */
        virtual bool handleEvent(const sf::Event& evt) = 0;
        /*!
        \brief Receives system messages from the message bus via the state stack to
        which the state belongs

        Messages can either be handle here, passed on to state members (such as a Scene)
        or both.
        */
        virtual void handleMessage(const Message&) = 0;
        /*!
        \brief Passes in the frame time once per frame

        Members of the state such as a Scene object can be updated here.
        */
        virtual bool update(float dt) = 0;
        /*!
        \brief Function to handle drawing

        The App derived class should handle the clear() and display()
        functions outside of the state stack to which this state belongs
        so any drawing can be done here via getContext().renderWindow
        */
        virtual void draw() = 0;
        /*!
        \brief Returns the unique ID of this state
        */
        virtual StateID stateID() const = 0;
        /*!
        \brief Applies a new context for this state

        Used by xygine to update the default view should the render window
        be resized or toggled between full screen
        */
        void setContext(Context);

    protected:
        /*!
        \brief Request a new instance of a state with the given ID
        be pushed on top of the state stack to which this state belongs.
        \param id StateID of state to be pushed on to the stack
        */
        void requestStackPush(StateID id);
        /*!
        \brief Request that the state on top of the stack to which this
        state belongs is popped (and therefore destroyed)
        */
        void requestStackPop();
        /*!
        \brief Request that all active states in the stack to which this
        state belongs are destroyed.
        */
        void requestStackClear();
        /*!
        \brief Returns a copy of the state's current context
        */
        Context getContext() const;

        /*!
        Optionally call these at the beginning and end
        of a state ctor which loads a lot of resources.
        Main game updates are suspended so you MUST
        call quitLoadingScreen() when finished.
        */
        void launchLoadingScreen();
        void quitLoadingScreen();

        /*!
        Optionally overload this to draw custom graphics during loading.
        As this runs in its own thread SYNCHRONISATION IS UP TO YOU.
        */
        virtual void updateLoadingScreen(float dt, sf::RenderWindow&);

        /*!
        \brief Returns the number of active states on this state's stack
        */
        std::size_t getStateCount() const;

    private:
        StateStack& m_stateStack;
        Context m_context;

        std::atomic<bool> m_threadRunning;
        sf::Thread m_loadingThread;
        sf::Clock m_threadClock;
        sf::RectangleShape m_loadingIcon;
        void loadingScreenThread();
    };
}
#endif //XY_STATE_HPP_