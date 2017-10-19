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
    constexpr float Stiffness = -5.f;
    constexpr float Damping = -1.f;
}

SpringFlowerSystem::SpringFlowerSystem(xy::MessageBus& mb)
    : xy::System(mb, typeid(SpringFlowerSystem))
{
    requireComponent<SpringFlower>();
    requireComponent<xy::Transform>();
    requireComponent<xy::Drawable>();
}

//public
void SpringFlowerSystem::process(float dt)
{
    auto& entities = getEntities();
    for (auto& entity : entities)
    {
        auto& flower = entity.getComponent<SpringFlower>();
        flower.captured = false;

        const auto& tx = entity.getComponent<xy::Transform>();

        //see who's moving past
        auto worldPos = tx.getTransform().transformPoint(flower.headPos);
        auto otherEnts = getScene()->getSystem<xy::QuadTree>().queryPoint(worldPos);
        for (auto other : otherEnts)
        {
            auto otherPos = other.getComponent<xy::Transform>().getPosition();
            if (std::abs(otherPos.x - worldPos.x) < 15.f)
            {
                flower.captured = true;
                auto localPos = tx.getInverseTransform().transformPoint(otherPos);
                auto amount = xy::Util::Math::clamp(localPos.x, -3.f, 3.f);

                flower.headPos.x += amount;// *(1.f - (std::abs(amount) / 15.f));
            }
        }

        if (!flower.captured)
        {
            //free to sproing

            //F = -kx
            sf::Vector2f fSpring = Stiffness * ((flower.headPos - flower.rootPos) - flower.restPos);

            //- bv
            sf::Vector2f fDamp = Damping * flower.velocity;

            //a = f/m
            sf::Vector2f acceleration = (fSpring + fDamp) / flower.mass;

            flower.velocity += acceleration * dt;
            flower.headPos += flower.velocity * dt;
        }

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
}


//private
void SpringFlowerSystem::onEntityAdded(xy::Entity entity)
{
    auto& dwb = entity.getComponent<xy::Drawable>();
    dwb.getVertices().resize(4);
    dwb.setPrimitiveType(sf::Quads);
}
