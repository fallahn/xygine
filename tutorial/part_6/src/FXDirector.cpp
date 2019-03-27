/*********************************************************************
(c) Matt Marchant 2019

This file is part of the xygine tutorial found at
https://github.com/fallahn/xygine

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

#include <xyginext/resources/ResourceHandler.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/AudioEmitter.hpp>
#include <xyginext/ecs/Scene.hpp>

namespace
{
    std::size_t AudioBufferHandle = 0;
    const std::size_t MinEntities = 12;
}

FXDirector::FXDirector(xy::ResourceHandler& rh)
    : m_resources(rh),
    m_nextFreeEntity(0)
{
    m_particleSettings.loadFromFile("assets/particles/impact.xyp", rh);

    AudioBufferHandle = rh.load<sf::SoundBuffer>("assets/sound/boop.wav");
}

//public
void FXDirector::handleMessage(const xy::Message& msg)
{
    if (msg.id == MessageID::BlockMessage)
    {
        const auto& data = msg.getData<BlockEvent>();
        if (data.action == BlockEvent::Destroyed)
        {
            doEffect(data.position);
        }
    }
}

void FXDirector::process(float)
{
    //check all entities and free any which have finished playing all the effects
    for (auto i = 0u; i < m_nextFreeEntity; ++i)
    {
        if (m_entities[i].getComponent<xy::AudioEmitter>().getStatus() == xy::AudioEmitter::Stopped
            && m_entities[i].getComponent<xy::ParticleEmitter>().stopped())
        {
            //swaps the expired entity with the last active entity
            //and decrements i so we're up to date in the loop
            auto entity = m_entities[i];
            m_nextFreeEntity--;
            m_entities[i] = m_entities[m_nextFreeEntity];
            m_entities[m_nextFreeEntity] = entity;
            i--;
        }
    }
}

//private
xy::Entity FXDirector::getNextFreeEntity()
{
    if (m_nextFreeEntity == m_entities.size())
    {
        resizeEntities(m_entities.size() + MinEntities);
    }
    return m_entities[m_nextFreeEntity++];
}

void FXDirector::resizeEntities(std::size_t size)
{
    auto currSize = m_entities.size();
    m_entities.resize(size);

    for (auto i = currSize; i < size; ++i)
    {
        m_entities[i] = getScene().createEntity();
        m_entities[i].addComponent<xy::Transform>();
        m_entities[i].addComponent<xy::AudioEmitter>().setSource(m_resources.get<sf::SoundBuffer>(AudioBufferHandle));
        m_entities[i].addComponent<xy::ParticleEmitter>().settings = m_particleSettings;
    }
}

void FXDirector::doEffect(sf::Vector2f position)
{
    auto entity = getNextFreeEntity();
    entity.getComponent<xy::Transform>().setPosition(position);
    entity.getComponent<xy::AudioEmitter>().play();
    entity.getComponent<xy::ParticleEmitter>().start();
}