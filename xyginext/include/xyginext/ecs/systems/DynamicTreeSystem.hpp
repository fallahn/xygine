/*********************************************************************
(c) Matt Marchant 2017 - 2019
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

#include <SFML/Graphics/Rect.hpp>

#include <vector>
#include <limits>
#include <cstdint>
#include <array>

namespace xy
{
    //node struct used by the tree
    struct TreeNode final
    {
        static constexpr std::int32_t Null = -1;
        bool isLeaf() const
        {
            return childA == Null;
        }

        //this is in world coordinates
        sf::FloatRect fatBounds;
        xy::Entity entity;

        union
        {
            std::int32_t parent;
            std::int32_t next;
        };

        std::int32_t childA = Null;
        std::int32_t childB = Null;

        //leaf == 0, else Null if free
        std::int32_t height = Null;
    };

    /*!
    \brief Dynamic AABB tree for broadphase queries. Based on
    Erin Catto's dynamic tree in Box2D (http://www.box2d.org)
    which is in turn inspired by Nathanael Presson's btDbvt.
    (https://pybullet.org/wordpress/)

    Dynamic trees can perform 33%-50% faster on a scene containing a lot
    of moving objects compared to a Quad Tree (in personal bench marks
    at least). On the other hand a Quad Tree is usually more optimal for
    a scene with a lot of static geometry (such as a platformer game).
    One or the other should usually be chosen based on the type of scene
    to which it is applied - although bench marking will give the most
    accurate results. Using both in a single scene is generally considered
    redundant.
    */

    class XY_EXPORT_API DynamicTreeSystem final : public xy::System
    {
    public:
        explicit DynamicTreeSystem(xy::MessageBus&);

        void process(float) override;
        void onEntityAdded(xy::Entity) override;
        void onEntityRemoved(xy::Entity) override;

        /*!
        \brief returns a list of entities whose broadphase bounds
        intersect the given query area.
        \param area Area in world coordinates to query
        \param filter Only entities with BroadphaseComponents matching
        the given bit flags are returned. Defaults to all flags set.
        */
        std::vector<xy::Entity> query(sf::FloatRect area, std::uint64_t filter = std::numeric_limits<std::uint64_t>::max()) const;

    private:

        std::int32_t addToTree(xy::Entity);
        void removeFromTree(std::int32_t);
        //moves a proxy with the specified treeID. If the entity
        //has moved outside of the node's fattened AABB then it
        //is removed from the tree and reinsterted.
        bool moveNode(std::int32_t, sf::FloatRect, sf::Vector2f);
        sf::FloatRect getFatAABB(std::int32_t) const;
        std::int32_t getMaxBalance() const;
        float getAreaRatio() const;

        std::int32_t allocateNode();
        void freeNode(std::int32_t);

        void insertLeaf(std::int32_t);
        void removeLeaf(std::int32_t);

        std::int32_t balance(std::int32_t);

        std::int32_t computeHeight() const;
        std::int32_t computeHeight(std::int32_t) const;

        void validateStructure(std::int32_t) const;
        void validateMetrics(std::int32_t) const;

        std::int32_t m_root;

        std::size_t m_nodeCount;
        std::size_t m_nodeCapacity;
        std::vector<TreeNode> m_nodes;

        std::int32_t m_freeList; //must be signed!

        std::size_t m_path;

        std::size_t m_insertionCount;
    };

    //growable stack using preallocated memory
    namespace Detail
    {
        template <typename T, std::size_t SIZE>
        class FixedStack final
        {
        public:

            T pop()
            {
                XY_ASSERT(m_size != 0, "Stack is empty!");
                m_size--;
                return m_data[m_size];
            }

            void push(T data)
            {
                XY_ASSERT(m_size < m_data.size(), "Stack is full!");

                m_data[m_size++] = data;
            }

            std::size_t size() const
            {
                return m_size;
            }

        private:
            std::array<T, SIZE> m_data;
            std::size_t m_size = 0; //current size / next free index
        };
    }
}