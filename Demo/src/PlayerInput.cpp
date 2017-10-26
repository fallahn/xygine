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

namespace
{
    const float deadZone = 20.f;
}

PlayerInput::PlayerInput(xy::NetClient& nc, const InputBinding& ib)
    : m_netClient   (nc),
    m_currentInput  (0),
    m_prevPad       (0),
    m_prevStick     (0),
    m_enabled       (false),
    m_playerEntity  (0,0),
    m_inputBinding  (ib)
{

}

//public
void PlayerInput::handleEvent(const sf::Event& evt)
{
    //apply to input mask
    if (evt.type == sf::Event::KeyPressed)
    {
        if (evt.key.code == m_inputBinding.keys[InputBinding::Jump])
        {
            m_currentInput |= InputFlag::Up;
        }
        else if (evt.key.code == m_inputBinding.keys[InputBinding::Left])
        {
            m_currentInput |= InputFlag::Left;
        }
        else if (evt.key.code == m_inputBinding.keys[InputBinding::Right])
        {
            m_currentInput |= InputFlag::Right;
        }
        else if (evt.key.code == m_inputBinding.keys[InputBinding::Shoot])
        {
            m_currentInput |= InputFlag::Shoot;
        }
    }
    else if (evt.type == sf::Event::KeyReleased)
    {
        if (evt.key.code == m_inputBinding.keys[InputBinding::Jump])
        {
            m_currentInput &= ~InputFlag::Up;
        }
        else if (evt.key.code == m_inputBinding.keys[InputBinding::Left])
        {
            m_currentInput &= ~InputFlag::Left;
        }
        else if (evt.key.code == m_inputBinding.keys[InputBinding::Right])
        {
            m_currentInput &= ~InputFlag::Right;
        }
        else if (evt.key.code == m_inputBinding.keys[InputBinding::Shoot])
        {
            m_currentInput &= ~InputFlag::Shoot;
        }
    }
    else if (evt.type == sf::Event::JoystickButtonPressed)
    {
        if (evt.joystickButton.joystickId == m_inputBinding.controllerID)
        {
            if (evt.joystickButton.button == m_inputBinding.buttons[InputBinding::Jump])
            {
                m_currentInput |= InputFlag::Up;
            }
            else if (evt.joystickButton.button == m_inputBinding.buttons[InputBinding::Shoot])
            {
                m_currentInput |= InputFlag::Shoot;
            }
        }
    }
    else if (evt.type == sf::Event::JoystickButtonReleased)
    {
        if (evt.joystickButton.joystickId == m_inputBinding.controllerID)
        {
            if (evt.joystickButton.button == m_inputBinding.buttons[InputBinding::Jump])
            {
                m_currentInput &= ~InputFlag::Up;
            }
            else if (evt.joystickButton.button == m_inputBinding.buttons[InputBinding::Shoot])
            {
                m_currentInput &= ~InputFlag::Shoot;
            }
        }
    }
}

void PlayerInput::update()
{
    if (m_playerEntity.getIndex() == 0) return;

    //check state of controller axis
    checkControllerInput();

    //set local player input
    auto& player = m_playerEntity.getComponent<Player>();

    Input input;
    input.mask = m_enabled ? m_currentInput : 0;
    input.timestamp = m_clientTimer.getElapsedTime().asMicroseconds();

    //update player input history
    player.history[player.currentInput].input = input;
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

//private
void PlayerInput::checkControllerInput()
{
    if (m_inputBinding.controllerID > sf::Joystick::Count) return;

    auto startInput = m_currentInput;

    m_prevPad = 0; 

    //DPad
    if (sf::Joystick::getAxisPosition(m_inputBinding.controllerID, sf::Joystick::PovX) < -deadZone)
    {
        m_currentInput |= InputFlag::Left;
    }
    else if (m_prevPad & InputFlag::Left)
    {
        m_currentInput &= ~InputFlag::Left;
    }

    if (sf::Joystick::getAxisPosition(m_inputBinding.controllerID, sf::Joystick::PovX) > deadZone)
    {
        m_currentInput |= InputFlag::Right;
    }
    else if (m_prevPad & InputFlag::Right)
    {
        m_currentInput &= ~InputFlag::Right;
    }
    if (startInput ^ m_currentInput)
    {
        m_prevPad = m_currentInput;
        return; //prevent analogue stick overwriting state
    }

    
    m_prevStick = 0;
    //left stick (xbox controller)
    if (sf::Joystick::getAxisPosition(m_inputBinding.controllerID, sf::Joystick::X) < -deadZone)
    {
        m_currentInput |= InputFlag::Left;
    }
    else if (m_prevStick & InputFlag::Left)
    {
        m_currentInput &= ~InputFlag::Left;
    }

    if (sf::Joystick::getAxisPosition(m_inputBinding.controllerID, sf::Joystick::X) > deadZone)
    {
        m_currentInput |= InputFlag::Right;
    }
    else if (m_prevStick & InputFlag::Right)
    {
        m_currentInput &= ~InputFlag::Right;
    }
    if (startInput ^ m_currentInput)
    {
        m_prevStick = m_currentInput;
    }
}