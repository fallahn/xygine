/*********************************************************************
© Matt Marchant 2014 - 2017
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

//plays music as requested by the current state

#ifndef XY_MUSIC_HPP_
#define XY_MUSIC_HPP_

#include <SFML/Audio/Music.hpp>

#include <string>

namespace xy
{
    class MusicPlayer final : private sf::NonCopyable
    {
    public:
        MusicPlayer();
        ~MusicPlayer() = default;
        MusicPlayer(const MusicPlayer&) = delete;
        const MusicPlayer& operator = (const MusicPlayer&) = delete;

        void play(const std::string& file, bool loop = false);
        void stop();
        void setPaused(bool paused);
        bool playing() const;
        void setVolume(float volume);
        float getVolume() const;

        void update(float);

    private:

        float m_volume;
        sf::Music m_music;
    };
}

#endif //XY_MUSIC_HPP_