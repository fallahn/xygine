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

//stack of game states

#ifndef XY_STATESTACK_HPP_
#define XY_STATESTACK_HPP_

#include <xygine/State.hpp>

#include <map>
#include <functional>
#include <vector>

namespace sf
{
    class RenderWindow;
    class Event;
}

namespace xy
{
    /*!
    \brief Maintains a stack of active states

    States are used to encapsulate different parts of an application or
    game, such as menus or the game state. States can be stacked upon each
    other, for example a pause state may be pushed on top of the game state
    so that it consumes events and input (in a pause menu) but makes sure the
    game state is not updated - hence paused - without losing its state altogether.

    A game or app derived from the App class usually has just one state stack
    to manage any customs states which are created by inheriting the abstract
    base class State. Custom states should be assigned a unique 32 bit ID
    and registered with the state stack.
    \see App::registerStates
    */
    class XY_EXPORT_API StateStack final
    {
    public:
        enum class Action
        {
            Push,
            Pop,
            Clear
        };

        /*!
        \brief Constructor
        \param context State context containing the application state
        */
        explicit StateStack(State::Context context);
        ~StateStack() = default;
        StateStack(const StateStack&) = delete;
        const StateStack& operator = (const StateStack&) = delete;

        /*!
        \brief Registers a custom state with its ID

        When creating new states an ID should be assigned so that
        when a state is requested it can be correctly constructed
        */
        template <typename T>
        void registerState(StateID id)
        {
            static_assert(std::is_base_of<State, T>::value, "Must derive from State class");
            m_factories[id] = [this]()
            {
                return std::make_unique<T>(*this, m_context);
            };
        }
        /*!
        \brief Overload for registering states which have custom
        construction parameters

        \param id Value to register with state
        \param args List of arguments to be passed to the state constructor
        */
        template <typename T, typename... Args>
        void registerState(StateID id, Args&&... args)
        {
            static_assert(std::is_base_of<State, T>::value, "Must derive from State class");
            m_factories[id] = [&args..., this]()
            {
                return std::make_unique<T>(*this, m_context, std::forward<Args>(args)...);
            };
        }

        /*!
        \brief Updates each currently active state

        This should be called from the App::update function
        so that the current frame time is passed down to each
        active state on the stack.
        */
        void update(float dt);
        /*!
        \brief Draws each of the active states on the stack.

        This should happen between the render window clear() and
        display() calls.
        */
        void draw();
        /*!
        \brief Passes the current window event down to all active
        states, as long as the state's event handler returns true
        */
        void handleEvent(const sf::Event& evt);
        /*!
        \brief Passes the current system message down to all active states
        */
        void handleMessage(const Message&);
        /*!
        \brief Push a new instance of a state with the given ID to the stack
        \param id Integer representing the ID of t he state to push on the stack
        */
        void pushState(StateID id);
        /*!
        \brief Pops the top most state from the stack
        */
        void popState();
        /*!
        \brief Clears all states from the state stack
        */
        void clearStates();
        /*!
        \brief Returns true if there are no active states on the stack
        */
        bool empty() const;
        /*!
        \brief Forces the stack to reevaluate the current view in relation to
        the resolution of the render window.

        The stack then correctly calculates the letterboxing and aspect ratio
        before updating the context of existing states  to reflect the change
        */
        sf::View updateView();
        /*!
        \brief Applies any changes requested by states active on the stack.
        */
        void applyPendingChanges();
        /*!
        \brief Returns the number of actives states on the stack
        */
        std::size_t getStackSize() const { return m_stack.size(); }

    private:
        static const StateID bufferID = -100;
        //gets pushed on top of new states for one frame to consume
        //spurious window events
        class BufferState final : public State
        {
        public:
            BufferState(StateStack& s, Context c) : State(s, c) {}
            ~BufferState() = default;

            bool update(float) override { requestStackPop(); return false; }
            bool handleEvent(const sf::Event&) override { return false; }
            void handleMessage(const Message&) override{}
            void draw() override {}
            StateID stateID() const override { return bufferID; }
        };

        struct Pendingchange
        {
            explicit Pendingchange(Action, StateID id = 0, bool = false);
            Action action;
            StateID id = 0;
            bool suspendPrevious = false;
        };

        std::vector<State::Ptr> m_stack;
        std::vector<std::pair<StateID, State::Ptr>> m_suspended;
        std::vector<Pendingchange> m_pendingChanges;
        std::vector<Pendingchange> m_activeChanges;
        State::Context m_context;
        std::map<StateID, std::function<State::Ptr()>> m_factories;
        State::Ptr createState(StateID id);

    };
}
#endif //XY_STATESTACK_HPP_