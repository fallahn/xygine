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

#include <xyginext/ecs/systems/UISystem.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Camera.hpp>
#include <xyginext/ecs/components/UIHitBox.hpp>
#include <xyginext/ecs/Scene.hpp>

#include <xyginext/util/Vector.hpp>

#include <SFML/Window/Event.hpp>


using namespace xy;

UISystem::UISystem(MessageBus& mb)
    : System    (mb, typeid(UISystem))
{
    requireComponent<UIHitBox>();
    requireComponent<Transform>();

    m_buttonCallbacks.push_back([](Entity, sf::Uint64) {}); //default callback for components which don't have one assigned
    m_movementCallbacks.push_back([](Entity, sf::Vector2f) {});
}

void UISystem::handleEvent(const sf::Event& evt)
{
    switch (evt.type)
    {
    default: break;
    //case SDL_MOUSEMOTION:
    //    m_eventPosition = toWorldCoords(evt.motion.x, evt.motion.y);
    //    m_movementDelta = m_eventPosition - m_prevMousePosition;
    //    m_prevMousePosition = m_eventPosition;
    //    break;
    //case SDL_MOUSEBUTTONDOWN:
    //    m_eventPosition = toWorldCoords(evt.button.x, evt.button.y);
    //    m_previousEventPosition = m_eventPosition;
    //    switch (evt.button.button)
    //    {
    //    default: break;
    //    case SDL_BUTTON_LEFT:
    //        m_downEvents.push_back(LeftMouse);
    //        break;
    //    case SDL_BUTTON_RIGHT:
    //        m_downEvents.push_back(RightMouse);
    //        break;
    //    case SDL_BUTTON_MIDDLE:
    //        m_downEvents.push_back(MiddleMouse);
    //        break;
    //    }
    //    break;
    //case SDL_MOUSEBUTTONUP:
    //    m_eventPosition = toWorldCoords(evt.button.x, evt.button.y);
    //    switch (evt.button.button)
    //    {
    //    default: break;
    //    case SDL_BUTTON_LEFT:
    //        m_upEvents.push_back(Flags::LeftMouse);
    //        break;
    //    case SDL_BUTTON_RIGHT:
    //        m_upEvents.push_back(Flags::RightMouse);
    //        break;
    //    case SDL_BUTTON_MIDDLE:
    //        m_upEvents.push_back(Flags::MiddleMouse);
    //        break;
    //    }
    //    break;
    //    /*
    //    NOTE!!!
    //    SDL registers both touch AND mouse events for a single touch
    //    on Android. Be warned this will execute the same callback twice!!
    //    */

    //case SDL_FINGERMOTION:
    //    m_eventPosition = toWorldCoords(evt.tfinger.x, evt.tfinger.y);
    //    //TODO check finger IDs for gestures etc
    //    break;
    //case SDL_FINGERDOWN:
    //    m_eventPosition = toWorldCoords(evt.tfinger.x, evt.tfinger.y);
    //    m_previousEventPosition = m_eventPosition;
    //    //TODO check finger IDs for gestures etc
    //    m_downEvents.push_back(Finger);
    //    //Logger::log("Touch pos: " + std::to_string(m_eventPosition.x) + ", " + std::to_string(m_eventPosition.y), Logger::Type::Info);
    //    break;
    //case SDL_FINGERUP:
    //    m_eventPosition = toWorldCoords(evt.tfinger.x, evt.tfinger.y);
    //    m_upEvents.push_back(Finger);
    //    break;
    }
}

void UISystem::process(float dt)
{    
    //TODO we probably want some partitioning? Checking every entity for a collision could be a bit pants
    auto& entities = getEntities();
    for (auto& e : entities)
    {
        //TODO probably want to cache these and only update if control moved
        auto tx = e.getComponent<Transform>().getWorldTransform();
        auto& input = e.getComponent<UIHitBox>();

        auto area = tx.transformRect(input.area);
        if (area.contains(m_eventPosition))
        {
            if (!input.active)
            {
                //mouse has entered
                input.active = true;
                m_movementCallbacks[input.callbacks[UIHitBox::MouseEnter]](e, m_movementDelta);
            }
            for (auto f : m_downEvents)
            {
                m_buttonCallbacks[input.callbacks[UIHitBox::MouseDown]](e, f);
            }
            for (auto f : m_upEvents)
            {
                m_buttonCallbacks[input.callbacks[UIHitBox::MouseUp]](e, f);
            }
            if (Util::Vector::lengthSquared(m_movementDelta) > 0)
            {
                m_movementCallbacks[input.callbacks[UIHitBox::MouseMotion]](e, m_movementDelta);
            }
        }
        else
        {
            if (input.active)
            {
                //mouse left
                input.active = false;
                m_movementCallbacks[input.callbacks[UIHitBox::MouseExit]](e, m_movementDelta);
            }
        }
    }

    //DPRINT("Window Pos", std::to_string(m_eventPosition.x) + ", " + std::to_string(m_eventPosition.y));

    m_previousEventPosition = m_eventPosition;
    m_upEvents.clear();
    m_downEvents.clear();
    m_movementDelta = {};
}

void UISystem::handleMessage(const Message& msg)
{
    //if (msg.id == Message::WindowMessage)
    //{
    //    const auto& data = msg.getData<Message::WindowEvent>();
    //    if (data.event == SDL_WINDOWEVENT_SIZE_CHANGED)
    //    {
    //        m_windowSize.x = data.data0;
    //        m_windowSize.y = data.data1;
    //    }
    //}
}

sf::Uint32 UISystem::addCallback(const ButtonCallback& cb)
{
    m_buttonCallbacks.push_back(cb);
    return static_cast<sf::Uint32>(m_buttonCallbacks.size() - 1);
}

sf::Uint32 UISystem::addCallback(const MovementCallback& cb)
{
    m_movementCallbacks.push_back(cb);
    return static_cast<sf::Uint32>(m_movementCallbacks.size() - 1);
}

//private
