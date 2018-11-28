/*********************************************************************
(c) Matt Marchant 2017 - 2018
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
    : m_textureCount(0),
    m_floatCount    (0),
    m_vec2Count     (0),
    m_vec3Count     (0),
    m_boolCount     (0),
    m_matCount      (0),
    m_cull          (true),
    m_croppingArea  (-std::numeric_limits<float>::max() / 2.f, -std::numeric_limits<float>::max() / 2.f,
                    std::numeric_limits<float>::max(), std::numeric_limits<float>::max()),
    m_cropped       (false)
{

}

Drawable::Drawable(const sf::Texture& texture)
    : m_textureCount(0),
    m_floatCount    (0),
    m_vec2Count     (0),
    m_vec3Count     (0),
    m_boolCount     (0),
    m_matCount      (0),
    m_cull          (true),
    m_croppingArea  (-std::numeric_limits<float>::max() / 2.f, -std::numeric_limits<float>::max() / 2.f,
                    std::numeric_limits<float>::max(), std::numeric_limits<float>::max()),
    m_cropped       (false)
{
    m_states.texture = &texture;
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
    if (m_textureCount < MaxBindings)
    {
        auto result = std::find_if(m_textureBindings.begin(), m_textureBindings.end(),
            [&name](const std::pair<std::string, const sf::Texture*>& pair)
        {
            return pair.first == name;
        });
        if (result == m_textureBindings.end())
        {
            m_textureBindings[m_textureCount] = std::make_pair(name, &texture);
            m_textureCount++;
        }
        else
        {
            result->second = &texture;
        }
    }
    else
    {
        Logger::log(name + ": uniform not bound, MaxBindings reached", xy::Logger::Type::Info);
    }
}

void Drawable::bindUniform(const std::string& name, float value)
{
    if (m_floatCount < MaxBindings)
    {
        auto result = std::find_if(m_floatBindings.begin(), m_floatBindings.end(),
            [&name](const std::pair<std::string, float>& pair)
        {
            return pair.first == name;
        });
        if (result == m_floatBindings.end())
        {
            m_floatBindings[m_floatCount] = std::make_pair(name, value);
            m_floatCount++;
        }
        else
        {
            result->second = value;
        }
    }
    else
    {
        Logger::log(name + ": uniform not bound, MaxBindings reached", xy::Logger::Type::Info);
    }
}

void Drawable::bindUniform(const std::string& name, sf::Vector2f value)
{
    if (m_vec2Count < MaxBindings)
    {
        auto result = std::find_if(m_vec2Bindings.begin(), m_vec2Bindings.end(),
            [&name](const std::pair<std::string, sf::Vector2f>& pair)
        {
            return pair.first == name;
        });
        if (result == m_vec2Bindings.end())
        {
            m_vec2Bindings[m_vec2Count] = std::make_pair(name, value);
            m_vec2Count++;
        }
        else
        {
            result->second = value;
        }
    }
    else
    {
        Logger::log(name + ": uniform not bound, MaxBindings reached", xy::Logger::Type::Info);
    }
}

void Drawable::bindUniform(const std::string& name, sf::Vector3f value)
{
    if (m_vec3Count < MaxBindings)
    {
        auto result = std::find_if(m_vec3Bindings.begin(), m_vec3Bindings.end(),
            [&name](const std::pair<std::string, sf::Vector3f>& pair)
        {
            return pair.first == name;
        });
        if (result == m_vec3Bindings.end())
        {
            m_vec3Bindings[m_vec3Count] = std::make_pair(name, value);
            m_vec3Count++;
        }
        else
        {
            result->second = value;
        }
    }
    else
    {
        Logger::log(name + ": uniform not bound, MaxBindings reached", xy::Logger::Type::Info);
    }
}

void Drawable::bindUniform(const std::string& name, bool value)
{
    if (m_boolCount < MaxBindings)
    {
        auto result = std::find_if(m_boolBindings.begin(), m_boolBindings.end(),
            [&name](const std::pair<std::string, bool>& pair)
        {
            return pair.first == name;
        });
        if (result == m_boolBindings.end())
        {
            m_boolBindings[m_boolCount] = std::make_pair(name, value);
            m_boolCount++;
        }
        else
        {
            result->second = value;
        }
    }
    else
    {
        Logger::log(name + ": uniform not bound, MaxBindings reached", xy::Logger::Type::Info);
    }
}

void Drawable::bindUniform(const std::string& name, const float* matrix)
{
    if (m_matCount < MaxBindings)
    {
        auto result = std::find_if(m_matBindings.begin(), m_matBindings.end(),
            [&name](const std::pair<std::string, const float*>& pair)
        {
            return pair.first == name;
        });
        if (result == m_matBindings.end())
        {
            m_matBindings[m_matCount] = std::make_pair(name, matrix);
            m_matCount++;
        }
        else
        {
            result->second = matrix;
        }
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
    m_localBounds.left = std::numeric_limits<float>::max();
    m_localBounds.top = std::numeric_limits<float>::max();
    m_localBounds.width = 0.f;
    m_localBounds.height = 0.f;

    for (auto& v : m_vertices)
    {
        if (v.position.x < m_localBounds.left)
        {
            m_localBounds.left = v.position.x;
        }
        else if (v.position.x - m_localBounds.left > m_localBounds.width)
        {
            m_localBounds.width = v.position.x - m_localBounds.left;
        }

        if (v.position.y < m_localBounds.top)
        {
            m_localBounds.top = v.position.y;
        }
        else if (v.position.y - m_localBounds.top > m_localBounds.height)
        {
            m_localBounds.height = v.position.y - m_localBounds.top;
        }
    }
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
    for (auto i = 0u; i < m_textureCount; ++i)
    {
        const auto& [name, value] = m_textureBindings[i];
        shader->setUniform(name, *value);
    }
    for (auto i = 0u; i < m_floatCount; ++i)
    {
        const auto&[name, value] = m_floatBindings[i];
        shader->setUniform(name, value);
    }
    for (auto i = 0u; i < m_vec2Count; ++i)
    {
        const auto&[name, value] = m_vec2Bindings[i];
        shader->setUniform(name, value);
    }
    for (auto i = 0u; i < m_vec3Count; ++i)
    {
        const auto&[name, value] = m_vec3Bindings[i];
        shader->setUniform(name, value);
    }
    for (auto i = 0u; i < m_boolCount; ++i)
    {
        const auto&[name, value] = m_boolBindings[i];
        shader->setUniform(name, value);
    }
    for (auto i = 0u; i < m_matCount; ++i)
    {
        const auto&[name, value] = m_matBindings[i];
        shader->setUniform(name, sf::Glsl::Mat4(value));
    }
}

//private
void Drawable::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_vertices.data(), m_vertices.size(), m_primitiveType, states);
}