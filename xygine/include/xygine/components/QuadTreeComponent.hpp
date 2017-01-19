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
    /*!
    \brief QuadTree Component

    Entities with QuadTree components attached are automatically added
    to the scene's quad tree when the entity is added to the scene. 
    Entities which are part of the quad tree are then legible to be
    returned as part of a quad tree query via the scene. Quad tree
    query results contain a list of quad tree components found in the
    queried area - the parent entities can then be retrieved from the
    resulting components.
    \see Scene
    */
    class XY_EXPORT_API QuadTreeComponent final : public Component
    {
    public:
        using Ptr = std::unique_ptr<QuadTreeComponent>;

        /*!
        \brief Constructor
        \param MessageBus
        \param bounds The area in local coordinates which the parent entity
        should occupy in the quad tree space
        */
        QuadTreeComponent(MessageBus&, const sf::FloatRect& bounds);
        ~QuadTreeComponent();

        Component::Type type() const override;
        void entityUpdate(Entity&, float) override;
        void onStart(Entity&) override;
        void destroy() override;

        /*!
        \brief Return the AABB of the quad tree component in local coords
        */
        sf::FloatRect localBounds() const override;
        /*!
        \brief Return the component's AABB in world coordinates

        Takes into account any translation or rotation of the parent entity.
        */
        sf::FloatRect globalBounds() const override;
        /*!
        \brief Returns the component's AABB in world coordinates

        Takes into account any translation or rotation of the parent entity.
        QuadTree components buffer around 600ms of transforms so that bounds
        queries can be performed over the network using lag compensation

        \param latency Number of milliseconds in to the past when querying
        transform buffer.
        */
        sf::FloatRect globalBounds(sf::Int32 latency);

        /*!
        \brief Used to update the quad tree once a component has been transformed
        */
        void updateQuadTree();
        /*!
        \brief Removes this component from the quad tree
        */
        void removeFromQuadTree();

        //TODO rather than expose these friend QuadTreeNode?
        void setQuadTree(QuadTree*);
        void setQuadTreeNode(QuadTreeNode*);

        /*!
        \brief Returns a pointer to the component's parent entity
        or nullptr if no entity exists.
        */
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