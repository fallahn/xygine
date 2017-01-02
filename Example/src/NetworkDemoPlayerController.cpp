/*********************************************************************
Matt Marchant 2014 - 2017
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

#include <NetworkDemoPlayerController.hpp>

#include <xygine/Entity.hpp>

using namespace NetDemo;

namespace
{
    const float maxMovement = 20.f;
    const sf::Vector2f bounds(70.f, 1010.f);
}

PlayerController::PlayerController(xy::MessageBus& mb)
    : xy::Component (mb, this),
    m_lastInputId   (0),
    m_entity        (nullptr),
    m_velocity      (0.f),
    m_lastPosition  (0.f)
{

}

//public
void PlayerController::entityUpdate(xy::Entity& entity, float dt)
{
    parseCurrentInput();
    
    auto position = entity.getWorldPosition();
    position.y = std::min(bounds.y, std::max(bounds.x, m_currentInput.position));
    entity.setWorldPosition(position);
}

void PlayerController::onStart(xy::Entity& entity)
{
    m_entity = &entity;
}

void PlayerController::setInput(const Input& ip, bool keep)
{
    if (keep) m_reconcileInputs.push_back(ip);

    m_inputBuffer.push(ip);
}

void PlayerController::reconcile(float position, sf::Uint64 inputId)
{
    while (!m_reconcileInputs.empty() &&
        m_reconcileInputs.front().counter <= inputId)
    {
        m_reconcileInputs.pop_front();
    }

    auto pos = m_entity->getWorldPosition();
    pos.y = position;
    m_entity->setWorldPosition(pos);

    std::queue<Input> newQueue;
    std::swap(m_inputBuffer, newQueue);

    for (const auto& input : m_reconcileInputs)
    {
        m_currentInput = input;
        entityUpdate(*m_entity, 0.f);
    }
}

//private
void PlayerController::parseCurrentInput()
{
    if (!m_inputBuffer.empty())
    {
        m_currentInput = m_inputBuffer.front();
        m_inputBuffer.pop();
    }

    m_velocity = m_currentInput.position - m_lastPosition;
    //if (m_velocity > maxMovement)
    //{
    //    float diff = m_velocity - maxMovement;
    //    m_currentInput.position -= diff; //make sure we can't cheat by sending large movements
    //    m_velocity = maxMovement;
    //}

    m_lastInputId = m_currentInput.counter;
    m_lastPosition = m_currentInput.position;
}


//packet operators
sf::Packet& operator <<(sf::Packet& packet, const NetDemo::Input& ip)
{
    return packet << ip.clientID << ip.counter << ip.position << ip.timestamp;
}

sf::Packet& operator >>(sf::Packet& packet, NetDemo::Input& ip)
{
    return packet >> ip.clientID >> ip.counter >> ip.position >> ip.timestamp;
}