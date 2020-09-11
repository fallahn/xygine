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
#include "xyginext/core/App.hpp"
#include "xyginext/ecs/Entity.hpp"
#include "xyginext/ecs/Component.hpp"
#include "xyginext/ecs/System.hpp"
#include "xyginext/ecs/systems/CommandSystem.hpp"
#include "xyginext/ecs/Director.hpp"
#include "xyginext/graphics/postprocess/PostProcess.hpp"

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Drawable.hpp>

#include <functional>
#include <array>
#include <optional>

namespace xy
{
    class MessageBus;

    /*!
    \brief Encapsulates a single scene.
    The scene class contains everything needed to create a scene graph by encapsulating
    the ECS and providing factory functions for entities. Multple scenes
    can exist at one time, for instance one to draw the game world, and another to draw
    the HUD. Everything is rendered through renderable systems which, in turn, require
    an ECS - therefore every state which wishes to draw something requires at least a scene,
    right down to menus. All systems should be added before attempting to create any
    entities else existing entities will not be processed by new systems.
    */

    class XY_EXPORT_API Scene final : public sf::Drawable, public xy::GuiClient
    {
    public:
        /*!
        \brief Constructor.
        \param messageBus Reference to the active message bus
        \param initialPoolSize Components are pooled in memory, and this
        is the initial number of components for which memory is allocated.
        Memory pools will resize at runtime if necessary, but an initial
        size can be set here. The default is 256 components. As the maximum
        number of entity IDs is 1024 before they are recycled, values greater
        than this have no effect other than allocating unusable memory.
        */
        explicit Scene(MessageBus& messageBus, std::size_t initialPoolSize = 256);


        ~Scene() = default;
        Scene(const Scene&) = delete;
        Scene(Scene&&) = delete;
        Scene& operator = (const Scene&) = delete;
        Scene& operator = (Scene&&) = delete;

        /*!
        \brief Executes one simulation step.
        \param dt The time elapsed since the last simulation step
        */
        void update(float dt);

        /*!
        \brief Creates a new entity in the Scene, and returns a handle to it
        */
        Entity createEntity();

        /*!
        \brief Destroys the given entity and removes it from the scene
        */
        void destroyEntity(Entity);

        /*|
        \brief Returns a handle to the entity with the given ID if it exists,
        otherwise an invalid handle.
        \see Entity::isValid()
        */
        Entity getEntity(Entity::ID) const;

        /*!
        \brief Creates a new system of the given type.
        All systems need to be fully created before adding entities, else
        entities will not be correctly processed by desired systems.
        \returns Reference to newly created system (or existing system
        if it has been previously created)
        */
        template <typename T, typename... Args>
        T& addSystem(Args&&... args);

        /*!
        \brief Returns a reference to the Scene's system of this type, if it exists
        */
        template <typename T>
        T& getSystem();

        /*!
        \brief Sets the given system type active or inactive in the scene.
        Inactive systems are moved from the processing list and are ignored
        until set active again. If the system type give doesn't exist then
        this function does nothing.
        */
        template <typename T>
        void setSystemActive(bool active);

        /*!
        \brief Adds a Director to the Scene.
        Directors are used to control in game entities and events through
        observed messages and events. Returns a reference to the newly added
        director.
        \see Director
        */
        template <typename T, typename... Args>
        T& addDirector(Args&&... args);

        /*!
        \brief Returns a reference to the the requested Director, if it exists
        */
        template <typename T>
        T& getDirector();

        /*!
        \brief Adds a post process effect to the scene.
        Any post processes added to the scene are performed on the *entire* output.
        To add post processes to a portion of the scene then
        a second scene should be created to draw overlays such as the UI
        */
        template <typename T, typename... Args>
        T& addPostProcess(Args&&... args);

        /*!
        \brief Enables or disables any added post processes added to the scene
        */
        void setPostEnabled(bool);

        /*!
        \brief Returns a copy of the entity containing the default camera
        */
        Entity getDefaultCamera() const;

        /*!
        \brief Sets the active camera used when rendering.
        This camera will be used to render the entire visible scene.
        A single scene may also be rendered multiple times
        with different cameras, for example when rendering split screen.
        \returns Entity used as the previous camera for the scene
        */
        Entity setActiveCamera(Entity);

        /*!
        \brief Returns the entity containing the active camera component
        */
        Entity getActiveCamera() const;

        /*!
        \brief Sets the active listener when processing audio.
        Usually this will be on the same entity as the active camera,
        although effects such as remote monitoring can be used by setting
        a different listener.
        \returns Entity containing the component used as the previous active listener.
        */
        Entity setActiveListener(Entity);

        /*!
        \brief Returns the entity containing the active AudioListener component
        */
        Entity getActiveListener() const;

        /*!
        \brief Forwards any events to Directors in the Scene
        */
        void forwardEvent(const sf::Event&);

        /*!
        \brief Forwards messages to the systems in the scene
        */
        void forwardMessage(const Message&);

        /*!
        \brief Returns the active entity count.
        Debugging util.
        */
        std::size_t getEntityCount() const { return m_entityManager.getEntityCount(); }

        /*!
        \brief Draws a window with some information on the current active systems.
        Debug util - show ECS system info, not hardware system
        */
        void showSystemInfo(bool show = true) { m_systemManager.showSystemInfo(show); }

        /*!
        \brief Returns a reference to the Scene's MessageBus
        */
        MessageBus& getMessageBus() { return m_messageBus; }

    private:
        MessageBus& m_messageBus;
        Entity m_defaultCamera;
        Entity m_activeCamera;
        Entity m_activeListener;

        std::vector<Entity> m_pendingEntities;
        std::vector<Entity> m_destroyedEntities;
        std::vector<Entity> m_destroyedBuffer;

        ComponentManager m_componentManager;
        EntityManager m_entityManager;
        SystemManager m_systemManager;

        std::vector<std::unique_ptr<Director>> m_directors;
        template <typename T>
        bool directorExists() const;

        std::vector<sf::Drawable*> m_drawables;

        sf::RenderTexture m_sceneBuffer;
        std::array<sf::RenderTexture, 2u> m_postBuffers;
        std::vector<std::unique_ptr<PostProcess>> m_postEffects;

        void postRenderPath(sf::RenderTarget&, sf::RenderStates);
        std::function<void(sf::RenderTarget&, sf::RenderStates)> currentRenderPath;

        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };

#include "Scene.inl"
}
