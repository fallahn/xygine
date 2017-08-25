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

#ifndef XY_UI_SYSTEM_HPP_
#define XY_UI_SYSTEM_HPP_

#include <xyginext/ecs/System.hpp>

#include <SFML/Config.hpp>
#include <SFML/graphics/Rect.hpp>

#include <functional>

namespace xy
{
    class XY_EXPORT_API UISystem final : public System
    {
    public:
        //passes in the entity for whom the callback was triggered and a copy of the flags
        //which contain the input which triggered it. Use the Flags enum to find the input type
        using ButtonCallback = std::function<void(Entity, sf::Uint64 flags)>;
        using MovementCallback = std::function<void(Entity, sf::Vector2f)>;

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
        \brief Adds a button event callback.
        \returns ID of the callback. This should be used to assigned the callback
        to the relative callback slot of a UIInput component. eg:
        auto id = system.addCallback(cb);
        component.callbacks[UIInput::MouseDown] = id;
        */
        sf::Uint32 addCallback(const ButtonCallback&);

        /*!
        \brief Adds a mouse or touch input movement callback.
        This is similar to button even callbacks, only the movement delta is
        passed in as a parameter instead of a button ID. These are also used for
        mouse enter/exit events
        */
        sf::Uint32 addCallback(const MovementCallback&);

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

    private:

        std::vector<ButtonCallback> m_buttonCallbacks;
        std::vector<MovementCallback> m_movementCallbacks;

        sf::Vector2f m_prevMousePosition;
        sf::Vector2f m_previousEventPosition; //in screen coords
        sf::Vector2f m_eventPosition;
        sf::Vector2f m_movementDelta; //in world coords

        std::vector<Flags> m_downEvents;
        std::vector<Flags> m_upEvents;
    };
}

#endif //XY_UI_SYSTEM_HPP_