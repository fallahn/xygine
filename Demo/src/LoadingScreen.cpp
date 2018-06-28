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

#include "LoadingScreen.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <xyginext/core/FileSystem.hpp>

#include <array>

namespace
{
    constexpr float frametime = 1.f / 12.f;
    float currFrametime = 0.f;

    const std::array<sf::IntRect, 5> frames =
    {{
        sf::IntRect( 0,512,128,256 ),
        { 128,512,128,256 },
        { 256,512,128,256 },
        { 384,512,128,256 },
        { 512,512,128,256 }
    }};
}

LoadingScreen::LoadingScreen()
    : m_frame(0)
{
    m_texture.loadFromFile(xy::FileSystem::getResourcePath() + "assets/images/menu_sprites.png");
    m_sprite.setTexture(m_texture);
    m_sprite.setTextureRect(frames[0]);
    m_sprite.setScale(-1.f, 1.f);
    m_sprite.setPosition(148.f, 20.f);
}

//public
void LoadingScreen::update(float dt)
{
    currFrametime += dt;
    if (currFrametime > frametime)
    {
        currFrametime = 0.f;
        m_frame = (m_frame + 1) % frames.size();
        m_sprite.setTextureRect(frames[m_frame]);
    }
}

//private
void LoadingScreen::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_sprite, states);
}
