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

#pragma once

#include "xyginext/ecs/System.hpp"

#include <SFML/Config.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <functional>

namespace xy
{
    /*!
    \brief Updates entities with UIHitBox components.
    Any entity with a UIHitBox component will be processed by this system,
    which detects user input and activates the appropriate callback as necessary.
    \see UIHitBox
    */
    class XY_EXPORT_API UISystem final : public System
    {
    public:
        //passes in the entity for whom the callback was triggered and a copy of the flags
        //which contain the input which triggered it. Use the Flags enum to find the input type
        using MouseButtonCallback = std::function<void(Entity, sf::Uint64 flags)>;
        using MovementCallback = std::function<void(Entity, sf::Vector2f)>;
        using KeyboardCallback = std::function<void(Entity, sf::Keyboard::Key)>;
        using SelectionChangedCallback = std::function<void(Entity)>;
        using ControllerCallback = std::function<void(Entity, sf::Uint32, sf::Uint32)>;

        explicit UISystem(MessageBus&);

        /*!
        \brief Used to pass in any events the system may be interested in
        */
        void handleEvent(const sf::Event&);

        /*!
        \brief Performs processing
        */
        void process(float) override;

        /*!
        \brief Message handler
        */
        void handleMessage(const Message&) override;

        /*!
        \brief Adds a mouse button event callback.
        \returns ID of the callback. This should be used to assigned the callback
        to the relative callback slot of a UIInput component. eg:
        auto id = system.addCallback(cb);
        component.callbacks[UIInput::MouseDown] = id;
        Note that a single callback ID may be assigned to multiple UIHitbox components
        */
        sf::Uint32 addMouseButtonCallback(const MouseButtonCallback&);

        /*!
        \brief Adds a mouse or touch input movement callback.
        This is similar to button even callbacks, only the movement delta is
        passed in as a parameter instead of a button ID. These are also used for
        mouse enter/exit events
        Note that a single callback ID may be assigned to multiple UIHitbox components
        */
        sf::Uint32 addMouseMoveCallback(const MovementCallback&);

        /*!
        \brief Adds a KeyEvent callback.
        \param callback KeyboardCallback which provides the current entity and the 
        SFML key which triggered the event
        \returns ID of the callback. This should be used to assigned the callback
        to the relative callback slot of a UIInput component. eg:
        auto id = system.addCallback(cb);
        component.callbacks[UIInput::KeyDown] = id;
        Note that a single callback ID may be assigned to multiple UIHitbox components
        */
        sf::Uint32 addKeyCallback(const KeyboardCallback&);

        /*!
        \brief Adds a selection changed callback.
        This is raised for each UIHitbox component as it either either
        selected or unselected. The callback function passes in the entity
        which is affected by the callback.
        Note that a single callback ID may be assigned to multiple UIHitbox components
        */
        sf::Uint32 addSelectionCallback(const SelectionChangedCallback&);

        /*!
        \brief Adds a Controller Button callback.
        The callback passes in the entity which is affects, as well as the controller
        ID and Button ID which triggered the event
        */
        sf::Uint32 addControllerCallback(const ControllerCallback&);

        /*!
        \brief Input flags.
        Use these with the callback bitmask to find which input triggered it
        */
        enum Flags
        {
            RightMouse = 0x1,
            LeftMouse = 0x2,
            MiddleMouse = 0x4,
            Finger = 0x8
        };

        /*!
        \brief Selects the input at the given index if it exists
        */
        void selectInput(std::size_t);

        /*!
        \brief Enables controlling the mouse cursor with the controller
        connected to port 0.
        When this is enabled all mouse movement callbacks are executed
        as they would with a normal mouse move, controller button and mouse button
        callbacks remain unchanged.
        \param active If true enables controlling the mouse cursor with the controller
        */
        void setJoypadCursorActive(bool active);

    private:

        std::vector<MouseButtonCallback> m_buttonCallbacks;
        std::vector<MovementCallback> m_movementCallbacks;
        std::vector<KeyboardCallback> m_keyboardCallbacks;
        std::vector<SelectionChangedCallback> m_selectionCallbacks;
        std::vector<ControllerCallback> m_controllerCallbacks;

        sf::Vector2f m_prevMousePosition;
        sf::Vector2f m_previousEventPosition; //in screen coords
        sf::Vector2f m_eventPosition;
        sf::Vector2f m_movementDelta; //in world coords

        std::vector<Flags> m_mouseDownEvents;
        std::vector<Flags> m_mouseUpEvents;

        std::size_t m_selectedIndex;
        std::vector<sf::Keyboard::Key> m_keyDownEvents;
        std::vector<sf::Keyboard::Key> m_keyUpEvents;

        std::vector<std::pair<sf::Uint32, sf::Uint32>> m_controllerDownEvents;
        std::vector<std::pair<sf::Uint32, sf::Uint32>> m_controllerUpEvents;
        sf::Uint8 m_controllerMask;
        sf::Uint8 m_prevControllerMask;
        enum ControllerBits
        {
            Up = 0x1, Down = 0x2, Left = 0x4, Right = 0x8
        };

        bool m_joypadCursorActive;

        void selectNext();
        void selectPrev();

        void unselect(std::size_t);
        void select(std::size_t);

        void onEntityAdded(xy::Entity) override;
        void onEntityRemoved(xy::Entity) override;
    };
}
