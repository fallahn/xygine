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

//abstract base class for entity components

#ifndef XY_COMPONENT_HPP_
#define XY_COMPONENT_HPP_

#include <xygine/MessageBus.hpp>
#include <xygine/Config.hpp>

#include <SFML/Config.hpp>
#include <SFML/Graphics/Rect.hpp>

#include <memory>
#include <string>

#include <typeinfo>
#include <typeindex>
#include <vector>
#include <functional>

namespace xy
{
    class Entity;
    class Message;
    class MessageBus;

    /*!
    \brief Abstract base class for creating new components

    New components can be created by publicly inheriting the Component
    class and implementing the pure virtual functions.
    */
    class XY_EXPORT_API Component
    {
    public:
        /*!
        \brief Component Message Handler

        Allows simple callback behaviour for components to handle
        message events. Set the ID to that of the message type to
        handle and an action to perform. The action receives a 
        pointer to the Component receiving the message, and a reference
        the the message itself. Handlers can be added to a Component
        via addMessageHandler.
        */
        struct MessageHandler final
        {
            using Action = std::function<void(Component*, const Message&)>;
            Message::ID id;
            Action action;
        };

        using Ptr = std::unique_ptr<Component>;
        using UniqueType = std::type_index;

        /*!
        \brief Component type.

        When implementing the pure virtual function type()
        your component should return one of these types.
        Drawable types must indicate to xygine that they are
        drawable so that the scene render knows to draw them.
        Physics types are generally reserved for components
        which implement xygine's default physics engine. All
        other components which implement some sort of behaviour
        or logic should return Type::Script.
        */
        enum class Type
        {
            Drawable,
            Physics,
            Script,
            Mesh
        };

        /*!
        \brief Component constructor

        When calling this from the initialisation list of
        derived classes the derived class shout pass its this
        pointer to T so that the component can correctly generate
        its correct UniqueType value.
        */
        template <typename T>
        Component(MessageBus& m, T*)
            : m_messageBus  (m),
            m_destroyed     (false),
            m_parentUID     (0u),
            m_typeIndex     (typeid(T)){}

        virtual ~Component();
        Component(const Component&) = delete;
        const Component& operator = (const Component&) = delete;

        /*!
        \brief Returns the general type of component

        Implement this and return one of the values of the Type enum.
        \see Component::Type
        */
        virtual Type type() const = 0;
        const UniqueType& uniqueType() const { return m_typeIndex; };

        /*!
        \brief Used to update the component's state

        This is called once per frame by the component's parent entity
        providing the opportunity to update not only the component but
        also the entity and potentially other components attached to it

        \param Entity reference to parent entity of this component
        \param float The current frame time aka delta time
        */
        virtual void entityUpdate(Entity&, float) = 0;
        /*!
        \brief Message handler

        Called internally by xygine this forwards all messages
        to registered message handlers.

        \param Message Reference to the message to be handled
        */
        void handleMessage(const Message&);

        /*!
        \brief Add a message handler to the component

        To handle custom messages message handlers can be created
        and added to a component. Multiple handlers can be added to
        a single component, and custom message handlers can be
        added to individual instances of a component.

        \param MessageHandler Reference to MessageHandler struct to add
        */
        void addMessageHandler(const MessageHandler&);

        /*!
        \brief onStart event

        This is called when a component is added to a parent entity.
        Usually this means that the enitity is not in a completely
        ready state, so that although a reference to the new parent
        entity is passed to the event normally it's wise to store a
        reference to the entity for later use, rahter than perform
        any particular commands on it immediately.

        \param Entity parent entity to which this component has just 
        been attached
        */
        virtual void onStart(Entity&);
        /*!
        \breief onDelayedStartEvent

        This is called when pending components are moved to a live
        entity. It offers a better guarentee that the entity is in
        a more complete state (such as having a valid scene) than
        onStart(). This usually happens the first time an entity is
        updated.

        \param Entity The component's parent entity
        */
        virtual void onDelayedStart(Entity&);

        /*!
        \brief Notification of parent entity's impending destruction

        Use this is you need to tidy up the state of a component befor it is
        destroyed by its parent entity's destruction
        */
        virtual void onParentDestroyed(Entity&) {}

        /*!
        \brief Destroys this component

        This marks the component ready to be destroyed, although
        actual destruction is delayed to the beginning of the next frame
        so xygine can ensure that it is in a safe state to do so. It
        is optionally overridable for components which need to do special
        clean-up on destruction. NOTE any overriding functions *MUST*
        call Component::destroy() to ensure proper destruction.
        */
        virtual void destroy();
        /*!
        \brief Returns true if the component has been marked for destruction
        */
        bool destroyed() const;
        /*!
        \brief Sets the parent UID of the component

        This is automatically called by xygine, and should not be used for any
        other purpose. You have been warned.
        */
        void setParentUID(sf::Uint64 uid);
        /*!
        \brief Returns the UID of this component's parent.
        */
        sf::Uint64 getParentUID() const;
        /*!
        \brief Sets the name of this component

        Optionally components can be given a name which with
        they can be searched for from the parent entity

        \param std::string Name to give the component
        */
        void setName(const std::string&);
        /*!
        \brief Returns the component's name if it has one

        \see setName
        */
        const std::string& getName() const;

        /*!
        \brief Returns the AABB of the component in local space

        Drawables should always override this to return their
        local bounds, as should any components which contribute
        spatially to an entity which has a QuadTree component.
        \see globalBounds
        */
        virtual sf::FloatRect localBounds() const;
        /*!
        \brief Returns the AABB of the component in world space

        When using the QuadTree with a component the component
        must override this to provide accurate spatial information
        which then contributes to the overall bounding box of the
        parent entity. This is normally only needed when implementing
        a drawable component, but applies to any entity which may
        affect the parent entitiy spatially, such as custom collision
        components. Normally this returns the localBounds transformed
        by the component's transform if it has one, else it should
        return localBounds. NOTE Drawables *MUST* implement this
        as it is used in culling calculations when rendering. If a
        custom renderable component is not being drawn this is the
        first place to check. Global bounds can be visualised by
        enabling debug output of the Scene.
        */
        virtual sf::FloatRect globalBounds() const;

        /*!
        \brief Utility function for creating components
        */
        template <typename T, typename... Args>
        static std::unique_ptr<T> create(Args&&... args)
        {
            static_assert(std::is_base_of<Component, T>::value, "Must derive from Component class");
            return std::move(std::make_unique<T>(std::forward<Args>(args)...));
        }

    protected:
        template <typename T>
        T* sendMessage(sf::Int32 id)
        {
            return m_messageBus.post<T>(id);
        }
        MessageBus& getMessageBus() const;

    private:
        MessageBus& m_messageBus;
        bool m_destroyed;

        sf::Uint64 m_parentUID;
        std::string m_name;
        UniqueType m_typeIndex;

        std::vector<MessageHandler> m_messageHandlers;
    };
}
#endif //XY_COMPONENT_HPP_
