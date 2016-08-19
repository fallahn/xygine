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

#include <xygine/components/MeshDrawable.hpp>
#include <xygine/Entity.hpp>
#include <xygine/Scene.hpp>

using namespace xy;

MeshDrawable::MeshDrawable(MessageBus& mb, MeshRenderer& mr, const MeshRenderer::Lock&)
    : Component     (mb, this),
    m_meshRenderer  (mr)
{
    m_renderTexture.create(static_cast<sf::Uint32>(xy::DefaultSceneSize.x), static_cast<sf::Uint32>(xy::DefaultSceneSize.y));
    m_sprite.setTexture(m_renderTexture.getTexture());
    m_offset = xy::DefaultSceneSize / 2.f;
    mr.setView(m_renderTexture.getDefaultView());
}

//public
void MeshDrawable::entityUpdate(Entity& e, float)
{
    m_sprite.setPosition(e.getScene()->getView().getCenter() - m_offset);
}

//private
void MeshDrawable::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    m_renderTexture.clear(sf::Color::Transparent);
    m_renderTexture.draw(m_meshRenderer);
    m_renderTexture.display();

    rt.draw(m_sprite, states);
}