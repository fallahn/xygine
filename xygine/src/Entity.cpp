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

#include <xygine/Entity.hpp>
#include <xygine/components/Component.hpp>
#include <xygine/MessageBus.hpp>
#include <xygine/Command.hpp>
#include <xygine/Scene.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

#include <iostream>

using namespace xy;

namespace
{
    sf::Uint64 uid = 1u; //use 0 for no entity
    const std::size_t MAX_ENTS = 512u;

    Detail::ObjectPool<Entity> objectPool(MAX_ENTS);
}

Entity::Entity(MessageBus& mb, const Entity::Priv&)
 : m_destroyed          (false),
 m_uid                  (uid++),
 m_messageBus           (mb),
 m_commandCategories    (Command::All),
 m_parent               (nullptr),
 m_scene                (nullptr)
{}

//public
Entity::Ptr Entity::create(MessageBus& mb)
{
    return std::move(objectPool.get(mb, Priv()));
}

Entity* Entity::addChild(Entity::Ptr& child)
{
    child->m_parent = this;
    child->setScene(m_scene);
    m_children.push_back(std::move(child));
    return m_children.back().get();
}

Entity::Ptr Entity::removeChild(Entity& child)
{
    auto result = std::find_if(m_children.begin(), m_children.end(),
        [&child](const Ptr& p)
    {
        return (p.get() == &child);
    });

    if (result != m_children.end())
    {
        Ptr found = std::move(*result);
        found->m_parent = nullptr;
        found->m_scene = nullptr;
        m_children.erase(result);
        return found;
    }
    return nullptr;
}

const std::vector<Entity::Ptr>& Entity::getChildren() const
{
    return m_children;
}

Entity* Entity::findEntity(sf::Uint64 uid)
{
    if (getUID() == uid) return this;

    for (const auto& c : m_children)
    {
        auto entity = c->findEntity(uid);
        if (entity) return entity;
    }
    return nullptr;
}

Entity* Entity::getRoot()
{
    if (m_parent)
    {
        return m_parent->getRoot();
    }
    else
    {
        return this;
    }
}

sf::Vector2f Entity::getWorldPosition() const
{
    return getWorldTransform() * sf::Vector2f();
}

sf::Transform Entity::getWorldTransform() const
{
    auto t = sf::Transform::Identity;
    for (const auto* ent = this; ent != nullptr; ent = ent->m_parent)
        t = ent->getTransform() * t;

    return t;
}

void Entity::setWorldPosition(sf::Vector2f position)
{
    if (m_parent) position -= m_parent->getWorldPosition();
    setPosition(position);
}

void Entity::update(float dt)
{       
    //remove destroyed components
    m_components.erase(std::remove_if(m_components.begin(), m_components.end(),
        [&](const Component::Ptr& p)
    {
        if (p->destroyed())
        {
            if (p->type() == Component::Type::Drawable)
            {
                auto r = std::find(m_drawables.begin(), m_drawables.end(),
                    dynamic_cast<sf::Drawable*>(p.get()));

                if (r != m_drawables.end()) m_drawables.erase(r);
            }
            return true;
        }
        return false;
    }), m_components.end());

    //allow entity / components to update each other
    for (auto& c : m_components)
    {
        c->entityUpdate(*this, dt);
    }

    //copy any new components we may have aquired via update
    for (auto& c : m_pendingComponents)
    {
        c->onDelayedStart(*this);
        m_components.push_back(std::move(c));
    }
    m_pendingComponents.clear();


    //remove any dead children (there's a line I never thought I'd write...)
    for (const auto& dc : m_deadChildren)
    {
        removeChild(*dc);
    }

    //update all children
    m_deadChildren.clear();
    for (auto& c : m_children)
    {
        c->update(dt);
        if (c->destroyed())
            m_deadChildren.push_back(&(*c));
    }
}

void Entity::destroy()
{
    for (auto& c : m_components)
    {
        c->onParentDestroyed(*this);
        c->destroy();
    }

    for (auto& c : m_children)
        c->destroy();

    auto msg = m_messageBus.post<Message::EntityEvent>(Message::EntityMessage);
    msg->action = Message::EntityEvent::Destroyed;
    msg->entity = this;

    m_destroyed = true;
}

bool Entity::destroyed() const
{
    return m_destroyed;
}

void Entity::setUID(sf::Uint64 id)
{
    m_uid = id;
}

sf::Uint64 Entity::getUID() const
{
    return m_uid;
}

void Entity::handleMessage(const Message& msg)
{
    for (auto& c : m_components)
        c->handleMessage(msg);
    
    switch (msg.id)
    {
    case Message::Type::ComponentSystemMessage:
    {
        auto& msgData = msg.getData<Message::ComponentEvent>();
        if (msgData.entityId == m_uid)
        {
            switch (msgData.action)
            {
            case Message::ComponentEvent::Deleted:
                m_components.erase(std::remove_if(m_components.begin(), m_components.end(),
                    [&msgData](const Component::Ptr& p)
                {
                    return msgData.ptr == p.get();
                }), m_components.end());

                m_drawables.erase(std::remove_if(m_drawables.begin(), m_drawables.end(),
                    [&msgData](const sf::Drawable* p)
                {
                    return msgData.ptr == (Component*)p;
                }), m_drawables.end());
                break;
            default: break;
            }
        }
        break;
    }
    default: break;
    }

    //pass down to children
    for (auto& c : m_children)
        c->handleMessage(msg);
}

void Entity::setScene(Scene* scene)
{
    m_scene = scene;
    for (auto& c : m_children) c->setScene(scene);
}

Scene* Entity::getScene()
{
    return m_scene;
}

bool Entity::doCommand(const Command& cmd, float dt)
{
    bool consumed = false;
    if ((consumed = (cmd.entityID == m_uid)) ||
        (cmd.category & m_commandCategories))
    {
        cmd.action(*this, dt);
    }
    if (consumed) return true;

    for (auto& c : m_children)
    {
        consumed = c->doCommand(cmd, dt);
        if (consumed) break;
    }
    
    return consumed;
}

void Entity::addCommandCategories(sf::Int32 mask)
{
    m_commandCategories |= mask;
}

bool Entity::hasCommandCategories(sf::Int32 categories)
{
    return ((m_commandCategories & categories) != 0);
}

sf::Uint32 Entity::size() const
{
    sf::Int32 size = 1;
    for (const auto& c : m_children)
        size += c->size();

    return size;
}

void Entity::getVertices(std::vector<sf::Vertex>& verts)
{
    auto bounds = globalBounds();
    verts.emplace_back(sf::Vector2f(bounds.left, bounds.top), sf::Color::Transparent);
    verts.emplace_back(sf::Vector2f(bounds.left, bounds.top), sf::Color::Magenta);
    verts.emplace_back(sf::Vector2f(bounds.left + bounds.width, bounds.top), sf::Color::Magenta);
    verts.emplace_back(sf::Vector2f(bounds.left + bounds.width, bounds.top + bounds.height), sf::Color::Magenta);
    verts.emplace_back(sf::Vector2f(bounds.left, bounds.top + bounds.height), sf::Color::Magenta);
    verts.emplace_back(sf::Vector2f(bounds.left, bounds.top), sf::Color::Magenta);
    verts.emplace_back(sf::Vector2f(bounds.left, bounds.top), sf::Color::Transparent);

    for (const auto& c : m_children) c->getVertices(verts);
}

sf::FloatRect Entity::globalBounds() const
{
    sf::FloatRect bounds({}, {1.f, 1.f});
    //find corner
    for (const auto& c : m_components)
    {
        if (c->type() != Component::Type::Drawable) continue;
        auto componentBounds = c->globalBounds();
        if (componentBounds.left < bounds.left) bounds.left = componentBounds.left;
        if (componentBounds.top < bounds.top) bounds.top = componentBounds.top;
    }

    //find size
    for (const auto& c : m_components)
    {
        if (c->type() != Component::Type::Drawable) continue;
        auto componentBounds = c->globalBounds();

        /*auto right = bounds.left + bounds.width;
        auto componentRight = componentBounds.left + componentBounds.width;
        if (componentRight > right)
        {
            bounds.width = componentRight - bounds.left;
        }

        auto bottom = bounds.top + bounds.height;
        auto componentBottom = componentBounds.top + componentBounds.height;
        if (componentBottom > bottom)
        {
            bounds.height = componentBottom - bounds.top;
        }*/

        auto width = componentBounds.width + componentBounds.left;
        if (width - bounds.left > bounds.width) bounds.width = width - bounds.left;

        auto height = componentBounds.height + componentBounds.top;
        if (height - bounds.top > bounds.height) bounds.height = height - bounds.top;
    }
    return getWorldTransform().transformRect(bounds);
}

//private
void Entity::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= getTransform();    

    drawSelf(rt, states);    
    for (const auto& c : m_children)
        rt.draw(*c, states);
}

void Entity::drawSelf(sf::RenderTarget& rt, sf::RenderStates states) const
{
    auto visibleArea = m_scene->getVisibleArea();
    if (visibleArea.intersects(globalBounds()))
    {
        //TODO should we second pass cull per drawable?
        //trouble is we don't have access to the component property here
        for (const auto& d : m_drawables)
        {
            rt.draw(*d, states);
        }
    }
}