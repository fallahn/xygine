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
    \brief Performs a given callback function on an entity.
    This is useful for behaviour code on entities which aren't used often.
    For example when a single entity of a type exists and is used rarely
    it can mean a lot of overhead writing a specific system and component
    for behaviour which is not often employed. In these cases it's easier
    to attach a Callback component and supply a function with entity specific
    behaviour. Overuse of this system is not recommended, however, as there
    is a certain amount of overhead in calling the behavioural functions,
    so this system should be reserved for specific cases where it makes sense
    or for rapid prototying of ideas that may or may not be expanded to a full
    system.
    */
    class XY_EXPORT_API CallbackSystem final : public System
    {
    public:
        explicit CallbackSystem(MessageBus&);

        void process(float) override;
    };
}
