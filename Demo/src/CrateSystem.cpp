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

#include "CrateSystem.hpp"
#include "ClientServerShared.hpp"

#include <xyginext/ecs/components/Transform.hpp>

#include <xyginext/util/Vector.hpp>

namespace
{
    const float MinVelocity = 25.f; //min len sqr
}

CrateSystem::CrateSystem(xy::MessageBus& mb)
    : xy::System(mb, typeid(CrateSystem))
{
    requireComponent<Crate>();
    requireComponent<xy::Transform>();
}

//public
void CrateSystem::process(float dt)
{
    auto& entities = getEntities();
    for (auto& entity : entities)
    {
        auto& crate = entity.getComponent<Crate>();
        switch (crate.state)
        {
        default:break;
        case Crate::Ground:
            crate.velocity *= 0.99f; //friction
            groundCollision(entity);
            break;
        case Crate::Falling:
            crate.velocity.y += Gravity * dt;
            airCollision(entity);
            break;
        case Crate::Breaking:
            //TODO probably moot state, we'll just spawn some particles instead
            break;
        }

        auto& tx = entity.getComponent<xy::Transform>();
        tx.move(crate.velocity * dt);

        if (xy::Util::Vector::lengthSquared(crate.velocity) < MinVelocity)
        {
            crate.velocity = {};
        }
    }
}

//private
void CrateSystem::groundCollision(xy::Entity entity)
{

}

void CrateSystem::airCollision(xy::Entity entity)
{

}