/*********************************************************************
(c) Matt Marchant 2017 - 2019
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

#include "xyginext/graphics/postprocess/PostProcess.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <vector>

using namespace xy;

namespace
{
    sf::Vector2f outputSize;
    sf::Vector2u lastSize;

    std::vector<sf::Vertex> vertexArray(4);
    void updateQuad()
    {
        vertexArray[0] = { sf::Vector2f(), sf::Vector2f(0.f, 1.f) };
        vertexArray[1] = { sf::Vector2f(outputSize.x, 0.f), sf::Vector2f(1.f, 1.f) };
        vertexArray[2] = { outputSize, sf::Vector2f(1.f, 0.f) };
        vertexArray[3] = { sf::Vector2f(0.f, outputSize.y), sf::Vector2f() };
    }
}

//protected
void PostProcess::applyShader(const sf::Shader& shader, sf::RenderTarget& dest)
{
    auto size = dest.getSize();
    if (lastSize != size)
    {
        outputSize = static_cast<sf::Vector2f>(size);
        lastSize = size;
        updateQuad();
    }

    //All the seagulls are belong to us.
    sf::RenderStates states;
    states.shader = &shader;
    states.blendMode = sf::BlendNone;

    dest.draw(vertexArray.data(), vertexArray.size(), sf::Quads, states);
}

void PostProcess::resizeBuffer(sf::Int32 w, sf::Int32 h)
{
    m_bufferSize = { w,h };
    bufferResized();
}
