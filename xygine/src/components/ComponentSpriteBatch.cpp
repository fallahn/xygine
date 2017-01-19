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

#include <xygine/Entity.hpp>
#include <xygine/components/SpriteBatch.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Texture.hpp>

using namespace xy;

SpriteBatch::SpriteBatch(xy::MessageBus& mb)
    : xy::Component (mb, this),
    m_texture       (nullptr)
{

}

//public
void SpriteBatch::entityUpdate(xy::Entity& entity, float)
{
    //clear destroyed sprites
    m_sprites.erase(std::remove_if(m_sprites.begin(), m_sprites.end(),
        [](const Sprite* spr)
    {
        return spr->destroyed();
    }), m_sprites.end());

    //rebuild varray
    m_globalBounds = sf::FloatRect();

    m_vertices.clear();
    m_vertices.reserve(m_sprites.size() * 4u);
    for (const auto spr : m_sprites)
    {
        for (const auto& p : spr->m_vertices)
        {
            auto vert = p;
            vert.position = spr->m_transform.transformPoint(vert.position);

            //update global bounds
            if (vert.position.x < m_globalBounds.left)
            {
                m_globalBounds.left = vert.position.x;
            }
            else if (vert.position.x > m_globalBounds.left + m_globalBounds.width)
            {
                m_globalBounds.width = vert.position.x - m_globalBounds.left;
            }
            if (vert.position.y < m_globalBounds.top)
            {
                m_globalBounds.top = vert.position.y;
            }
            else if (vert.position.y > m_globalBounds.top + m_globalBounds.height)
            {
                m_globalBounds.height = vert.position.y - m_globalBounds.top;
            }

            m_vertices.push_back(vert);
        }
    }
}

std::unique_ptr<Sprite> SpriteBatch::addSprite(xy::MessageBus& mb)
{
    auto sprite = xy::Component::create<Sprite>(mb);
    m_sprites.push_back(sprite.get());

    //set the texture rect if a texture already set
    if (m_texture)
    {
        sf::Vector2f size(m_texture->getSize());
        sprite->setTextureRect({ {}, size });
    }
    else
    {
        sprite->setTextureRect({0.f, 0.f, 10.f, 10.f });
    }

    return std::move(sprite);
}

void SpriteBatch::setTexture(const sf::Texture* t)
{
    m_texture = t;

    //update the texture rect of existing sprites
    if (t)
    {
        for (auto spr : m_sprites)
        {
            sf::Vector2f size(m_texture->getSize());
            spr->setTextureRect({ {}, size });
        }
    }
}

//private
void SpriteBatch::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.texture = m_texture;
    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
}

//--------Sprite-------//
Sprite::Sprite(xy::MessageBus& mb)
    : xy::Component(mb, this)
{

}

//public
void Sprite::entityUpdate(xy::Entity& entity, float dt)
{
    m_transform = entity.getTransform();
}

void Sprite::setTextureRect(const sf::FloatRect& rect)
{
    m_vertices[0].texCoords = { rect.left, rect.top };
    m_vertices[1].texCoords = { rect.left + rect.width, rect.top };
    m_vertices[2].texCoords = { rect.left + rect.width, rect.top + rect.height };
    m_vertices[3].texCoords = { rect.left, rect.top + rect.height };

    m_vertices[1].position.x = rect.width;
    m_vertices[2].position = { rect.width, rect.height };
    m_vertices[3].position.y = rect.height;
}

void Sprite::setColour(const sf::Color& c)
{
    for (auto& v : m_vertices)
    {
        v.color = c;
    }
}