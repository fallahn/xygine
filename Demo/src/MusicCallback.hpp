/*********************************************************************
(c) Matt Marchant 2017
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

#ifndef DEMO_MUSIC_CALLBACK_HPP_
#define DEMO_MUSIC_CALLBACK_HPP_

#include <xyginext/ecs/components/AudioEmitter.hpp>
#include <xyginext/ecs/Entity.hpp>
#include <xyginext/core/App.hpp>

#include <array>
#include <string>

class MusicCallback final
{
private:
    enum class State
    {
        In, Hold, Out
    };

    static constexpr float FadeTime = 3.f;
    static constexpr float PlayTime = 90.f;
    static constexpr float MaxVolume = 0.25f;

public:
    explicit MusicCallback()
        : m_playTime    (MusicCallback::PlayTime),
        m_fadeTime      (MusicCallback::FadeTime),
        m_state         (State::Hold),
        m_currentTrack  (0)
    {
        m_trackNames = 
        {
            "assets/sound/music/01.ogg",
            "assets/sound/music/02.ogg",
            "assets/sound/music/03.ogg"
        };
    }

    void operator () (xy::Entity entity, float dt)
    {
        switch (m_state)
        {
        default: break;
        case State::In:
        {
            entity.getComponent<xy::AudioEmitter>().setVolume((1.f - (m_fadeTime / FadeTime)) * MaxVolume);
            m_fadeTime -= dt;

            if (m_fadeTime < 0)
            {
                m_fadeTime = FadeTime;
                entity.getComponent<xy::AudioEmitter>().setVolume(MaxVolume);
                m_state = State::Hold;
                m_playTime = PlayTime;
            }
        }
            break;
        case State::Out:
            entity.getComponent<xy::AudioEmitter>().setVolume((m_fadeTime / FadeTime) * MaxVolume);
            m_fadeTime -= dt;
            m_fadeTime = std::max(m_fadeTime, 0.f);

            if (m_fadeTime == 0)
            {
                auto& emitter = entity.getComponent<xy::AudioEmitter>();
                emitter.setVolume(0.f);
                
                m_currentTrack = (m_currentTrack + 1) % m_trackNames.size();
                emitter.stop();
                emitter.setSource(m_trackNames[m_currentTrack]);
                emitter.play();
                emitter.setLooped(true);

                m_fadeTime = FadeTime;
                m_state = State::In;
            }
            break;
        case State::Hold:
            m_playTime -= dt;
            if (m_playTime < 0)
            {
                m_state = State::Out;
            }
            break;
        }
    }

private:
    float m_playTime;
    float m_fadeTime;

    State m_state;

    std::array<std::string, 3u> m_trackNames;
    std::size_t m_currentTrack;
};


#endif //DEMO_MUSIC_CALLBACK_HPP_