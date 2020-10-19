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

#pragma once

#include "InputBinding.hpp"

#include <xyginext/ecs/Entity.hpp>

#include <SFML/Config.hpp>
#include <SFML/System/Clock.hpp>

namespace xy
{
    class NetClient;
}

namespace sf
{
    class Event;
}

/*!
brief Updates the current input mask and applies it to the
current player, along with the client timestamp.
*/
class PlayerInput final
{
public:
    explicit PlayerInput(xy::NetClient&, const InputBinding&);

    void handleEvent(const sf::Event&);
    void update();

    void setPlayerEntity(xy::Entity);
    xy::Entity getPlayerEntity() const;

    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

private:

    xy::NetClient& m_netClient;

    std::uint16_t m_currentInput;
    sf::Clock m_clientTimer;

    std::uint16_t m_prevPad;
    std::uint16_t m_prevStick;

    bool m_enabled;

    xy::Entity m_playerEntity;

    InputBinding m_inputBinding;

    void checkControllerInput();
};
