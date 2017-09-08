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

#ifndef DEMO_SERVER_MESSAGES_HPP_
#define DEMO_SERVER_MESSAGES_HPP_

#include <array>
#include <string>

/*
As a local server runs in its own thread, this can cause problems
when trying to print to the console running on the client thread.
Because of this the server will emit packets with message idents
which the client can use to look up a message which needs to be printed.
*/

namespace MessageIdent
{
    enum
    {
        StopServer,

        Count
    };
}

static const std::array<std::string, MessageIdent::Count> serverMessages = 
{
    "Stopping server"
};

#endif //DEMO_SERVER_MESSAGES_HPP_