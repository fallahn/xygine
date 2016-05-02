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

#include <xygine/ParticleAffectors.hpp>
#include <xygine/components/ParticleSystem.hpp>
#include <xygine/util/Random.hpp>

namespace
{
    sf::Uint8 lerp(sf::Uint8 start, sf::Uint8 end, float amount)
    {
        float val = static_cast<float>(end - start) * amount;
        return start + static_cast<sf::Uint8>(val);
    }
}

//------------------------------------
using namespace xy;

ForceAffector::ForceAffector(const sf::Vector2f& force) 
    : m_force   (force),
    m_random    (false){}

void ForceAffector::operator() (Particle& p, float dt)
{
    if (m_random)
    {
        m_force.x = Util::Random::value(m_randomStart.x, m_randomEnd.x);
        m_force.y = Util::Random::value(m_randomStart.y, m_randomEnd.y);
    }

    p.velocity += m_force * dt;
}

void ForceAffector::setRandom(const sf::Vector2f& rangeStart, const sf::Vector2f& rangeEnd)
{
    XY_ASSERT(rangeStart.x < rangeEnd.x && rangeStart.y < rangeEnd.y, "invalid range value");
    
    m_randomStart = rangeStart;
    m_randomEnd = rangeEnd;
    m_random = true;
}

//------------------------------------
ColourAffector::ColourAffector(const sf::Color& start, const sf::Color& end, float duration)
    : m_duration(duration), m_start(start), m_end(end){}

void ColourAffector::operator() (Particle& p, float dt)
{
    float ratio = (m_duration - p.lifetime) / m_duration;
    ratio = std::max(0.f, std::min(ratio, 1.f));

    p.colour.r = lerp(m_start.r, m_end.r, ratio);
    p.colour.g = lerp(m_start.g, m_end.g, ratio);
    p.colour.b = lerp(m_start.b, m_end.b, ratio);
}

//------------------------------------
RotateAffector::RotateAffector(float rotation)
    : m_rotation(rotation){}

void RotateAffector::operator() (Particle& p, float dt)
{
    p.rotate(m_rotation * dt);
}

//------------------------------------
ScaleAffector::ScaleAffector(const sf::Vector2f& scale)
    : m_scale(scale){}

void ScaleAffector::operator()(Particle& p, float dt)
{
    p.setScale(p.getScale() + (m_scale *  dt));
}

//------------------------------------
VelocityAffector::VelocityAffector(const sf::Vector2f& scale)
    : m_scale(scale) {}

void VelocityAffector::operator()(Particle& p, float dt)
{
    p.velocity.x *= m_scale.x;
    p.velocity.y *= m_scale.y;
}