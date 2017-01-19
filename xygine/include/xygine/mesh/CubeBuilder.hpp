/*********************************************************************
© Matt Marchant 2014 - 2017
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

#ifndef XY_CUBE_BUILDER_HPP_
#define XY_CUBE_BUILDER_HPP_

#include <xygine/mesh/ModelBuilder.hpp>
#include <xygine/mesh/BoundingBox.hpp>

#include <array>

namespace xy
{
    /*!
    \brief Implements the ModelBuilder interface for quickly creating
    cube meshes.
    \see ModelBuilder
    */
    class XY_EXPORT_API CubeBuilder final : public ModelBuilder
    {
    public:
        explicit CubeBuilder(float size);
        ~CubeBuilder() = default;

        void build() override;
        VertexLayout getVertexLayout() const override;
        const float* getVertexData() const override { return m_vertexData.data(); }
        std::size_t getVertexCount() const override { return 24; }
        const BoundingBox& getBoundingBox() const override { return m_boundingBox; }

    private:

        float m_size;
        BoundingBox m_boundingBox;
        std::array<std::uint8_t, 36> m_indices;

        std::vector<float> m_vertexData;
        std::vector<VertexLayout::Element> m_elements;
    };
}

#endif //XY_CUBE_BUILDER_HPP_