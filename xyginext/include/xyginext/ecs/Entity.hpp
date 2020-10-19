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

#include "xyginext/core/Assert.hpp"
#include "xyginext/Config.hpp"

#include "xyginext/ecs/ComponentPool.hpp"
#include "xyginext/ecs/Component.hpp"

#include <SFML/Config.hpp>

#include <bitset>
#include <vector>
#include <deque>
#include <memory>
#include <limits>

namespace xy
{
    namespace Detail
    {
        enum
        {
            MaxComponents = 64, //this is max number of types on a single entity (and max bits in a bitset)
            IndexBits = 24,
            GenerationBits = 8,
            MaxIDsPerGeneration = 1024 //max number of IDs (and therefore max component buffer size) to generate before looking to recycle ID
        };
    }
    
    using ComponentMask = std::bitset<Detail::MaxComponents>;
    class EntityManager;

    /*!
    \brief Entity class
    Entity objects are handles to the underlying collection of
    components, and as such should be treated similarly to a pointer.
    That is, they are lightweight enough to copy around, but
    you should remain aware of the lifetime of the entity being
    handled, using Entity::isValid() or Entity::destroyed()
    */
    class XY_EXPORT_API Entity final
    {
    public:
        using ID = std::uint32_t;
        using Generation = std::uint8_t;

        Entity();

        /*
        \brief Returns the index of this entity
        */
        ID getIndex() const;
        /*!
        \brief Returns the generation of this entity
        */
        Generation getGeneration() const;

        /*!
        \brief Returns true if the entity is marked for destruction
        */
        bool destroyed() const;

        /*!
        \brief Adds a copy of the given instance of a component to
        the entity
        */
        template <typename T>
        void addComponent(const T&);

        /*
        \brief Constructs a component from the given parameters,
        adds it to the entity and returns a reference to it
        */
        template <typename T, typename... Args>
        T& addComponent(Args&&...);

        /*!
        \brief returns true if the component type exists on thie entity
        */
        template <typename T>
        bool hasComponent() const;

        /*!
        \brief Returns a reference to the component if it exists
        */
        template <typename T>
        T& getComponent();

        template <typename T>
        const T& getComponent() const;

        /*!
        \brief Returns a reference to the CompnentMask associated with this entity
        */
        const ComponentMask& getComponentMask() const;

        /*!
        \brief Returns true if this entity has a valid scene associated with it
        ie it was created by Scene::createEntity()
        */
        bool isValid() const;

        /*!
        \brief Sets the label of this entity, useful for debugging
        */
        void setLabel(const std::string&);

        /*!
        \brief Returns the label assigned to t his entity. May be empty
        */
        const std::string& getLabel() const;

        bool operator == (Entity r)
        {
            return getIndex() == r.getIndex();
        }
        bool operator != (Entity r)
        {
            return getIndex() != r.getIndex();
        }
        bool operator < (Entity r)
        {
            return getIndex() < r.getIndex();
        }
        bool operator <= (Entity r)
        {
            return getIndex() <= r.getIndex();
        }
        bool operator > (Entity r)
        {
            return getIndex() > r.getIndex();
        }
        bool operator >= (Entity r)
        {
            return getIndex() >= r.getIndex();
        }

        friend bool operator < (const Entity& l, const Entity& r)
        {
            return (l.getIndex() < r.getIndex());
        }

        friend bool operator == (const Entity& l, const Entity& r)
        {
            return (l.getIndex() == r.getIndex());
        }
    private:

        explicit Entity(ID index, Generation generation);

        ID m_id;
        bool m_destroyed;
        EntityManager* m_entityManager;
        friend class EntityManager;
        friend class Scene;
    };

    class ComponentManager;
    class MessageBus;
    /*!
    \brief Manages the relationship between an Entity and its components
    */
    class XY_EXPORT_API EntityManager final
    {
    public:
        EntityManager(MessageBus&, ComponentManager&, std::size_t = 256);

        ~EntityManager() = default;
        EntityManager(const EntityManager&) = delete;
        EntityManager(EntityManager&&) = delete;
        EntityManager& operator = (const EntityManager&) = delete;
        EntityManager& operator = (EntityManager&&) = delete;

        /*!
        \brief Creates a new Entity
        */
        Entity createEntity();
        /*!
        \brief Destroys the given Entity
        */
        void destroyEntity(Entity);
        /*!
        \brief Returns true if the entity is destroyed or marked for destruction
        */
        bool entityDestroyed(Entity) const;
        /*!
        \brief Returns the entity at the given index if it exists TODO what if it doesn't?
        */
        Entity getEntity(Entity::ID) const;

        /*!
        \brief Adds a copy of the given component to the given Entity
        */
        template <typename T>
        void addComponent(Entity, T);

        /*!
        \brief Constructs a component on the given entity using the given params
        */
        template <typename T, typename... Args>
        T& addComponent(Entity, Args&&... args);

        /*!
        \brief Returns true if the given Entity has a component of this type
        */
        template <typename T>
        bool hasComponent(Entity) const;

        /*!
        \brief Returns a pointer to the component of this type if it exists
        on the given Entity else returns nullptr
        */
        template <typename T>
        T& getComponent(Entity);

        /*!
        \brief Returns a reference to the component mask of the given Entity.
        Component masks are used to identify whether an Entity has a particular component
        */
        const ComponentMask& getComponentMask(Entity) const;

        /*!
        \brief Returns true if this manager owns the given entity
        */
        bool owns(Entity) const;

        /*!
        \breif Sets the label on the given entity
        */
        void setLabel(Entity, const std::string&);

        /*!
        \brief Returns the label assigned to the given entity.
        May be empty.
        */
        const std::string& getLabel(Entity) const;

        /*!
        \brief Returns the number of active entities in the manager
        */
        std::size_t getEntityCount() const { return m_entityCount; }

    private:
        MessageBus& m_messageBus;
        ComponentManager& m_componentManager;
        std::deque<Entity::ID> m_freeIDs;
        std::vector<Entity::Generation> m_generations; // < indexed by entity ID
        std::vector<std::unique_ptr<Detail::Pool>> m_componentPools; // < index is component ID. Pool index is entity ID.
        std::size_t m_initialPoolSize;
        std::vector<ComponentMask> m_componentMasks;
        std::vector<std::string> m_labels;

        std::size_t m_entityCount;

        template <typename T>
        Detail::ComponentPool<T>& getPool();
    };

#include "Entity.inl"
#include "EntityManager.inl"
}

XY_EXPORT_API bool operator < (const xy::Entity& l, const xy::Entity& r);
