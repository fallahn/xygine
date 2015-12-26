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

//represents a sound source in the scene. if the supplied sound file
//is mono then the sound is automatically panned based on the parent 
//entity's position and the position of the scene's audio listener

#ifndef XY_AUDIO_SOURCE_HPP_
#define XY_AUDIO_SOURCE_HPP_

#include <xygine/components/Component.hpp>

#include <SFML/System/Vector3.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/Music.hpp>

namespace xy
{
    class SoundResource;
    class AudioSource final : public Component
    {
    public:
        using Ptr = std::unique_ptr<AudioSource>;
    private:
        using FactoryFunc = Ptr(&)(MessageBus&, SoundResource&);
    public:
        const static FactoryFunc create;

        //long files such as music should normally
        //be set to stream, whereas shorter files
        //such as effects can be cached in memory.
        enum class Mode
        {
            Stream,
            Cached
        };

        AudioSource(MessageBus&, SoundResource&);
        ~AudioSource() = default;

        Component::Type type() const override { return Component::Type::Script; }

        void entityUpdate(Entity&, float) override;
        void handleMessage(const Message&) override;
        void destroy() override;

        //sets the sound to be played by this sound source.
        //mode should reflect whether or not the sound should be
        //streamed from disk or loaded into memory
        void setSound(const std::string& path, Mode = Mode::Cached);
        //sets the rate at which the volume of this sound is
        //reduced as it moves away from the scene's audio listener
        void setAttenuation(float);
        //returns the current attenuation of this sound source
        float getAttenuation() const;
        //sets the minimum distance from the audio listener at which
        //the sound source starts to become audible
        void setMinimumDistance(float);
        //gets the current minimum distance for this sound source
        float getMinimumDistance() const;
        //sets the time in seconds before the sound reaches the
        //currently set volume
        void setFadeInTime(float);
        //returns the current fade in time in seconds
        float getFadeInTime() const;
        //sets the maximum volume for this sound source
        void setVolume(float);
        //gets the current maximum volume for this source
        float getVolume() const;
        //sets the pitch as a ratio where 1 is normal
        void setPitch(float);
        //gets the current pitch of the audio source
        float getPitch() const;

        //void setFadeOutTime(float);
        //float getFadeOutTime() const;

        //plays the sound from the beginning if the sound source
        //is stopped, or resumes play if it is paused.
        void play(bool looped = false);
        //pauses playback of the sound if it is playing
        void pause();
        //stops and rewinds the sound if it is playing
        void stop();

        enum class Status
        {
            Playing,
            Paused,
            Stopped
        };
        //returns the current status of the sound source
        Status getStatus() const;

    private:
        float m_maxVolume;
        float m_currentVolume;
        float m_fadeTime;
        float m_pitch;
        float m_attenuation;
        float m_minDistance2D;
        float m_minDistance3D;
        Status m_currentStatus;
        Mode m_mode;

        sf::Sound m_sound;
        sf::Music m_music;
        sf::SoundSource* m_currentSource;
        sf::Clock m_fadeClock;
        SoundResource& m_soundResource;
    };
}

#endif //XY_AUDIO_SOURCE_HPP_