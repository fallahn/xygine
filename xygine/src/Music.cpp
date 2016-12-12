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

#include <xygine/Music.hpp>
#include <xygine/Log.hpp>

#include <xygine/Assert.hpp>
#include <algorithm>

using namespace xy;

MusicPlayer::MusicPlayer()
    : m_volume      (100.f)
{

}

void MusicPlayer::play(const std::string& title, bool loop)
{
    if (m_music.openFromFile(title))
    {
        m_music.setVolume(m_volume);
        m_music.setLoop(loop);
        m_music.play();
    }
    else
    {
        //TODO this probably warns anyway
        LOG("failed to open " + title, Logger::Type::Error);
    }
}

void MusicPlayer::stop()
{
    m_music.stop();
}

void MusicPlayer::setPaused(bool paused)
{
    (paused) ? m_music.pause() : m_music.play();
}

bool MusicPlayer::playing() const
{
    return (m_music.getStatus() == sf::SoundSource::Playing);
}

void MusicPlayer::setVolume(float volume)
{
    XY_ASSERT(volume >= 0.f && volume <= 100.f, "volume value out of range 0 - 100");

    m_volume = volume;
    if(m_music.getStatus() == sf::Music::Playing)
        m_music.setVolume(volume);
}

float MusicPlayer::getVolume() const
{
    return m_volume;
}