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

#include "xyginext/Config.hpp"
#include "xyginext/ecs/Entity.hpp"
#include "xyginext/ecs/Component.hpp"
#include "xyginext/core/MessageBus.hpp"
#include "xyginext/gui/GuiClient.hpp"

#include <vector>
#include <typeindex>

namespace xy
{
    class Scene;
    class ComponentManager;

    using UniqueType = std::type_index;

    /*!
    \brief Base class for systems.
    Systems should all derive from this base class, and instanciated before any entities
    are created. Concrete system types should declare a list component types via requireComponent()
    on construction, so that only entities with the relevant components are added to the system.
    */
    class XY_EXPORT_API System
    {
    public:

        using Ptr = std::unique_ptr<System>;

        /*!
        \brief Constructor.
        Pass in a reference to the concrete implementation to generate
        a unique type ID for this system.
        */
        System(MessageBus& mb, UniqueType t);


        virtual ~System() = default;

        /*!
        \brief Returns the unique type ID of the system
        */
        UniqueType getType() const { return m_type; }

        /*!
        \brief Returns a list of entities that this system is currently interested in
        */
        //std::vector<Entity>& getEntities();
        const std::vector<Entity>& getEntities() const;

        /*!
        \brief Adds an entity to the list to process
        */
        void addEntity(Entity);

        /*!
        \brief Removes an entity from the list to process
        */
        void removeEntity(Entity);

        /*!
        \brief Returns the component mask used to mask entities with corresponding
        components for this system to process
        */
        const ComponentMask& getComponentMask() const;

        /*!
        \brief Used to process any incoming system messages
        */
        virtual void handleMessage(const Message&);

        /*!
        \brief Implement this for system specific processing to entities.
        */
        virtual void process(float);

        /*!
        \brief Returns true if the system is currently active.
        Systems can be activeated and deactivated with Scene::setSystemActive()
        */
        bool isActive() const { return m_active; }

    protected:

        /*!
        \brief Adds a component type to the list of components required by the
        system for it to be interested in a particular entity. This should only
        be used in the constructor of the System else types will not be registered.
        */
        template <typename T>
        void requireComponent();

        std::vector<Entity>& getEntities();

        /*!
        \brief Optional callback performed when an entity is added
        */
        virtual void onEntityAdded(Entity) {}

        /*!
        \brief Optional callback performed when an entity is removed
        */
        virtual void onEntityRemoved(Entity) {}

        /*!
        \brief Posts a message on the system wide message bus
        */
        template <typename T>
        T* postMessage(Message::ID id);

        /*!
        \brief Returns a reference to the MessageBus
        */
        MessageBus& getMessageBus() { return m_messageBus; }

        /*
        \brief Used by the SystemManager to supply the active scene
        */
        void setScene(Scene&);

        /*!
        \brief Returns a pointer to the scene to which this system belongs
        */
        Scene* getScene();

    private:

        MessageBus& m_messageBus;
        UniqueType m_type;

        ComponentMask m_componentMask;
        std::vector<Entity> m_entities;

        Scene* m_scene;
        std::size_t m_updateIndex; //ensures when the system is active that it is updated in the order in which is was added to the manager

        bool m_active; //used by system manager to check if it has been added to the active list
        friend class SystemManager;

        //list of types populated by requireComponent then processed by SystemManager
        //when the system is created
        std::vector<std::type_index> m_pendingTypes;
        void processTypes(ComponentManager&);
    };

    class XY_EXPORT_API SystemManager final : public GuiClient
    {
    public:
        SystemManager(Scene&, ComponentManager&);

        ~SystemManager() = default;
        SystemManager(const SystemManager&) = delete;
        SystemManager(const SystemManager&&) = delete;
        SystemManager& operator = (const SystemManager&) = delete;
        SystemManager& operator = (const SystemManager&&) = delete;

        /*!
        \brief Adds a system of a given type to the manager.
        If the system already exists nothing is changed.
        \returns Reference to the system, for instance a rendering
        system maybe required elsewhere so a reference to it can be kept.
        */
        template <typename T, typename... Args>
        T& addSystem(Args&&... args);

        /*!
        \brief Removes the system of this type, if it exists
        */
        template <typename T>
        void removeSystem();

        /*!
        \brief Sets a system active or inactive by adding or removing it
        from the active systems processing list.
        \param active Set true to enable the system or false to disable.
        If the systems does not exist this function has no effect.
        */
        template <typename T>
        void setSystemActive(bool active);

        /*!
        \brief Returns a reference to this system type, if it exists
        */
        template <typename T>
        T& getSystem();

        /*!
        \brief Returns true if a system of this type exists within the manager
        */
        template <typename T>
        bool hasSystem() const;

        /*!
        \brief Submits an entity to all available systems
        */
        void addToSystems(Entity);

        /*!
        \brief Removes the given Entity from any systems to which it may belong
        */
        void removeFromSystems(Entity);

        /*!
        \brief Forwards messages to all systems
        */
        void forwardMessage(const Message&);

        /*!
        \brief Runs a simulation step by calling process() on each system
        */
        void process(float);

        /*!
        \brief Shows information in an ImGui window for each active system
        */
        void showSystemInfo(bool show = true) { m_showSystemInfo = show; }

    private:
        Scene& m_scene;
        ComponentManager& m_componentManager;
        std::vector<std::unique_ptr<System>> m_systems;
        std::vector<System*> m_activeSystems;

        bool m_showSystemInfo;

        template <typename T>
        void removeFromActive();
    };

#include "System.inl"
#include "SystemManager.inl"
}
