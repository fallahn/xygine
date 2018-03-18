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

#include "ErrorState.hpp"
#include "SharedStateData.hpp"
#include "MessageIDs.hpp"

#include <xyginext/core/App.hpp>
#include <xyginext/gui/Gui.hpp>

#include <SFML/Graphics/Image.hpp>

ErrorState::ErrorState(xy::StateStack& stack, xy::State::Context ctx, const SharedStateData& shared)
    : xy::State(stack, ctx),
    m_message(shared.error)
{
    sf::Image img;
    img.create(1, 1, { 0,0,0,160 });
    m_backgroundTexture.loadFromImage(img);
    m_backgroundSprite.setTexture(m_backgroundTexture);
    m_backgroundSprite.setScale(xy::DefaultSceneSize);

    registerWindow(
        [&]()
    {
        XY_ASSERT(xy::App::getRenderTarget(), "no valid window");

        auto windowSize = sf::Vector2f(xy::App::getRenderTarget()->getSize());
        auto boxSize = sf::Vector2f(400.f, 100.f);
        windowSize = (windowSize - boxSize) / 2.f;

        xy::Nim::setNextWindowPosition(windowSize.x, windowSize.y);
        xy::Nim::setNextWindowSize(boxSize.x, boxSize.y);
        xy::Nim::begin("Error");      
        xy::Nim::text(m_message);
        if (xy::Nim::button("OK", 40.f, 16.f))
        {
            /*requestStackClear();
            requestStackPush(StateID::MainMenu);*/
            getContext().appInstance.getMessageBus().post<MenuEvent>(MessageID::MenuMessage)->action = MenuEvent::QuitGameClicked;
        }
        xy::Nim::end();
    });

    xy::App::setMouseCursorVisible(true);
}

bool ErrorState::handleEvent(const sf::Event&)
{
    return false;
}

void ErrorState::handleMessage(const xy::Message&)
{

}

bool ErrorState::update(float)
{
    return false;
}

void ErrorState::draw()
{
    XY_ASSERT(xy::App::getRenderTarget(), "no valid window");

    auto& rt = *getContext().appInstance.getRenderTarget();
    rt.setView(getContext().defaultView);
    rt.draw(m_backgroundSprite);
}
