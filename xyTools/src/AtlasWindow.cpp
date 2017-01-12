/*********************************************************************
Matt Marchant 2014 - 2017
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

#include <AtlasWidow.hpp>

#include <xygine/Resource.hpp>
#include <xygine/Log.hpp>

#include <SFML/Graphics/RenderTarget.hpp>

AtlasWindow::AtlasWindow(xy::MessageBus& mb, xy::TextureResource& tr)
    : xy::Component(mb, this),
    m_textureResource(tr)
{

}

//public
void AtlasWindow::entityUpdate(xy::Entity&, float)
{

}

void AtlasWindow::setSpriteSheet(const std::string& path)
{
    sf::Texture t;
    if (t.loadFromFile(path))
    {
        sf::Sprite spr(t);
        m_renderTexture.create(t.getSize().x, t.getSize().y);
        m_renderTexture.clear(sf::Color::Transparent);
        m_renderTexture.draw(spr);
        m_renderTexture.display();

        m_previewSprite.setTexture(m_renderTexture.getTexture());
    }
    else
    {
        xy::Logger::log("failed opening " + path, xy::Logger::Type::Error);
    }
}

//private
void AtlasWindow::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_previewSprite, states);
}