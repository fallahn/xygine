/*********************************************************************
© Matt Marchant 2014 - 2017
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

#include <xygine/components/PointLight.hpp>
#include <xygine/Entity.hpp>
#include <xygine/Assert.hpp>

using namespace xy;

PointLight::PointLight(MessageBus& mb, float range, float radius, const sf::Color& diffuse, const sf::Color& spec)
    : Component     (mb, this),
    m_range         (range),
    m_radius        (radius),
    m_inverseRange  (1.f / m_range),
    m_position      (0.f, 0.f, 1300.f),
    m_intensity     (1.f),
    m_diffuseColour (diffuse),
    m_specularColour(spec),
    m_castShadows   (false)
{
    XY_ASSERT(radius > 0, "light radius must be greater than zero");
}

//public
void PointLight::entityUpdate(Entity& entity, float dt)
{
    auto worldPos = entity.getWorldPosition();
    m_position.x = worldPos.x;
    m_position.y = worldPos.y;
}

void PointLight::setDepth(float depth)
{
    m_position.z = depth;
}

void PointLight::setIntensity(float intensity)
{
    XY_ASSERT(intensity >= 0, "Intensity must be positive value");
    m_intensity = intensity;
}

void PointLight::setRange(float range)
{
    //don't div0 kplzthx
    XY_ASSERT(range > 0, "Range must be more than 0");
    m_range = range;
    m_inverseRange = 1.f / range;
}

void PointLight::setRadius(float radius)
{
    XY_ASSERT(radius > 0, "light radius must be greater than zero");
    m_radius = radius;
}

void PointLight::setDiffuseColour(const sf::Color& c)
{
    m_diffuseColour = c;
}

void PointLight::setSpecularColour(const sf::Color& c)
{
    m_specularColour = c;
}

const sf::Vector3f& PointLight::getWorldPosition() const
{
    return m_position;
}

float PointLight::getIntensity() const
{
    return m_intensity;
}

float PointLight::getInverseRange() const
{
    return m_inverseRange;
}

float PointLight::getRadius() const
{
    return m_radius;
}

const sf::Color& PointLight::getDiffuseColour() const
{
    return m_diffuseColour;
}

const sf::Color& PointLight::getSpecularColour() const
{
    return m_specularColour;
}