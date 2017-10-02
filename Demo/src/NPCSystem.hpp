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

#ifndef DEMO_NPC_SYSTEM_HPP_
#define DEMO_NPC_SYSTEM_HPP_

#include <xyginext/ecs/System.hpp>

namespace xy
{
    class NetHost;
}

struct NPC final
{
    enum class State
    {
        Normal, Bubble, Jumping, Thinking, Dying
    }state = State::Normal;
    sf::Vector2f velocity;
    sf::Vector2f lastVelocity; //so can be restored when bubble bursts
    float thinkTimer = 0.f;
    bool canLand = true;
    sf::Int32 bubbleOwner = -1;
    bool angry = false;
    xy::Entity target; //used if targetting a player
};

class NPCSystem final : public xy::System
{
public:
    NPCSystem(xy::MessageBus&, xy::NetHost&);

    void handleMessage(const xy::Message&) override;

    void process(float) override;

    void despawn(xy::Entity, sf::Uint8);
private:

    xy::NetHost& m_host;

    void onEntityAdded(xy::Entity) override;

    std::size_t m_currentThinkTime;

    void updateWhirlybob(xy::Entity, float);
    void updateClocksy(xy::Entity, float);
    void updateGoobly(xy::Entity, float);

    void updateBubbleState(xy::Entity, float);
    void updateDyingState(xy::Entity, float);

};

#endif //DEMO_NPC_SYSTEM_HPP_