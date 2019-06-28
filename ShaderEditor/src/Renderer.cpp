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
#include "imgui/imgui-SFML.h"
#include "GLCheck.hpp"
#include "tinyfiledialogs.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/OpenGL.hpp>

namespace
{
    const float ComboWidth = 80.f;
    const float InputWidth = 180.f;
    const float SliderMax = 100.f;
    const GLsizei MaxUniformChars = 36;
    const float MaxPreviewSize = 318.f;

    //TODO look up all file types supported by sfml
    const char* filters[] = { "*.png", "*.jpg", "*.jpeg", "*.tga" };

    auto comboCallback = [](void* vec, int idx, const char** out_text)
    {
        auto& vector = *static_cast<std::vector<std::string>*>(vec);
        if (idx < 0 || idx >= static_cast<int>(vector.size()))
        {
            return false;
        }
        *out_text = vector.at(idx).c_str();
        return true;
    };
}

Renderer::Renderer()
    : m_shaderIndex (1),
    m_firstTexture  (nullptr),
    m_previewZoom   (1.f)
{
    m_vertices[1] = { sf::Vector2f(MaxPreviewSize, 0.f) };
    m_vertices[2] = { sf::Vector2f(MaxPreviewSize, MaxPreviewSize) };
    m_vertices[3] = { sf::Vector2f(0.f, MaxPreviewSize) };
    setOrigin(MaxPreviewSize / 2.f, MaxPreviewSize / 2.f);
    setPosition(getOrigin());
    setScale(1.f, -1.f);

    m_previewTexture.create(static_cast<std::uint32_t>(MaxPreviewSize), static_cast<std::uint32_t>(MaxPreviewSize));
}

void Renderer::update(std::bitset<WindowFlags::Count>& windowFlags)
{
    ImGui::SetNextWindowPos({ 594.f, 24.f }, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({ 408.f, 358.f }, ImGuiCond_FirstUseEver);

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
        else if (windowFlags.test(TextChanged))
        {
            ImGui::PushStyleColor(ImGuiCol_Button, 0xff00f7ff);
            ImGui::PushStyleColor(ImGuiCol_Text, 0xff000000);
            ImGui::Button("Not Compiled");
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

    ImGui::BeginTabBar("##0");


    drawUniformTab(windowFlags);
    drawTextureTab();
    drawOptionsTab();

    ImGui::EndTabBar();
    ImGui::End();

    ImGui::SetNextWindowPos({ 594.f, 392.f }, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({ 408.f, 354.f }, ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Preview", nullptr))
    {
        ImGui::End();
        return;
    }
    ImGui::Image(m_previewTexture.getTexture());
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
        flags.set(WindowFlags::TextChanged, false);

        readUniforms();

        return;
    }
    flags.set(WindowFlags::CompileSuccess, false);
    flags.set(WindowFlags::TextChanged, false);
    return;
}

void Renderer::draw()
{
    sf::RenderStates states;
    states.shader = m_shaders[m_shaderIndex].get();
    states.transform *= getTransform();
    states.texture = m_firstTexture;

    m_previewTexture.clear(sf::Color::Transparent);
    m_previewTexture.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
    m_previewTexture.display();

}

//private
void Renderer::updateVertices()
{
    sf::Vector2f size(m_firstTexture->getSize());
    auto coords = size;

    float ratio = 1.f;
    bool width = true;
    if (size.x > size.y)
    {
        ratio = size.y / size.x;
    }
    else
    {
        ratio = size.x / size.y;
        width = false;
    }

    if (width && size.x > MaxPreviewSize)
    {
        size.x = MaxPreviewSize;
        size.y = MaxPreviewSize * ratio;
    }
    else if(!width && size.y > MaxPreviewSize)
    {
        size.y = MaxPreviewSize;
        size.x = MaxPreviewSize * ratio;
    }

    m_vertices[1].texCoords.x = coords.x;
    m_vertices[1].position.x = size.x;
    m_vertices[2].texCoords = coords;
    m_vertices[2].position = size;
    m_vertices[3].texCoords.y = coords.y;
    m_vertices[3].position.y = size.y;

    setOrigin(size / 2.f);

    //reset zoom in case we're loading a larger texture
    float maxZoom = MaxPreviewSize / m_vertices[1].position.x;
    float zoom = std::min(maxZoom, m_previewZoom);
    setScale(zoom, -zoom);
}

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
            if (m_uniforms[name].GLType != type)
            {
                switch (type)
                {
                default: break;
                case GL_FLOAT:
                    m_uniforms[name] = {};
                    m_uniforms[name].data = std::make_any<float>();
                    break;
                case GL_FLOAT_VEC2:
                    m_uniforms[name] = {};
                    m_uniforms[name].data = std::make_any<sf::Glsl::Vec2>();
                    break;
                case GL_FLOAT_VEC3:
                    m_uniforms[name] = {};
                    m_uniforms[name].data = std::make_any<sf::Glsl::Vec3>();
                    break;
                case GL_FLOAT_VEC4:
                    m_uniforms[name] = {};
                    m_uniforms[name].data = std::make_any<sf::Glsl::Vec4>(1.f, 1.f, 1.f, 1.f);
                    break;
                case GL_SAMPLER_2D:
                    m_uniforms[name] = {};
                    m_uniforms[name].data = std::make_any<sf::Texture*>(nullptr);
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
                m_uniforms.insert(std::make_pair(name, Uniform(type, 0.f)));
                break;
            case GL_FLOAT_VEC2:
                m_uniforms.insert(std::make_pair(name, Uniform(type, sf::Glsl::Vec2())));
                break;
            case GL_FLOAT_VEC3:
                m_uniforms.insert(std::make_pair(name, Uniform(type, sf::Glsl::Vec3())));
                break;
            case GL_FLOAT_VEC4:
                m_uniforms.insert(std::make_pair(name, Uniform(type, sf::Glsl::Vec4())));
                break;
            case GL_SAMPLER_2D:
            {
                sf::Texture* ptr = nullptr;
                m_uniforms.insert(std::make_pair(name, Uniform(type, ptr)));
            }
                break;
            }
        }
    }
}

void Renderer::drawUniformTab(std::bitset<WindowFlags::Count>& windowFlags)
{
    if (ImGui::BeginTabItem("Uniforms"))
    {
        //create the texture list first so we only do it once for all items
        std::vector<std::string> textureItems;
        for (auto i = 0u; i < m_textures.size(); ++i)
        {
            textureItems.push_back("Texture " + std::to_string(i));
        }

        std::int32_t uIdx = 0; //used to give unique ID to each uniform control

        //list uniforms
        for (auto& [name, uniform] : m_uniforms)
        {
            switch (uniform.GLType)
            {
            case GL_FLOAT:
            {
                const char* items[] = { "Slider", "Value", "Time" };

                std::string label = "##float" + std::to_string(uIdx++);

                ImGui::PushItemWidth(ComboWidth);
                ImGui::Combo(label.c_str(), &uniform.UISelection, items, IM_ARRAYSIZE(items));
                ImGui::SameLine();
                ImGui::PopItemWidth();

                ImGui::PushItemWidth(InputWidth);
                if (uniform.UISelection == 2)
                {
                    ImGui::TextUnformatted(name.c_str());
                    m_shaders[m_shaderIndex]->setUniform(name, m_shaderClock.getElapsedTime().asSeconds());
                }
                else
                {
                    auto& val = std::any_cast<float&>(uniform.data);
                    (uniform.UISelection == 1) ?
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

                std::string label = "##vec2" + std::to_string(uIdx++);

                ImGui::PushItemWidth(ComboWidth);
                ImGui::Combo(label.c_str(), &uniform.UISelection, items, IM_ARRAYSIZE(items));
                ImGui::SameLine();
                ImGui::PopItemWidth();

                ImGui::PushItemWidth(InputWidth);
                auto& val = std::any_cast<sf::Glsl::Vec2&>(uniform.data);
                float arr[] = { val.x, val.y };
                (uniform.UISelection == 1) ?
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

                std::string label = "##vec3" + std::to_string(uIdx++);

                ImGui::PushItemWidth(ComboWidth);
                ImGui::Combo(label.c_str(), &uniform.UISelection, items, IM_ARRAYSIZE(items));
                ImGui::SameLine();
                ImGui::PopItemWidth();

                auto& val = std::any_cast<sf::Glsl::Vec3&>(uniform.data);
                float arr[] = { val.x, val.y, val.z };

                if (uniform.UISelection != 2)
                {
                    ImGui::PushItemWidth(InputWidth);

                    (uniform.UISelection == 1) ?
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

                std::string label = "##vec4" + std::to_string(uIdx++);

                static int selected = 0;
                ImGui::PushItemWidth(ComboWidth);
                ImGui::Combo(label.c_str(), &selected, items, IM_ARRAYSIZE(items));
                ImGui::SameLine();
                ImGui::PopItemWidth();

                auto& val = std::any_cast<sf::Glsl::Vec4&>(uniform.data);
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
            {
                if (m_textures.empty())
                {
                    ImGui::TextUnformatted(name.c_str());
                    ImGui::SameLine();
                    ImGui::Text("No Textures Loaded");
                }
                else
                {
                    ImGui::PushItemWidth(100.f);
                    ImGui::Combo(name.c_str(), &uniform.UISelection, comboCallback, static_cast<void*>(&textureItems), textureItems.size());
                    ImGui::PopItemWidth();

                    //woo, pointy.
                    auto t = std::any_cast<sf::Texture*&>(uniform.data);
                    t = m_textures[uniform.UISelection].get();

                    m_shaders[m_shaderIndex]->setUniform(name, *t);
                }
            }
                break;
            default:
                ImGui::TextUnformatted(name.c_str());
                ImGui::SameLine();
                ImGui::Text(" Uniform type not supported (yet)");
                break;
            }
        }
        ImGui::EndTabItem();
    }
}

void Renderer::drawTextureTab()
{
    if (ImGui::BeginTabItem("Textures"))
    {
        if (ImGui::Button("Add Texture"))
        {
            const char* result = tinyfd_openFileDialog("Open...", "", 4, filters, "Image Files", 0);
            if (result)
            {
                std::unique_ptr<sf::Texture> tex = std::make_unique<sf::Texture>();
                if (tex->loadFromFile(result))
                {
                    if (m_textures.empty())
                    {
                        m_firstTexture = tex.get();

                        updateVertices();
                    }
                    m_textures.push_back(std::move(tex));
                }
                else
                {
                    //idk, there should be some sort of error in the log anyway
                }
            }
        }

        int i = 0;
        for (const auto& t : m_textures)
        {
            std::string label = "Texture " + std::to_string(i);
            std::string buttonID = "Change##" + std::to_string(i);
            std::string repeatLabel = "Wrap UV##" + std::to_string(i);

            ImGui::Image(*t, { 64.f, 64.f });
            ImGui::SameLine();
            ImGui::TextUnformatted(label.c_str());
            ImGui::SameLine();
            if (ImGui::Button(buttonID.c_str()))
            {
                const char* result = tinyfd_openFileDialog("Open...", "", 4, filters, "Image Files", 0);
                if (result)
                {
                    /*
                    OK We can either load a new texture and swap if the load succeeds - but this won't update
                    active uniforms immediately. On the other hand we can reload the existing texture, but load
                    failures aren't then handled...
                    */

                    /*std::unique_ptr<sf::Texture> tex = std::make_unique<sf::Texture>();
                    if (tex->loadFromFile(result))
                    {
                        m_textures[i].swap(tex);
                        if (i == 0)
                        {
                            m_firstTexture = tex.get();
                        }
                    }*/

                    m_textures[i]->loadFromFile(result);
                    if (i == 0)
                    {
                        updateVertices();
                    }
                }
            }

            bool repeat = t->isRepeated();
            bool oldRepeat = repeat;
            ImGui::SameLine();
            ImGui::Checkbox(repeatLabel.c_str(), &repeat);
            if (repeat != oldRepeat)
            {
                t->setRepeated(repeat);
            }

            i++;
        }

        ImGui::EndTabItem();
    }
}

void Renderer::drawOptionsTab()
{
    if (ImGui::BeginTabItem("View Options"))
    {
        ImGui::SliderFloat("Zoom", &m_previewZoom, 1.f, 50.f);

        float maxZoom = MaxPreviewSize / m_vertices[1].position.x;
        float zoom = std::min(maxZoom, m_previewZoom);
        setScale(zoom, -zoom);

        ImGui::EndTabItem();
    }
}
