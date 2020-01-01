/*********************************************************************
(c) Matt Marchant 2017 - 2020
http://trederia.blogspot.com

xygineXT - Zlib license.

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

#pragma once

#include "xyginext/ecs/System.hpp"

#include <SFML/Config.hpp>

#ifdef DDRAW
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Vertex.hpp>
#endif

#include <memory>
#include <array>
#include <limits>
#include <cstdint>

namespace xy
{
    class QuadTree;

    /*!
    \brief Nodes which make up the branches and leaves of the QuadTree
    */
    class QuadTreeNode final
    {
    public:
        using Ptr = std::unique_ptr<QuadTreeNode>;

        QuadTreeNode(sf::FloatRect area, sf::Int32 level, QuadTreeNode* parent, QuadTree* quadTree);

        void addEntity(xy::Entity);
        sf::FloatRect getArea() const;

        sf::Int32 getNumEntsBelow() const;

        void update(xy::Entity);
        void removeEntity(xy::Entity);

        const std::vector<xy::Entity>& getEntities() const;

        bool hasChildren() const;
        const std::array<Ptr, 4u>& getChildNodes() const;

        std::size_t getEntityCount() const;

#ifdef DDRAW
        void getVertices(std::vector<sf::Vertex>&);
#endif

    private:
        QuadTreeNode* m_parent;
        QuadTree* m_tree;

        bool m_hasChildren;
        std::array<Ptr, 4u> m_childNodes; //we have to dynamically create these as the recursive nature will pop the stack if pre-allocated
        std::vector<xy::Entity> m_entities;

        sf::FloatRect m_area;
        sf::Int32 m_level;
        sf::Int32 m_numEntsBelow;

        void getSubEntities();
        sf::Vector2i getPossiblePosition(xy::Entity) const;
        void addToThis(xy::Entity);
        bool addToChildren(xy::Entity);
        void destroyChildren();

        void split();
        void join();
    };

    /*!
    \brief Partition system for entities with QuadTreeItem components.
    Entities are sorted in a tree hierarchy which can be queried with a
    given area to return a set of entities which are contained or intersect
    said area. Generally more useful for scenes with a lot of static entities
    such as a platform game with fixed scenery. If using a scene with a lot
    of dynamic objects consider using xy::DynamicTreeSystem instead.
    \see DynamicTreeSystem
    */
    class XY_EXPORT_API QuadTree final : public xy::System
#ifdef DDRAW
        , public sf::Drawable
#endif
    {
    public:
        /*!
        \brief Constructor.
        \param rootArea Area in world coordinates for the root
        node of the tree. All subsequent nodes are contained within this area
        */
        QuadTree(xy::MessageBus&, sf::FloatRect rootArea);

        void process(float) override;

        /*!
        \brief Queries the QuadTree with the given area.
        Returns a vector of entities whose QuadTreeItems are contained
        in tree nodes which intersect the given area.
        /param area The area to query
        /param filterFlags Only entities with QuadTreeItems matching
        the given bit flags are returns. Defaults to all flags set.
        */
        std::vector<xy::Entity> queryArea(sf::FloatRect area, std::uint64_t = std::numeric_limits<std::uint64_t>::max()) const;

        /*!
        \brief Queries the quad tree with the given position.
        Returns a vector of entities whose QuadTreeItems are contained
        in tree nodes which also contain the given point
        /param area The area to query
        /param filterFlags Only entities with QuadTreeItems matching
        the given bit flags are returns. Defaults to all flags set.
        */
        std::vector<xy::Entity> queryPoint(sf::Vector2f, std::uint64_t = std::numeric_limits<std::uint64_t>::max()) const;

        /*!
        \brief Returns the area with which the QuadTree was created
        */
        sf::FloatRect getRootArea() const;

        /*!
        \brief Returns a vector of any entities which fall outside of
        the root area.
        */
        const std::vector<xy::Entity>& getOutsideRootEnts() const;
        std::vector<xy::Entity>& getOutsideRootEnts();

        /*!
        \brief Returns the total number of entities which fall within
        the root area.
        */
        std::size_t getEntityCount() const;

        static constexpr sf::Int32 MinNodeEntities = 3;
        static constexpr std::size_t MaxNodeEntities = 6u;
        static constexpr sf::Int32 MaxLevels = 40u;

    private:

        std::vector<xy::Entity> m_outsideRoot;
        mutable QuadTreeNode m_rootNode;

        mutable std::vector<xy::Entity> m_queryVector;

        void onEntityAdded(xy::Entity) override;
        void onEntityRemoved(xy::Entity) override;

#ifdef DDRAW
        mutable std::vector<sf::Vertex> m_vertices;
        void draw(sf::RenderTarget&, sf::RenderStates) const override;
#endif
    };
}
