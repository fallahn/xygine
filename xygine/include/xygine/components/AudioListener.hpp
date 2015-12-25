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

//sets the listener's position in the scene, so that all audio sources
//are automatically panned relative to this position. usually this
//component would be attached to the same entity as the active camera
//but may also be set to a neutral position in scenarios such as split
//screen multi player. there can only be a single instance of the
//listener and creating a new instance will remove any existing instances

#ifndef XY_AUDIO_LISTENER_HPP_
#define XY_AUDIO_LISTENER_HPP_

#include <xygine/components/Component.hpp>

namespace xy
{
    class AudioListener final : public Component
    {
    public:
        explicit AudioListener(MessageBus&);
        ~AudioListener();

        Component::Type type() const { return Component::Type::Script; }

        void entityUpdate(Entity&, float) override;
        void handleMessage(const Message&) override;

    private:

    };
}

#endif //XY_AUDIO_LISTENER_HPP_