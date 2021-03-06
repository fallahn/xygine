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

#include "FXDirector.hpp"
#include "MessageIDs.hpp"
#include "MapData.hpp"
#include "AnimationController.hpp"

#include <xyginext/ecs/components/AudioEmitter.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/Scene.hpp>

namespace
{
    const std::size_t MinEntities = 4;
}

FXDirector::FXDirector()
    : m_nextFreeEntity(0)
{
    //TODO set up mapping in xygine
    m_soundResource.get("assets/sound/collect.wav");
    m_soundResource.get("assets/sound/jump.wav");
    m_soundResource.get("assets/sound/npc_pop.wav");
    m_soundResource.get("assets/sound/shoot.wav");
    m_soundResource.get("assets/sound/hurry.wav");
    m_soundResource.get("assets/sound/player_die.wav");
    m_soundResource.get("assets/sound/lightning_die.wav");
    m_soundResource.get("assets/sound/flame.wav");
    m_soundResource.get("assets/sound/powerup_pop.wav");
    m_soundResource.get("assets/sound/pop.wav");
    m_soundResource.get("assets/sound/bonus.wav");
    m_soundResource.get("assets/sound/get_hat.wav");
    m_soundResource.get("assets/sound/hat_land.wav");
    m_soundResource.get("assets/sound/crate_break.wav");
    m_soundResource.get("assets/sound/one_up.wav");
    m_soundResource.get("assets/sound/explode.wav");
}

//public
void FXDirector::handleMessage(const xy::Message& msg)
{ 
    switch (msg.id)
    {
    default: break;
    case MessageID::PlayerMessage:
    {
        const auto& data = msg.getData<PlayerEvent>();
        switch (data.type)
        {
        default:break;
        case PlayerEvent::GotExtraLife:
            playSound(m_soundResource.get("assets/sound/one_up.wav"));
            break;
        case PlayerEvent::GotHat:
            playSound(m_soundResource.get("assets/sound/get_hat.wav"));
            break;
        case PlayerEvent::LostHat:
            playSound(m_soundResource.get("assets/sound/hat_land.wav"));
            break;
        }
    }
        break;
    case MessageID::SceneMessage:
    {
        const auto& data = msg.getData<SceneEvent>();
        if (data.type == SceneEvent::ActorRemoved)
        {
            switch (data.actorID)
            {
            default: break;
            case ActorID::FruitSmall:
            case ActorID::FruitLarge:
            {
                playSound(m_soundResource.get("assets/sound/collect.wav"));
            }
            break;
            case ActorID::BubbleOne:
            case ActorID::BubbleTwo:
            case ActorID::Bonus:
                playSound(m_soundResource.get("assets/sound/pop.wav"));
                break;
            case ActorID::Crate:
                playSound(m_soundResource.get("assets/sound/crate_break.wav"));
                break;
            }
        }
        else if (data.type == SceneEvent::ActorSpawned)
        {
            switch (data.actorID)
            {
            default: break;
            case ActorID::BubbleOne:
            case ActorID::BubbleTwo:
            {
                playSound(m_soundResource.get("assets/sound/shoot.wav"));
            }
            break;
            case ActorID::MagicHat:
                playSound(m_soundResource.get("assets/sound/pop.wav"));
            break;
            case ActorID::Explosion:
                playSound(m_soundResource.get("assets/sound/explode.wav"));
                break;
            }
        }
    }
        break;
    case MessageID::AnimationMessage:
    {
        const auto& data = msg.getData<AnimationEvent>();
        switch (data.newAnim)
        {
        default: break;
        case AnimationController::Dead:
            {
            if (data.entity.hasComponent<Actor>())
            {
                const auto& actor = data.entity.getComponent<Actor>();
                switch (actor.type)
                {
                default: break;
                case ActorID::MagicHat:
                    playSound(m_soundResource.get("assets/sound/hat_land.wav"));
                    break;
                }
            }
            }
                break;
        case AnimationController::Die:
        {
            if (data.entity.hasComponent<Actor>())
            {
                const auto& actor = data.entity.getComponent<Actor>();
                switch (actor.type)
                {
                default: break;
                case ActorID::Clocksy:
                case ActorID::Whirlybob:
                case ActorID::Balldock:
                case ActorID::Squatmo:
                    playSound(m_soundResource.get("assets/sound/npc_pop.wav"));
                    break;
                case ActorID::PlayerOne:
                case ActorID::PlayerTwo:
                    playSound(m_soundResource.get("assets/sound/player_die.wav"));
                    break;
                case ActorID::FlameOne:
                case ActorID::FlameTwo:
                    playSound(m_soundResource.get("assets/sound/flame.wav"));
                    break;
                case ActorID::LightningOne:
                case ActorID::LightningTwo:
                    playSound(m_soundResource.get("assets/sound/lightning_die.wav"));
                    break;
                }
            }
        }
        break;
        case AnimationController::JumpUp:
        {
            const auto& actor = data.entity.getComponent<Actor>();
            switch (actor.type)
            {
            default: 
                playSound(m_soundResource.get("assets/sound/jump.wav"));
                break;
            case ActorID::MagicHat: break;
            }
        }
        break;
        case AnimationController::Walk:
        {
            if (data.entity.hasComponent<Actor>())
            {
                const auto& actor = data.entity.getComponent<Actor>();
                switch (actor.type)
                {
                default: break;
                case ActorID::FlameOne:
                case ActorID::FlameTwo:
                case ActorID::LightningOne:
                case ActorID::LightningTwo:
                    playSound(m_soundResource.get("assets/sound/powerup_pop.wav"));
                    break;
                }
            }
        }
            break;
        }
    }
        break;
    case MessageID::MapMessage:
    {
        const auto& data = msg.getData<MapEvent>();
        if (data.type == MapEvent::HurryUp)
        {
            playSound(m_soundResource.get("assets/sound/hurry.wav"));
        }
        else if (data.type == MapEvent::BonusSwitch)
        {
            playSound(m_soundResource.get("assets/sound/bonus.wav"));
        }
    }
        break;
    }
}

void FXDirector::process(float)
{
    //check our ents and free up finished sounds
    for (auto i = 0u; i < m_nextFreeEntity; ++i)
    {
        if (m_entities[i].getComponent<xy::AudioEmitter>().getStatus() == xy::AudioEmitter::Status::Stopped)
        {
            auto entity = m_entities[i];
            m_nextFreeEntity--;
            m_entities[i] = m_entities[m_nextFreeEntity];
            m_entities[m_nextFreeEntity] = entity;
            i--;
        }
    }
}

//private
void FXDirector::resizeEntities()
{
    m_entities.resize(m_entities.size() + MinEntities);
    for (auto i = m_entities.size() - MinEntities; i < m_entities.size(); ++i)
    {
        m_entities[i] = getScene().createEntity();
        m_entities[i].addComponent<xy::AudioEmitter>().setSource(m_soundResource.get("placeholder"));
        m_entities[i].getComponent<xy::AudioEmitter>().setVolume(200.f);
        m_entities[i].getComponent<xy::AudioEmitter>().setMinDistance(1920.f);
        m_entities[i].getComponent<xy::AudioEmitter>().setRelativeTolistener(true);
        m_entities[i].addComponent<xy::Transform>();
    }
}

xy::Entity FXDirector::getNextEntity()
{
    if (m_nextFreeEntity == m_entities.size())
    {
        resizeEntities();
    }
    auto ent = m_entities[m_nextFreeEntity++];
    return ent;
}

void FXDirector::playSound(sf::SoundBuffer& buffer)
{
    auto ent = getNextEntity();
    ent.getComponent<xy::AudioEmitter>().setSource(buffer);
    ent.getComponent<xy::AudioEmitter>().play();
}