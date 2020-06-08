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

#include "ParticleDirector.hpp"
#include "MessageIDs.hpp"
#include "MapData.hpp"
#include "AnimationController.hpp"

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/ParticleEmitter.hpp>

#include <xyginext/ecs/Scene.hpp>

#include <SFML/Window/Event.hpp>

namespace
{
    const std::size_t MinEmitters = 4;
    const sf::Color bubble(255, 106, 105, 200);
}

ParticleDirector::ParticleDirector(xy::TextureResource& tr)
    : m_nextFreeEmitter  (0)
{
    //load particle presets
    m_settings[SettingsID::BubblePop].loadFromFile("assets/particles/pop.xyp", tr);
    m_settings[SettingsID::Score].loadFromFile("assets/particles/score.xyp", tr);
    m_settings[SettingsID::SpawnNPC].loadFromFile("assets/particles/spawn.xyp", tr);
    m_settings[SettingsID::BreakCrate].loadFromFile("assets/particles/crate.xyp", tr);
    m_settings[SettingsID::Explosion].loadFromFile("assets/particles/explode.xyp", tr);
}

//public
void ParticleDirector::handleMessage(const xy::Message& msg)
{
    //fire a particle system based on event
    if (msg.id == MessageID::SceneMessage)
    {
        const auto& data = msg.getData<SceneEvent>();
        if (data.type == SceneEvent::ActorRemoved)
        {
            auto ent = getNextEntity();
            switch (data.actorID)
            {
            default: return;            
            case ActorID::Squatmo:
            case ActorID::Clocksy:
            case ActorID::Whirlybob:
            case ActorID::Balldock:
                ent.getComponent<xy::ParticleEmitter>().settings = m_settings[SettingsID::Score];
                ent.getComponent<xy::ParticleEmitter>().settings.colour = bubble;
                break;
            case ActorID::FruitSmall:
                ent.getComponent<xy::ParticleEmitter>().settings = m_settings[SettingsID::Score];
                break;
            case ActorID::Goobly:
                ent.getComponent<xy::ParticleEmitter>().settings = m_settings[SettingsID::SpawnNPC];
                break;
            case ActorID::LightningOne:
            case ActorID::LightningTwo:
            case ActorID::FlameOne:
            case ActorID::FlameTwo:
            case ActorID::BubbleOne:
            case ActorID::BubbleTwo:
            case ActorID::Bonus:
                //if (data.entity.getComponent<AnimationController>().currentAnim == AnimationController::Idle)
                {
                    ent.getComponent<xy::ParticleEmitter>().settings = m_settings[SettingsID::BubblePop];
                }
                break;
            case ActorID::Crate:
                ent.getComponent<xy::ParticleEmitter>().settings = m_settings[SettingsID::BreakCrate];
                break;
            }
            ent.getComponent<xy::Transform>().setPosition(data.x, data.y);
            ent.getComponent<xy::ParticleEmitter>().start();
        }
        else if (data.type == SceneEvent::ActorSpawned)
        {
            //hm do we want to actually get a new entity
            //if most cases aren't handled?
            auto ent = getNextEntity();
            switch (data.actorID)
            {
            default: return;
            case ActorID::Clocksy:
            case ActorID::Whirlybob:
            case ActorID::Goobly:
            case ActorID::Balldock:
            case ActorID::Squatmo:
            case ActorID::MagicHat:
            case ActorID::Crate:
                ent.getComponent<xy::ParticleEmitter>().settings = m_settings[SettingsID::SpawnNPC];
                break;
            /*case ActorID::BubbleOne:
            case ActorID::BubbleTwo:
                
                break;*/
            case ActorID::Explosion:
                ent.getComponent<xy::ParticleEmitter>().settings = m_settings[SettingsID::Explosion];
                break;
            }
            ent.getComponent<xy::Transform>().setPosition(data.x, data.y);
            ent.getComponent<xy::ParticleEmitter>().start();
        }

    }
    else if (msg.id == MessageID::AnimationMessage)
    {
        const auto& data = msg.getData<AnimationEvent>();
        if (data.entity.isValid() &&
            data.entity.hasComponent<Actor>())
        {
            auto actorID = data.entity.getComponent<Actor>().type;
            switch (actorID)
            {
            case ActorID::Clocksy:
            case ActorID::Whirlybob:
            case ActorID::Balldock:
            case ActorID::Squatmo:
                if (data.oldAnim == AnimationController::TrappedOne
                    || data.oldAnim == AnimationController::TrappedTwo)
                {
                    auto ent = getNextEntity();
                    ent.getComponent<xy::ParticleEmitter>().settings = m_settings[SettingsID::Score];
                    ent.getComponent<xy::Transform>().setPosition(data.x, data.y);
                    ent.getComponent<xy::ParticleEmitter>().start();
                }
                break;
            case ActorID::LightningOne:
            case ActorID::LightningTwo:
            case ActorID::FlameOne:
            case ActorID::FlameTwo:
                if (data.newAnim == AnimationController::Walk)
                {
                    auto ent = getNextEntity();
                    ent.getComponent<xy::ParticleEmitter>().settings = m_settings[SettingsID::BubblePop];
                    ent.getComponent<xy::ParticleEmitter>().settings.colour = bubble;
                    ent.getComponent<xy::Transform>().setPosition(data.x, data.y);
                    ent.getComponent<xy::ParticleEmitter>().start();
                }
            default: break;
            }
        }
    }
}

void ParticleDirector::handleEvent(const sf::Event&)
{

}

void ParticleDirector::process(float)
{
    //check for finished systems then free up by swapping
    for (auto i = 0u; i < m_nextFreeEmitter; ++i)
    {
        if (m_emitters[i].getComponent<xy::ParticleEmitter>().stopped())
        {
            auto entity = m_emitters[i];
            m_nextFreeEmitter--;
            m_emitters[i] = m_emitters[m_nextFreeEmitter];
            m_emitters[m_nextFreeEmitter] = entity;
            i--;
        }
    }
}

//private
void ParticleDirector::resizeEmitters()
{
    m_emitters.resize(m_emitters.size() + MinEmitters);
    for (auto i = m_emitters.size() - MinEmitters; i < m_emitters.size(); ++i)
    {
        m_emitters[i] = getScene().createEntity();
        m_emitters[i].addComponent<xy::ParticleEmitter>();
        m_emitters[i].addComponent<xy::Transform>();
    }
}

xy::Entity ParticleDirector::getNextEntity()
{
    if (m_nextFreeEmitter == m_emitters.size())
    {
        resizeEmitters();
    }
    auto ent = m_emitters[m_nextFreeEmitter++];
    return ent;
}