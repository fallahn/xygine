/*********************************************************************
Matt Marchant 2014 - 2017
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

#ifndef XY_SPHERE_BUILDER_HPP_
#define XY_SPHERE_BUILDER_HPP_

#include <xygine/mesh/ModelBuilder.hpp>
#include <xygine/mesh/BoundingBox.hpp>

namespace xy
{
    /*!
    \brief Implements the ModelBuilder interface for creating spherical meshes.
    Spherical meshes are created using cubic mapping coordinates, mapping 6 faces
    in a single texture, aligned in a 2x3 layout: 
    X+, X-,
    Y+, Y-,
    Z+, Z-
    */
    class XY_EXPORT_API SphereBuilder final : public ModelBuilder
    {
    public:
        /*!
        \brief Constructor.
        \param radius Size in world units for the radius of the sphere
        \param resolution Number of quads per side. Defaults to 4 which
        is 4*4 per side, 6 sides = 96 quads for 192 triangles. Higher
        numbers result in a smoother mesh but increased triangle count.
        \param backface If this is true the 6th, rear-facing face is built.
        If a sphere is not likely to be rotated in a scene it can be optimised
        by not generating faces which will not be seen.
        */
        SphereBuilder(float radius, std::size_t resolution = 4u, bool backface = false);
        ~SphereBuilder() = default;

        void build() override;
        VertexLayout getVertexLayout() const override;
        const float* getVertexData() const override { return m_vertexData.data(); }
        std::size_t getVertexCount() const override { return m_vertexCount; }
        const BoundingBox& getBoundingBox() const override { return m_boundingBox; }

    private:

        float m_radius;
        std::size_t m_resolution;
        bool m_generateBackface;
        BoundingBox m_boundingBox;
        std::vector<float> m_vertexData;
        std::size_t m_vertexCount;
        std::vector<VertexLayout::Element> m_elements;
        std::vector<std::uint16_t> m_indices;
    };
}

#endif //XY_SPHERE_BUILDER_HPP_