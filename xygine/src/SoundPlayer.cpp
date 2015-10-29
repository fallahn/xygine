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

#include <xygine/SoundPlayer.hpp>

#include <SFML/Audio/Listener.hpp>

#include <cmath>
#include <xygine/Assert.hpp>

namespace
{
    const float listenerDepth = 900.f;
    const float attenuation = 1.f;
    const float minDistance2D = 700.f; //bear in mind listener won't be moving and we want to hear everything on screen
    const float minDistance3D = std::sqrt((minDistance2D * minDistance2D) + (listenerDepth * listenerDepth));

    float volume = 60.f; //static value as it is shared by all sound players

    const std::size_t maxSounds = 160u;
}

using namespace xy;

SoundPlayer::SoundPlayer()
{

}

//public
void SoundPlayer::update()
{
    flushSounds();
}

void SoundPlayer::play(const sf::SoundBuffer& sb, bool loop)
{
    if (m_sounds.size() < maxSounds)
    {
        m_sounds.emplace_back();

        auto& sound = m_sounds.back();
        sound.setBuffer(sb);
        sound.setVolume(volume);
        sound.setLoop(loop);
        sound.play();
    }
}

void SoundPlayer::setListenerPosition(const sf::Vector2f& position)
{
    sf::Listener::setPosition(position.x, -position.y, listenerDepth);
}

sf::Vector2f SoundPlayer::getListenerPosition() const
{
    auto pos = sf::Listener::getPosition();
    return{ pos.x, -pos.y };
}

void SoundPlayer::setVolume(float vol)
{
    volume = vol;
}

float SoundPlayer::getVolume()
{
    return volume;
}

//private
void SoundPlayer::flushSounds()
{
    m_sounds.remove_if([](const sf::Sound& s){return (s.getStatus() == sf::Sound::Stopped); });
}
