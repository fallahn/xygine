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

//abstract base class for game states

#ifndef XY_STATE_HPP_
#define XY_STATE_HPP_

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
    using StateId = sf::Int32;
    
    class StateStack;
    class App;
    class Message;
    class State
    {
    public:
        using Ptr = std::unique_ptr<State>;

        struct Context
        {
            Context(sf::RenderWindow& renderWindow, App& app);
            sf::RenderWindow& renderWindow;
            App& appInstance;
            sf::View defaultView;
        };

        State(StateStack& stateStack, Context context);
        virtual ~State() = default;
        State(const State&) = delete;
        const State& operator =(const State&) = delete;

        virtual bool handleEvent(const sf::Event& evt) = 0;
        virtual void handleMessage(const Message&) = 0;
        virtual bool update(float dt) = 0;
        virtual void draw() = 0;
        virtual StateId stateID() const = 0;

        void setContext(Context);

    protected:
        void requestStackPush(StateId id);
        void requestStackPop();
        void requestStackClear();

        Context getContext() const;

        //optionally call these at the beginning and end
        //of a state ctor which loads a lot of resources
        //main game updates are suspended so you MUST
        //call quitLoadingScreen() when finished.
        void launchLoadingScreen();
        void quitLoadingScreen();

        //optionally overload this to draw custom
        //graphics. SYNCHRONISATION IS UP TO YOU.
        virtual void updateLoadingScreen(float dt, sf::RenderWindow&);

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