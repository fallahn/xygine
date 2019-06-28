/*********************************************************************
(c) Matt Marchant 2019
http://trederia.blogspot.com

xygineXT Shader Editor - Zlib license.

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

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/System/Clock.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

#include "EditorWindow.hpp"
#include "WindowFunctions.hpp"
#include "WindowFlags.hpp"
#include "Renderer.hpp"
#include "glad.h"

#include <bitset>
#include <iostream>
#include <fstream>
#include <cstring>

int main(int argc, char** argsv)
{
    sf::Vector2u windowSize(1024, 768);
    const std::string settingsName("window.set");
    std::ifstream file(settingsName, std::ios::binary);
    if (file.is_open() && file.good())
    {
        file.seekg(0, file.end);
        auto fileSize = file.tellg();
        file.seekg(file.beg);

        if (fileSize == sizeof(windowSize))
        {
            std::vector<char> buff(sizeof(windowSize));
            file.read(buff.data(), buff.size());
            std::memcpy(&windowSize, buff.data(), buff.size());
        }
        file.close();
    }

    sf::RenderWindow window;
    window.create({ windowSize.x, windowSize.y }, "Shader Editor");
    window.setVerticalSyncEnabled(true);

    if (!gladLoadGL())
    {
        std::cerr << "Failed loading OpenGL functions\n";
        window.close();
        return -1;
    }

    ImGui::SFML::Init(window);
    sf::Clock frameClock;

    std::bitset<WindowFlags::Count> windowFlags;
    EditorWindow textEditor;
    Renderer renderer;

    while (window.isOpen())
    {
        sf::Event evt;
        while (window.pollEvent(evt))
        {
            ImGui::SFML::ProcessEvent(evt);
            if (evt.type == sf::Event::Closed)
            {
                window.close();
            }
            else if (evt.type == sf::Event::Resized)
            {
                sf::View view;
                view.setSize(sf::Vector2f(window.getSize()));
                view.setCenter(view.getSize() / 2.f);
                window.setView(view);
            }

            else if (evt.type == sf::Event::KeyReleased)
            {
#ifdef XY_DEBUG
                if(evt.key.code == sf::Keyboard::Escape)
                {
                    window.close();
                }
#endif //XY_DEBUG
                switch (evt.key.code)
                {
                default: break;
                case sf::Keyboard::F7:
                    windowFlags.set(WindowFlags::RunShader);
                    break;
                }
            }

        }

        ImGui::SFML::Update(window, frameClock.restart());
        textEditor.update(windowFlags);
        renderer.update(windowFlags);

        if (windowFlags.test(ShowDemo))
        {
            ImGui::ShowDemoWindow();
        }
        if (windowFlags.test(RunShader))
        {
            //update the renderer
            renderer.compileShader(textEditor.getString(), windowFlags);
            windowFlags.set(RunShader, false);
        }

        renderer.draw();

        window.clear();
        ImGui::SFML::Render(window);
        window.display();
    }

    //stash the window size
    windowSize = window.getSize();
    std::ofstream oFile(settingsName, std::ios::binary);
    if (oFile.is_open() && oFile.good())
    {
        std::vector<char> buff(sizeof(windowSize));
        std::memcpy(buff.data(), &windowSize, buff.size());

        oFile.write(buff.data(), buff.size());
        oFile.close();
    }

    return 0;
}