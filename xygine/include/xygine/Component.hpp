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

//abstract base class for entity components

#ifndef COMPONENT_HPP_
#define COMPONENT_HPP_

#include <SFML/Config.hpp>
#include <SFML/Graphics/Rect.hpp>

#include <memory>
#include <string>

namespace xy
{
    class Entity;
    class Message;
    class MessageBus;
    class Component
    {
    public:
        using Ptr = std::unique_ptr<Component>;

        enum class Type
        {
            Drawable,
            Physics,
            Script
        };

        using UniqueType = sf::Int32;

        enum UniqueId
        {
            AnimatedDrawableId = 0,
            AnimationControllerId,
            ParticleControllerId,
            ParticleSystemId,
            QuadTreeComponentId,
            TextDrawableId,
            Count
        };

        explicit Component(MessageBus&);
        virtual ~Component();
        Component(const Component&) = delete;
        const Component& operator = (const Component&) = delete;

        virtual Type type() const = 0;
        virtual UniqueType uniqueType() const = 0;
        //this is called once per frame by the component's parent entity
        //providing the opportunity to update each other
        virtual void entityUpdate(Entity&, float) = 0;
        virtual void handleMessage(const Message&) = 0;

        //called when the component is first added to an entity
        virtual void onStart(Entity&);

        virtual void destroy();
        bool destroyed() const;

        void setParentUID(sf::Uint64 uid);
        sf::Uint64 getParentUID() const;

        void setName(const std::string&);
        const std::string& getName() const;

        virtual sf::FloatRect localBounds() const;
        virtual sf::FloatRect globalBounds() const;

    protected:
        void sendMessage(const Message&);
        MessageBus& getMessageBus() const;

    private:
        MessageBus& m_messageBus;
        bool m_destroyed;

        sf::Uint64 m_parentUID;
        std::string m_name;
    };
}
#endif //COMPONENT_HPP_