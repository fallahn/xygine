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

#include "NetConf.hpp"

#include <enet/enet.h>
#include "xyginext/core/Log.hpp"

using namespace xy;

std::unique_ptr<NetConf> NetConf::instance;

NetConf::NetConf()
    : m_initOK(false)
{
    if (enet_initialize() == 0)
    {
        m_initOK = true;
        Logger::log("Network subsystem initialised");
    }
    else
    {
        Logger::log("Failed initialising network subsystem", Logger::Type::Error);
    }
}

NetConf::~NetConf()
{
    if (m_initOK)
    {
        enet_deinitialize();
        m_initOK = false;
        //Logger::log("Network subsystem deinitialised");
    }
}
