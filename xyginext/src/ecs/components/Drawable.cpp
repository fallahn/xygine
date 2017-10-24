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

#include <xyginext/ecs/components/Drawable.hpp>

#include <limits>

using namespace xy;

Drawable::Drawable(const sf::Texture& texture)
{
    m_states.texture = &texture;
}

void Drawable::setTexture(const sf::Texture* texture) 
{
    m_states.texture = texture;
}

void Drawable::setShader(sf::Shader* shader)
{
    m_states.shader = shader;
}

void Drawable::setBlendMode(sf::BlendMode mode)
{
    m_states.blendMode = mode;
}

const sf::Texture* Drawable::getTexture() const
{
    return m_states.texture;
}

const sf::Shader* Drawable::getShader() const
{
    return m_states.shader;
}

sf::FloatRect Drawable::getLocalBounds() const
{
    return m_localBounds;
}

void Drawable::updateLocalBounds()
{
    m_localBounds.left = std::numeric_limits<float>::max();
    m_localBounds.top = std::numeric_limits<float>::max();
    m_localBounds.width = 0.f;
    m_localBounds.height = 0.f;

    for (auto& v : m_vertices)
    {
        if (v.position.x < m_localBounds.left)
        {
            m_localBounds.left = v.position.x;
        }
        else if (v.position.x - m_localBounds.left > m_localBounds.width)
        {
            m_localBounds.width = v.position.x - m_localBounds.left;
        }

        if (v.position.y < m_localBounds.top)
        {
            m_localBounds.top = v.position.y;
        }
        else if (v.position.y - m_localBounds.top > m_localBounds.height)
        {
            m_localBounds.height = v.position.y - m_localBounds.top;
        }
    }
}