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
    /*!
    \brief Represents a sound source in the scene.
    
    if the supplied sound file is mono then the sound is automatically
    panned based on the parent entity's position and the position of the
    scene's audio listener
    */
    class XY_EXPORT_API AudioSource final : public Component
    {
    public:
        using Ptr = std::unique_ptr<AudioSource>;

        /*!
        long files such as music should normally
        be set to stream, whereas shorter files
        such as effects can be cached in memory.
        */
        enum class Mode
        {
            Stream,
            Cached
        };

        AudioSource(MessageBus&, SoundResource&);
        ~AudioSource();

        Component::Type type() const override { return Component::Type::Script; }

        void entityUpdate(Entity&, float) override;
        void destroy() override;
        /*!
        \brief Sets the sound to be played by this sound source.

        \param path relative path to the sound file on disk
        \param mode should reflect whether or not the sound should be
        streamed from disk or loaded into memory
        */
        void setSound(const std::string& path, Mode = Mode::Cached);
        /*!
        \brief Sets the SoundBuffer for Cached mode sounds.
        Has no effect on streaming sounds. When creating a lot
        of components on the fly it may be more effecient to cache
        SoundBuffers elsewhere to save a string lookup for the
        SoundBuffer audio path each time.
        \param sf::SoundBuffer New sound buffer for the audio
        */
        void setSoundBuffer(sf::SoundBuffer&);
        /*!
        \brief Sets the attenuation rate
        
        sets the rate at which the volume of this sound is
        reduced as it moves away from the scene's audio listener
        \param amount must be greater than zero, set to 1 by default
        */
        void setAttenuation(float);
        /*!
        \brief Returns the current attenuation of this sound source
        \see setAttenuation
        */
        float getAttenuation() const;
        /*!
        \brief Sets the minimum distance from the audio listener at which
        the sound source starts to become audible

        \param float distance in world units
        */
        void setMinimumDistance(float);
        /*!
        \brief Gets the current minimum distance for this sound source

        \see setMinimumDistance
        */
        float getMinimumDistance() const;
        /*!
        \brief Sets the time before the sound reaches the
        currently set maximum volume

        \param float time in seconds for the fade duration
        */
        void setFadeInTime(float);
        /*!
        \brief Returns the current fade in time in seconds

        \see setFadeInTime
        */
        float getFadeInTime() const;
        /*!
        \brief Sets the amount of time it takes to fade the audio to 0

        Unlooped sounds will attempt to fade out for this duration when
        reaching the end of playback, and looped sounds will fade out
        before stopping, when stop is called.
        \param float Time in seconds to fade out for
        */
        void setFadeOutTime(float);
        /*!
        \brief Returns the current duration of the audio fade out, in seconds
        */
        float getFadeOutTime() const;
        /*!
        \brief Returns the duration in seconds of the currently loaded audio
        */
        float getDuration() const;
        /*!
        \brief Sets the maximum volume for this sound source

        \param float new maximum volume. Must be between 0 and 100
        */
        void setVolume(float);
        /*!
        \brief Gets the current maximum volume for this source

        \see setVolume
        */
        float getVolume() const;
        /*!
        \brief Sets the pitch of the sound by altering its playback
        speed.
        
        \param float as a ratio where 1 is normal 2 is twice as fast etc.
        */
        void setPitch(float);
        /*!
        \brief Gets the current pitch of the audio source

        \see setPitch
        */
        float getPitch() const;

        //void setFadeOutTime(float);
        //float getFadeOutTime() const;

        /*!
        \brief plays the sound from the beginning if the sound source
        is stopped, or resumes play if it is paused.

        \param looped Sets whether or not the audio should be played
        once or continually in a loop
        */
        void play(bool looped = false);
        /*!
        \brief Pauses playback of the sound if it is playing
        */
        void pause();
        /*!
        \brief Stops and rewinds the sound if it is playing
        */
        void stop();

        enum class Status
        {
            Playing,
            Paused,
            Stopping,
            Stopped
        };
        /*!
        \brief Returns the current status of the sound source
        */
        Status getStatus() const;

    private:
        float m_maxVolume;
        float m_currentVolume;
        float m_stoppingVolume;
        float m_fadeInTime;
        float m_fadeOutTime;
        float m_fadeElapsed;
        float m_duration;
        float m_pitch;
        float m_attenuation;
        float m_minDistance2D;
        float m_minDistance3D;
        Status m_currentStatus;
        Mode m_mode;
        bool m_looped;

        sf::Sound m_sound;
        sf::Music m_music;
        sf::SoundSource* m_currentSource;
        sf::Clock m_fadeClock;
        SoundResource& m_soundResource;
    };
}

#endif //XY_AUDIO_SOURCE_HPP_