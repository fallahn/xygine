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

#include <xyginext/ecs/components/Sprite.hpp>

#include <SFML/Graphics/Texture.hpp>

using namespace xy;

Sprite::Sprite()
    : m_dirty       (true),
    m_animationCount(0),
    m_zDepth        (0),
    m_wantsSorting  (false)
{

}

Sprite::Sprite(const sf::Texture& texture)
    : m_dirty       (true),
    m_animationCount(0),
    m_zDepth        (0),
    m_wantsSorting  (false)
{
    setTexture(texture);
}

//public
void Sprite::setTexture(const sf::Texture& texture)
{
    m_states.texture = &texture;
    auto size = static_cast<sf::Vector2f>(texture.getSize());
    setTextureRect({ sf::Vector2f(), size });
}

void Sprite::setTextureRect(sf::FloatRect rect)
{
    m_textureRect = rect;
    m_dirty = true;
}

void Sprite::setColour(sf::Color c)
{
    for (auto& v : m_vertices)
    {
        v.color = c;
    }
}

void Sprite::setShader(sf::Shader* shader)
{
    m_states.shader = shader;
}

void Sprite::setBlendMode(sf::BlendMode mode)
{
    m_states.blendMode = mode;
}

const sf::Texture* Sprite::getTexture() const
{
    return m_states.texture;
}

sf::Color Sprite::getColour() const
{
    return m_vertices[0].color;
}

const sf::Shader* Sprite::getShader() const
{
    return m_states.shader;
}

