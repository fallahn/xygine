/*********************************************************************
(c) Matt Marchant 2017 - 2020
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

#include "xyginext/ecs/components/Drawable.hpp"
#include "xyginext/core/Log.hpp"
#include "xyginext/core/Assert.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Shader.hpp>

#include <limits>

using namespace xy;

Drawable::Drawable()
    : m_primitiveType   (sf::Quads),
    m_buffer            (m_primitiveType, sf::VertexBuffer::Dynamic),
    m_zDepth            (0),
    m_wantsSorting      (true),
    m_filterFlags       (DefaultFilterFlag),
    m_cull              (true),
    m_croppingArea      (std::numeric_limits<float>::lowest() / 2.f, std::numeric_limits<float>::lowest() / 2.f,
                        std::numeric_limits<float>::max(), std::numeric_limits<float>::max()),
    m_cropped           (false),
    m_glFlagIndex       (0),
    m_depthWriteEnabled (true)
{
    m_buffer.create(0);
}

Drawable::Drawable(const sf::Texture& texture)
    : m_primitiveType   (sf::Quads),
    m_buffer            (m_primitiveType, sf::VertexBuffer::Dynamic),
    m_zDepth            (0),
    m_wantsSorting      (true),    
    m_filterFlags       (DefaultFilterFlag),
    m_cull              (true),
    m_croppingArea      (std::numeric_limits<float>::lowest() / 2.f, std::numeric_limits<float>::lowest() / 2.f,
                        std::numeric_limits<float>::max(), std::numeric_limits<float>::max()),
    m_cropped           (false),
    m_glFlagIndex       (0),
    m_depthWriteEnabled (true)
{
    m_states.texture = &texture;
    m_buffer.create(0);
}

Drawable::Drawable(Drawable&& other) noexcept
{
    *this = std::move(other);
}

Drawable& Drawable::operator=(Drawable&& other)
{
    if (&other != this)
    {
        std::swap(m_primitiveType, other.m_primitiveType);
        std::swap(m_states, other.m_states);
        std::swap(m_vertices, other.m_vertices);
        m_buffer.swap(other.m_buffer);
        std::swap(m_zDepth, other.m_zDepth);
        std::swap(m_wantsSorting, other.m_wantsSorting);
        std::swap(m_filterFlags, other.m_filterFlags);
        std::swap(m_localBounds, other.m_localBounds);

std::swap(m_textureBindings, other.m_textureBindings);
std::swap(m_floatBindings, other.m_floatBindings);
std::swap(m_vec2Bindings, other.m_vec2Bindings);
std::swap(m_vec3Bindings, other.m_vec3Bindings);
std::swap(m_boolBindings, other.m_boolBindings);
std::swap(m_colourBindings, other.m_colourBindings);
std::swap(m_matBindings, other.m_matBindings);
std::swap(m_currentTexBindings, other.m_currentTexBindings);

std::swap(m_cull, other.m_cull);
std::swap(m_croppingArea, other.m_croppingArea);
std::swap(m_croppingWorldArea, other.m_croppingWorldArea);
std::swap(m_cropped, other.m_cropped);
std::swap(m_glFlags, other.m_glFlags);
std::swap(m_glFlagIndex, other.m_glFlagIndex);
    }

    return *this;
}

void Drawable::setTexture(const sf::Texture* texture)
{
    m_states.texture = texture;
}

void Drawable::setShader(sf::Shader* shader)
{
    m_states.shader = shader;
}

void Drawable::setDepth(sf::Int32 depth)
{
    if (m_zDepth != depth)
    {
        m_zDepth = depth;
        m_wantsSorting = true;
    }
}

void Drawable::bindUniform(const std::string& name, const sf::Texture& texture)
{
    bindUniform(name, &texture, m_textureBindings);
}

void Drawable::bindUniform(const std::string& name, float value)
{
    bindUniform(name, value, m_floatBindings);
}

void Drawable::bindUniform(const std::string& name, sf::Vector2f value)
{
    bindUniform(name, value, m_vec2Bindings);
}

void Drawable::bindUniform(const std::string& name, sf::Vector3f value)
{
    bindUniform(name, value, m_vec3Bindings);
}

void Drawable::bindUniform(const std::string& name, bool value)
{
    bindUniform(name, value, m_boolBindings);
}

void Drawable::bindUniform(const std::string& name, sf::Color value)
{
    bindUniform(name, sf::Glsl::Vec4(value), m_colourBindings);
}

void Drawable::bindUniform(const std::string& name, const float* matrix)
{
    bindUniform(name, matrix, m_matBindings);
}

void Drawable::bindUniformToCurrentTexture(const std::string& name)
{
    auto result = std::find(m_currentTexBindings.begin(), m_currentTexBindings.end(), name);
    if (result == m_currentTexBindings.end())
    {
        m_currentTexBindings.emplace_back(name);
    }
}

void Drawable::setBlendMode(sf::BlendMode mode)
{
    m_states.blendMode = mode;
}

void Drawable::setCroppingArea(sf::FloatRect area)
{
    m_croppingArea = area;
}

void Drawable::setUsage(sf::VertexBuffer::Usage usage)
{
    m_buffer.setUsage(usage);
}

sf::VertexBuffer::Usage Drawable::getUsage() const
{
    return m_buffer.getUsage();
}

sf::Texture* Drawable::getTexture()
{
    return const_cast<sf::Texture*>(m_states.texture);
}

sf::Shader* Drawable::getShader()
{
    return const_cast<sf::Shader*>(m_states.shader);
}

sf::FloatRect Drawable::getLocalBounds() const
{
    return m_localBounds;
}

void Drawable::updateLocalBounds()
{
    m_buffer.update(m_vertices.data(), m_vertices.size(), 0);

    if (m_vertices.empty())
    {
        return;
    }
    //m_vertices.clear();

    auto xExtremes = std::minmax_element(m_vertices.begin(), m_vertices.end(),
        [](const sf::Vertex& lhs, const sf::Vertex& rhs)
        {
            return lhs.position.x < rhs.position.x;
        });

    auto yExtremes = std::minmax_element(m_vertices.begin(), m_vertices.end(),
        [](const sf::Vertex& lhs, const sf::Vertex& rhs)
        {
            return lhs.position.y < rhs.position.y;
        });

    m_localBounds.left = xExtremes.first->position.x;
    m_localBounds.top = yExtremes.first->position.y;
    m_localBounds.width = xExtremes.second->position.x - m_localBounds.left;
    m_localBounds.height = yExtremes.second->position.y - m_localBounds.top;
}

void Drawable::updateLocalBounds(sf::FloatRect rect)
{
    m_localBounds = rect;
}

sf::RenderStates Drawable::getStates() const
{
    return m_states;
}

void Drawable::applyShader() const
{
    XY_ASSERT(m_states.shader, "No shader set!");

    sf::Shader* shader = const_cast<sf::Shader*>(m_states.shader);
    for (const auto& [name, value] : m_textureBindings)
    {
        shader->setUniform(name, *value);
    }
    for (const auto& [name, value] : m_floatBindings)
    {
        shader->setUniform(name, value);
    }
    for (const auto& [name, value] : m_vec2Bindings)
    {
        shader->setUniform(name, value);
    }
    for (const auto& [name, value] : m_vec3Bindings)
    {
        shader->setUniform(name, value);
    }
    for (const auto& [name, value] : m_boolBindings)
    {
        shader->setUniform(name, value);
    }
    for (const auto& [name, value] : m_colourBindings)
    {
        shader->setUniform(name, value);
    }
    for (const auto& [name, value] : m_matBindings)
    {
        shader->setUniform(name, sf::Glsl::Mat4(value));
    }

    for(const auto& str : m_currentTexBindings)
    {
        shader->setUniform(str, sf::Shader::CurrentTexture);
    }
}

void Drawable::addGlFlag(std::int32_t flag)
{
    if (m_glFlagIndex < m_glFlags.size())
    {
        m_glFlags[m_glFlagIndex++] = flag;
    }
    else
    {
        xy::Logger::log("Failed adding flag to drawable, max flags reached", xy::Logger::Type::Warning);
    }
}

//private
void Drawable::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    //rt.draw(m_vertices.data(), m_vertices.size(), m_primitiveType, states);
    rt.draw(m_buffer, states);
}