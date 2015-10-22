/*********************************************************************
Matt Marchant 2014 - 2015
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

#include <xygine/PostProcess.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/VertexArray.hpp>

using namespace xy;

PostProcess::PostProcess(){}

//protected
void PostProcess::applyShader(const sf::Shader& shader, sf::RenderTarget& dest)
{
    auto outputSize = static_cast<sf::Vector2f>(dest.getSize());

    sf::VertexArray verts(sf::TrianglesStrip, 4);
    verts[0] = { {}, { 0.f, 1.f } };
    verts[1] = { { outputSize.x, 0.f }, { 1.f, 1.f } };
    verts[2] = { { 0.f, outputSize.y }, sf::Vector2f() };
    verts[3] = { outputSize, { 1.f, 0 } };

    //All the seagulls are belong to us.
    sf::RenderStates states;
    states.shader = &shader;
    states.blendMode = sf::BlendNone;

    dest.draw(verts, states);
}
