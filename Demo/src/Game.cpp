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
#include "PauseState.hpp"
#include "GameOverState.hpp"
#include "IntroState.hpp"
#include "RemotePauseState.hpp"
#include "GameCompleteState.hpp"

#include <SFML/Window/Event.hpp>

namespace
{
    SharedStateData sharedData;
    //sf::Cursor cursor;
}

Game::Game()
    : xy::App   (/*sf::ContextSettings(0, 0, 0, 3, 2, sf::ContextSettings::Core)*/),
    m_stateStack({ *getRenderWindow(), *this })
{
    /*sf::Image img;
    img.loadFromFile("assets/images/cursor.png");

    if (cursor.loadFromPixels(img.getPixelsPtr(), img.getSize(), { 0,0 }))
    {
        getRenderWindow()->setMouseCursor(cursor);
    }*/

    setWindowIcon("assets/images/icon.png");
    getRenderWindow()->setTitle("xygine - Castle Clamber (F1 for Options)");
}

//private
void Game::handleEvent(const sf::Event& evt)
{    
#ifdef XY_DEBUG
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

void Game::initialise()
{
    registerStates();
#ifdef XY_DEBUG
    m_stateStack.pushState(StateID::GameComplete);
#else
    m_stateStack.pushState(StateID::Intro);
#endif

    XY_ASSERT(getRenderWindow(), "no valid window");
    getRenderWindow()->setKeyRepeatEnabled(false);
}

void Game::finalise()
{
    m_stateStack.clearStates();
    m_stateStack.applyPendingChanges();
}

void Game::registerStates()
{
    m_stateStack.registerState<GameState>(StateID::Game, sharedData, m_loadingScreen);
    m_stateStack.registerState<MenuState>(StateID::MainMenu, sharedData, m_loadingScreen);
    m_stateStack.registerState<ErrorState>(StateID::Error, sharedData);
    m_stateStack.registerState<PauseState>(StateID::Pause);
    m_stateStack.registerState<GameoverState>(StateID::GameOver, sharedData);
    m_stateStack.registerState<IntroState>(StateID::Intro);
    m_stateStack.registerState<RemotePauseState>(StateID::RemotePause);
    m_stateStack.registerState<GameCompleteState>(StateID::GameComplete, sharedData);
}
