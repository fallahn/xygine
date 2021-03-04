/*********************************************************************
(c) Matt Marchant 2017 - 2021
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

#include "xyginext/core/StateStack.hpp"
#include "xyginext/core/App.hpp"
#include "xyginext/core/Log.hpp"
#include "xyginext/core/Assert.hpp"
#include "xyginext/core/Message.hpp"

#include <SFML/Graphics/RenderWindow.hpp>

using namespace xy;

StateStack::StateStack(State::Context context)
    : m_context (context)
{
    updateView();
}

//public
void StateStack::unregisterState(StateID id)
{
    if (m_factories.count(id) != 0)
    {
        m_factories.erase(id);
    }
}

void StateStack::update(float dt)
{
    applyPendingChanges();
    for (auto i = m_stack.rbegin(); i != m_stack.rend(); ++i)
    {
        if (!(*i)->update(dt)) break;
    }
}

void StateStack::draw()
{
    for (auto& s : m_stack)
    {
        s->draw();
    }
}

void StateStack::handleEvent(const sf::Event& evt)
{
    for (auto i = m_stack.rbegin(); i != m_stack.rend(); ++i)
    {
        if (!(*i)->handleEvent(evt)) break;
    }
}

void StateStack::handleMessage(const Message& msg)
{   
    //update the view first so that any states handling this
    //will have an up-to-date context available.
    if (msg.id == Message::WindowMessage)
    {
        const auto& data = msg.getData<Message::WindowEvent>();
        if (data.type == Message::WindowEvent::Resized)
        {
            updateView();
        }
    }

    for (auto& s : m_stack)
    {
        s->handleMessage(msg);
    }
}

void StateStack::pushState(StateID id)
{
    if (empty() || m_stack.back()->stateID() != id)
    {
        m_pendingChanges.emplace_back(Action::Push, id, false);
    }
}

void StateStack::popState()
{
    m_pendingChanges.emplace_back(Action::Pop);
}

void StateStack::clearStates()
{
    m_pendingChanges.emplace_back(Action::Clear);
}

bool StateStack::empty() const
{
    return m_stack.empty();
}

sf::View StateStack::updateView()
{
    //calculate the correct view size / ratio for window size
    m_context.defaultView.setSize(DefaultSceneSize);
    m_context.defaultView.setCenter(DefaultSceneSize / 2.f);

    auto vModeWidth = static_cast<float>(m_context.appInstance.getVideoSettings().VideoMode.width);
    auto vModeHeight = static_cast<float>(m_context.appInstance.getVideoSettings().VideoMode.height);

    auto winSize = sf::Vector2f(vModeWidth, vModeHeight);
    float windowRatio = winSize.x / winSize.y;
    float viewRatio = DefaultSceneSize.x / DefaultSceneSize.y;

    float sizeY = windowRatio / viewRatio;
    float top = (1.f - sizeY) / 2.f;

    m_context.defaultView.setViewport({ { 0.f, top }, { 1.f, sizeY } });

    for (auto& state : m_stack)
    {
        state->setContext(m_context);
    }
    
    return m_context.defaultView;
}

//private
State::Ptr StateStack::createState(StateID id)
{
    auto result = m_factories.find(id);
    XY_ASSERT(result != m_factories.end(), std::to_string(id) + ": state not found (not registered?)");

    return result->second();
}

void StateStack::applyPendingChanges()
{
    m_activeChanges.swap(m_pendingChanges);
    for (const auto& change : m_activeChanges)
    {
        switch (change.action)
        {
        case Action::Push:
            if (change.suspendPrevious && !m_stack.empty())
            {
                m_suspended.push_back(std::make_pair(change.id, std::move(m_stack.back())));
                m_stack.pop_back();
            }

            m_stack.emplace_back(createState(change.id));
            m_stack.emplace_back(std::make_unique<BufferState>(*this, m_context));

            {
                auto* msg = m_context.appInstance.getMessageBus().post<Message::StateEvent>(Message::StateMessage);
                msg->type = Message::StateEvent::Pushed;
                msg->id = change.id;
            }

            break;
        case Action::Pop:
        {
            auto id = m_stack.back()->stateID();
            m_stack.pop_back();
            
            auto* msg = m_context.appInstance.getMessageBus().post<Message::StateEvent>(Message::StateMessage);
            msg->type = Message::StateEvent::Popped;
            msg->id = id;

            if (!m_suspended.empty() && m_suspended.back().first == id)
            {
                //restore suspended state
                m_stack.push_back(std::move(m_suspended.back().second));
                m_suspended.pop_back();
                if (id != bufferID)
                {
                    m_stack.emplace_back(std::make_unique<BufferState>(*this, m_context));
                }
            }
        }
            break;
        case Action::Clear:
            m_stack.clear();
            m_suspended.clear();

            {
                auto* msg = m_context.appInstance.getMessageBus().post<Message::StateEvent>(Message::StateMessage);
                msg->type = Message::StateEvent::Cleared;
            }

            break;
        default: break;
        }
    }
    m_activeChanges.clear();
}

//---------------------------------------

StateStack::Pendingchange::Pendingchange(Action a, StateID i, bool suspend)
    : action        (a),
    id              (i),
    suspendPrevious (suspend)
{

}
