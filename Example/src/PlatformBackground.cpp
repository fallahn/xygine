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

#include <PlatformBackground.hpp>

#include <xygine/Resource.hpp>
#include <xygine/Entity.hpp>
#include <xygine/Scene.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

using namespace Plat;

namespace
{
    const float nearOffset = 540.f;
}

Background::Background(xy::MessageBus& mb, xy::TextureResource& tr)
    : xy::Component (mb, this),
    m_nearTexture   (nullptr),
    m_farTexture    (nullptr)
{
    m_nearTexture = &tr.get("assets/images/platform/background_near.png");
    m_farTexture = &tr.get("assets/images/platform/background_far.png");

    m_nearTexture->setRepeated(true);
    m_farTexture->setRepeated(true);

    m_farVertices =
    {
        sf::Vertex({ 0.f, 0.f }, { 0.f, 0.f}),
        sf::Vertex({ xy::DefaultSceneSize.x, 0.f }, { xy::DefaultSceneSize.x, 0.f }),
        sf::Vertex({ xy::DefaultSceneSize }, { xy::DefaultSceneSize }),
        sf::Vertex({ 0.f, xy::DefaultSceneSize.y }, { 0.f, xy::DefaultSceneSize.y })
    };

    m_nearVertices =
    {
        sf::Vertex({ 0.f, nearOffset },{ 0.f, 0.f }),
        sf::Vertex({ xy::DefaultSceneSize.x, nearOffset },{ xy::DefaultSceneSize.x, 0.f }),
        sf::Vertex({ xy::DefaultSceneSize.x, xy::DefaultSceneSize.y + nearOffset },{ xy::DefaultSceneSize }),
        sf::Vertex({ 0.f, xy::DefaultSceneSize.y + nearOffset },{ 0.f, xy::DefaultSceneSize.y })
    };
}

//public
void Background::entityUpdate(xy::Entity& entity, float dt)
{
    const auto& tx = xy::Scene::getViewMatrix().getInverse();
    sf::Vector2f offset = tx.transformPoint({});
    offset *= 0.3f;

    m_nearVertices[0].texCoords = offset;
    m_nearVertices[1].texCoords = offset + sf::Vector2f(xy::DefaultSceneSize.x, 0.f);
    m_nearVertices[2].texCoords = offset + xy::DefaultSceneSize;
    m_nearVertices[3].texCoords = offset + sf::Vector2f(0.f, xy::DefaultSceneSize.y);
}

//private
void Background::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.texture = m_farTexture;
    states.transform *= xy::Scene::getViewMatrix().getInverse();
    rt.draw(m_farVertices.data(), m_farVertices.size(), sf::Quads, states);

    states.texture = m_nearTexture;
    rt.draw(m_nearVertices.data(), m_nearVertices.size(), sf::Quads, states);
}