/*********************************************************************
Matt Marchant 2014 - 2016
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

#include <PhysicsDemoInputController.hpp>
#include <PhysicsDemoLine.hpp>

#include <xygine/App.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <xygine/Util.hpp>
#include <xygine/components/AudioSource.hpp>

namespace
{
    const float maxImpulse = 70.f;
    const float maxImpulseSquared = maxImpulse * maxImpulse;
}

using namespace PhysDemo;
PlayerController::PlayerController(xy::MessageBus& mb)
    : xy::Component (mb, this),
    m_entity        (nullptr)
{
    update = [](xy::Entity&, float) {};
}

//public
void PlayerController::entityUpdate(xy::Entity& entity, float dt)
{
    update(entity, dt);
}

void PlayerController::onStart(xy::Entity& entity)
{
    m_entity = &entity;
}

void PlayerController::startInput()
{
    //TODO make sure balls are in a valid state, IE not moving
    
    //make line visible
    auto line = m_entity->getComponent<LineDrawable>();
    line->enable(true);
    
    update = [line](xy::Entity& entity, float)
    {
        //update position of line between ball and mouse
        auto a = entity.getWorldPosition();
        auto b = xy::App::getMouseWorldPosition();
        line->setPoints(a, b);

        //update line colour based on length
        const float len = std::min(xy::Util::Vector::lengthSquared(a - b), maxImpulseSquared);
        const float ratio = len / maxImpulseSquared;
        sf::Uint8 red = static_cast<sf::Uint8>(255.f * ratio);
        sf::Uint8 green = static_cast<sf::Uint8>(255.f * (1.f - ratio));
        line->setColour({ red, green, 0u });
    };
}

void PlayerController::endInput()
{
    XY_ASSERT(m_entity, "Entity is nullptr");
    
    //make line invisible
    m_entity->getComponent<LineDrawable>()->enable(false);

    //fire cueball by applying impulse to entity's physcomponent
    sf::Vector2f impulse = (m_entity->getWorldPosition() - xy::App::getMouseWorldPosition()) * 0.2f;
    if (xy::Util::Vector::lengthSquared(impulse) > maxImpulseSquared)
    {
        impulse = xy::Util::Vector::normalise(impulse) * maxImpulse;
    }

    auto body = m_entity->getComponent<xy::Physics::RigidBody>();
    XY_ASSERT(body, "body is nullptr");
    body->applyLinearImpulse(impulse, body->getWorldCentre());

    auto sound = m_entity->getComponent<xy::AudioSource>("tip_sound");
    XY_ASSERT(sound, "failed to find sound");
    sound->play();

    update = [this](xy::Entity&, float){};
}