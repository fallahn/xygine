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

//allow drawing arbitrary sets of polygons in a single batch

#ifndef XY_POLYBATCH_HPP_
#define XY_POLYBATCH_HPP_

#include <xygine/Config.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>

#include <vector>

namespace xy
{
    class Polygon;
    /*!
    \brief Allows drawing a set of polygons in a single draw call

    Polygons with user defined points can be added to a single batch
    so that they are drawn in a single call. Poly batches may also use
    a single texture or texture atlas which is used by all Polygons
    belonging to the same PolyBatch. PolyBatches must live at least as
    long as all the Polygons belonging to it, else modifying a Polygon
    will lead to undefined behaviour.
    */
    class XY_EXPORT_API PolyBatch final : public sf::Drawable
    {
        friend class Polygon;
    public:
        /*!
        \brief Constructor

        \param count Maximum number of vertices to allocate to this batch.
        Trying to add more Polygons than there are vertices available will
        cause an assertion error
        \param sf::PrimitiveType The type of primitives to be drawn by this
        batch, sf::Quads by default. Note that other types such as line or
        triangle strips may require degenerative triangles to be created by
        the user when adding a new Polygon to the batch.
        */
        explicit PolyBatch(std::size_t count, sf::PrimitiveType = sf::Quads);
        ~PolyBatch() = default;
        PolyBatch(const PolyBatch&) = delete;
        PolyBatch& operator = (const PolyBatch&) = delete;

        /*!
        \brief Add a new polygon to this batch

        Adding a new polygon requests a new set of vertices to be assigned
        from this batch to a Polygon. If there are not enough vertices left
        an assertion error is thrown, in which case the batch should be
        created with a larger initial number of vertices.

        \param vertexCount Number of vertices to assign to the Polygon. Note
        that when using PrimitiveTypes other than sf::Quad you may need to
        explicitly request extra vertices to create degenerate triangles to
        prevent artifacting between existing Polygons.
        \returns New Polygon shape belonging to this batch.
        */
        Polygon addPolygon(std::size_t vertexCount);

        /*!
        \brief Sets the primitive type that this PolyBatch will use

        \param type New primitive type to use
        */
        void setPrimitiveType(sf::PrimitiveType type) { m_primitiveType = type; }

        /*!
        \brief Set the texture to be used by this PolyBatch

        All Polygons belonging to the batch will be drawn using this texture.
        Texture atlases are useful here because the texture coordinates of
        Polygons can be individually set so that they can use subregions of the
        same texture. This reduces the amount of texture switching required to
        draw many different shapes.

        \param texture Pointer to the texture to use. This can be nullptr to
        tell the PolyBatch to use no texture. As this is only a reference the
        original texture must remain alive for at least as long as the PolyBatch
        to prevent running into the white-square problem.
        */
        void setTexture(const sf::Texture* texture) { m_texture = texture; }

    private:
        sf::PrimitiveType m_primitiveType;
        const sf::Texture* m_texture;
        std::vector<sf::Vertex> m_vertices;
        int m_nextIndex;

        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };

    /*!
    \brief A Polygon created from an arbitrary number of vertices, belonging to a PolyBatch

    Note Polygons are movable, but not copyable. When Polygons are destroyed their vertices
    are not returned to the batch, but will no longer appear visible.
    */
    class XY_EXPORT_API Polygon final
    {
    public:
        /*!
        \brief Constructor

        \param PolyBatch The PolyBatch to which this polygon will belong
        \param count number of vertices to request from the PolyBatch
        */
        Polygon(PolyBatch&, std::size_t count);
        ~Polygon();
        Polygon(const Polygon&) = delete;
        Polygon& operator = (const Polygon&) = delete;
        Polygon(Polygon&&) = default;
        Polygon& operator = (Polygon&&) = default;

        /*!
        \brief Returns the number of vertices in this Polygon
        */
        std::size_t getVertexCount() const;

        /*!
        \brief Set a vertex position.
        Allows setting the position of the vertex at the given
        index in the internal vertex array.
        \param sf::Vector2f position to set the vertex to
        \param std::size_t index of the vertex whose position is to be set
        */
        void setVertexPosition(const sf::Vector2f&, std::size_t);

        /*!
        \brief set the positions of the vertices belonging to this Polygon
        \param std::vector<sf::Vector2f> A vector containing the desired
        positions of the vertices. Must be less than or equal to the vertex
        count in size, and always starts at the first vertex.
        */
        void setVertexPositions(const std::vector<sf::Vector2f>&);

        /*!
        \brief Sets all vertices to the given colour
        */
        void setVertexColour(const sf::Color&);
        /*!
        \brief Sets the vertex at the given index to the given colour
        \param sf::Color The colour to set
        \param std::size_t The index of the vertex whose colour is to be set
        */
        void setVertexColour(const sf::Color&, std::size_t);
        /*!
        \brief Set the vertex colours.

        Providing a vector of sf::Color will set the colour of each
        vertex at the corresponding index to that of the given vector.
        The vector must be smaller or equal in size to that of the vertex
        count. Colour assignment always starts at the first vertex.
        */
        void setVertexColours(const std::vector<sf::Color>&);

        /*!
        \brief Sets the texture coordinate of the vertex at the given index
        */
        void setTextureCoord(const sf::Vector2f&, std::size_t);
        /*!
        \brief Sets the texture coordinates.

        Using the given vector of coordinates each vertex and the corresponding
        index will have its texture coordinate set. The vector must be smaller
        or equal in size to the number of vertices.
        \param std::vector<sf::Vector2f> vector of texture coordinates
        */
        void setTextureCoords(const std::vector<sf::Vector2f>&);
        /*!
        \brief Sets the vertex data to that of the given vector of vertices

        The vector must be smaller or equal to the vertex count in size.
        Vertex data is copied so the given vector may be modified and used
        on multiple Polygons without affecting this one.
        */
        void setVertices(const std::vector<sf::Vertex>&);

    private:
        PolyBatch& m_batch;
        std::size_t m_vertexOffset;
        std::size_t m_vertexCount;
    };
}

#endif //XY_POLYBATCH_HPP_