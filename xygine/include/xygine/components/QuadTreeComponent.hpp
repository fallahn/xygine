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

//represents an AABB for quad tree partitioning

#ifndef XY_QUADTREE_COMPONENT_HPP_
#define XY_QUADTREE_COMPONENT_HPP_

#include <xygine/components/Component.hpp>

#include <SFML/Graphics/Transform.hpp>
#include <SFML/System/Clock.hpp>

#include <list>

namespace xy
{
    class QuadTree;
    class QuadTreeNode;
    class QuadTreeComponent final : public Component
    {
    public:
        using Ptr = std::unique_ptr<QuadTreeComponent>;
    private:
        using FactoryFunc = Ptr(&)(MessageBus&, const sf::FloatRect&);
    public:
        static const FactoryFunc create;

        QuadTreeComponent(MessageBus&, const sf::FloatRect& bounds);
        ~QuadTreeComponent();

        Component::Type type() const override;
        void entityUpdate(Entity&, float) override;
        void handleMessage(const Message&) override;
        void onStart(Entity&) override;
        void destroy() override;

        sf::FloatRect localBounds() const override;
        sf::FloatRect globalBounds() const override;
        sf::FloatRect globalBounds(sf::Int32 latency);

        void updateQuadTree();
        void removeFromQuadTree();

        //TODO rather than expose these friend QuadTreeNode?
        void setQuadTree(QuadTree*);
        void setQuadTreeNode(QuadTreeNode*);

        Entity* getEntity();

    private:
        sf::Transform m_transform;
        sf::FloatRect m_bounds;

        QuadTree* m_quadTree;
        QuadTreeNode* m_quadTreeNode;

        sf::Clock m_timer;
        std::list<std::pair<sf::Int32, sf::Transform>> m_transformBuffer;

        Entity* m_entity;
    };
}
#endif //XY_QUADTREE_COMPONENT_HPP_