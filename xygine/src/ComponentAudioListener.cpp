/*********************************************************************
Matt Marchant 2014 - 2015
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

#include <SFML/Audio/Listener.hpp>

using namespace xy;

namespace
{
    AudioListener* instance = nullptr;
    const float listenerDepth = 900.f;
}

AudioListener::FactoryFunc AudioListener::create = std::make_unique<AudioListener>;

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
}

//public
void AudioListener::entityUpdate(Entity& entity, float)
{
    auto pos = entity.getWorldPosition();
    sf::Listener::setPosition({ pos.x, -pos.y, listenerDepth });
}

void AudioListener::handleMessage(const Message&) {}

float AudioListener::getListenerDepth()
{
    return listenerDepth;
}