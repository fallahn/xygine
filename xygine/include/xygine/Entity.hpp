/*********************************************************************
Matt Marchant 2014 - 2016
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

//an entity defined by its collection of components

#ifndef XY_ENTITY_HPP_
#define XY_ENTITY_HPP_

#include <xygine/Log.hpp>
#include <xygine/Command.hpp>
#include <xygine/components/Component.hpp>
#include <xygine/detail/ObjectPool.hpp>

#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <memory>
#include <vector>
#include <type_traits>

namespace xy
{
    class Scene;
    /*!
    \brief Entity class

    Entities represent a transform within a scene, and allow creation of
    game objects via composition - attaching multiple Components to define
    the look and behaviour of an in-game entity. Entities are memory managed
    to help prevent fragmentation of the heap and improve data locality,
    therefore entities must be created via the create() factory function.

    Returned entities are unique_ptrs and therefore typical ownership
    semantics should be considered. Once an entity is added to a scene the
    scene will take ownership, invalidating the original reference.
    \see Scene
    */
    class XY_EXPORT_API Entity final : public sf::Transformable, public sf::Drawable
    {
    public:
        using Ptr = Detail::ObjectPool<Entity>::Ptr;
    private:
        class Priv final {};
    public:

        template<typename CONDITION>
        using enable_if = typename std::enable_if<CONDITION::value>::type;

        explicit Entity(MessageBus&, const Priv&);
        ~Entity() = default;
        Entity(const Entity&) = delete;
        Entity& operator = (const Entity&) = delete;
        /*!
        \brief Factory function for Entities

        Entities can only be created via this function, to allow
        xygine to internally manage the memory allocated.
        \param MessageBus Reference to the system message bus
        */
        static Ptr create(MessageBus&);

        /*!
        \brief Adds another entity as a child of this one

        Entities can have a child / parent relationship where
        child entities first take on the transform of their parent
        before applying their own. Adding a child to an entity
        means that the parent takes ownership, invalidating the
        original child pointer. There the function returns a
        pointer to the hild once it has been added.
        \param Entity entity to add as a child to this one
        \returns Pointer to newly added child
        */
        Entity* addChild(Ptr&);
        /*!
        \brief Removes a child entity
        \param Entity reference to the child to remove
        \returns Ptr to the removed child
        */
        Ptr removeChild(Entity&);
        /*!
        \brief Returns a reference to the vector containing
        all of this entity's children
        */
        const std::vector<Ptr>& getChildren() const;
        std::vector<Ptr>& getChildren();
        /*!
        \brief Searches the Entity's children recursively
        for and entity with the given ID
        \param UID of the entity to find
        \returns Pointer to entity if it is found, else returns nullptr
        */
        Entity* findEntity(sf::Uint64);
        /*!
        \brief Returns the root node of this entity's hierachy
        */
        Entity* getRoot();

        /*!
        \brief Returns the entity's position in world coordinates
        */
        sf::Vector2f getWorldPosition() const;
        /*!
        \brief Returns the world transform of this entity including
        any applied to it by parent entities.
        */
        sf::Transform getWorldTransform() const;
        /*!
        \brief Sets the position of this entity in world coordinates
        */
        void setWorldPosition(sf::Vector2f);
        /*!
        \brief Updates the entity

        The current frame time is passed to the entity, which then updates
        any attached components. This is automatically called on entities
        belonging to a scene and doesn't require manual usage.
        */
        void update(float dt);

        /*!
        \brief Adds a component to the entity.

        Added components must be derived from the Component class
        \see Component
        */
        template <typename T, enable_if<std::is_base_of<Component, T>>...>
        T* addComponent(std::unique_ptr<T>& component)
        {
            T* ret = component.get();
            Component::Ptr c(static_cast<Component*>(component.release()));
            if (c->type() == Component::Type::Drawable)
            {
                //store a reference to drawables so they can be drawn
                m_drawables.push_back(dynamic_cast<sf::Drawable*>(c.get()));
            }
            c->setParentUID(m_uid);
            c->onStart(*this);
            
            m_pendingComponents.push_back(std::move(c));
            return ret;
        }

        /*!
        \brief finds a commponent with a given name
        \returns Pointer to the component if it is found, else returns nullptr
        */
        template <typename T, enable_if<std::is_base_of<Component, T>>...>
        T* getComponent(const std::string& name) const
        {
            if (name.empty()) return nullptr;
            auto result = std::find_if(m_components.begin(), m_components.end(), FindByName(name));

            if (result == m_components.end())
            {
                result = std::find_if(m_pendingComponents.begin(), m_pendingComponents.end(), FindByName(name));

                if (result == m_pendingComponents.end())
                {
                    return nullptr;
                }
            }
            return dynamic_cast<T*>(result->get());
        }

        /*!
        \brief Find a component by type

        If multiple components of the same type are attached this function
        only returns the first instance found.
        \returns Pointer to component if it is found, else returns nullptr
        */
        template <typename T, enable_if<std::is_base_of<Component, T>>...>
        T* getComponent() const
        {
            std::type_index idx(typeid(T));
            auto result = std::find_if(m_components.begin(), m_components.end(), FindByTypeIndex(idx));
            if (result == m_components.end())
            {
                result = std::find_if(m_pendingComponents.begin(), m_pendingComponents.end(), FindByTypeIndex(idx));
                if (result == m_pendingComponents.end())
                {
                    return nullptr;
                }
            }
            return dynamic_cast<T*>(result->get());
        }
        /*!
        \brief Returns a vector of pointers to all components of type T

        If no components are found the vector is empty
        */
        template <typename T, enable_if<std::is_base_of<Component, T>>...>
        std::vector<T*> getComponents() const
        {
            std::vector<T*> retval;
            std::type_index idx(typeid(T));
            for (const auto& c : m_components)
            {
                if (c->uniqueType() == idx)
                {
                    retval.push_back(dynamic_cast<T*>(c.get()));
                }
            }
            for (const auto& c : m_pendingComponents)
            {
                if (c->uniqueType() == idx)
                {
                    retval.push_back(dynamic_cast<T*>(c.get()));
                }
            }
            return std::move(retval);
        }

        /*!
        \brief Removes a component if it exists
        \returns Original component if it is found or nullptr

        Returning the original component allows ownership to be
        taken by another entity. If nothing is done with the returned
        Component then the component is automatically destroyed.
        */
        template <typename T>
        std::unique_ptr<T> removeComponent(const T* component)
        {
            static_assert(std::is_base_of<Component, T>::value, "Must be a component");
            auto it = std::find_if(m_components.begin(), m_components.end(), 
                [component](const Component::Ptr& ptr)
            {
                return ptr.get() == component;
            });

            if (it != m_components.end())
            {
                std::unique_ptr<T> retVal(dynamic_cast<T*>(it->release()));
                m_components.erase(it);
                if (component->type() == Component::Type::Drawable)
                {
                    std::remove_if(m_drawables.begin(), m_drawables.end(), [component](const sf::Drawable* ptr)
                    {
                        return ptr == dynamic_cast<const sf::Drawable*>(component);
                    });
                }
                retVal->setParentUID(0u);
                return std::move(retVal);
            }

            return nullptr;
        }

        /*!
        \brief Mark this entity to be destroyed

        Entities are not destroyed immediately, rather are marked for
        destruction on the next frame to ensure the entity is in a 
        safe state to be destroyed. Destroying an entity also deletes
        all attached components so care should be taken when keeping any
        references which will become invalidated. When an entity is
        destroyed a message containing the entity ID is emitted over
        the message bus.
        */
        void destroy();
        /*!
        \brief Returns true if this entity is marked for destruction 
        on the next frame
        */
        bool destroyed() const;
        /*!
        \brief Sets the entity's unique identifier.
        Normally this should not be set manually but may be useful
        when syncronising scenes across a network
        */
        void setUID(sf::Uint64);
        /*!
        \brief Returns the Unique ID (UID) of the entity
        */
        sf::Uint64 getUID() const;
        /*!
        \brief Passes system messages on to attach components

        This is called automatically on entities belonging to a scene
        */
        void handleMessage(const Message&);
        /*!
        \brief Sets the entity's Scene pointer

        Used internally by xygine
        */
        void setScene(Scene*);
        /*!
        \brief Returns a pointer to the Scene to which this entity belongs
        else returns nullptr if not yet added to a scene
        */
        Scene* getScene();

        /*!
        \brief Used by the Scene to execute commands targeted at this entity
        */
        bool doCommand(const Command&, float);
        /*!
        \brief Add a command category to this entity

        Entities can belong to multiple command categories which are
        OR'd together. Any commands targeting a category to which this
        entity belongs are executed on this entity
        */
        void addCommandCategories(sf::Int32);
        /*!
        \brief Returns true if this Entity belongs to any of the
        given categories.
        */
        bool hasCommandCategories(sf::Int32);

        /*!
        \brief Returns the total number of entities parented to this
        one IE the depth of the entity in the scene graph

        The size returned includes this entity, so an entity with no
        parents will return 1, and an entity belonging to a Scene will
        always return at least 2.
        */
        sf::Uint32 size() const;

        /*!
        \brief Returns the vertices used to draw the entity's AABB

        Used by the Scene class when drawing debug output
        */
        void getVertices(std::vector<sf::Vertex>&);
        /*!
        \brief Returns the sum global bounds of the entity
        composed of the AABBs of all of its drawables (if any).
        Can be used in collision detection, and is used by the
        Scene class to draw debug output. Requires custom
        drawable components to override the globalBounds function
        to return the drawable's AABB relative to its parent entity
        */
        sf::FloatRect globalBounds() const;
    private:

        bool m_destroyed;
        sf::Uint64 m_uid;
        MessageBus& m_messageBus;
        sf::Int32 m_commandCategories;

        std::vector<std::unique_ptr<Component>> m_pendingComponents;
        std::vector<std::unique_ptr<Component>> m_components;
        std::vector<sf::Drawable*> m_drawables;

        std::vector<Ptr> m_children;
        std::vector<Entity*> m_deadChildren;
        Entity* m_parent;
        Scene* m_scene;

        

        void draw(sf::RenderTarget& rt, sf::RenderStates state) const override;
        void drawSelf(sf::RenderTarget&, sf::RenderStates) const;

        struct FindByName
        {
            FindByName(const std::string& name) : m_name(name){}
            bool operator()(const Component::Ptr& c){ return (c && c->getName() == m_name); }
        private:
            std::string m_name;
        };

        struct FindByTypeIndex
        {
            FindByTypeIndex(const std::type_index& ti) : m_ti(ti){}
            bool operator() (const Component::Ptr& c) { return c->uniqueType() == m_ti; }
        private:
            const std::type_index& m_ti;
        };
    };
}
#endif //XY_ENTITY_HPP_