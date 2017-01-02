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

#ifndef XY_VERTEX_LAYOUT_HPP_
#define XY_VERTEX_LAYOUT_HPP_

#include <xygine/Config.hpp>

#include <SFML/Config.hpp>

#include <vector>

namespace xy
{
    /*!
    \brief Describes the particular properties of a vertex
    in a vertex array, and the order in which they are laid out
    */
    class XY_EXPORT_API VertexLayout final
    {
    public:
        /*!
        \brief Describes an element in a vertex
        */
        struct XY_EXPORT_API Element final
        {
            /*!
            \brief Element type.
            Vertex properties are made up of elements which describe that
            property by type and by size.
            */
            enum class Type
            {
                NONE,
                Position = 1,
                Colour,
                Normal,
                Tangent,
                Bitangent,
                BlendIndices,
                BlendWeights,
                UV0,
                UV1
            }type;
            /*!
            \brief Number of float values this element occupies in a vertex.
            For example if a vertex a Position element may use 3 values (X, Y, Z),
            or a colour element might use 4 values(RGBA)
            */
            sf::Uint32 size;
            Element();
            Element(Type, std::size_t);

            bool operator == (const Element&) const;
            bool operator != (const Element&) const;
        };
        /*!
        \brief Constructor.
        \param Vector of elements describing the layout of a single vertex
        */
        explicit VertexLayout(const std::vector<Element>&);
        ~VertexLayout() = default;
        VertexLayout(const VertexLayout&) = default;
        VertexLayout(VertexLayout&&) noexcept = default;
        /*!
        \brief Returns a reference to the element at the given index
        */
        const Element& getElement(std::size_t) const;
        /*!
        \brief Returns the number of elements in the vertex layout
        */
        std::size_t getElementCount() const;
        /*!
        \brief Returns the total size *in bytes* of the vertex
        */
        std::size_t getVertexSize() const;
        /*!
        \brief Returns the index in the element array of the given
        element type or -1 if that element doesn't exist.
        For example this will return 1 when requesting the second
        element type in the vertex, regardless of the element size.
        */
        sf::Int32 getElementIndex(Element::Type) const;
        /*!
        \brief Returns the index in the vertex array of the given
        element type if it exists, else returns -1.
        For example requesting the third element will return the
        sum of the sizes of the first two elements.
        */
        sf::Int32 getElementOffset(Element::Type) const;

        bool operator == (const VertexLayout&) const;
        bool operator != (const VertexLayout&) const;

    private:
        std::vector<Element> m_elements;
        std::size_t m_vertexSize;
    };
}

#endif //XY_VERTEX_LAYOUT_HPP_