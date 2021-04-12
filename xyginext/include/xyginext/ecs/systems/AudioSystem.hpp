/*********************************************************************
(c) Matt Marchant 2017 - 2021
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

#pragma once

#include "xyginext/ecs/System.hpp"

#include <SFML/Audio/SoundBuffer.hpp>

namespace xy
{
	/*!
	\brief Audio System.
	AudioSystem instance is required in the scene to update positional audio, as well
	as apply the values stored in the AudioMixer. Positional audio is applied to both
    entities with an AudioEmitter component, as well as optionally updating the
    Listener property of the Scene to which this system is added.
	*/
    class XY_API AudioSystem final : public System
    {
    public:
        /*!
        \brief Constructor
        \param MessageBus A reference to the active message bus
        \param updateListener In cases where muliple AudioSystems are
        active, eg in both a Scene which renders a game, and another
        Scene which renders the UI, it may not be desirable for the
        AudioSystem instance in the UI Scene to overwrite the Listener
        properties, in which case this should be set false. Defaults to true.
        */
        explicit AudioSystem(MessageBus& messageBus, bool updateListener = true);

        void handleMessage(const xy::Message&) override;

        void process(float) override;

        /*!
        \brief Sets the volume of AudioEmitters assigned to this system
        When using multiple scenes either in the same state of multiple
        active states it is sometimes desirable to set the volume of a 
        group of emitters within a specific system. The accepted volume
        is in the range, automatically clamped, of 0 - 1. This is
        multiplied with emitter volume so that AudioEmitter settings are
        left unaffected.
        */
        void setVolume(float volume);

        /*!
        \brief Returns the current volume of the system
        \see setVolume
        */
        float getVolume() const { return m_volume; }

    private:

        //there's a bug in the SFML AlResource counter which
        //sometimes causes the audio device to be unnecessarily
        //destroyed. Creating a single resource here attempts to
        //work around this by preventing the count reaching zero
        //all the time the AudioSystem exists.
        sf::SoundBuffer m_dummyBuffer;

        float m_volume;
        bool m_updateListener;

        void onEntityRemoved(xy::Entity) override;
    };
}
