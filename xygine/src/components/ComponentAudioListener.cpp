/*********************************************************************
Matt Marchant 2014 - 2016
http://trederia.blogspot.com

xygine - Zlib license.

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

#include <xygine/components/AudioListener.hpp>
#include <xygine/Entity.hpp>
#include <xygine/Reports.hpp>

#include <SFML/Audio/Listener.hpp>

using namespace xy;

namespace
{
    AudioListener* instance = nullptr;
    const float listenerDepth = 900.f;
}

const float AudioListener::MaxVolume = 100.f;

AudioListener::AudioListener(MessageBus& mb)
    : Component(mb, this)
{
    if (instance && instance != this)
    {
        instance->destroy();
    }
    instance = this;
}

AudioListener::~AudioListener()
{
    if (instance == this)
    {
        instance = nullptr;
    }
    for (auto& s : m_dyingSounds)
    {
        s.stop();
    }
}

//public
void AudioListener::entityUpdate(Entity& entity, float)
{
    auto pos = entity.getWorldPosition();
    sf::Listener::setPosition({ pos.x, -pos.y, listenerDepth });

    m_dyingSounds.remove_if([](const sf::Sound& s) {return (s.getStatus() == sf::Sound::Stopped); });
    REPORT("Sound Cache Buffer", std::to_string(m_dyingSounds.size()));
}

float AudioListener::getListenerDepth()
{
    return listenerDepth;
}

void AudioListener::setMasterVolume(float vol)
{
    XY_ASSERT(vol > 0 && vol < MaxVolume, "Invalid audio value");
    sf::Listener::setGlobalVolume(vol);
}

//private
void AudioListener::addDyingSound(const sf::Sound& sound)
{
    XY_ASSERT(instance, "No listener instance");
    instance->m_dyingSounds.push_back(sound);
}