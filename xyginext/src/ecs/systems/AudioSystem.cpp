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

#include "xyginext/ecs/systems/AudioSystem.hpp"

#include "xyginext/ecs/components/Transform.hpp"
#include "xyginext/ecs/components/AudioEmitter.hpp"
#include "xyginext/ecs/components/AudioListener.hpp"
#include "xyginext/ecs/Scene.hpp"

#include "xyginext/audio/Mixer.hpp"
#include "xyginext/core/App.hpp"

#include <SFML/Audio/Listener.hpp>

using namespace xy;

AudioSystem::AudioSystem(MessageBus& mb)
    : System(mb, typeid(AudioSystem))
{
    requireComponent<AudioEmitter>();
    requireComponent<Transform>();
}

//public
void AudioSystem::process(float)
{
    //set listener position to active camera
    auto listenerEnt = getScene()->getActiveListener();
    auto listenerPos = listenerEnt.getComponent<xy::Transform>().getWorldTransform().transformPoint({});
    const auto& listener = listenerEnt.getComponent<AudioListener>();

    sf::Listener::setPosition({ listenerPos.x, listenerPos.y, listener.m_depth });
    sf::Listener::setGlobalVolume(listener.m_volume * AudioMixer::getMasterVolume() * 100.f);

    auto& entities = getEntities();
    for (auto& entity : entities)
    {
        //update position of entities
        const auto& tx = entity.getComponent<xy::Transform>();
        auto& audio = entity.getComponent<xy::AudioEmitter>();

        auto pos = tx.getWorldTransform().transformPoint({});
        audio.setPosition({ pos.x, pos.y, 0.f });
        audio.applyMixerSettings();
        audio.update();
    }
}

//private
void AudioSystem::onEntityRemoved(xy::Entity entity)
{
    auto& ae = entity.getComponent<xy::AudioEmitter>();
    if (ae.hasSource() && ae.getStatus() == AudioEmitter::Playing)
    {
        ae.stop();
    }
}
