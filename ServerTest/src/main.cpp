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

#include <Server.hpp>

#include <SFML/Config.hpp>
#include <SFML/System/Thread.hpp>
#include <SFML/System/Sleep.hpp>

#include <iostream>
#include <string>

#ifdef __linux
#include <X11/Xlib.h>
#endif // __linux

void threadFunc(const bool* running)
{
    GameServer server;
    server.start();

    while (*running) {}

    server.stop();
}

int main()
{
#ifdef __linux
    XInitThreads();
#endif //__linux
    
    bool running = true;

    sf::Thread thread(&threadFunc, &running);
    thread.launch();
    sf::sleep(sf::milliseconds(100));
    
    std::cout << "Dedicated server is running!" << std::endl;
    std::cout << "Type Q ";
#ifdef __linux
        std::cout << " or ctrl ^ C ";
#endif
        std::cout << "to quit." << std::endl;
    std::cout << std::endl;

    while (running)
    {
        char buns;
        std::cin >> buns;
        if (buns == 'q' || buns == 'Q')
        {
            running = false;
        }
    }
    thread.wait();

    return 0;
}