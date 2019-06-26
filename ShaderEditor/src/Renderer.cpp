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
#include "GLCheck.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/OpenGL.hpp>

namespace
{
    const float ComboWidth = 80.f;
    const float InputWidth = 180.f;
    const float SliderMax = 100.f;
    const GLsizei MaxUniformChars = 36;
}

Renderer::Renderer()
    : m_shaderIndex(1)
{
    m_vertices[1] = { sf::Vector2f(256.f, 0.f) };
    m_vertices[2] = { sf::Vector2f(256.f, 256.f) };
    m_vertices[3] = { sf::Vector2f(0.f, 256.f) };
}

void Renderer::update(std::bitset<WindowFlags::Count>& windowFlags)
{
    ImGui::SetNextWindowPos({ 594.f, 24.f });
    ImGui::SetNextWindowSize({ 408.f, 378.f });

    if (!ImGui::Begin("Options", nullptr, ImGuiWindowFlags_HorizontalScrollbar))
    {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Compile"))
    {
        windowFlags.set(RunShader);
    }
    ImGui::SameLine();
    if (m_shaders[m_shaderIndex])
    {
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
    }
    else
    {
        ImGui::Button("No Shader Created");
    }

    //list uniforms
    for (auto& [name, data] : m_uniforms)
    {
        //ImGui::Text(name.c_str());
        //ImGui::SameLine();
        switch (data.first)
        {
        case GL_FLOAT:
        {
            const char* items[] = { "Slider", "Value", "Time" };
            static int selected = 0;
            ImGui::PushItemWidth(ComboWidth);
            ImGui::Combo("##Input0", &selected, items, IM_ARRAYSIZE(items));
            ImGui::SameLine();
            ImGui::PopItemWidth();

            ImGui::PushItemWidth(InputWidth);
            if (selected == 2)
            {
                ImGui::Text(name.c_str());
                m_shaders[m_shaderIndex]->setUniform(name, m_shaderClock.getElapsedTime().asSeconds());
            }
            else
            {
                auto& val = std::any_cast<float&>(data.second);
                (selected == 1) ?
                    ImGui::InputFloat(name.c_str(), &val) :
                    ImGui::SliderFloat(name.c_str(), &val, 0.f, SliderMax);

                m_shaders[m_shaderIndex]->setUniform(name, val);
            }
            ImGui::PopItemWidth();
        }
            break;
        case GL_FLOAT_VEC2:
        {
            const char* items[] = { "Slider", "Value" };
            static int selected = 0;
            ImGui::PushItemWidth(ComboWidth);
            ImGui::Combo("##Input1", &selected, items, IM_ARRAYSIZE(items));
            ImGui::SameLine();
            ImGui::PopItemWidth();

            ImGui::PushItemWidth(InputWidth);
            auto& val = std::any_cast<sf::Glsl::Vec2&>(data.second);
            float arr[] = { val.x, val.y };
            (selected == 1) ?
                ImGui::InputFloat2(name.c_str(), arr) :
                ImGui::SliderFloat2(name.c_str(), arr, 0.f, SliderMax);

            val = { arr[0],arr[1] };
            m_shaders[m_shaderIndex]->setUniform(name, val);
            ImGui::PopItemWidth();
        }
            break;
        case GL_FLOAT_VEC3:
        {
            const char* items[] = { "Slider", "Value", "Picker" };
            static int selected = 0;
            ImGui::PushItemWidth(ComboWidth);
            ImGui::Combo("##Input2", &selected, items, IM_ARRAYSIZE(items));
            ImGui::SameLine();
            ImGui::PopItemWidth();

            auto& val = std::any_cast<sf::Glsl::Vec3&>(data.second);
            float arr[] = { val.x, val.y, val.z };

            if (selected != 2)
            {
                ImGui::PushItemWidth(InputWidth);

                (selected == 1) ?
                    ImGui::InputFloat3(name.c_str(), arr) :
                    ImGui::SliderFloat3(name.c_str(), arr, 0.f, SliderMax);

                ImGui::PopItemWidth();
            }
            else
            {
                ImGui::ColorEdit3(name.c_str(), arr);
            }
            val = { arr[0],arr[1],arr[2] };
            m_shaders[m_shaderIndex]->setUniform(name, val);
        }
            break;
        case GL_FLOAT_VEC4:
        {
            const char* items[] = { "Slider", "Value", "Picker" };
            static int selected = 0;
            ImGui::PushItemWidth(ComboWidth);
            ImGui::Combo("##Input3", &selected, items, IM_ARRAYSIZE(items));
            ImGui::SameLine();
            ImGui::PopItemWidth();

            auto& val = std::any_cast<sf::Glsl::Vec4&>(data.second);
            float arr[] = { val.x, val.y, val.z, val.w };

            if (selected != 2)
            {
                ImGui::PushItemWidth(InputWidth);

                (selected == 1) ?
                    ImGui::InputFloat4(name.c_str(), arr) :
                    ImGui::SliderFloat4(name.c_str(), arr, 0.f, SliderMax);

                ImGui::PopItemWidth();
            }
            else
            {
                ImGui::ColorEdit4(name.c_str(), arr);
            }
            val = { arr[0],arr[1],arr[2],arr[3] };
            m_shaders[m_shaderIndex]->setUniform(name, val);
        }
            break;
        case GL_SAMPLER_2D:

            break;
        default:
            ImGui::Text(name.c_str());
            ImGui::SameLine();
            ImGui::Text(" Uniform type not supported (yet)");
            break;
        }
    }

    ImGui::End();
}

//public
void Renderer::compileShader(const std::string& source, std::bitset<WindowFlags::Count>& flags)
{
    //TODO we want to be able to cover vertex shaders eventually too...
    //perhaps have two editor tabs?

    if (source.empty() || source.size() == 1) //might contain a single newline char
    {
        flags.set(WindowFlags::CompileSuccess, false);
        return;
    }

    auto nextShader = (m_shaderIndex + 1) % m_shaders.size();
    m_shaders[nextShader] = std::make_unique<sf::Shader>();
    if (m_shaders[nextShader]->loadFromMemory(source, sf::Shader::Fragment))
    {
        m_shaderIndex = nextShader;
        flags.set(WindowFlags::CompileSuccess, true);

        readUniforms();

        return;
    }
    flags.set(WindowFlags::CompileSuccess, false);
    return;
}

//private
using namespace Detail;
void Renderer::readUniforms()
{
    sf::Shader::bind(m_shaders[m_shaderIndex].get());
    auto programID = m_shaders[m_shaderIndex]->getNativeHandle();

    GLint uniformCount = 0;
    glCheck(glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &uniformCount));

    std::unordered_map<std::string, std::int32_t> newUniforms;
    for (auto i = 0; i < uniformCount; ++i)
    {
        char buff[MaxUniformChars];
        GLsizei written = 0;
        GLint size = 0;
        GLenum type = 0;
        glCheck(glGetActiveUniform(programID, i, MaxUniformChars, &written, &size, &type, buff));

        newUniforms.insert(std::make_pair(buff, type));
    }
    sf::Shader::bind(nullptr);

    //remove any old uniforms
    //why doesn't this work?
    /*m_uniforms.erase(std::remove_if(m_uniforms.begin(), m_uniforms.end(),
        [&newUniforms](const std::pair<std::string, std::pair<std::int32_t, std::any>>& uniform)
        {
            return newUniforms.count(uniform.first) == 0;
        }), m_uniforms.end());*/
    std::vector<std::string> found;
    for (const auto& [name, data] : m_uniforms) 
    {
        if (newUniforms.count(name) == 0)
        {
            found.push_back(name);
        }
    }
    for (const auto& name : found)
    {
        m_uniforms.erase(name);
    }

    //update or add new uniforms
    for (auto& [name, type] : newUniforms)
    {
        if (m_uniforms.count(name) != 0)
        {
            if (m_uniforms[name].first != type)
            {
                switch (type)
                {
                default: break;
                case GL_FLOAT:
                    m_uniforms[name] = std::make_pair(type, std::make_any<float>());
                    break;
                case GL_FLOAT_VEC2:
                    m_uniforms[name] = std::make_pair(type, std::make_any<sf::Glsl::Vec2>());
                    break;
                case GL_FLOAT_VEC3:
                    m_uniforms[name] = std::make_pair(type, std::make_any<sf::Glsl::Vec3>());
                    break;
                case GL_FLOAT_VEC4:
                    m_uniforms[name] = std::make_pair(type, std::make_any<sf::Glsl::Vec4>());
                    //this is a mini hack to initialise alpha to 1 which could otherwise
                    //cause confusing when using these values as a colour
                    std::any_cast<sf::Glsl::Vec4&>(m_uniforms[name].second).w = 1.f;
                    break;
                case GL_SAMPLER_2D:
                    m_uniforms[name] = std::make_pair(type, std::make_any<sf::Texture*>());
                    break;
                }
            }
        }
        else
        {
            switch (type)
            {
            default: break;
            case GL_FLOAT:
                m_uniforms.insert(std::make_pair(name, std::make_pair(type, std::make_any<float>())));
                break;
            case GL_FLOAT_VEC2:
                m_uniforms.insert(std::make_pair(name, std::make_pair(type, std::make_any<sf::Glsl::Vec2>())));
                break;
            case GL_FLOAT_VEC3:
                m_uniforms.insert(std::make_pair(name, std::make_pair(type, std::make_any<sf::Glsl::Vec3>())));
                break;
            case GL_FLOAT_VEC4:
                m_uniforms.insert(std::make_pair(name, std::make_pair(type, std::make_any<sf::Glsl::Vec4>())));
                break;
            case GL_SAMPLER_2D:
                m_uniforms.insert(std::make_pair(name, std::make_pair(type, std::make_any<sf::Texture*>())));
                break;
            }
        }
    }
}

void Renderer::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.shader = m_shaders[m_shaderIndex].get();
    states.transform *= getTransform();

    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
}