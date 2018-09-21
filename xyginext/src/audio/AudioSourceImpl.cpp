/*********************************************************************
(c) Matt Marchant 2017 - 2018
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

#include "xyginext/audio/AudioSourceImpl.hpp"
#include "xyginext/core/FileSystem.hpp"

using namespace xy;
using namespace xy::Detail;

//-------------------------------------------------
AudioSound::AudioSound(const sf::SoundBuffer& buff)
{
    m_sound.setBuffer(buff);
}

AudioSound::~AudioSound()
{
    if (m_sound.getStatus() == sf::Sound::Playing)
    {
        m_sound.stop();
    }
}

void AudioSound::play()
{
    m_sound.play();
}

void AudioSound::pause()
{
    m_sound.pause();
}

void AudioSound::stop()
{
    m_sound.stop();
}

void AudioSound::setPitch(float pitch)
{
    m_sound.setPitch(pitch);
}

void AudioSound::setVolume(float vol)
{
    m_sound.setVolume(vol * 100.f);
}

void AudioSound::setPosition(sf::Vector3f pos)
{
    m_sound.setPosition(pos);
}

void AudioSound::setRelativeTolistener(bool rel)
{
    m_sound.setRelativeToListener(rel);
}

void AudioSound::setMinDistance(float dist)
{
    m_sound.setMinDistance(dist);
}

void AudioSound::setAttenuation(float att)
{
    m_sound.setAttenuation(att);
}

void AudioSound::setLooped(bool loop)
{
    m_sound.setLoop(loop);
}

float AudioSound::getPitch() const
{
    return m_sound.getPitch();
}

float AudioSound::getVolume() const
{
    return m_sound.getVolume() / 100.f;
}

sf::Vector3f AudioSound::getPosition() const
{
    return m_sound.getPosition();
}

bool AudioSound::isRelativeToListener() const
{
    return m_sound.isRelativeToListener();
}

float AudioSound::getMinDistance() const
{
    return m_sound.getMinDistance();
}

float AudioSound::getAttenuation() const
{
    return m_sound.getAttenuation();
}

bool AudioSound::isLooped() const
{
    return m_sound.getLoop();
}

sf::Int32 AudioSound::getStatus() const
{
    return m_sound.getStatus();
}

sf::Time AudioSound::getDuration() const
{
    return m_sound.getBuffer()->getDuration();
}

sf::Time AudioSound::getPlayingOffset() const
{
    return m_sound.getPlayingOffset();
}

void AudioSound::setPlayingOffset(sf::Time offset)
{
    m_sound.setPlayingOffset(offset);
}

//--------------------------------------------------

AudioMusic::AudioMusic(const std::string& path)
{
    m_valid = m_music.openFromFile(xy::FileSystem::getResourcePath() + path);
}

AudioMusic::~AudioMusic()
{
    m_music.stop();
}

void AudioMusic::play()
{
    m_music.play();
}

void AudioMusic::pause()
{
    m_music.pause();
}

void AudioMusic::stop()
{
    m_music.stop();
}

void AudioMusic::setPitch(float pitch)
{
    m_music.setPitch(pitch);
}

void AudioMusic::setVolume(float vol)
{
    m_music.setVolume(vol * 100.f);
}

void AudioMusic::setPosition(sf::Vector3f pos)
{
    m_music.setPosition(pos);
}

void AudioMusic::setRelativeTolistener(bool rel)
{
    m_music.setRelativeToListener(rel);
}

void AudioMusic::setMinDistance(float dist)
{
    m_music.setMinDistance(dist);
}

void AudioMusic::setAttenuation(float att)
{
    m_music.setAttenuation(att);
}

void AudioMusic::setLooped(bool loop)
{
    m_music.setLoop(loop);
}

float AudioMusic::getPitch() const
{
    return m_music.getPitch();
}

float AudioMusic::getVolume() const
{
    return m_music.getVolume() / 100.f;
}

sf::Vector3f AudioMusic::getPosition() const
{
    return m_music.getPosition();
}

bool AudioMusic::isRelativeToListener() const
{
    return m_music.isRelativeToListener();
}

float AudioMusic::getMinDistance() const
{
    return m_music.getMinDistance();
}

float AudioMusic::getAttenuation() const
{
    return m_music.getAttenuation();
}

bool AudioMusic::isLooped() const
{
    return m_music.getLoop();
}

sf::Int32 AudioMusic::getStatus() const
{
    return m_music.getStatus();
}

sf::Time AudioMusic::getDuration() const
{
    return m_music.getDuration();
}

sf::Time AudioMusic::getPlayingOffset() const
{
    return m_music.getPlayingOffset();
}

void AudioMusic::setPlayingOffset(sf::Time offset)
{
    m_music.setPlayingOffset(offset);
}