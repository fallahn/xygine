/*********************************************************************
(c) Matt Marchant 2017 - 2019
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

#include "xyginext/core/State.hpp"
#include "xyginext/core/StateStack.hpp"
#include "xyginext/core/App.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

using namespace xy;

State::Context::Context(sf::RenderWindow& rw, App& app)
    : renderWindow(rw), appInstance(app){}


//-----------------------------------------------------

namespace
{
    const float iconSize = 50.f;
}

State::State(StateStack& stateStack, Context context)
    : m_stateStack  (stateStack),
    m_context       (context),
    m_threadRunning (false),
    m_loadingThread (&State::loadingScreenThread, this),
    m_loadingIcon   ({iconSize, iconSize})
{    
    m_loadingIcon.setOrigin(iconSize / 2.f, iconSize / 2.f);
    m_loadingIcon.setPosition(iconSize, iconSize);
}

void State::setContext(Context c)
{
    m_context.defaultView = c.defaultView;
}

//protected
void State::requestStackPush(StateID id)
{
    m_stateStack.pushState(id);
}

void State::requestStackPop()
{
    m_stateStack.popState();
}

void State::requestStackClear()
{
    m_stateStack.clearStates();
}

std::size_t State::getStackSize() const
{
    return m_stateStack.getStackSize();
}

State::Context State::getContext() const
{
    return m_context;
}

void State::launchLoadingScreen()
{
    //m_context.appInstance.pause();
    m_context.renderWindow.setActive(false);
    m_threadRunning = true;
    m_loadingThread.launch();
}

void State::quitLoadingScreen()
{
    m_threadRunning = false;
    //LOG("Quitting thread...", Logger::Type::Info);
    m_loadingThread.wait();
    //LOG("Thread quit.", Logger::Type::Info);
    //m_context.appInstance.resume();
}

void State::updateLoadingScreen(float dt, sf::RenderWindow& rw)
{
    m_loadingIcon.rotate(1400.f * dt);
    rw.draw(m_loadingIcon);
}

void State::loadingScreenThread()
{
    //sf::Event evt;
    while (m_threadRunning)
    {
        //consume events
        //while (m_context.renderWindow.pollEvent(evt)) {}

        m_context.renderWindow.clear(App::getClearColour());
        updateLoadingScreen(m_threadClock.restart().asSeconds(), m_context.renderWindow);
        m_context.renderWindow.display();
    }
    m_context.renderWindow.setActive(false);
}

std::size_t State::getStateCount() const
{
    return m_stateStack.getStackSize();
}
