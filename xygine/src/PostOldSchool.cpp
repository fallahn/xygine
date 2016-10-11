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

#include <xygine/PostOldSchool.hpp>
#include <xygine/shaders/PostOldSchool.hpp>

using namespace xy;

namespace
{
    const float divisor = 2.f;
}

PostOldSchool::PostOldSchool()
{
    if (!m_shader.loadFromMemory(xy::Shader::PostOldSchool::fragment, sf::Shader::Fragment))
    {
        xy::Logger::log("Failed creating shader for Old School Post Process", xy::Logger::Type::Error, xy::Logger::Output::All);
    }

    sf::Vector2u size(xy::DefaultSceneSize / divisor);
    m_buffer.create(size.x, size.y);

    m_quad[1] = { sf::Vector2f(xy::DefaultSceneSize.x, 0.f), sf::Vector2f(xy::DefaultSceneSize.x / divisor, 0.f) };
    m_quad[2] = { xy::DefaultSceneSize, xy::DefaultSceneSize / divisor };
    m_quad[3] = { sf::Vector2f(0.f, xy::DefaultSceneSize.y), sf::Vector2f(0.f, xy::DefaultSceneSize.y / divisor) };
}

void PostOldSchool::apply(const sf::RenderTexture& src, sf::RenderTarget& dst)
{
    m_shader.setUniform("u_sourceTexture", src.getTexture());
    applyShader(m_shader, dst);
    //m_buffer.display();

    //sf::RenderStates states;
    //states.texture = &m_buffer.getTexture();
    //dst.draw(m_quad.data(), m_quad.size(), sf::Quads, states);
}