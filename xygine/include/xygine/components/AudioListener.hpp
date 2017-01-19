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

//sets the listener's position in the scene, so that all audio sources
//are automatically panned relative to this position. usually this
//component would be attached to the same entity as the active camera
//but may also be set to a neutral position in scenarios such as split
//screen multi player. there can only be a single instance of the
//listener and creating a new instance will remove any existing instances

#ifndef XY_AUDIO_LISTENER_HPP_
#define XY_AUDIO_LISTENER_HPP_

#include <xygine/components/Component.hpp>

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include <list>

namespace xy
{
    /*!
    \brief Audio listener component

    The audio listener component defines where in the scene
    the player's virtual ears are. Audio sources played in the scene
    are automatically panned relative to the listener position. The
    listener position also affects the falloff in volume of audio
    sources the further away they are from the listener. Usually
    the audio listener would be attached to the same entity as the
    active scene camera, or possibly the entity to which the player
    is attached. There can only ever be one instance of a listener
    in any scene, creating a new listener component automatically
    destroys any existing ones.
    */
    class XY_EXPORT_API AudioListener final : public Component
    {
        friend class AudioSource;
    public:
        using Ptr = std::unique_ptr<AudioListener>;

        explicit AudioListener(MessageBus&);
        ~AudioListener();

        Component::Type type() const override { return Component::Type::Script; }

        void entityUpdate(Entity&, float) override;

        /*!
        \brief Returns the theoretical depth, in world units
        of the AudioListener component. This value is fixed.
        */
        static float getListenerDepth();

        /*!
        \brief Sets the master volume for all sounds heard.
        The volume range is from 0 - 100
        */
        static void setMasterVolume(float);

        /*!
        \brief Maximum allowed volume for any audio
        */
        static const float MaxVolume;
    private:

        std::list<sf::Sound> m_dyingSounds;
        static void addDyingSound(const sf::Sound&);

        sf::SoundBuffer dummyBuffer;//kludge to keep the AL device valid
    };
}

#endif //XY_AUDIO_LISTENER_HPP_