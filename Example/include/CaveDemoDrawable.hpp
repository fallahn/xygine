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

//pseudo random cave generation

#ifndef CAVE_DRAWABLE_HPP_
#define CAVE_DRAWABLE_HPP_

#include <xygine/components/Component.hpp>
#include <xygine/ShaderProperty.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <vector>
#include <array>

namespace CaveDemo
{
    class CaveDrawable final : public xy::Component, public sf::Drawable, public xy::ShaderProperty
    {
    public:
        explicit CaveDrawable(xy::MessageBus&);
        ~CaveDrawable() = default;

        xy::Component::Type type()const override { return xy::Component::Type::Drawable; }
        void entityUpdate(xy::Entity&, float) override;

        sf::Vector2f getSize() const;

        sf::FloatRect globalBounds() const override;

        const std::vector<std::vector<sf::Vector2f>>& getEdges() const
        {
            return m_edges;
        }

        void setTexture(sf::Texture& t)
        {
            t.setRepeated(true);
            m_texture = &t;
        }

        void setNormalMap(sf::Texture& t)
        {
            t.setRepeated(true);
            m_normalMap = &t;
        }

        void setMaskMap(sf::Texture& t)
        {
            t.setRepeated(true);
            m_maskMap = &t;
        }

    private:
        //structs used in marching square algorithm
        struct Node
        {
            sf::Vector2f position;
            sf::Int32 idx = -1;
            Node() = default;
            Node(const sf::Vector2f& pos)
                : position(pos) {}
        };

        struct ControlNode final : public Node
        {
            bool active = false;
            Node aboveNode;
            Node rightNode;

            ControlNode(bool a, const sf::Vector2f& pos, float size)
                : Node      (pos),
                active      (a),
                aboveNode   (pos),
                rightNode   (pos)
            {
                aboveNode.position.y += (size / 2.f);
                rightNode.position.x += (size / 2.f);
            }
        };

        struct Square final
        {
            ControlNode& topLeft;
            ControlNode& topRight;
            ControlNode& bottomRight;
            ControlNode& bottomLeft;

            Node& centreTop;
            Node& centreRight;
            Node& centreBottom;
            Node& centreLeft;

            sf::Uint16 mask = 0u;

            Square(ControlNode& tl, ControlNode& tr, ControlNode& br, ControlNode& bl)
                : topLeft(tl), topRight(tr), bottomRight(br), bottomLeft(bl),
                centreTop(tl.rightNode), centreRight(br.aboveNode), centreBottom(bl.rightNode), centreLeft(bl.aboveNode)
            {
                if (tl.active) mask |= 0x8;
                if (tr.active) mask |= 0x4;
                if (br.active) mask |= 0x2;
                if (bl.active) mask |= 0x1;
            }
        };

        struct Triangle final
        {
            Triangle(std::size_t a, std::size_t b, std::size_t c)
            {
                m_indices[0] = a, m_indices[1] = b; m_indices[2] = c;
            }
            std::size_t& operator[](std::size_t i)
            {
                return m_indices[i];
            }
            std::size_t operator[](std::size_t i) const
            {
                return m_indices[i];
            }
            bool contains(std::size_t idx) const
            {
                return (m_indices[0] == idx || m_indices[1] == idx || m_indices[2] == idx);
            }
        private:
            std::array<std::size_t, 3u> m_indices;
        };

        std::vector<sf::Vertex> m_vertices;
        std::vector<sf::Uint8> m_tileData;
        std::vector<std::vector<sf::Vector2f>> m_edges;

        sf::FloatRect m_globalBounds;

        sf::Texture* m_texture;
        sf::Texture* m_normalMap;
        sf::Texture* m_maskMap;

        void fillRand();
        void smooth();
        sf::Uint8 getNeighbourCount(std::size_t);
        void addQuad(std::size_t);

        void buildVertexArray();

        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };
}

#endif //CAVE_DRAWABLE_HPP_