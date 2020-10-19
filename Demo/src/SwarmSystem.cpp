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

#include "Swarm.hpp"

#include <xyginext/util/Random.hpp>
#include <xyginext/util/Vector.hpp>
#include <xyginext/util/Math.hpp>

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Drawable.hpp>

namespace
{
    const float BugSpeed = 60.f;
    const float MaxRadiusSq = (256.f * 256.f);
    const float MinRadiusSq = (223.f * 223.f);
}

Swarm::Swarm()
{
    //distribute init pos with poisson disc
    auto dist = xy::Util::Random::poissonDiscDistribution({ 0.f, 0.f, 512.f, 512.f }, 50.f, bugs.size());
    for (auto i = 0u; i < bugs.size(); ++i)
    {
        bugs[i].position = dist[i];

        //set random velocity
        bugs[i].velocity = 
        {
            xy::Util::Random::value(-1.f, 1.f),
            xy::Util::Random::value(-1.f, 1.f)
        };
        bugs[i].velocity = xy::Util::Vector::normalise(bugs[i].velocity);
        bugs[i].brightness = xy::Util::Random::value(0.2f, 1.f);
    }
}

SwarmSystem::SwarmSystem(xy::MessageBus& mb)
    : xy::System(mb, typeid(SwarmSystem))
{
    requireComponent<Swarm>();
    requireComponent<xy::Transform>();
    requireComponent<xy::Drawable>();
}

//public
void SwarmSystem::process(float dt)
{
    auto& entities = getEntities();
    for (auto& entity : entities)
    {
        auto& dwb = entity.getComponent<xy::Drawable>();
        dwb.getVertices().clear();

        auto& swarm = entity.getComponent<Swarm>();
        const auto& tx = entity.getComponent<xy::Transform>().getTransform();

        //update velocity and brightness
        auto bugsCopy = swarm.bugs;
        std::sort(bugsCopy.begin(), bugsCopy.end(), [](const Bug& a, const Bug& b) {return a.brightness > b.brightness; });
        for (auto& bug : swarm.bugs)
        {
            //find brightest bug within radius and go towards it
            for (const auto& otherBug : bugsCopy)
            {
                auto lenSqr = xy::Util::Vector::lengthSquared(otherBug.position - bug.position);
                if (lenSqr == 0)
                {
                    continue;
                }
                else if ((lenSqr < MaxRadiusSq) && (lenSqr > MinRadiusSq))
                {
                    bug.velocity = xy::Util::Vector::normalise(otherBug.position - bug.position);
                    bug.brightness = xy::Util::Math::clamp((MaxRadiusSq - lenSqr) / (MaxRadiusSq - MinRadiusSq), 0.f, 1.f);
                    break;
                }

                //TODO track closest so if there are no bugs nearby we follow that
            }
        }

        //update position
        for (auto& bug : swarm.bugs)
        {
            bug.position += (bug.velocity * BugSpeed * dt);
            dwb.getVertices().push_back(sf::Vertex(tx.transformPoint(bug.position), sf::Color(20, 250, 120, static_cast<std::uint8_t>(255.f * bug.brightness))));
        }
        dwb.updateLocalBounds();
    }
}