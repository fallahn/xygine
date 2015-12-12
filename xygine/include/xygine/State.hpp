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

//abstract base class for game states

#ifndef XY_STATE_HPP_
#define XY_STATE_HPP_

#include <SFML/Graphics/View.hpp>

#include <memory>

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

        //TODO will we need loading screen? Could launch a new thread from here

    private:
        StateStack& m_stateStack;
        Context m_context;
    };
}
#endif //XY_STATE_HPP_