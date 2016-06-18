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

#ifndef XY_AUDIO_MANAGER_HPP_
#define XY_AUDIO_MANAGER_HPP_

#include <xygine/SoundPlayer.hpp>
#include <xygine/Music.hpp>

namespace xy
{
    class Message;
    class AudioManager final
    {
    public:
        AudioManager();
        ~AudioManager() = default;
        AudioManager(const AudioManager&) = delete;
        const AudioManager& operator = (const AudioManager&) = delete;

        void update(float);
        void handleMessage(const Message&);

        void mute(bool);

    private:

        float m_fadeInTime;
        float m_currentFadeTime;

        MusicPlayer m_musicPlayer;
        SoundPlayer m_soundPlayer;

        enum SoundIDs
        {
            Size
        };

        std::vector<sf::SoundBuffer> m_impactSounds;
        std::vector<sf::SoundBuffer> m_fxSounds;

        bool m_muted;
    };
}
#endif //XY_AUDIO_MANAGER_HPP_