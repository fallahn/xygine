/*********************************************************************
(c) Matt Marchant 2017 - 2020
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
	While AudioEmitter components will still be audible without and active AudioSystem,
	and AudioSystem instance is required in the scene to update positional audio, as well
	as apply the values stored in the AudioMixer. It is, therefore, recommended that any
	scene which employs AudioEmitter components also has an AudioSystem.
	*/
    class XY_EXPORT_API AudioSystem final : public System
    {
    public:
        explicit AudioSystem(MessageBus&);

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

        void onEntityRemoved(xy::Entity) override;
    };
}
