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

#ifndef XY_SOUNDPLAYER_HPP_
#define XY_SOUNDPLAYER_HPP_

#include <xygine/components/Component.hpp>

#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/Sound.hpp>

#include <map>
#include <list>
#include <array>

namespace xy
{
    class SoundResource;

    /*!
    \brief Sound Player component.
    As an alternative to audio source components, the sound
    player component can be attached to a single scene entity
    and be used to play sounds via xy::Command or xy::Message
    objects. Sounds are pre-cached and assigned to an audio channel.
    Audio channels have their own volume property, so that groups
    of sounds can be adjusted together. The volume of a channel
    is relative to the master audio volume of the sound player.
    */
    class XY_EXPORT_API SoundPlayer final : public xy::Component
    {
    public:
        SoundPlayer(xy::MessageBus&, xy::SoundResource&);
        ~SoundPlayer() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Script; }
        void entityUpdate(xy::Entity&, float) override;

        using ResourceID = sf::Int32;
        /*!
        \brief Pre-load a sound file for playing.
        \param ResourceID ID to map to this sound resource
        \param std::string Path to resource to load
        \param sf::Uint8 Channel via which the sound should be played.
        There are a maximum of 16 different channels
        */
        void preCache(ResourceID, const std::string&, sf::Uint8 = 0);
        /*!
        \brief Plays a pre-cached sound.
        \param ResourceID ID of the sound to be played
        \param float X position in world coordinates of the sound
        \param float Y position of the sound in world coordinates
        \param float Pitch of the sound
        */
        void playSound(ResourceID, float x, float y, float pitch = 1.f);
        /*!
        \brief Sets the master volume of the sound player.
        Range is 0.f - 1.f
        */
        void setMasterVolume(float);
        /*!
        \brief Sets the volume of a given channel.
        \param sf::Uint8 channel to set the volume for
        \param float Value to set the volume to in range 0 - 1.
        The volume is a percentage of the master volume.
        There are a maximum of 16 different channels.
        */
        void setChannelVolume(sf::Uint8 channel, float);

        /*!
        \brief Mutes or unmutes a given channel
        \param channel Channel to mute or unmute
        \param bool true to mute else false to unmute
        */
        void setChannelMuted(sf::Uint8 channel, bool mute);

    private:
        xy::SoundResource& m_soundResource;
        struct Buffer final
        {
            Buffer() = default;
            Buffer(sf::SoundBuffer& b) : buffer(b) {}
            sf::SoundBuffer buffer;
            sf::Uint8 channel = 0;
        };
        std::map<ResourceID, Buffer> m_buffers;
        std::list<sf::Sound> m_sounds;
        float m_volume;

        std::array<std::pair<float, float>, 16u> m_channelVolumes;
    };
}

#endif //XY_SOUNDPLAYER_HPP_