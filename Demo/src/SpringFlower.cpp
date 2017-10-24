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

#include "SpringFlower.hpp"

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Drawable.hpp>
#include <xyginext/ecs/Scene.hpp>
#include <xyginext/ecs/systems/QuadTree.hpp>

#include <xyginext/core/App.hpp>

#include <xyginext/util/Math.hpp>
#include <xyginext/util/Wavetable.hpp>

namespace
{
    constexpr float WindStrength = 3.f;
}

SpringFlowerSystem::SpringFlowerSystem(xy::MessageBus& mb)
    : xy::System(mb, typeid(SpringFlowerSystem)),
    m_windIndex (0),
    m_modulatorIndex(0)
{
    requireComponent<SpringFlower>();
    requireComponent<xy::Transform>();
    requireComponent<xy::Drawable>();

    m_windTable = xy::Util::Wavetable::sine(0.125f, WindStrength);
    m_windModulator = xy::Util::Wavetable::sine(0.01f);
    for (auto& f : m_windModulator)
    {
        f += 1.f;
        f /= 2.f;
    }
}

//public
void SpringFlowerSystem::process(float dt)
{
    auto& entities = getEntities();
    for (auto& entity : entities)
    {
        auto& flower = entity.getComponent<SpringFlower>();

        const auto& tx = entity.getComponent<xy::Transform>();

        //see who's moving past
        auto worldPos = tx.getTransform().transformPoint(flower.headPos);
        auto otherEnts = getScene()->getSystem<xy::QuadTree>().queryPoint(worldPos);
        for (auto other : otherEnts)
        {
            auto otherPos = other.getComponent<xy::Transform>().getPosition();
            if (std::abs(otherPos.x - worldPos.x) < 15.f)
            {
                auto amount = other.getComponent<xy::Transform>().getScale().x * -150.f;
                flower.externalForce.x += amount;
            }
        }

        
        //add wind (would look icer with noise but hey)
        flower.externalForce.x += (m_windTable[m_windIndex] + WindStrength) * m_windModulator[m_modulatorIndex];

        //F = -kx
        sf::Vector2f fSpring = flower.stiffness * ((flower.headPos - flower.rootPos) - flower.restPos);
        fSpring += flower.externalForce;
        flower.externalForce *= 0.9f;

        //- bv
        sf::Vector2f fDamp = flower.damping * flower.velocity;

        //a = f/m
        sf::Vector2f acceleration = (fSpring + fDamp) / flower.mass;

        flower.velocity += acceleration * dt;
        flower.headPos += flower.velocity * dt;
        

        //update vertices
        auto& verts = entity.getComponent<xy::Drawable>().getVertices();
        verts[0].position = flower.headPos;
        verts[0].position.x -= flower.textureRect.width / 2.f;
        verts[0].texCoords = { flower.textureRect.left, 0.f };
        verts[0].color = flower.colour;

        verts[1].position = verts[0].position;
        verts[1].position.x += flower.textureRect.width;
        verts[1].texCoords.x = flower.textureRect.left + flower.textureRect.width;
        verts[1].color = flower.colour;

        verts[2].position = flower.rootPos;
        verts[2].position.x += flower.textureRect.width / 2.f;
        verts[2].texCoords = { flower.textureRect.left + flower.textureRect.width, -flower.headPos.y };
        verts[2].color = flower.colour;

        verts[3].position = verts[2].position;
        verts[3].position.x -= flower.textureRect.width;
        verts[3].texCoords = { flower.textureRect.left,  -flower.headPos.y };
        verts[3].color = flower.colour;

        entity.getComponent<xy::Drawable>().updateLocalBounds();
    }

    //update this once per frame, not once per ent DUH
    m_windIndex = (m_windIndex + 1) % m_windTable.size();
    m_modulatorIndex = (m_modulatorIndex + 1) % m_windModulator.size();
}


//private
void SpringFlowerSystem::onEntityAdded(xy::Entity entity)
{
    auto& dwb = entity.getComponent<xy::Drawable>();
    dwb.getVertices().resize(4);
    dwb.setPrimitiveType(sf::Quads);
}
