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

#include "LuggageDirector.hpp"
#include "MessageIDs.hpp"
#include "CrateSystem.hpp"
#include "Hitbox.hpp"
#include "ClientServerShared.hpp"

#include <xyginext/ecs/Entity.hpp>
#include <xyginext/ecs/Scene.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/systems/QuadTree.hpp>

namespace
{

}

LuggageDirector::LuggageDirector() {}

//public
void LuggageDirector::handleMessage(const xy::Message& msg)
{
    if (msg.id == MessageID::PlayerMessage)
    {
        const auto& data = msg.getData<PlayerEvent>();
        if (data.type == PlayerEvent::FiredWeapon)
        {
            auto playerEnt = data.entity;
            auto& luggage = playerEnt.getComponent<Luggage>();

            if (luggage.enabled)
            {
                if (luggage.entityID > 0)
                {
                    //we're carrying so drop
                    auto crateEnt = getScene().getEntity(luggage.entityID);
                    crateEnt.getComponent<Crate>().state = Crate::Falling;
                    crateEnt.getComponent<Crate>().parentID = 0;

                    luggage.entityID = 0;
                }
                else
                {
                    //search immediate area and pick up anything if we find it
                    auto& playerTx = playerEnt.getComponent<xy::Transform>();
                    auto worldPoint = LuggageOffset;
                    worldPoint.x *= playerEnt.getComponent<Player>().sync.direction == Player::Direction::Right ? -1.f : 1.f;
                    worldPoint += playerTx.getPosition();

                    auto queryArea = CrateBounds;
                    queryArea.left = worldPoint.x - (CrateBounds.width / 2.f);
                    queryArea.top = worldPoint.y - (CrateBounds.height / 2.f);

                    const auto& entList = getScene().getSystem<xy::QuadTree>().queryArea(queryArea);
                    std::cout << entList.size() << std::endl;

                    for (const auto& e : entList)
                    {
                        if (e.getComponent<CollisionComponent>().getCollisionCategoryBits()
                            == CollisionFlags::Crate)
                        {                            
                            if (e.getComponent<xy::Transform>().getTransform().transformRect(CrateBounds).contains(worldPoint))
                            {
                                //we have a crate!
                                auto crateEnt = e;
                                crateEnt.getComponent<Crate>().state = Crate::Carried;
                                crateEnt.getComponent<Crate>().velocity = {};
                                crateEnt.getComponent<Crate>().parentID = playerEnt.getIndex();
                                luggage.entityID = crateEnt.getIndex();
                                break;
                            }
                        }
                    }
                }
            }
        }
        else if (data.type == PlayerEvent::Died)
        {
            auto playerEnt = data.entity;
            auto& luggage = playerEnt.getComponent<Luggage>();            
            
            //drop a crate if it's carried
            //and not destroyed.
            if (luggage.entityID > 0)
            {
                //we're carrying so drop
                auto crateEnt = getScene().getEntity(luggage.entityID);
                crateEnt.getComponent<Crate>().state = Crate::Falling;
                crateEnt.getComponent<Crate>().parentID = 0;

                luggage.entityID = 0;
            }
        }
    }
}