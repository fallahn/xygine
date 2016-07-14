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

#include <xygine/mesh/RenderQuad.hpp>
#include <xygine/detail/GLCheck.hpp>

#include <SFML/Graphics/Shader.hpp>

#include <array>

using namespace xy;

namespace
{
    const std::vector<xy::VertexLayout::Element> elements = { {xy::VertexLayout::Element::Type::Position, 3}, {xy::VertexLayout::Element::Type::UV0, 2} };
    const xy::VertexLayout layout(elements);
}

RenderQuad::RenderQuad(const sf::Vector2f& size, sf::Shader& shader)
    : m_mesh    (layout, 4),
    m_material  (shader),
    m_vao       (m_mesh, m_material)
{
    std::array<float, 20> vertData = 
    {
        0.f, size.y, 0.f, 0.f, 1.f,
        0.f, 0.f, 0.f, 0.f, 0.f,
        size.x, size.y, 0.f, 1.f, 1.f,
        size.x, 0.f, 0.f, 1.f, 0.f
    };
    m_mesh.setVertexData(vertData.data());
    m_mesh.setPrimitiveType(xy::Mesh::PrimitiveType::TriangleStrip);
}

//public
sf::Shader& RenderQuad::getShader() const { return m_material.getShader(); }

void RenderQuad::addRenderPass(RenderPass::ID id, sf::Shader& shader)
{
    m_material.addRenderPass(id, shader);
}

void RenderQuad::setActivePass(RenderPass::ID id)
{
    m_material.setActivePass(id);
}

//private
void RenderQuad::draw(sf::RenderTarget&, sf::RenderStates) const
{
    m_material.bind();
    m_vao.bind();
    glCheck(glDrawArrays(static_cast<GLenum>(m_mesh.getPrimitiveType()), 0, m_mesh.getVertexCount()));
    m_vao.unbind();
}