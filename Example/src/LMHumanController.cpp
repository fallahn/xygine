/*********************************************************************
Matt Marchant 2014 - 2016
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

#include <LMHumanController.hpp>
#include <CommandIds.hpp>

#include <xygine/Entity.hpp>
#include <xygine/util/Vector.hpp>
#include <xygine/util/Wavetable.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/components/SfDrawableComponent.hpp>

#include <SFML/Graphics/CircleShape.hpp>

using namespace lm;

namespace
{
    const float walkSpeed = 120.f;

    const auto waveTable = xy::Util::Wavetable::sine(5.f, 3.f);
}

HumanController::HumanController(xy::MessageBus& mb)
    : xy::Component     (mb, this),
    m_gotoDestination   (false),
    m_waveTableIndex    (xy::Util::Random::value(0, waveTable.size() - 1))
{

}

//public
void HumanController::entityUpdate(xy::Entity& entity, float dt)
{
    if (m_gotoDestination)
    {
        auto position = entity.getPosition();
        if (std::abs(position.x - m_destination.x) > 2.f)
        {
            float movement = (position.x < m_destination.x) ? walkSpeed : -walkSpeed;
            entity.move(movement * dt, 0.f);
        }
        else
        {
            //we should always be starting below our destination
            entity.move(0.f, -walkSpeed * dt);
            if (position.y - m_destination.y < 2.f)
            {
                //ladies and gentlemen, we have reached out destination
                entity.destroy();
                auto msg = getMessageBus().post<LMEvent>(LMMessageId::LMMessage);
                msg->type = LMEvent::HumanPickedUp;
            }
        }
    }

    //do a little bobbing animation. This assumes as shape component
    //is attached - you'll need to change this if replacing the drawable
    m_waveTableIndex = (m_waveTableIndex + 1) % waveTable.size();
    entity.getComponent<xy::SfDrawableComponent<sf::CircleShape>>()->setPosition(0.f, waveTable[m_waveTableIndex]);
}

void HumanController::setDestination(const sf::Vector2f& dest)
{
    m_destination = dest;
    m_gotoDestination = true;
}