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

#include <Game.hpp>
#include <StateIds.hpp>
#include <MenuMainState.hpp>
#include <MenuOptionState.hpp>
#include <ParticleDemoState.hpp>
#include <PhysicsDemoState.hpp>
#include <RacingDemoState.hpp>

#include <SFML/Window/Event.hpp>

Game::Game()
    : m_stateStack({ getRenderWindow(), *this })
{
    registerStates();
    m_stateStack.pushState(States::ID::MenuMain);
}

//private
void Game::handleEvent(const sf::Event& evt)
{
    /*if (evt.type == sf::Event::KeyReleased)
    {
        switch (evt.key.code)
        {
        default: break;
        }
    }*/    
    
    m_stateStack.handleEvent(evt);
}

void Game::handleMessage(const xy::Message& msg)
{
    switch (msg.id)
    {
    case xy::Message::Type::UIMessage:
    {
        auto& msgData = msg.getData<xy::Message::UIEvent>();
        switch (msgData.type)
        {
        case xy::Message::UIEvent::ResizedWindow:
            m_stateStack.updateView();
            break;
        default: break;
        }
        break;
    }
    default: break;
    }
    
    m_stateStack.handleMessage(msg);
}

void Game::updateApp(float dt)
{
    m_stateStack.update(dt);
}

void Game::pauseApp(float dt)
{
    //do nothing, we're paused!
}

void Game::draw()
{
    getRenderWindow().clear(sf::Color::Black);
    m_stateStack.draw();
    getRenderWindow().display();
}

void Game::finalise()
{
    m_stateStack.clearStates();
    m_stateStack.applyPendingChanges();
}

void Game::registerStates()
{
    m_stateStack.registerState<MenuMainState>(States::ID::MenuMain);
    m_stateStack.registerState<MenuOptionState>(States::ID::MenuOptions);
    m_stateStack.registerState<PhysicsDemoState>(States::ID::PhysicsDemo);
    m_stateStack.registerState<ParticleDemoState>(States::ID::ParticleDemo);
    m_stateStack.registerState<RacingDemoState>(States::ID::RacingDemo);
}
