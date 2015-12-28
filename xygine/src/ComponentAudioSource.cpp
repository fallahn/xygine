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

#include <xygine/components/AudioSource.hpp>
#include <xygine/components/AudioListener.hpp>
#include <xygine/Resource.hpp>
#include <xygine/Entity.hpp>
#include <xygine/Assert.hpp>

using namespace xy;

namespace
{
}

AudioSource::FactoryFunc AudioSource::create = std::make_unique<AudioSource>;


AudioSource::AudioSource(MessageBus& mb, SoundResource& sr)
    :Component      (mb, this),
    m_maxVolume     (100.f),
    m_currentVolume (0.f),
    m_fadeTime      (0.f),
    m_pitch         (1.f),
    m_attenuation   (1.f),
    m_minDistance2D (900.f),
    m_minDistance3D (std::sqrt(m_minDistance2D * m_minDistance2D + AudioListener::getListenerDepth() * AudioListener::getListenerDepth())),
    m_currentStatus (Status::Stopped),
    m_mode          (Mode::Cached),
    m_currentSource (&m_sound),
    m_soundResource (sr)
{

}

//public
void AudioSource::entityUpdate(Entity& entity, float)
{
    auto pos = entity.getWorldPosition();

    m_currentSource->setPosition(pos.x, pos.y, 0.f);
    
    if (m_currentVolume < m_maxVolume)
    {
        //fade in
        float ratio = m_fadeClock.getElapsedTime().asSeconds() / m_fadeTime;
        m_currentVolume = m_maxVolume * ratio;

        m_currentVolume = std::min(m_currentVolume, m_maxVolume);
        m_currentSource->setVolume(m_currentVolume);
    }
}

void AudioSource::handleMessage(const Message&)
{}

void AudioSource::destroy()
{
    Component::destroy();
    stop();
}

void AudioSource::setSound(const std::string& path, Mode mode)
{
    m_mode = mode;

    if (mode == Mode::Cached)
    {
        m_currentSource = &m_sound;

        m_sound.setBuffer(m_soundResource.get(path));
    }
    else
    {
        m_currentSource = &m_music;

        m_music.openFromFile(path);
    }
}

void AudioSource::setAttenuation(float attn)
{
    XY_ASSERT(attn >= 0, "attenuation must be a positive value");
    m_attenuation = attn;
    m_currentSource->setAttenuation(attn);
}

float AudioSource::getAttenuation() const
{
    return m_attenuation;
}

void AudioSource::setMinimumDistance(float dist)
{
    m_minDistance2D = dist;
    m_minDistance3D = std::sqrt(dist * dist + AudioListener::getListenerDepth() * AudioListener::getListenerDepth());
    m_currentSource->setMinDistance(m_minDistance3D);
}

float AudioSource::getMinimumDistance() const
{
    return m_minDistance2D;
}

void AudioSource::setPitch(float pitch)
{
    XY_ASSERT(pitch > 0, "pitch must be a positive value");
    m_pitch = pitch;
    m_currentSource->setPitch(pitch);
}

float AudioSource::getPitch() const
{
    return m_pitch;
}

void AudioSource::setFadeInTime(float time)
{
    XY_ASSERT(time >= 0, "fade time must be a positive value");
    m_fadeTime = time;
}

float AudioSource::getFadeInTime() const
{
    return m_fadeTime;
}

void AudioSource::setVolume(float volume)
{
    XY_ASSERT(volume >= 0 && volume <= 100.f, "volume must be 0-100");
    m_maxVolume = volume;

    if (m_currentSource->getVolume() > volume)
    {
        m_currentSource->setVolume(volume);
    }
}

float AudioSource::getVolume() const
{
    return m_maxVolume;
}

void AudioSource::play(bool looped)
{   
    m_fadeClock.restart();
    m_currentStatus = Status::Playing;

    if (m_mode == Mode::Cached)
    {
        m_sound.setLoop(looped);
        m_sound.setAttenuation(m_attenuation);
        m_sound.setMinDistance(m_minDistance3D);
        m_sound.setPitch(m_pitch);
        m_sound.setVolume(m_currentVolume);
        m_sound.play();
    }
    else
    {
        m_music.setLoop(looped);
        m_music.setAttenuation(m_attenuation);
        m_music.setMinDistance(m_minDistance3D);
        m_music.setPitch(m_pitch);
        m_music.setVolume(m_currentVolume);
        m_music.play();
    }
}

void AudioSource::pause()
{
    m_currentStatus = Status::Paused;

    (m_mode == Mode::Cached) ?
        m_sound.pause()
        :
        m_music.pause();
}

void AudioSource::stop()
{
    m_currentStatus = Status::Stopped;

    (m_mode == Mode::Cached) ?
        m_sound.stop()
        :
        m_music.stop();

    m_currentVolume = 0.f;
}

AudioSource::Status AudioSource::getStatus() const
{
    return m_currentStatus;
}