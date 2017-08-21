/*********************************************************************
(c) Matt Marchant 2017
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

//this should always be included first on windows, to ensure it is
//included before windows.h (in this case by Log.hpp)
#include <enet/enet.h> 

#include "Config.hpp"
#include <xyginext/network/NetHost.hpp>
#include <xyginext/core/Log.hpp>



using namespace xy;

NetHost::NetHost()
    : m_host    (nullptr)
{
    if (!NetConf::instance)
    {
        NetConf::instance = std::make_unique<NetConf>();
    }
}

NetHost::~NetHost()
{
    if (m_host)
    {
        enet_host_destroy(m_host);
    }
}

//public
bool NetHost::start(const std::string& address, sf::Uint16 port, std::size_t maxClients, std::size_t maxChannels, sf::Uint32 incoming, sf::Uint32 outgoing)
{
    if (!NetConf::instance->m_initOK)
    {
        Logger::log("Network subsystem not initialised, creating host failed", Logger::Type::Error);
        return false;
    }

    //TODO ASSERT parameters

    ENetAddress add;
    if (address.empty())
    {
        add.host = ENET_HOST_ANY;
    }
    else
    {
        enet_address_set_host(&add, address.c_str());
    }
    add.port = port;

    m_host = enet_host_create(&add, maxClients, maxChannels, incoming, outgoing);
    if (!m_host)
    {
        Logger::log("There was an error creating the host", Logger::Type::Error);
        return false;
    }

    LOG("Created host on port " + std::to_string(port), Logger::Type::Info);
    return true;
}

void NetHost::stop()
{
    //TODO forcefully remove clients

    if (m_host)
    {
        enet_host_destroy(m_host);
    }
}