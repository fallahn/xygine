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

#include <SFML/Config.hpp>
#include <SFML/Graphics/Rect.hpp>

#include <memory>
#include <string>

#include <typeinfo>
#include <typeindex>

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
    class Component
    {
    public:
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
            Script
        };

        /*!
        \brief Componenet constructor

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

        This can be implemented by any component. It is used to handle
        any system-wide messages which are broadcast by the message bus.
        This is automatically called by xygine and doesn't need to be
        manually called.

        \param Message Reference to the message to be handled
        */
        virtual void handleMessage(const Message&) {};

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

        When using the QuadTree with a component the component
        must override this to provide accurate spatial information
        which then contributes to the overall bounding box of the
        parent entity. This is normally only needed when implementing
        a drawable component.
        */
        virtual sf::FloatRect localBounds() const;
        /*!
        \brief Returns the AABB of the component in world space

        When using the QuadTree with a component the component
        must override this to provide accurate spatial information
        which then contributes to the overall bounding box of the
        parent entity. This is normally only needed when implementing
        a drawable component.
        */
        virtual sf::FloatRect globalBounds() const;

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

    };
}
#endif //XY_COMPONENT_HPP_
