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

#include "Game.hpp"
#include "GameState.hpp"
#include "MenuState.hpp"
#include "ErrorState.hpp"

#include <SFML/Window/Event.hpp>

namespace
{
    SharedStateData sharedData;
}

Game::Game()
    : xy::App   (/*sf::ContextSettings(0, 0, 0, 3, 2, sf::ContextSettings::Core)*/),
    m_stateStack({ getRenderWindow(), *this })
{

}

//private
void Game::handleEvent(const sf::Event& evt)
{    
    if (evt.type == sf::Event::KeyPressed)
    {
        switch(evt.key.code)
        {
            default: break;
            case sf::Keyboard::Escape:
                xy::App::quit();
                break;
        }
    }

    m_stateStack.handleEvent(evt);
}

void Game::handleMessage(const xy::Message& msg)
{    
    m_stateStack.handleMessage(msg);
}

void Game::updateApp(float dt)
{
    m_stateStack.update(dt);
}

void Game::draw()
{
    m_stateStack.draw();
}

void Game::initialise()
{
    registerStates();
    m_stateStack.pushState(StateID::MainMenu);

    getRenderWindow().setKeyRepeatEnabled(false);
}

void Game::finalise()
{
    m_stateStack.clearStates();
    m_stateStack.applyPendingChanges();
}

void Game::registerStates()
{
    m_stateStack.registerState<GameState>(StateID::Game, sharedData);
    m_stateStack.registerState<MenuState>(StateID::MainMenu, sharedData);
    m_stateStack.registerState<ErrorState>(StateID::Error, sharedData);
}
