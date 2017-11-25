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

#ifndef DEMO_EXPLOSION_HPP_
#define DEMO_EXPLOSION_HPP_

#include "MapAnimator.hpp"

#include <xyginext/ecs/System.hpp>
#include <xyginext/ecs/Entity.hpp>
#include <xyginext/ecs/Scene.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/network/NetHost.hpp>

struct Explosion final
{
    float lifetime = 0.8f;
    sf::Uint8 owner = 3;
};

class ExplosionSystem final : public xy::System
{
public:
    ExplosionSystem(xy::MessageBus& mb, xy::NetHost& host)
        : xy::System(mb, typeid(ExplosionSystem)),
        m_host(host)
    {
        requireComponent<Explosion>();
    }

    void process(float dt) override
    {
        auto& entities = getEntities();
        for (auto& entity : entities)
        {
            auto& exp = entity.getComponent<Explosion>();
            exp.lifetime -= dt;
            if (exp.lifetime < 0)
            {
                getScene()->destroyEntity(entity);

                const auto& tx = entity.getComponent<xy::Transform>();

                ActorEvent evt;
                evt.actor = entity.getComponent<Actor>();
                evt.type = ActorEvent::Died;
                evt.x = tx.getPosition().x;
                evt.y = tx.getPosition().y;

                m_host.broadcastPacket(PacketID::ActorEvent, evt, xy::NetFlag::Reliable, 1);
            }
        }
    }

private:
    xy::NetHost& m_host;
};

#endif //DEMO_EXPLOSION_HPP_