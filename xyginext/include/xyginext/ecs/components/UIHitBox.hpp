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

#include "xyginext/Config.hpp"

#include <SFML/Config.hpp>
#include <SFML/Graphics/Rect.hpp>

#include <array>

namespace xy
{
    /*!
    \brief Used to trigger callbacks when hit events occur in the component's area
    */
    class XY_EXPORT_API UIHitBox final
    {
    public:
        enum CallbackID
        {
            MouseEnter = 1,
            MouseExit,
            MouseDown,
            MouseUp,
            MouseMotion,
            MouseWheel,
            KeyDown,
            KeyUp,
            Selected,
            Unselected,
            ControllerButtonDown,
            ControllerButtonUp,
            Count
        };

        sf::FloatRect area;
        bool active = false; //TODO do we need this as the system's SelectedIndex property activates the necessary control
        std::array<std::uint32_t, CallbackID::Count> callbacks{};
        std::int32_t ID = -1; //TODO I can't remember what this is for but I don't want to remove it...

        /*!
        \brief Sets the group of UIHitbox controls to which
        this component belongs.
        By default all UIHitbox components are added to
        group 0. When creating multiple menus it is sometimes
        advantagous to activate smaller groups of components
        at a time. Use UISystem::setActiveGroup() to control
        which group of UIHitboxes currently receive input.
        */
        void setGroup(std::size_t group)
        {
            m_previousGroup = m_group;
            m_group = group;
            m_updateGroup = true;
        }

        /*!
        \brief Defines the order in which components in a group are selected.
        By default this is set based on the order in which components
        are added to a group.
        */
        void setSelectionIndex(std::size_t index)
        {
            m_selectionIndex = index;
            m_updateGroup = true;
        }

    private:
        std::size_t m_previousGroup = 0;
        std::size_t m_group = 0;
        std::size_t m_selectionIndex = 0;
        bool m_updateGroup = true; //do order sorting by default

        friend class UISystem;
    };
}
