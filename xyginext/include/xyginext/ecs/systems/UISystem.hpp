/*********************************************************************
(c) Matt Marchant 2017 - 2020
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
#include <unordered_map>

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
        using MouseButtonCallback = std::function<void(Entity, std::uint64_t flags)>;
        //passes in the entity for which the callback is called, plus the delta movement
        //of the mouse move which triggered the callback
        using MovementCallback = std::function<void(Entity, sf::Vector2f)>;
        //passes in the entity for which the callback is called, whether the mousewheel
        //is vertical (true) or horizontal (false) and the delta movement
        using MouseWheelCallback = std::function<void(Entity, bool, float)>;
        //passes in the entity for which the callback was called, plus the
        //keyboard ident of the key which triggered the callback
        using KeyboardCallback = std::function<void(Entity, sf::Keyboard::Key)>;
        //passes in the entity whose selection state was changed
        using SelectionChangedCallback = std::function<void(Entity)>;
        //passes in the entity for which the callback was triggered, followed
        //by the ID of the controller, and the ID of button which triggered the callback
        using ControllerCallback = std::function<void(Entity, std::uint32_t, std::uint32_t)>;

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
        std::uint32_t addMouseButtonCallback(const MouseButtonCallback&);

        /*!
        \brief Adds a mouse or touch input movement callback.
        This is similar to button even callbacks, only the movement delta is
        passed in as a parameter instead of a button ID. These are also used for
        mouse enter/exit events
        Note that a single callback ID may be assigned to multiple UIHitbox components
        */
        std::uint32_t addMouseMoveCallback(const MovementCallback&);

        /*!
        \brief Adds a mouse wheel input callback.
        Mouse wheel callbacks handle scroll events from the mouse wheel.
        Callback parameters are:
        Entity the current UI entity which triggered this callback
        bool true if the mouse wheel is vertical (most common) false if it is horizontal
        float delta - positive is up/left, negative is down/right. May be non-integral on some mice
        \returns A callback ID which may be assigned to multiple UIHitboxes
        */
        std::uint32_t addMouseWheelCallback(const MouseWheelCallback&);

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
        std::uint32_t addKeyCallback(const KeyboardCallback&);

        /*!
        \brief Adds a selection changed callback.
        This is raised for each UIHitbox component as it either either
        selected or unselected. The callback function passes in the entity
        which is affected by the callback.
        Note that a single callback ID may be assigned to multiple UIHitbox components
        */
        std::uint32_t addSelectionCallback(const SelectionChangedCallback&);

        /*!
        \brief Adds a Controller Button callback.
        The callback passes in the entity which is affects, as well as the controller
        ID and Button ID which triggered the event
        */
        std::uint32_t addControllerCallback(const ControllerCallback&);

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
        \brief Selects the input at the given index if it exists.
        This is applied to the active group of components.
        \see setActiveGroup()
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

        /*!
        \brief Sets the active group of UIHitbox components.
        By default all components are added to group 0. If a single
        UISystem handles multiple menus, for example, UIHitbox components
        can be grouped by a given index, one for each menu. This function
        will set the active group of UIHitbox components to recieve events.
        \see xy::UIHitbox::setGroup()
        */
        void setActiveGroup(std::size_t);

        /*!
        \brief Returns the current active group
        */
        std::size_t getActiveGroup() const { return m_activeGroup; }

        /*!
        \brief Returns the selected control index within the current group
        */
        std::size_t getSelectedInput() const { return m_selectedIndex; }

        /*
        \brief Set the number of columns displayed in the active group.
        By default the 'up' and 'down' controls move the selected control index
        in the same direction as the 'left' and 'right' controls - that is, +1
        or -1 from the currently selected index. In layouts where multiple
        columns of UI controls exists this feels counter-intuitive as the selected
        index does not visually move up and down the screen. By setting this value
        to the number of columns in the display the 'up' and 'down' controls will
        jump by +/- this number of indices, effectively moving an entire row at a
        time. This value is not saved per group so when setting a new group active
        that has a different number of columns, this function should be used to
        set the new active value.

        \param count The number of columns in the display (or number of indices to
        skip when moving up/down). Must be greater than zero.
        */
        void setColumnCount(std::size_t count);

    private:

        std::vector<MouseButtonCallback> m_buttonCallbacks;
        std::vector<MovementCallback> m_movementCallbacks;
        std::vector<MouseWheelCallback> m_wheelCallbacks;
        std::vector<KeyboardCallback> m_keyboardCallbacks;
        std::vector<SelectionChangedCallback> m_selectionCallbacks;
        std::vector<ControllerCallback> m_controllerCallbacks;

        sf::Vector2f m_prevMousePosition;
        sf::Vector2f m_previousEventPosition; //in screen coords
        sf::Vector2f m_eventPosition;
        sf::Vector2f m_movementDelta; //in world coords

        std::vector<Flags> m_mouseDownEvents;
        std::vector<Flags> m_mouseUpEvents;

        struct WheelEvent final
        {
            bool vertical = true;
            float delta = 0.f;
        };
        std::vector<WheelEvent> m_mouseWheelEvents;

        std::size_t m_selectedIndex;
        std::vector<sf::Keyboard::Key> m_keyDownEvents;
        std::vector<sf::Keyboard::Key> m_keyUpEvents;

        std::vector<std::pair<std::uint32_t, std::uint32_t>> m_controllerDownEvents;
        std::vector<std::pair<std::uint32_t, std::uint32_t>> m_controllerUpEvents;
        std::uint8_t m_controllerMask;
        std::uint8_t m_prevControllerMask;
        enum ControllerBits
        {
            Up = 0x1, Down = 0x2, Left = 0x4, Right = 0x8
        };

        bool m_joypadCursorActive;

        std::unordered_map<std::size_t, std::vector<Entity>> m_groups;
        std::size_t m_activeGroup;

        std::size_t m_columnCount;

        void selectNext(std::size_t);
        void selectPrev(std::size_t);

        void unselect(std::size_t);
        void select(std::size_t);

        void updateGroupAssignments();

        void onEntityAdded(xy::Entity) override;
        void onEntityRemoved(xy::Entity) override;
    };
}
