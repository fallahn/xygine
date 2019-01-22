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

#include "xyginext/Config.hpp"

#include <SFML/Graphics/Transformable.hpp>

#include <vector>

namespace xy
{
    /*!
    \brief Wraps the SFML transformable class in a component
    friendly format, parentable to other transforms in a scene graph hierachy.
    Transforms are non-copyable, but are moveable
    */
    class XY_EXPORT_API Transform final : public sf::Transformable
    {
    public:
        Transform();
        ~Transform();

        Transform(const Transform&) = delete;
        Transform(Transform&&);
        Transform& operator = (const Transform&) = delete;
        Transform& operator = (Transform&&);

        /*!
        \brief Adds a child transform to this one
        */
        void addChild(Transform&);

        /*!
        \brief Removes the given child transform from this one,
        if it exists, else does nothing.
        */
        void removeChild(Transform&);

        /*!
        \brief Returns the world position of this transform by
        multiplying it with any parent transforms it may have
        */
        sf::Transform getWorldTransform() const;

        /*!
        \brief Returns the world position of the transform.
        This is effectively a shortcut for getWorldTransform().transformPoint({}).
        If this transform does not have a parent transform then getPosition()
        is probably preferable.
        */
        sf::Vector2f getWorldPosition() const;

        /*!
        \brief Returns the sum of this and all the transform's parents rotations
        if they exist, else returns the transform's current rotation
        */
        float getWorldRotation() const;

        /*!
        \brief Returns the depth of this transform in the scene hierachy.
        A value of 0 means it has no parent, 1 means one parent and so on
        */
        std::size_t getDepth() const { return m_depth; }

    private:
        Transform* m_parent;
        std::vector<Transform*> m_children;
        std::size_t m_depth;

        void setDepth(std::size_t);
    };
}
