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

#pragma once

#include "CommandIDs.hpp"
#include "ClientServerShared.hpp"

#include <xyginext/ecs/Scene.hpp>
#include <xyginext/ecs/Entity.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Text.hpp>
#include <xyginext/ecs/components/CommandTarget.hpp>
#include <xyginext/ecs/systems/CommandSystem.hpp>

#include <xyginext/util/Vector.hpp>
#include <xyginext/util/Random.hpp>

//flashes text
class Flasher final
{
private:
    static constexpr float flashTime = 0.25f;

public:
    Flasher(xy::Scene& scene) : m_scene(scene) {}
    void operator ()(xy::Entity entity, float dt)
    {
        m_flashTime -= dt;

        if (m_flashTime < 0)
        {
            m_flashTime = flashTime;
            m_colour.a = (m_colour.a == 255) ? 0 : 255;
            entity.getComponent<xy::Text>().setFillColour(m_colour);
        }

        auto& tx = entity.getComponent<xy::Transform>();
        tx.move(-600.f * dt, 0.f);
        if (tx.getPosition().x < -1000.f)
        {
            m_scene.destroyEntity(entity);

            xy::Command cmd;
            cmd.targetFlags = CommandID::SceneMusic;
            cmd.action = [](xy::Entity entity, float)
            {
                entity.getComponent<xy::AudioEmitter>().play();
                entity.getComponent<xy::AudioEmitter>().setPitch(1.1f);
            };
            m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);
        }
    }
private:
    xy::Scene& m_scene;
    float m_flashTime = flashTime;
    sf::Color m_colour = sf::Color::Red;
};

//makes the balls drop
class BallDropper final
{
public:
    explicit BallDropper(xy::Scene& scene) : m_scene(scene) {}

    void operator()(xy::Entity entity, float dt)
    {
        m_velocity.y += Gravity * dt;
        
        auto& tx = entity.getComponent<xy::Transform>();
        tx.move(m_velocity * dt);

        if (m_bounce && (tx.getPosition().y > (xy::DefaultSceneSize.y / 2.f)))
        {
            m_velocity.y = -m_velocity.y * xy::Util::Random::value(0.7f, 0.8f);
            tx.move(m_velocity * dt);
            m_bounce--;
        }

        if (tx.getPosition().y > MapBounds.height)
        {
            m_scene.destroyEntity(entity);

            xy::Command cmd;
            cmd.targetFlags = CommandID::SceneMusic;
            cmd.action = [](xy::Entity entity, float)
            {
                if (entity.getComponent<xy::AudioEmitter>().getStatus() != xy::AudioEmitter::Playing)
                {
                    entity.getComponent<xy::AudioEmitter>().play();
                }
            };
            m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);
        }
    }

private:
    xy::Scene& m_scene;
    sf::Vector2f m_velocity;
    std::uint8_t m_bounce = 4;
};
