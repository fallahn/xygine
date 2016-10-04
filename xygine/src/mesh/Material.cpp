/*********************************************************************
Matt Marchant 2014 - 2016
http://trederia.blogspot.com

xygine - Zlib license.

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

#include <xygine/mesh/Material.hpp>
#include <xygine/mesh/UniformBuffer.hpp>
#include <xygine/Assert.hpp>

#include <SFML/Graphics/Shader.hpp>

using namespace xy;

Material::Material(sf::Shader& shader)
    : m_activePass (nullptr)
{
    //add default pass
    m_passes.insert(std::make_pair(RenderPass::Default, RenderPass(shader)));
    m_activePass = &m_passes.find(RenderPass::Default)->second;
}

//public
void Material::addRenderPass(RenderPass::ID id, sf::Shader& shader)
{
    if (m_passes.count(id) == 0)
    {        
        m_passes.insert(std::make_pair(id, RenderPass(shader)));
        auto pass = m_passes.find(id);

        //update with existing uniforms/properties
        const auto& properties = m_activePass->getProperties();
        for (const auto& p : properties)
        {
            pass->second.addProperty(p);
        }
        for (auto u : m_uniformBuffers)
        {
            pass->second.addUniformBuffer(*u);
        }
    }
    else
    {
        Logger::log("Failed adding pass with ID: " + std::to_string(id) + " pass already exists", Logger::Type::Error);
    }
}

RenderPass* Material::getRenderPass(RenderPass::ID id)
{
    auto result = m_passes.find(id);
    if (result != m_passes.end())
    {
        return &result->second;
    }
    return nullptr;
}

void Material::removeRenderPass(RenderPass::ID id)
{
    //m_passes.erase(id);
    //NOT that simple. This won't update the VAOs bound in the material
}

bool Material::setActivePass(RenderPass::ID id) const
{
    auto result = m_passes.find(id);
    if (result != m_passes.end())
    {
        //godverdomme. sort this out.
        m_activePass = const_cast<RenderPass*>(&result->second);
        return true;
    }
    //LOG("Pass not found in Material", xy::Logger::Type::Warning);
    return false;
}

void Material::bind() const
{
    m_activePass->bind();
}

void Material::addProperty(const MaterialProperty& prop)
{
    //add to all passes
    for (auto& p : m_passes)
    {
        p.second.addProperty(prop);
    }
}

MaterialProperty* Material::getProperty(const std::string& name)
{
    return m_activePass->getProperty(name);
}

void Material::addUniformBuffer(const UniformBuffer& ubo)
{
    for (auto& p : m_passes)
    {
        p.second.addUniformBuffer(ubo);
    }
    //storee this to apply to any new passes added later
    m_uniformBuffers.push_back(&ubo);
    //return false;
}

VertexAttribID Material::getVertexAttributeID(const std::string& str) const
{
    return m_activePass->getVertexAttributeID(str);
}

sf::Shader& Material::getShader() const
{
    return m_activePass->getShader();
}