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

#include "SoundEffectsDirector.hpp"

#include <xyginext/ecs/components/AudioEmitter.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/Scene.hpp>

#include <xyginext/resources/Resource.hpp>
#include <xyginext/util/Random.hpp>

#include <array>

namespace
{
    //IDs used to index paths below
    enum AudioID
    {
        Sound01, Sound02, etc,

        Count
    };

    //paths to audio files - non streaming only
    //must match with enum above
    const std::array<std::string, AudioID::Count> paths = 
    {
        "assets/sound/effects/sound01.wav",
        "assets/sound/effects/sound02.wav",
        "assets/sound/effects/etc3.wav",
    };

    const std::size_t MinEntities = 32;
}

SFXDirector::SFXDirector(xy::AudioResource& ar)
    : m_audioResource   (ar),
    m_nextFreeEntity    (0)
{
    //pre-load sounds
    for (const auto& str : paths)
    {
        m_audioResource.get(str);
    }
}

//public
void SFXDirector::handleMessage(const xy::Message& msg)
{
    //create sounds based on events here
    /*eg 
    if(msg.id == MessageID::SomeMessage)
    {
        //note playSound() returns a reference to the emitter just played
        //so that properties such as volume or attenuation can quickly be set
        
        playSound(m_audioResource(paths[AudioID::Sound01], msg.getData<SomeEvent>().position);
    }
    */
}

void SFXDirector::process(float dt)
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
xy::Entity SFXDirector::getNextFreeEntity()
{
    if (m_nextFreeEntity == m_entities.size())
    {
        resizeEntities(m_entities.size() + MinEntities);
    }
    return m_entities[m_nextFreeEntity++];
}

void SFXDirector::resizeEntities(std::size_t size)
{
    auto currSize = m_entities.size();
    m_entities.resize(size);

    for (auto i = currSize; i < m_entities.size(); ++i)
    {
        m_entities[i] = getScene().createEntity();
        m_entities[i].addComponent<xy::Transform>();
        m_entities[i].addComponent<xy::AudioEmitter>().setSource(m_audioResource.get("placeholder"));
    }
}

xy::AudioEmitter& SFXDirector::playSound(sf::SoundBuffer& buffer, sf::Vector2f position)
{
    auto entity = getNextFreeEntity();
    entity.getComponent<xy::Transform>().setPosition(position);
    auto& emitter = entity.getComponent<xy::AudioEmitter>();
    emitter.setSource(buffer);
    //must reset values here incase they were changed prior to recycling from pool
    emitter.setAttenuation(1.f);
    emitter.setMinDistance(5.f);
    emitter.setVolume(1.f);
    emitter.setPitch(1.f);
    emitter.play();
    return emitter;
}