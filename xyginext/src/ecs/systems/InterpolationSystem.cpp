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

#include <xyginext/ecs/systems/InterpolationSystem.hpp>

#include <xyginext/ecs/components/NetInterpolation.hpp>
#include <xyginext/ecs/components/Transform.hpp>

#include <xyginext/util/Vector.hpp>

using namespace xy;

namespace
{
    const float MaxDistSqr = 460.f * 460.f; //if we're bigger than this go straight to dest to hide flickering
}

InterpolationSystem::InterpolationSystem(xy::MessageBus& mb)
    : System(mb, typeid(InterpolationSystem)),
    m_enabled(true)
{
    requireComponent<Transform>();
    requireComponent<NetInterpolate>();
}

//public
void InterpolationSystem::process(float dt)
{
    if (!m_enabled) return;
    
    auto& entities = getEntities();
    for (auto& entity : entities)
    {
        auto& tx = entity.getComponent<Transform>();
        auto& interp = entity.getComponent<NetInterpolate>();

        //if time has been reset we ought to have reached the previous target by now.
        if (interp.m_elapsedTime == 0)
        {
            tx.setPosition(interp.m_previousPosition);
        }

        interp.m_elapsedTime += dt;
        
        auto diff = (interp.m_targetPosition - interp.m_previousPosition);
        if (Util::Vector::lengthSquared(diff) > MaxDistSqr)
        {
            interp.m_elapsedTime = 0;
            tx.setPosition(interp.m_targetPosition);
            return;
        }

        //previous position + diff * timePassed
        tx.setPosition(interp.m_previousPosition + (diff * std::min(interp.m_elapsedTime / interp.m_timeDifference, 1.f)));
    }
}

//private
void InterpolationSystem::onEntityAdded(Entity entity)
{
    //make sure the initial transform is applied
    auto pos = entity.getComponent<Transform>().getPosition();
    entity.getComponent<NetInterpolate>().m_targetPosition = pos;
    entity.getComponent<NetInterpolate>().m_previousPosition = pos;
}