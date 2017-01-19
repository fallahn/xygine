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

//quad tree class for spatial partitioning

#ifndef XY_QUADTREE_HPP_
#define XY_QUADTREE_HPP_

#include <xygine/QuadTreeNode.hpp>

#include <SFML/Graphics/Drawable.hpp>

#include <vector>

namespace xy
{
    /*!
    \brief Quad Tree spatial partitioning class

    The quad tree is used to partition the world for faster collision
    querying and culling of renderables. xygines QuadTree instance is
    a member of the Scene class, and interaction should be done via
    the scene interface
    \see Scene
    */
    class XY_EXPORT_API QuadTree final : public sf::Drawable
    {
    public:
        QuadTree();
        QuadTree(const QuadTree&) = delete;
        QuadTree& operator = (const QuadTree&) = delete;
        ~QuadTree() = default;

        void create(const sf::FloatRect&);
        bool created() const;
        void reset();
        void add(QuadTreeComponent*);
        const sf::FloatRect& getRootArea() const;

        //std::vector<QuadTreeComponent*> queryPosition(const sf::Vector2f&);
        std::vector<QuadTreeComponent*> queryArea(const sf::FloatRect&) const;

        QuadTreeNode::Set& getOutsideRootSet();

        sf::Int32 minNodeComponents() const;
        sf::Uint32 maxNodeComponents() const;
        sf::Int32 maxLevels() const;

        std::size_t getComponentCount() const { return m_rootNode->getComponentCount(); }
    private:
        //store components which move outside of the root area here
        QuadTreeNode::Set m_outsideRootSet;
        QuadTreeNode::Ptr m_rootNode;

        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };
}
#endif //XY_QUADTREE_HPP_