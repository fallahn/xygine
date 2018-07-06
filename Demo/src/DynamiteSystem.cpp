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

#include "DynamiteSystem.hpp"
#include "Hitbox.hpp"
#include "ClientServerShared.hpp"
#include "MapData.hpp"
#include "PacketIDs.hpp"

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/Scene.hpp>
#include <xyginext/util/Vector.hpp>

#include <xyginext/network/NetHost.hpp>

DynamiteSystem::DynamiteSystem(xy::MessageBus& mb, xy::NetHost& host)
    : xy::System(mb, typeid(DynamiteSystem)),
    m_host(host)
{
    requireComponent<Dynamite>();
    requireComponent<CollisionComponent>();
    requireComponent<xy::Transform>();
}

//public
void DynamiteSystem::process(float dt)
{
    auto& entities = getEntities();
    for (auto& entity : entities)
    {
        auto& dynamite = entity.getComponent<Dynamite>();

        dynamite.lifetime -= dt;
        if (dynamite.lifetime < 0)
        {
            const auto& xForm = entity.getComponent<xy::Transform>();

            //broadcast to client
            ActorEvent evt;
            evt.actor = entity.getComponent<Actor>();
            evt.x = xForm.getPosition().x;
            evt.y = xForm.getPosition().y;
            evt.type = ActorEvent::Died;

            m_host.broadcastPacket(PacketID::ActorEvent, evt, xy::NetFlag::Reliable, 1);
            getScene()->destroyEntity(entity);

            dynamite.callback(255, { evt.x, evt.y });
        }

        if (dynamite.state == Dynamite::State::Falling)
        {
            dynamite.velocity *= 0.94f;
            dynamite.velocity.y += Gravity * dt;
            auto& tx = entity.getComponent<xy::Transform>();
            tx.move(dynamite.velocity * dt);

            const auto& collision = entity.getComponent<CollisionComponent>();
            const auto& hitbox = collision.getHitboxes()[0];
            for (auto i = 0u; i < hitbox.getCollisionCount(); ++i)
            {
                const auto& man = hitbox.getManifolds()[i];
                if (man.otherType == CollisionType::Teleport)
                {
                    if (man.normal.y < 0)
                    {
                        //move up
                        tx.move(0.f, -TeleportDistance);
                    }
                    else
                    {
                        //move down
                        tx.move(0.f, TeleportDistance);
                    }
                }
                else
                {
                    tx.move(man.penetration * man.normal);
                    dynamite.velocity = xy::Util::Vector::reflect(dynamite.velocity, man.normal);

                    if (xy::Util::Vector::lengthSquared(dynamite.velocity) <= ((man.penetration * man.penetration) * 4.f))
                    {
                        dynamite.state = Dynamite::State::Idle;
                    }
                }
            }
        }
    }
}
