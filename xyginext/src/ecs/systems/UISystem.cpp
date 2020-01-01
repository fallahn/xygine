/*********************************************************************
(c) Matt Marchant 2017 - 2020
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

#include "xyginext/ecs/systems/UISystem.hpp"
#include "xyginext/ecs/components/Transform.hpp"
#include "xyginext/ecs/components/Camera.hpp"
#include "xyginext/ecs/components/UIHitBox.hpp"
#include "xyginext/ecs/Scene.hpp"

#include "xyginext/util/Vector.hpp"

#include <SFML/Window/Event.hpp>

using namespace xy;

namespace
{
    sf::Vector2f toWorldCoords(sf::Int32 x, sf::Int32 y)
    {
        XY_ASSERT(App::getRenderWindow(), "no valid window");
        return App::getRenderWindow()->mapPixelToCoords({ x, y });
    }

    const float DeadZone = 20.f;
}

UISystem::UISystem(MessageBus& mb)
    : System    (mb, typeid(UISystem)),
    m_selectedIndex     (0),
    m_controllerMask    (0),
    m_prevControllerMask(0),
    m_joypadCursorActive(true)
{
    requireComponent<UIHitBox>();
    requireComponent<Transform>();

    //default callbacks for components which don't have one assigned
    m_buttonCallbacks.push_back([](Entity, sf::Uint64) {}); 
    m_movementCallbacks.push_back([](Entity, sf::Vector2f) {});
    m_keyboardCallbacks.push_back([](Entity, sf::Keyboard::Key) {});
    m_selectionCallbacks.push_back([](Entity) {});
    m_controllerCallbacks.push_back([](Entity, sf::Uint32, sf::Uint32) {});
}

void UISystem::handleEvent(const sf::Event& evt)
{
    if (!isActive()) return;

    switch (evt.type)
    {
    default: break;
    case sf::Event::MouseMoved:
        m_eventPosition = toWorldCoords(evt.mouseMove.x, evt.mouseMove.y);
        m_movementDelta = m_eventPosition - m_prevMousePosition;
        m_prevMousePosition = m_eventPosition;
        break;
    case sf::Event::MouseButtonPressed:
        m_eventPosition = toWorldCoords(evt.mouseButton.x, evt.mouseButton.y);
        m_previousEventPosition = m_eventPosition;
        switch (evt.mouseButton.button)
        {
        default: break;
        case sf::Mouse::Left:
            m_mouseDownEvents.push_back(LeftMouse);
            break;
        case sf::Mouse::Right:
            m_mouseDownEvents.push_back(RightMouse);
            break;
        case sf::Mouse::Middle:
            m_mouseDownEvents.push_back(MiddleMouse);
            break;
        }
        break;
    case sf::Event::MouseButtonReleased:
        m_eventPosition = toWorldCoords(evt.mouseButton.x, evt.mouseButton.y);
        switch (evt.mouseButton.button)
        {
        default: break;
        case sf::Mouse::Left:
            m_mouseUpEvents.push_back(Flags::LeftMouse);
            break;
        case sf::Mouse::Right:
            m_mouseUpEvents.push_back(Flags::RightMouse);
            break;
        case sf::Mouse::Middle:
            m_mouseUpEvents.push_back(Flags::MiddleMouse);
            break;
        }
        break;
    case sf::Event::KeyReleased:
        switch (evt.key.code)
        {
        case sf::Keyboard::Left:
        case sf::Keyboard::Up:
            selectPrev();
            break;
        case sf::Keyboard::Right:
        case sf::Keyboard::Down:
            selectNext();
            break;
        default:
            m_keyUpEvents.push_back(evt.key.code);
            break;
        }
        break;
    case sf::Event::KeyPressed:
        switch (evt.key.code)
        {
        case sf::Keyboard::Left:
        case sf::Keyboard::Up:
        case sf::Keyboard::Right:
        case sf::Keyboard::Down:
            break;
        default:
            m_keyDownEvents.push_back(evt.key.code);
            break;
        }
        break;
    case sf::Event::JoystickButtonPressed: 
        if (m_joypadCursorActive
            && evt.joystickButton.joystickId == 0)
        {
            //if controller mouse active push back mouse events
            switch (evt.joystickButton.button)
            {
            default: break;
            case 0:
                m_mouseDownEvents.push_back(LeftMouse);
                break;
            case 1:
                m_mouseDownEvents.push_back(RightMouse);
                break;
            }
            break;
        }
        m_controllerDownEvents.push_back(std::make_pair(evt.joystickButton.joystickId, evt.joystickButton.button));
        break;
    case sf::Event::JoystickButtonReleased:
        if (m_joypadCursorActive
            && evt.joystickButton.joystickId == 0)
        {
            //if controller mouse active push back mouse events
            switch (evt.joystickButton.button)
            {
            default: break;
            case 0:
                m_mouseUpEvents.push_back(LeftMouse);
                break;
            case 1:
                m_mouseUpEvents.push_back(RightMouse);
                break;
            }
            break;
        }
        m_controllerUpEvents.push_back(std::make_pair(evt.joystickButton.joystickId, evt.joystickButton.button));
        break;
    case sf::Event::JoystickMoved:
        switch (evt.joystickMove.axis)
        {
        case sf::Joystick::PovX:
        case sf::Joystick::X:
            if (evt.joystickMove.position > DeadZone)
            {
                m_controllerMask |= ControllerBits::Left;
            }
            else if (evt.joystickMove.position < -DeadZone)
            {
                m_controllerMask |= ControllerBits::Right;
            }
            break;
        case sf::Joystick::PovY:
            //sadly needs a special case on windows
#ifdef _WIN32
            if (evt.joystickMove.position > DeadZone)
            {
                m_controllerMask |= ControllerBits::Down;
            }
            else if (evt.joystickMove.position < -DeadZone)
            {
                m_controllerMask |= ControllerBits::Up;
            }
            break;
#endif
        case sf::Joystick::Y:
            if (evt.joystickMove.position > DeadZone)
            {
                m_controllerMask |= ControllerBits::Up;
            }
            else if (evt.joystickMove.position < -DeadZone)
            {
                m_controllerMask |= ControllerBits::Down;
            }
            break;
        default: break;
        }
        break;
    }
}

void UISystem::process(float)
{    
    //parse any controller events
    if (m_joypadCursorActive)
    {
        static const float moveSpeed = 0.125f;

        sf::Vector2f movement;
        float axis = sf::Joystick::getAxisPosition(0, sf::Joystick::X);
        if (axis < -DeadZone || axis > DeadZone) movement.x = axis;
        axis = sf::Joystick::getAxisPosition(0, sf::Joystick::Y);
        if (axis < -DeadZone || axis > DeadZone) movement.y = axis;

        //again for PoV
        axis = sf::Joystick::getAxisPosition(0, sf::Joystick::PovX);
        if (axis < -DeadZone || axis > DeadZone) movement.x = axis;
#ifdef _WIN32
        axis = -sf::Joystick::getAxisPosition(0, sf::Joystick::PovY);
#else
        axis = sf::Joystick::getAxisPosition(0, sf::Joystick::PovY);
#endif //_WIN32
        if (axis < -DeadZone || axis > DeadZone) movement.y = axis;

        auto pos = sf::Mouse::getPosition(*xy::App::getRenderWindow());
        pos += sf::Vector2i(movement * moveSpeed);
        sf::Mouse::setPosition(pos, *xy::App::getRenderWindow());
    }
    else
    {
        auto diff = m_prevControllerMask ^ m_controllerMask;
        for (auto i = 0; i < 4; ++i)
        {
            auto flag = (1 << i);
            if (diff & flag)
            {
                //something changed
                if (m_controllerMask & flag)
                {
                    //axis was pressed
                    switch (flag)
                    {
                    default: break;
                    case ControllerBits::Left:
                    case ControllerBits::Up:
                        selectNext();
                        break;
                    case ControllerBits::Right:
                    case ControllerBits::Down:
                        selectPrev();
                        break;
                    }
                }
            }
        }
        m_prevControllerMask = m_controllerMask;
        m_controllerMask = 0;
    }

    auto view = App::getRenderWindow()->getView();
    sf::FloatRect viewableArea((view.getCenter() - (view.getSize() / 2.f)), view.getSize());

    //TODO we probably want some partitioning? Checking every entity for a collision could be a bit pants
    std::size_t currentIndex = 0;
    auto& entities = getEntities();
    for (auto& e : entities)
    {
        //TODO probably want to cache these and only update if control moved
        auto tx = e.getComponent<Transform>().getWorldTransform();
        auto& input = e.getComponent<UIHitBox>();

        //-----movement input-----//
        auto area = tx.transformRect(input.area);

        //skip UI elements currently out of view
        if (!area.intersects(viewableArea))
        {
            continue;
        }

        bool contains = false;
        if ((contains = area.contains(m_eventPosition)))
        {
            if (!input.active)
            {
                //mouse has entered
                input.active = true;
                m_movementCallbacks[input.callbacks[UIHitBox::MouseEnter]](e, m_movementDelta);

                //update selection
                unselect(m_selectedIndex);
                m_selectedIndex = currentIndex;
                select(m_selectedIndex);
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


        //----button input----//
        if (contains)
        {
            for (auto f : m_mouseDownEvents)
            {
                m_buttonCallbacks[input.callbacks[UIHitBox::MouseDown]](e, f);
            }

            for (auto f : m_mouseUpEvents)
            {
                m_buttonCallbacks[input.callbacks[UIHitBox::MouseUp]](e, f);
            }
        }

        if (currentIndex == m_selectedIndex)
        {
            for (auto key : m_keyDownEvents)
            {
                m_keyboardCallbacks[input.callbacks[UIHitBox::KeyDown]](e, key);
            }

            for (auto key : m_keyUpEvents)
            {
                m_keyboardCallbacks[input.callbacks[UIHitBox::KeyUp]](e, key);
            }

            for (auto pair : m_controllerDownEvents)
            {
                m_controllerCallbacks[input.callbacks[UIHitBox::ControllerButtonDown]](e, pair.first, pair.second);
            }

            for (auto pair : m_controllerUpEvents)
            {
                m_controllerCallbacks[input.callbacks[UIHitBox::ControllerButtonUp]](e, pair.first, pair.second);
            }
        }
        currentIndex++;
    }

    //DPRINT("Window Pos", std::to_string(m_eventPosition.x) + ", " + std::to_string(m_eventPosition.y));
    //DPRINT("Selected Index", std::to_string(m_selectedIndex));

    m_previousEventPosition = m_eventPosition;
    m_movementDelta = {};

    m_mouseUpEvents.clear();
    m_mouseDownEvents.clear();

    m_keyDownEvents.clear();
    m_keyUpEvents.clear();

    m_controllerDownEvents.clear();
    m_controllerUpEvents.clear();
}

void UISystem::handleMessage(const Message&)
{

}

sf::Uint32 UISystem::addMouseButtonCallback(const MouseButtonCallback& cb)
{
    m_buttonCallbacks.push_back(cb);
    return static_cast<sf::Uint32>(m_buttonCallbacks.size() - 1);
}

sf::Uint32 UISystem::addMouseMoveCallback(const MovementCallback& cb)
{
    m_movementCallbacks.push_back(cb);
    return static_cast<sf::Uint32>(m_movementCallbacks.size() - 1);
}

sf::Uint32 UISystem::addKeyCallback(const KeyboardCallback& cb)
{
    m_keyboardCallbacks.push_back(cb);
    return static_cast<sf::Uint32>(m_keyboardCallbacks.size() - 1);
}

sf::Uint32 UISystem::addSelectionCallback(const SelectionChangedCallback& cb)
{
    m_selectionCallbacks.push_back(cb);
    return static_cast<sf::Uint32>(m_selectionCallbacks.size() - 1);
}

sf::Uint32 UISystem::addControllerCallback(const ControllerCallback& cb)
{
    m_controllerCallbacks.push_back(cb);
    return static_cast<sf::Uint32>(m_controllerCallbacks.size() - 1);
}

void UISystem::selectInput(std::size_t idx)
{
    XY_ASSERT(!getEntities().empty(), "No inputs are added!");
    idx = std::min(idx, getEntities().size() - 1);

    unselect(m_selectedIndex);
    m_selectedIndex = idx;
    select(m_selectedIndex);
}

void UISystem::setJoypadCursorActive(bool active)
{
    m_joypadCursorActive = active;
}

//private
void UISystem::selectNext()
{
    //call unselected on prev ent
    auto& entities = getEntities();
    unselect(m_selectedIndex);

    //get new index
    m_selectedIndex = (m_selectedIndex + 1) % entities.size();

    //and do selected callback
    select(m_selectedIndex);
}

void UISystem::selectPrev()
{
    //call unselected on prev ent
    auto& entities = getEntities();
    unselect(m_selectedIndex);

    //get new index
    m_selectedIndex = (m_selectedIndex + entities.size() - 1) % entities.size();

    //and do selected callback
    select(m_selectedIndex);
}

void UISystem::unselect(std::size_t entIdx)
{
    auto& entities = getEntities();
    auto idx = entities[entIdx].getComponent<UIHitBox>().callbacks[UIHitBox::Unselected];
    m_selectionCallbacks[idx](entities[entIdx]);    
}

void UISystem::select(std::size_t entIdx)
{
    auto& entities = getEntities();
    auto idx = entities[entIdx].getComponent<UIHitBox>().callbacks[UIHitBox::Selected];
    m_selectionCallbacks[idx](entities[entIdx]);
}

void UISystem::onEntityAdded(xy::Entity)
{
    //selectNext();
    //selectPrev();
    select(0);
    unselect(getEntities().size() - 1);
}

void UISystem::onEntityRemoved(xy::Entity)
{
    selectPrev();
}
