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

#include <LMSpeedMeter.hpp>

#include <SFML/Graphics/RenderTarget.hpp>

using namespace lm;

namespace
{
    const float barWidth = 40.f;
    const float barLength = 600.f;
}

SpeedMeter::SpeedMeter(xy::MessageBus& mb, float maxVal)
    :xy::Component  (mb, this),
    m_maxValue      (maxVal)
{

}

//public
void SpeedMeter::entityUpdate(xy::Entity&, float) 
{
    //drops down to near zero when no player
    setValue(m_currentValue * 0.9f);
}

void SpeedMeter::setValue(float val)
{
    const float ratio = std::min(val / m_maxValue, 1.f);

    m_shape.setSize({ barWidth, barLength * ratio });

    sf::Uint8 red = static_cast<sf::Uint8>(255.f * ratio);
    sf::Uint8 green = static_cast<sf::Uint8>(255.f * (1.f - ratio));

    m_shape.setFillColor({ red, green, 0 });

    m_currentValue = val;
}

//private
void SpeedMeter::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_shape, states);
}