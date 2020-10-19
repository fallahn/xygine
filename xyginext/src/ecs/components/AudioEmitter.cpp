/*********************************************************************
(c) Matt Marchant 2017 - 2020
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

#include "xyginext/ecs/components/AudioEmitter.hpp"
#include "xyginext/core/Assert.hpp"
#include "xyginext/audio/Mixer.hpp"

#include <SFML/Audio/SoundBuffer.hpp>

using namespace xy;

AudioEmitter::AudioEmitter()
    : m_mixerChannel    (0),
    m_volume            (1.f),
    m_nextStatus        (Status::Stopped)
{
    m_impl = std::make_unique<Detail::AudioNull>();
}

//public
void AudioEmitter::setSource(const sf::SoundBuffer& buf)
{
    m_impl = std::make_unique<Detail::AudioSound>(buf);
    m_impl->setVolume(m_volume * AudioMixer::getVolume(m_mixerChannel));
}

bool AudioEmitter::setSource(const std::string& path)
{
    m_impl = std::make_unique<Detail::AudioMusic>(path);
    m_impl->setVolume(m_volume * AudioMixer::getVolume(m_mixerChannel));
    return static_cast<Detail::AudioMusic*>(m_impl.get())->isValid();
}

bool AudioEmitter::hasSource() const
{
    return m_impl != nullptr;
}

void AudioEmitter::play()
{
    XY_ASSERT(m_impl, "No valid sound loaded");
    //m_impl->play();

    //this is delayed to allow any position updates
    //to be applied by the system before audio starts playing
    m_nextStatus = Status::Playing;
}

void AudioEmitter::pause()
{
    XY_ASSERT(m_impl, "No valid sound loaded");
    m_impl->pause();
}

void AudioEmitter::stop()
{
    XY_ASSERT(m_impl, "No valid sound loaded");
    m_impl->stop();
}

void AudioEmitter::setPitch(float pitch)
{
    XY_ASSERT(m_impl, "No valid sound loaded");
    m_impl->setPitch(pitch);
}

void AudioEmitter::setVolume(float vol)
{
    XY_ASSERT(m_impl, "No valid sound loaded");
    m_volume = std::max(0.f, vol);
}

void AudioEmitter::setPosition(sf::Vector3f position)
{
    XY_ASSERT(m_impl, "No valid sound loaded");
    m_impl->setPosition(position);
}

void AudioEmitter::setRelativeTolistener(bool relative)
{
    XY_ASSERT(m_impl, "No valid sound loaded");
    m_impl->setRelativeTolistener(relative);
}

void AudioEmitter::setMinDistance(float dist)
{
    XY_ASSERT(m_impl, "No valid sound loaded");
    m_impl->setMinDistance(dist);
}

void AudioEmitter::setAttenuation(float att)
{
    XY_ASSERT(m_impl, "No valid sound loaded");
    m_impl->setAttenuation(att);
}

void AudioEmitter::setLooped(bool loop)
{
    XY_ASSERT(m_impl, "No valid sound loaded");
    m_impl->setLooped(loop);
}

void AudioEmitter::setChannel(std::uint8_t chan)
{
    XY_ASSERT(chan < AudioMixer::MaxChannels, "Channel out of range");
    XY_ASSERT(m_impl, "No valid sound loaded");
    m_mixerChannel = chan;
    m_impl->setVolume(m_volume * AudioMixer::getVolume(m_mixerChannel) * AudioMixer::getPrefadeVolume(m_mixerChannel));
}

void AudioEmitter::setPlayingOffset(sf::Time offset)
{
    XY_ASSERT(m_impl, "No valid sound loaded");
    m_impl->setPlayingOffset(offset);
}

float AudioEmitter::getPitch() const
{
    XY_ASSERT(m_impl, "No valid sound loaded");
    return m_impl->getPitch();
}

float AudioEmitter::getVolume() const
{
    return m_volume;
}

sf::Vector3f AudioEmitter::getPosition() const
{
    XY_ASSERT(m_impl, "No valid sound loaded");
    return m_impl->getPosition();
}

bool AudioEmitter::isRelativeToListener() const 
{
    XY_ASSERT(m_impl, "No valid sound loaded");
    return m_impl->isRelativeToListener();
}

float AudioEmitter::getMinDistance() const
{
    XY_ASSERT(m_impl, "No valid sound loaded");
    return m_impl->getMinDistance();
}

float AudioEmitter::getAttenuation() const
{
    XY_ASSERT(m_impl, "No valid sound loaded");
    return m_impl->getAttenuation();
}

bool AudioEmitter::isLooped() const
{
    XY_ASSERT(m_impl, "No valid sound loaded");
    return m_impl->isLooped();
}

AudioEmitter::Status AudioEmitter::getStatus() const
{
    XY_ASSERT(m_impl, "No valid sound loaded");

    //we're expecting to be playing if we had a request so status should reflect that

    return m_nextStatus == Status::Stopped ? 
        static_cast<Status>(m_impl->getStatus())
        : Status::Playing; 
}

sf::Time AudioEmitter::getDuration() const
{
    XY_ASSERT(m_impl, "No valid sound loaded");
    return m_impl->getDuration();
}

sf::Time AudioEmitter::getPlayingOffset() const
{
    XY_ASSERT(m_impl, "No valid sound loaded");
    return m_impl->getPlayingOffset();
}

void AudioEmitter::applyMixerSettings(float multiplier)
{
    m_impl->setVolume(m_volume * AudioMixer::getVolume(m_mixerChannel) * AudioMixer::getPrefadeVolume(m_mixerChannel) * multiplier);
}

bool AudioEmitter::isStreaming() const
{
    if (hasSource())
    {
        return m_impl->getType() == Detail::AudioSourceImpl::Type::Music;
    }
    return false;
}

void AudioEmitter::update()
{
    XY_ASSERT(m_impl, "No valid sound loaded");

    if (m_nextStatus != Status::Stopped)
    {
        m_impl->play();
        m_nextStatus = Status::Stopped;
    }
}