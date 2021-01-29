/*********************************************************************
(c) Matt Marchant 2017 - 2021
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

#ifndef XY_NETCONF_HPP_
#define XY_NETCONF_HPP_

#include <memory>

namespace xy
{
    /*!
    \brief Used to ensure proper startup and shutdown
    of Enet networking libraries
    */
    class NetConf final
    {
    public:
        NetConf();
        ~NetConf();

        NetConf(const NetConf&) = delete;
        NetConf(NetConf&&) = delete;
        NetConf& operator = (const NetConf&) = delete;
        NetConf& operator = (NetConf&&) = delete;

    private:
        friend class EnetClientImpl;
        friend class EnetHostImpl;

        static std::unique_ptr<NetConf> instance;

        bool m_initOK;
    };
}

#endif //XY_NETCONF_HPP_