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

#ifndef DEMO_MENU_CALLBACKS_HPP_
#define DEMO_MENU_CALLBACKS_HPP_

#include <xyginext/ecs/Entity.hpp>
#include <xyginext/ecs/Scene.hpp>

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Sprite.hpp>

#include <xyginext/ecs/systems/UISystem.hpp>

#include <xyginext/util/Vector.hpp>

class HelpMenuCallback final
{
public:
    HelpMenuCallback(const bool& shown, xy::Scene& scene)
        : m_shown(shown),
        m_scene(scene)
    {

    }

    void operator()(xy::Entity entity, float dt)
    {
        float target = 0.f;
        if (!m_shown)
        {
            //scroll to hidden
            target = HidePosition;
        }
        
        auto movement = entity.getComponent<xy::Transform>().getPosition();
        movement.y = target - movement.y;
        
        if (std::abs(movement.y) < 3.f)
        {
            if (movement.y != 0)
            {
                movement.y = target;
                entity.getComponent<xy::Transform>().setPosition(movement);
                m_scene.setSystemActive<xy::UISystem>(!m_shown);
            }
        }
        else
        {
            movement.x = 0.f;
            entity.getComponent<xy::Transform>().move(movement * Speed * dt);
        }
    }

    static constexpr float HidePosition = -890.f;
    static constexpr float ShowPosition = 0.f;

private:
    const bool& m_shown;
    xy::Scene& m_scene; //actually the main scene, not the help scene

    static constexpr float Speed = 5.f;
};

class HelpBackgroundCallback final
{
private:
    static constexpr float MaxTime = 0.5f;
    static constexpr float MaxAlpha = 82.f;

public:
    HelpBackgroundCallback(const bool& shown)
        : m_shown(shown)
    {

    }

    void operator() (xy::Entity entity, float dt)
    {
        if (m_shown)
        {
            m_currentTime = std::min(m_currentTime + dt, 0.5f); //TODO find out why g++ thinks MaxTime is undef ref
        }
        else
        {
            m_currentTime = std::max(0.f, m_currentTime - dt);
        }

        float target = m_currentTime / MaxTime;
        sf::Uint8 targetAlpha = static_cast<sf::Uint8>(target * MaxAlpha);

        entity.getComponent<xy::Sprite>().setColour({ 255, 255, 255, targetAlpha });
    }

private:
    const bool& m_shown;

    float m_currentTime = 0.f;
};

class MenuSliderCallback final
{
public:
    explicit MenuSliderCallback(const sf::Vector2f& target)
        : m_target(target)
    {

    }

    void operator()(xy::Entity entity, float dt)
    {
        auto movement = m_target - entity.getComponent<xy::Transform>().getPosition();
        auto dist = xy::Util::Vector::lengthSquared(movement);
        
        if (dist < 9.f)
        {
            if (dist != 0)
            {
                entity.getComponent<xy::Transform>().setPosition(m_target);
                //TODO optional signal to say we finished moving
            }
        }
        else
        {
            entity.getComponent<xy::Transform>().move(movement * Speed * dt);
        }
    }

private:
    const sf::Vector2f& m_target;
    static constexpr float Speed = 5.f;
};

#endif //DEMO_MENU_CALLBACKS_HPP_