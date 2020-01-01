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

#include "xyginext/ecs/components/Transform.hpp"
#include "xyginext/core/Assert.hpp"

using namespace xy;

Transform::Transform()
    : m_parent      (nullptr),
    m_depth         (0)
{

}

Transform::~Transform()
{
    //remove this transform from its parent
    if (m_parent)
    {
        auto& siblings = m_parent->m_children;
        siblings.erase(
            std::remove_if(siblings.begin(), siblings.end(),
                [this](const Transform* ptr)
        {return ptr == this; }),
            siblings.end());
    }

    //orphan any children
    for(auto c : m_children)
    {
        c->m_parent = nullptr;
        c->setDepth(0);
    }
}

Transform::Transform(Transform&& other)
    : m_parent      (nullptr),
    m_depth         (0)
{
    if (other.m_parent != this)
    {
        //LOG("Moved tx via ctor", xy::Logger::Type::Info);

        //orphan any children
        for (auto c : m_children)
        {
            c->m_parent = nullptr;
            c->setDepth(0);
        }

        //and adopt new
        m_parent = other.m_parent;
        m_depth = other.m_depth;

        other.m_parent = nullptr;
        other.setDepth(0);

        //swap ourself into siblings list
        if (m_parent)
        {
            auto& siblings = m_parent->m_children;
            for (auto i = 0u; i < siblings.size(); ++i)
            {
                if (siblings[i] == &other)
                {
                    siblings[i] = this;
                    break;
                }
            }
        }
        m_children = std::move(other.m_children);

        //update the children's new parent
        for (auto* c : m_children)
        {
            if (c == this)
            {
                throw std::runtime_error("Transform already exists in child list");
            }

            c->m_parent = this;
            c->setDepth(m_depth + 1);
        }

        //actually take on the other transform
        setPosition(other.getPosition());
        setRotation(other.getRotation());
        setScale(other.getScale());
        setOrigin(other.getOrigin());

        other.setPosition({});
        other.setRotation(0.f);
        other.setScale({ 1.f, 1.f });
        other.setOrigin({});
    }
    else
    {
        xy::Logger::log("Move construction tried to parent transform to itself");
    }
}

Transform& Transform::operator=(Transform&& other)
{
    if (&other != this && other.m_parent != this)
    {
        //LOG("Moved tx via assignment", xy::Logger::Type::Info);

        //orphan any children
        for (auto c : m_children)
        {
            c->m_parent = nullptr;
            c->setDepth(0);
        }

        m_parent = other.m_parent;
        m_depth = other.m_depth;

        other.m_parent = nullptr;
        other.setDepth(0);

        //swap ourself into siblings list
        if (m_parent)
        {
            auto& siblings = m_parent->m_children;
            for (auto i = 0u; i < siblings.size(); ++i)
            {
                if (siblings[i] == &other)
                {
                    siblings[i] = this;
                    break;
                }
            }
        }

        m_children = std::move(other.m_children);

        //update the children's new parent
        for (auto c : m_children)
        {
            if (c == this)
            {
                throw std::runtime_error("Transform exists in list of children");
            }
            c->m_parent = this;
            c->setDepth(m_depth + 1);
        }

        //actually take on the other transform
        setPosition(other.getPosition());
        setRotation(other.getRotation());
        setScale(other.getScale());
        setOrigin(other.getOrigin());

        other.setPosition({});
        other.setRotation(0.f);
        other.setScale({ 1.f, 1.f });
        other.setOrigin({});
    }
    return *this;
}

//public
void Transform::addChild(Transform& child)
{
    XY_ASSERT(this != &child, "Can't parent to ourself!");

    if (&child == this)
    {
        xy::Logger::log("Cannot parent transform to itself");
        return;
    }

    //remove old parent first
    if (child.m_parent)
    {
        if (child.m_parent == this)
        {
            return; //already added!
        }

        auto& otherSiblings = child.m_parent->m_children;
        otherSiblings.erase(std::remove_if(otherSiblings.begin(), otherSiblings.end(),
            [&child](const Transform* ptr) 
        {
            return ptr == &child;
        }), otherSiblings.end());
    }
    child.m_parent = this;
    child.setDepth(m_depth + 1);
    m_children.push_back(&child);
    XY_ASSERT(m_depth < 250, "Too Deep!");
}

void Transform::removeChild(Transform& tx)
{
    if (tx.m_parent != this) return;

    tx.m_parent = nullptr;
    tx.setDepth(0);

    m_children.erase(std::remove_if(m_children.begin(), m_children.end(), 
        [&tx](const Transform* ptr)
    {
        return ptr == &tx;
    }), m_children.end());
}

sf::Transform Transform::getWorldTransform() const
{
    if (m_parent)
    {
        return m_parent->getWorldTransform() * getTransform();
    }
    return getTransform();
}

sf::Vector2f Transform::getWorldPosition() const
{
    return getWorldTransform().transformPoint({});
}

float Transform::getWorldRotation() const
{
    if (m_parent)
    {
        return m_parent->getWorldRotation() + getRotation();
    }
    return getRotation();
}

//private
void Transform::setDepth(std::size_t depth)
{
    m_depth = depth;
    for (auto& c : m_children)
    {
        c->setDepth(depth + 1);
    }

    XY_ASSERT(m_depth < 250, "Transform added with depth " + std::to_string(m_depth));
}
