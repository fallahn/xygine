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

#include "PlayerInput.hpp"
#include "PlayerSystem.hpp"
#include "MapData.hpp"
#include "PacketIDs.hpp"

#include <xyginext/ecs/Entity.hpp>
#include <xyginext/network/NetClient.hpp>
#include <xyginext/core/App.hpp>

#include "SFML/Window/Event.hpp"


PlayerInput::PlayerInput(xy::NetClient& nc)
    : m_netClient   (nc),
    m_currentInput  (0),
    m_playerEntity  (0, 0)
{

}

//public
void PlayerInput::handleEvent(const sf::Event& evt)
{
    //apply to input mask
    if (evt.type == sf::Event::KeyPressed)
    {
        switch (evt.key.code)
        {
        default: break;
        case sf::Keyboard::W:
            m_currentInput |= InputFlag::Up;
            break;
        case sf::Keyboard::S:
            m_currentInput |= InputFlag::Down;
            break;
        case sf::Keyboard::A:
            m_currentInput |= InputFlag::Left;
                break;
        case sf::Keyboard::D:
            m_currentInput |= InputFlag::Right;
            break;
        case sf::Keyboard::Space:
            m_currentInput |= InputFlag::Shoot;
            break;
        }

    }
    else if (evt.type == sf::Event::KeyReleased)
    {
        switch (evt.key.code)
        {
        default: break;
        case sf::Keyboard::W:
            m_currentInput &= ~InputFlag::Up;
            break;
        case sf::Keyboard::S:
            m_currentInput &= ~InputFlag::Down;
            break;
        case sf::Keyboard::A:
            m_currentInput &= ~InputFlag::Left;
            break;
        case sf::Keyboard::D:
            m_currentInput &= ~InputFlag::Right;
            break;
        case sf::Keyboard::Space:
            m_currentInput &= ~InputFlag::Shoot;
            break;
        }
    }
}

void PlayerInput::update()
{
    if (m_playerEntity.getIndex() == 0) return;

    //set local player input
    auto& player = m_playerEntity.getComponent<Player>();

    Input input;
    input.mask = m_currentInput;
    input.timestamp = m_clientTimer.getElapsedTime().asMicroseconds();

    //update player input history
    player.history[player.currentInput] = input;
    player.currentInput = (player.currentInput + 1) % player.history.size();

    //send input to server
    InputUpdate iu;
    iu.clientTime = input.timestamp;
    iu.input = input.mask;
    iu.playerNumber = player.playerNumber;

    m_netClient.sendPacket<InputUpdate>(PacketID::ClientInput, iu, xy::NetFlag::Reliable, 0);
}

void PlayerInput::setPlayerEntity(xy::Entity entity)
{
    m_playerEntity = entity;
    m_clientTimer.restart();
}

xy::Entity PlayerInput::getPlayerEntity() const
{
    return m_playerEntity;
}
