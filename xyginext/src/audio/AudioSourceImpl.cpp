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

#include "xyginext/audio/AudioSourceImpl.hpp"
#include "xyginext/core/FileSystem.hpp"
#include "xyginext/core/Log.hpp"

using namespace xy;
using namespace xy::Detail;

//-------------------------------------------------
AudioNull::AudioNull()
{

}

void AudioNull::play()
{
    LOG("No sound loaded.", xy::Logger::Type::Warning);
}

void AudioNull::pause()
{
    LOG("No sound loaded.", xy::Logger::Type::Warning);
}

void AudioNull::stop()
{
    LOG("No sound loaded.", xy::Logger::Type::Warning);
}

void AudioNull::setPitch(float)
{
    //LOG("No sound loaded. Pitch not set", xy::Logger::Type::Warning);
}

void AudioNull::setVolume(float)
{
    //LOG("No sound loaded. Volume not set", xy::Logger::Type::Warning);
}

void AudioNull::setPosition(sf::Vector3f)
{
    //LOG("No sound loaded. Position not set", xy::Logger::Type::Warning);
}

void AudioNull::setRelativeTolistener(bool)
{
    LOG("No sound loaded. Relativity not set", xy::Logger::Type::Warning);
}

void AudioNull::setMinDistance(float)
{
    //LOG("No sound loaded. MinDistance not set", xy::Logger::Type::Warning);
}

void AudioNull::setAttenuation(float)
{
    //LOG("No sound loaded. Attenuation not set", xy::Logger::Type::Warning);
}

void AudioNull::setLooped(bool)
{
    LOG("No sound loaded. Loop not set", xy::Logger::Type::Warning);
}

float AudioNull::getPitch() const
{
    //LOG("No sound loaded. Pitch not set", xy::Logger::Type::Warning);
    return 0.f;
}

float AudioNull::getVolume() const
{
    //LOG("No sound loaded. Volume not set", xy::Logger::Type::Warning);
    return 0.f;
}

sf::Vector3f AudioNull::getPosition() const
{
    //LOG("No sound loaded. Position not set", xy::Logger::Type::Warning);
    return {};
}

bool AudioNull::isRelativeToListener() const
{
    LOG("No sound loaded.", xy::Logger::Type::Warning);
    return false;
}

float AudioNull::getMinDistance() const
{
    //LOG("No sound loaded. MinDistance not set", xy::Logger::Type::Warning);
    return 0.f;
}

float AudioNull::getAttenuation() const
{
    //LOG("No sound loaded. Attenuation not set", xy::Logger::Type::Warning);
    return 0.f;
}

bool AudioNull::isLooped() const
{
    //LOG("No sound loaded. No loop set", xy::Logger::Type::Warning);
    return false;
}

sf::Int32 AudioNull::getStatus() const
{
    //LOG("No sound loaded. Sound not playing", xy::Logger::Type::Warning);
    return sf::Sound::Status::Stopped;
}

sf::Time AudioNull::getDuration() const
{
    LOG("No sound loaded. No duration", xy::Logger::Type::Warning);
    return sf::seconds(0.f);
}

sf::Time AudioNull::getPlayingOffset() const
{
    LOG("No sound loaded. Offset not set", xy::Logger::Type::Warning);
    return sf::seconds(0.f);
}

void AudioNull::setPlayingOffset(sf::Time)
{
    LOG("No sound loaded. Offset not set", xy::Logger::Type::Warning);
}

//--------------------------------------------------

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