/*********************************************************************
Matt Marchant 2014 - 2017
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

//contacts hold information describing the intersection of collision
//shapes. contacts are created when AABBs of shapes intersect, shapes
//themselves may not be touching, which can be queried with touching()

#ifndef XY_PHYS_CONTACT_HPP_
#define XY_PHYS_CONTACT_HPP_

#include <xygine/Config.hpp>

class b2Contact;
namespace xy
{
    namespace Physics
    {
        class CollisionShape;
        /*!
        \brief Contacts hold information describing the intersection of collision
        shapes.
        
        Contacts are created when AABBs of shapes intersect, shapes
        themselves may not be touching, which can be queried with touching()
        */
        class XY_EXPORT_API Contact final
        {
            friend class World;
            friend class ContactCallback;
        public:

            /*!
            \brief Returns true if the collision shapes are touching
            */
            bool touching() const;
            /*!
            \brief Set if this contact should be enabled
            */
            void enabled(bool);
            /*!
            \brief Returns true if this contact is enabled
            */
            bool enabled() const;
            /*!
            \brief Returns the first collision shape in the contact
            */
            CollisionShape* getCollisionShapeA();
            const CollisionShape* getCollisionShapeA() const;
            /*!
            \brief Returns the second collision shape in the contact
            */
            CollisionShape* getCollisionShapeB();
            const CollisionShape* getCollisionShapeB() const;
            /*!
            \brief Gets the calculated friction between the contact's collision shapes
            */
            float getFriction() const;
            /*!
            \brief Sets the friction between the contact's collision shapes
            */
            void setFriction(float);
            /*!
            \brief Resets the friction value to default value
            */
            void resetFriction();
            /*!
            \brief Get the calculated restitution between the collision shapes of this contact
            */
            float getRestitution() const;
            /*!
            \brief Set the restitution between the collision shapes of this contact
            */
            void setRestitution(float);
            /*!
            \brief Reset the restitution to the default value
            */
            void resetRestitution();
            /*!
            \brief Sets the tangent speed for conveyor like behaviour
            */
            void setTangentSpeed(float);
            /*!
            \brief Gets the current tangent speed of the contact
            */
            float getTangentSpeed() const;

        private:
            b2Contact* m_contact = nullptr;
        };
    }
}

#endif//XY_PHYS_CONTACT_HPP_