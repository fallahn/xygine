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

#include "RemotePauseState.hpp"

#include <SFML/Graphics/RenderWindow.hpp>

RemotePauseState::RemotePauseState(xy::StateStack& stack, xy::State::Context ctx)
    : xy::State(stack, ctx)
{
    m_font.loadFromFile("assets/fonts/Cave-Story.ttf");
    m_text.setFont(m_font);
    m_text.setString("Other Player Paused The Game");
    m_text.setFillColor(sf::Color::Red);
    m_text.setCharacterSize(120);
    auto bounds = m_text.getLocalBounds();
    m_text.setOrigin(bounds.width / 2.f, (bounds.height / 2.f) + bounds.top);
    m_text.setPosition(xy::DefaultSceneSize / 2.f);

    sf::Image img;
    img.create(1, 1, sf::Color(0, 0, 0, 180));
    m_backgroundTexture.loadFromImage(img);
    m_sprite.setTexture(m_backgroundTexture);
    m_sprite.setScale(xy::DefaultSceneSize);
}

//public
void RemotePauseState::draw()
{
    auto view = getContext().defaultView;
    auto& rt = getContext().renderWindow;
    rt.setView(view);

    rt.draw(m_sprite);
    rt.draw(m_text);
}