/*********************************************************************
(c) Matt Marchant 2019
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
#include "States.hpp"
#include "ParticleState.hpp"
#include "SpriteState.hpp"

#include <SFML/Window/Event.hpp>

Game::Game()
    : xy::App   (sf::ContextSettings(8)),
    m_stateStack({ *getRenderWindow(), *this })
{

}

//private
void Game::handleEvent(const sf::Event& evt)
{    
#ifdef XY_DEBUG
    if (evt.type == sf::Event::KeyReleased)
    {
        switch (evt.key.code)
        {
        default: break;
        case sf::Keyboard::Escape:
            quit();
            break;
        }
    }
#endif

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

bool Game::initialise()
{
    setWindowTitle("xygine Resource Editor - Press F1 for Options");

    registerStates();
    m_stateStack.pushState(States::ParticleState);

    getRenderWindow()->setKeyRepeatEnabled(false);

    return true;
}

void Game::finalise()
{
    m_stateStack.clearStates();
    m_stateStack.applyPendingChanges();
}

void Game::registerStates()
{
    m_stateStack.registerState<ParticleState>(States::ParticleState);
    m_stateStack.registerState<SpriteState>(States::SpriteState);
}
