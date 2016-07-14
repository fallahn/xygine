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

#ifndef XY_QUAD_BUILDER_HPP_
#define XY_QUAD_BUILDER_HPP_

#include <xygine/mesh/ModelBuilder.hpp>
#include <xygine/mesh/BoundingBox.hpp>

#include <array>

namespace xy
{
    /*!
    \brief Creates a simple quad mesh with normals and UV coordinates
    */
    class QuadBuilder final : public xy::ModelBuilder
    {
    public:
        explicit QuadBuilder(const sf::Vector2f& size)
            : m_size(size)
        {
            m_elements = 
            {
                {VertexLayout::Element::Type::Position, 3},
                {VertexLayout::Element::Type::Normal, 3},
                {VertexLayout::Element::Type::Tangent, 3},
                {VertexLayout::Element::Type::Bitangent, 3},
                {VertexLayout::Element::Type::UV0, 2}
            };
        }
        ~QuadBuilder() = default;

        void build() override
        {
            const float halfSizeX = m_size.x / 2.f;
            const float halfSizeY = m_size.y / 2.f;
            m_vertexData = 
            {
                -halfSizeX, halfSizeY, 0.f,  0.f, 0.f, 1.f,  1.f, 0.f, 0.f,  0.f, 1.f, 0.f,  0.f, 1.f,
                -halfSizeX, -halfSizeY, 0.f,  0.f, 0.f, 1.f,  1.f, 0.f, 0.f,  0.f, 1.f, 0.f,  0.f, 0.f,
                halfSizeX, halfSizeY, 0.f,  0.f, 0.f, 1.f,  1.f, 0.f, 0.f,  0.f, 1.f, 0.f,  1.f, 1.f,
                halfSizeX, -halfSizeY, 0.f,  0.f, 0.f, 1.f,  1.f, 0.f, 0.f,  0.f, 1.f, 0.f,  1.f, 0.f
            };

            m_boundingBox = { {-halfSizeX, -halfSizeY, -0.5f}, {halfSizeX, halfSizeY, 0.5f} };
        }
        VertexLayout getVertexLayout() const override { return VertexLayout(m_elements); }
        const float* getVertexData() const override { return m_vertexData.data(); }
        std::size_t getVertexCount() const override { return 4; }
        const BoundingBox& getBoundingBox() const override { return m_boundingBox; }

    private:
        sf::Vector2f m_size;
        std::array<float, 56> m_vertexData;
        std::vector<VertexLayout::Element> m_elements;
        BoundingBox m_boundingBox;
    };
}

#endif //XY_QUAD_BUILDER_HPP_