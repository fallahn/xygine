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
    m_soundResource.get("assets/sound/collect.wav");
    m_soundResource.get("assets/sound/jump.wav");
    m_soundResource.get("assets/sound/npc_pop.wav");
    m_soundResource.get("assets/sound/shoot.wav");
}

//public
void FXDirector::handleMessage(const xy::Message& msg)
{ 
    if (msg.id == MessageID::SceneMessage)
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
                auto ent = getNextEntity();
                ent.getComponent<xy::AudioEmitter>().setSource(m_soundResource.get("assets/sound/collect.wav"));
                ent.getComponent<xy::AudioEmitter>().play();
            }
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
                auto ent = getNextEntity();
                ent.getComponent<xy::AudioEmitter>().setSource(m_soundResource.get("assets/sound/shoot.wav"));
                ent.getComponent<xy::AudioEmitter>().play();
            }
            break;
            }
        }
    }
    else if (msg.id == MessageID::AnimationMessage)
    {
        const auto& data = msg.getData<AnimationEvent>();
        switch (data.newAnim)
        {
        default: break;
        case AnimationController::Die:
            if (data.entity.getComponent<Actor>().type == ActorID::Clocksy
                || data.entity.getComponent<Actor>().type == ActorID::Whirlybob)
            {
                auto ent = getNextEntity();
                ent.getComponent<xy::AudioEmitter>().setSource(m_soundResource.get("assets/sound/npc_pop.wav"));
                ent.getComponent<xy::AudioEmitter>().play();
            }
            break;
        case AnimationController::JumpUp:
        {
            auto ent = getNextEntity();
            ent.getComponent<xy::AudioEmitter>().setSource(m_soundResource.get("assets/sound/jump.wav"));
            ent.getComponent<xy::AudioEmitter>().play();
        }
            break;
        }
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
        m_entities[i].getComponent<xy::AudioEmitter>().setVolume(100.f);
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