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

#include <xygine/components/AudioSource.hpp>
#include <xygine/components/AudioListener.hpp>
#include <xygine/Resource.hpp>
#include <xygine/Entity.hpp>
#include <xygine/Assert.hpp>

using namespace xy;

namespace
{
}

AudioSource::AudioSource(MessageBus& mb, SoundResource& sr)
    :Component      (mb, this),
    m_maxVolume     (100.f),
    m_currentVolume (0.f),
    m_stoppingVolume(0.f),
    m_fadeInTime    (0.f),
    m_fadeOutTime   (0.f),
    m_fadeElapsed   (0.f),
    m_duration      (0.f),
    m_pitch         (1.f),
    m_attenuation   (1.f),
    m_minDistance2D (900.f),
    m_minDistance3D (std::sqrt(m_minDistance2D * m_minDistance2D + AudioListener::getListenerDepth() * AudioListener::getListenerDepth())),
    m_currentStatus (Status::Stopped),
    m_mode          (Mode::Cached),
    m_looped        (false),
    m_currentSource (&m_sound),
    m_soundResource (sr)
{

}

//public
void AudioSource::entityUpdate(Entity& entity, float dt)
{
    auto pos = entity.getWorldPosition();

    m_currentSource->setPosition(pos.x, pos.y, 0.f);
    
    if (m_currentVolume < m_maxVolume)
    {
        //fade in
        m_fadeElapsed += dt;
        const float ratio = m_fadeElapsed / m_fadeInTime;
        m_currentVolume = m_maxVolume * ratio;

        m_currentVolume = std::min(m_currentVolume, m_maxVolume);
        m_currentSource->setVolume(m_currentVolume);
    }

    //fade out if not looped
    if (!m_looped)
    {
        const float remain = (m_mode == Mode::Cached) ?
            m_duration - m_sound.getPlayingOffset().asSeconds()
            : m_music.getPlayingOffset().asSeconds();

        if (remain < m_fadeOutTime)
        {
            if (remain <= 0)
            {
                m_currentStatus = Status::Stopped;

                auto msg = getMessageBus().post<xy::Message::AudioEvent>(xy::Message::AudioMessage);
                msg->entityId = getParentUID();
                msg->action = xy::Message::AudioEvent::Stop;
            }

            const float ratio = std::min(1.f, std::abs(remain) / m_fadeOutTime);
            m_currentVolume = m_maxVolume * ratio;
            m_currentSource->setVolume(m_currentVolume);
        }
    }
    else
    {
        //fade out once stop has been requested
        if (m_currentStatus == Status::Stopping)
        {
            const float ratio = 1.f - std::min(1.f, m_fadeClock.getElapsedTime().asSeconds() / m_fadeOutTime);
            m_currentVolume = m_stoppingVolume * ratio;
            m_currentSource->setVolume(m_currentVolume);

            if (m_currentVolume == 0)
            {
                (m_mode == Mode::Cached) ?
                    m_sound.stop()
                    :
                    m_music.stop();
                m_currentStatus = Status::Stopped;

                auto msg = getMessageBus().post<xy::Message::AudioEvent>(xy::Message::AudioMessage);
                msg->entityId = getParentUID();
                msg->action = xy::Message::AudioEvent::Stop;
            }
        }
    }
}

void AudioSource::destroy()
{
    Component::destroy();
    
    if (m_mode == Mode::Cached/* && !m_sound.getLoop()*/)
    {
        m_sound.setLoop(false);
        AudioListener::addDyingSound(m_sound);
    }
    else
    {
        stop();
    }
}

void AudioSource::setSound(const std::string& path, Mode mode)
{
    m_mode = mode;

    if (mode == Mode::Cached)
    {
        m_currentSource = &m_sound;

        m_sound.setBuffer(m_soundResource.get(path));
        m_duration = m_sound.getBuffer()->getDuration().asSeconds();
    }
    else
    {
        m_currentSource = &m_music;

        m_music.openFromFile(path);
        m_duration = m_music.getDuration().asSeconds();
    }
}

void AudioSource::setSoundBuffer(sf::SoundBuffer& sb)
{
    XY_WARNING(m_mode != Mode::Cached, "setting this sound buffer will have no effect on streaming audio!");
    m_sound.setBuffer(sb);
    m_duration = sb.getDuration().asSeconds();
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
    m_fadeInTime = time;
}

float AudioSource::getFadeInTime() const
{
    return m_fadeInTime;
}

void AudioSource::setFadeOutTime(float time)
{
    XY_ASSERT(time >= 0, "fade time must be non-negative");
    m_fadeOutTime = time;
}

float AudioSource::getFadeOutTime() const
{
    return m_fadeOutTime;
}

float AudioSource::getDuration() const
{
    return m_duration;
}

void AudioSource::setVolume(float volume)
{
    XY_ASSERT(volume >= 0 && volume <= 100.f, "volume must be 0-100");
    
    if (m_maxVolume > volume)
    {
        m_currentSource->setVolume(volume);
        
    }
    m_maxVolume = volume;

    if (m_maxVolume < m_currentVolume)
    {
        m_currentVolume = volume;
    }
}

float AudioSource::getVolume() const
{
    return m_maxVolume;
}

void AudioSource::play(bool looped)
{
    if (m_currentStatus != Status::Playing)
    {
        m_fadeElapsed = 0.f;
        if (m_currentStatus != Status::Stopping)
        {
            m_currentVolume = 0.f;
        }
        else
        {
            //decide how far along the fade in we'd be
            const float ratio = m_currentVolume / m_maxVolume;
            m_fadeElapsed = m_fadeInTime * ratio;
        }
        m_looped = looped;

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

        auto msg = getMessageBus().post<xy::Message::AudioEvent>(xy::Message::AudioMessage);
        msg->entityId = getParentUID();
        msg->action = xy::Message::AudioEvent::Play;
    }
}

void AudioSource::pause()
{
    m_currentStatus = Status::Paused;

    (m_mode == Mode::Cached) ?
        m_sound.pause()
        :
        m_music.pause();

    auto msg = getMessageBus().post<xy::Message::AudioEvent>(xy::Message::AudioMessage);
    msg->entityId = getParentUID();
    msg->action = xy::Message::AudioEvent::Pause;
}

void AudioSource::stop()
{
    if (!m_looped) //if we're looped we'll fade out
    {
        (m_mode == Mode::Cached) ?
            m_sound.stop()
            :
            m_music.stop();

        m_currentVolume = 0.f;
        m_currentStatus = Status::Stopped;

        auto msg = getMessageBus().post<xy::Message::AudioEvent>(xy::Message::AudioMessage);
        msg->entityId = getParentUID();
        msg->action = xy::Message::AudioEvent::Stop;
    }
    else
    {
        //prevent jumping to maxVol if fading out while
        //still fading in
        m_stoppingVolume = m_currentVolume;
        m_currentStatus = Status::Stopping;
        m_fadeClock.restart();
    }
}

AudioSource::Status AudioSource::getStatus() const
{
    return m_currentStatus;
}