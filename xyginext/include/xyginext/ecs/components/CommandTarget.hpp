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

#include "xyginext/Config.hpp"

#include <SFML/Config.hpp>

namespace xy
{
    /*!
    \brief Attaches a command ID bitmask to an Entity.
    ID should be a bit mask of flags representing target IDs.
    For example:

    enum CommandIDs
    {
        Player = 0x1, Enemy = 0x2, NPC = 0x4
    };

    CommandTarget target; target.ID |= Enemy | NPC;

    The command target system will then apply any given commands to targets
    whose flags match one or more of the flags belonging to the command.
    \see CommandSystem
    */
    struct XY_EXPORT_API CommandTarget final
    {
        sf::Uint32 ID = 0;
    };
}
