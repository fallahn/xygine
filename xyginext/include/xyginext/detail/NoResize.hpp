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

#include <xyginext/Config.hpp>

namespace xy::Detail
{
    /*!
    \brief Declares pooled resources which inherit this not have their component
    pools resized in cases where it will harmfully invalidate references.

    Classes inheriting this should be components in the ECS (else this base class
    will have no effect), and will have the maximum memory pool size of 1024
    components allocated to them immediately.
    */
    class XY_EXPORT_API NonResizeable
    {
    public: virtual ~NonResizeable() {};
    };
}