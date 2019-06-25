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

#include "Renderer.hpp"
#include "imgui/imgui.h"

#include <SFML/Graphics/RenderTarget.hpp>

Renderer::Renderer()
    : m_shaderIndex(1)
{
    m_vertices[1] = { sf::Vector2f(256.f, 0.f) };
    m_vertices[2] = { sf::Vector2f(256.f, 256.f) };
    m_vertices[3] = { sf::Vector2f(0.f, 256.f) };
}

void Renderer::update(std::bitset<WindowFlags::Count>& windowFlags)
{
    if (!ImGui::Begin("Options"))
    {
        ImGui::End();
        return;
    }

    //TODO set up uniforms

    if (ImGui::Button("Compile"))
    {
        windowFlags.set(RunShader);
    }
    ImGui::SameLine();
    if (windowFlags.test(CompileSuccess))
    {
        //colours are ABGR
        ImGui::PushStyleColor(ImGuiCol_Button, 0xff00ff00);
        ImGui::PushStyleColor(ImGuiCol_Text, 0xff000000);
        ImGui::Button("Succeeded");
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Button, 0xff0000ff);
        ImGui::PushStyleColor(ImGuiCol_Text, 0xffffffff);
        ImGui::Button("Failed");
        ImGui::Text("See console for errors.");
    }
    ImGui::PopStyleColor(2);

    ImGui::End();
}

//public
void Renderer::compileShader(const std::string& source, std::bitset<WindowFlags::Count>& flags)
{
    //TODO we want to be able to cover vertex shaders eventually too...
    //perhaps have two editor tabs?

    if (source.empty())
    {
        flags.set(WindowFlags::CompileSuccess, false);
        return;
    }

    auto nextShader = (m_shaderIndex + 1) % m_shaders.size();
    m_shaders[nextShader] = std::make_unique<sf::Shader>();
    if (m_shaders[nextShader]->loadFromMemory(source, sf::Shader::Fragment))
    {
        //TODO lex/tokenise source to find uniforms

        m_shaderIndex = nextShader;
        flags.set(WindowFlags::CompileSuccess, true);
        return;
    }
    flags.set(WindowFlags::CompileSuccess, false);
    return;
}

//private
void Renderer::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.shader = m_shaders[m_shaderIndex].get();
    states.transform *= getTransform();

    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
}