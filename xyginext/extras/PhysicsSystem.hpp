/*********************************************************************
(c) Matt Marchant 2017 - 2018
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

#include <xyginext/ecs/System.hpp>

#include <chipmunk/chipmunk.h>

#include <array>

/*
Physics System class for xygine which wraps Chipmunk2D functions.
This class will wrap most of the boiler plate code used to create
update and destroy a Chipmunk2D physics world so that it can be used
in xygine as any other component/system type. It also exposes the 
underlying chipmunk physics type where appropriate for more fine
grained control.

To use this class add the following source files to your xygine
project:
PhysicsSystem.hpp
PhysicsSystem.cpp
PhysicsObject.cpp

You also need to link Chipmunk2D to your project. Entities which
use the PhysicsSystem also require a xy::Transform component.
*/

namespace Convert
{
    /*
    Utils to convert between physics coordinates an xy world space
    */

    //number of xy units per metre. For example a 64px high
    //sprite would be about 2m in world units. This should
    //generally be taken into consideration when deciding the
    //mass of collision shapes when adding them to a body.
    static constexpr float UnitsPerMetre = 32.f;

    static inline cpFloat toPhysFloat(float f)
    {
        return f / UnitsPerMetre;
    }

    static inline float toWorldFloat(cpFloat f)
    {
        return static_cast<float>(f) * UnitsPerMetre;
    }

    //WARNING this inverts the Y coordinate
    //this is generally OK as the Y direction runs inversely
    //in xygine to that of Chipmunk.
    static inline cpVect toPhysVec(sf::Vector2f v)
    {
        return cpv(toPhysFloat(v.x), toPhysFloat(-v.y));
    }

    //WARNING this inverts the Y coordinate
    //this is generally OK as the Y direction runs inversely
    //in xygine to that of Chipmunk.
    static inline sf::Vector2f toWorldVec(cpVect v)
    {
        return { toWorldFloat(v.x), toWorldFloat(-v.y) };
    }

    static constexpr double PI = 3.1415926535897932384626433832795028841971693993751058209749445923078164062;
    static constexpr double RADIANS = PI / 180.0;
    static constexpr double DEGREES = 180.0 / PI;

    static inline float toDegrees(cpFloat f)
    {
        return -static_cast<float>(f * DEGREES);
    }

    static inline cpFloat toRadians(float v)
    {
        return -v * RADIANS;
    }
}

class PhysicsSystem;

/*!
\brief Contains a set of properties applied to new shapes
when they are created.
*/
struct ShapeProperties final
{
    float elasticity = 0.f; //!< 'Bounciness' value between 0 and 1
    float mass = 1.f; //<! mass in kilograms. Works best when scaled in relation to Units per metre
    bool isSensor = false; //<! sensors only trigger collision callbacks, but generate no actual collision
    float friction = 1.f; //<! surface friction of a shape
    sf::Vector2f surfaceVelocity; //<! creates the effect similar to a conveyor belt
};

/*!
\brief A physics component.
PhysicsObjects represent a body within the physics system. A body
is composed of several attached shapes which define its overall mass.
Entities with a PhysicsObject also require a xy::Transform component.
The physics simulation will update the transform automatically, so
setting the transform manually is not necessary.

Default constructed PhysicsObjects are not valid. They must be created
via the factory function in PhysicsSystem like so:

entity.addComponent<PhysicsObject>() = physicsSystem.createObject(position, type);

or

entity.addComponent<PhysicsObject>(physicsSystem.createObject(position, type));
\see PhysicsSystem::createObject()

PhysicsObjects are automatically tidied up, there is no need to manually call
PhysicsSystem::removeObject().
*/
class PhysicsObject final
{
public:
    PhysicsObject(const PhysicsObject&) = delete;
    const PhysicsObject& operator = (const PhysicsObject&) = delete;

    PhysicsObject(PhysicsObject&&) noexcept;
    PhysicsObject& operator = (PhysicsObject&&) noexcept;

    PhysicsObject();
    ~PhysicsObject();

    /*!
    \brief returns true if the body is 'awake'.
    */
    bool awake() const;

    /*!
    \brief Body type.
    A type of None (default) shows that the component is currently invalid.
    Dynamic bodies are the default type, updated by the physics
    simulation.
    Kinematic bodies are usually controlled by user input. They have
    infinite mass and are unaffected by physics interactions, but affect
    other dynamic bodies on contact.
    Static bodies do not move. These are usually used to create terrain
    geometry.
    */
    enum class Type
    {
        None,
        Dynamic,
        Kinematic,
        Static
    };

    /*!
    \brief Returns the type of the body as it was created.
    */
    Type getType() const { return m_type; }

    /*!
    \brief Shape creation functions.
    Used to add shapes to the PhysicsObject if it is valid.
    Pass a reference to a ShapeProperties struct containing
    configuration properties of the shape such as mass and friction.

    These functions return the shape pointer which was created so that
    the existing chipmunk functions can be used to further configure
    properties. They return nullptr if creation fails for some reason.

    Currently PhysicsObjects are limited to 4 shapes, but this can be adjusted
    by changing the MaxShapes constant.
    */

    /*!
    \param radius Radius in xy units, automatically converted to cp units on creation
    \param offset Vector2f in xy units to offset the circle shape from the body's
    centre of gravity.
    */
    cpShape* addCircleShape(const ShapeProperties&, float radius, sf::Vector2f offset = {});
    /*!
    \param start Position in xygine units for the first segment point, relative to the
    centre of gravity of the PhysicsObject
    \param end Position in xygine units for the last segment point, relative to the centre
    of gravity of the PhysicsObject
    */
    cpShape* addLineShape(const ShapeProperties&, sf::Vector2f start, sf::Vector2f end);
    /*!
    \param points A vector of points which make up a convex polygon. Points must be in 
    xygine units and *clockwise* order, as these are re-wound during unit conversion to
    chipmunk coordinates.
    */
    cpShape* addPolygonShape(const ShapeProperties&, const std::vector<sf::Vector2f>& points);

    /*
    \param size Width and height in xygine units.
    */
    cpShape* addBoxShape(const ShapeProperties&, sf::Vector2f size);

private:

    Type m_type;
    cpBody* m_body;
    
    static constexpr std::size_t MaxShapes = 4u;
    std::array<cpShape*, MaxShapes> m_shapes;
    std::size_t m_shapeCount;

    PhysicsSystem* m_system;
    friend class PhysicsSystem;

    void applyProperties(const ShapeProperties&, cpShape*);
};

/*!
\brief System in charge of updating the physics world
and properly managing the memory and lifetimes of
PhysicsObjects.
*/
class PhysicsSystem final : public xy::System
{
public:
    explicit PhysicsSystem(xy::MessageBus&);
    ~PhysicsSystem();

    void process(float) override;

    /*!
    \brief Used to create a new PhysicsObject. PhysicsObjects
    not created via this function will be invalid.
    \param position The initial position of the body in the world
    \param type The requested type of PhysicsObject
    \see PhysicsObject
    */
    PhysicsObject createObject(sf::Vector2f position = {}, PhysicsObject::Type = PhysicsObject::Type::Dynamic);
    
    /*!
    \brief Used to tidy up spent PhysicsObjects.
    There is no need to call this yourself.
    */
    void removeObject(PhysicsObject&);

    void onEntityRemoved(xy::Entity) override;

private:
    cpSpace* m_space;

    friend class PhysicsObject;
};