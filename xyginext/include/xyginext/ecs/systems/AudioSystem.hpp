/*********************************************************************
(c) Matt Marchant 2017 - 2019
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

    private:
        void onEntityRemoved(xy::Entity) override;
    };
}
