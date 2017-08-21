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

#ifndef XY_NETHOST_HPP_
#define XY_NETHOST_HPP_

#include <xyginext/Config.hpp>
#include <SFML/Config.hpp>

#include <string>

struct ENetHost;

namespace xy
{
    /*!
    \brief Creates a network host.
    Network hosts, or servers, can have multiple clients connected
    to them, via a reliable UDP stream.
    */
    class XY_EXPORT_API NetHost final
    {
    public:
        NetHost();
        ~NetHost();

        NetHost(const NetHost&) = delete;
        NetHost& operator = (const NetHost&) = delete;
        NetHost(NetHost&&) = delete;
        NetHost& operator = (NetHost&&) = delete;

        /*!
        \brief Starts a host listening on the given address and port
        \param address String representing an IPv4 address in the form "x.x.x.x".
        This may be left empty to listen on any available address.
        \param port An unsigned short representing the port on which to listen
        \param maxClient Maximum number of connections to allow to the host
        \param maxChannels Maximum number of channels allowed (indexed from 0)
        \param incoming Limit the incoming bandwidth in bytes per second. 0
        is no limit (default)
        \param outgoing Limit the outgoing bandwidth in bytes per second. 0
        is no limit (default)
        \returns true if created successfully, else false.
        */
        bool start(const std::string& address, sf::Uint16 port, std::size_t maxClient, std::size_t maxChannels, sf::Uint32 incoming = 0, sf::Uint32 outgoing = 0);

        /*!
        \brief Stops the host, if it is running
        */
        void stop();

    private:

        ENetHost* m_host;
    };
}

#endif //XY_NETHOST_HPP_