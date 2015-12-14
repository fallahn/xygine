/*********************************************************************
Matt Marchant 2014 - 2015
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

#include <xygine/physics/World.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

std::function<void(float)> xy::Physics::World::update = [](float) {};

float xy::Physics::World::m_worldScale = 100.f;
b2Vec2 xy::Physics::World::m_gravity = { 0.f, -9.8f };
sf::Uint32 xy::Physics::World::m_velocityIterations = 6u;
sf::Uint32 xy::Physics::World::m_positionIterations = 2u;

xy::Physics::World::Ptr xy::Physics::World::m_world = nullptr;

void xy::Physics::World::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    if (!m_debugDraw)
    {
        m_debugDraw = std::make_unique<xy::Physics::DebugDraw>(rt);
        m_world->SetDebugDraw(m_debugDraw.get());
    }
    m_world->DrawDebugData();
}