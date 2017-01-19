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

//node within the quad tree containing quad tree components

#ifndef XY_QUADTREE_NODE_HPP_
#define XY_QUADTREE_NODE_HPP_

#include <xygine/components/QuadTreeComponent.hpp>

#include <SFML/Graphics/Vertex.hpp>

#include <array>
#include <memory>
#include <unordered_set>

namespace xy
{
    class QuadTree;
    /*!
    \brief A node in the QuadTree

    Each node represents a single partition, containing up to
    4 further partitions, and the positions of entities which
    have a QuadTreeComponent attached to them. Interaction 
    with the QuadTree in xygine should normally be done via
    the Scene class
    \see Scene
    */
    class XY_EXPORT_API QuadTreeNode final
    {
    public:
        using Ptr = std::unique_ptr<QuadTreeNode>;
        using Set = std::unordered_set<QuadTreeComponent*>;

        QuadTreeNode();
        QuadTreeNode(const sf::FloatRect& area, sf::Int32 level, QuadTreeNode* parent, QuadTree* quadTree);
        QuadTreeNode(const QuadTreeNode&) = delete;
        QuadTreeNode& operator = (const QuadTreeNode&) = delete;
        ~QuadTreeNode() = default;

        void create(const sf::FloatRect& area, sf::Int32 level, QuadTreeNode* parent, QuadTree* quadTree);
        QuadTree* getTree() const;
        void add(QuadTreeComponent*);
        const sf::FloatRect& getArea() const;

        sf::Int32 getNumComponentsBelow() const;

        void update(QuadTreeComponent*);
        void remove(QuadTreeComponent*);

        const Set& getComponents() const;
        bool hasChildren() const;
        const std::array<Ptr, 4u>& getChildren() const;

        //used to draw debug output
        void getVertices(std::vector<sf::Vertex>&);

        std::size_t getComponentCount() const;

    private:
        QuadTreeNode* m_parent;
        QuadTree* m_quadTree;

        bool m_hasChildren;
        std::array<Ptr, 4u> m_children;
        Set m_components;

        sf::FloatRect m_area;
        sf::Int32 m_level;
        sf::Int32 m_numComponentsBelow;

        void getSubComponents();
        sf::Vector2i getPossiblePosition(QuadTreeComponent*);
        void addToThis(QuadTreeComponent*);
        bool addToChildren(QuadTreeComponent*); //true if successful
        void destroyChildren();

        void split();
        void join();
        void clearDestroyed();
    };
}
#endif //XY_QUADTREE_NODE_HPP_