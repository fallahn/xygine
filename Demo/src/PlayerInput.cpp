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

#include <xyginext/ecs/entity.hpp>
#include <xyginext/network/NetClient.hpp>

#include "SFML/Window/Event.hpp"


PlayerInput::PlayerInput(xy::NetClient& nc)
    : m_netClient   (nc),
    m_currentInput  (0),
    m_playerEntity  (nullptr)
{

}

//public
void PlayerInput::handleEvent(const sf::Event& evt)
{
    //apply to input mask
}

void PlayerInput::update()
{
    if (!m_playerEntity) return;

    //set local player input
    auto& player = m_playerEntity->getComponent<Player>();
    player.input.mask = m_currentInput;
    player.input.timestamp = m_clientTimer.getElapsedTime().asMilliseconds();

    //update player input history
    player.history[player.currentInput] = player.input;
    player.currentInput = (player.currentInput + 1) % player.history.size();

    //send input to server
}

void PlayerInput::setPlayerEntity(xy::Entity& entity)
{
    m_playerEntity = &entity;
    m_clientTimer.restart();
}

xy::Entity PlayerInput::getPlayerEntity() const
{
    return *m_playerEntity;
}