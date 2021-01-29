/*********************************************************************
(c) Matt Marchant 2017 - 2021
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

#include "xyginext/ecs/systems/QuadTree.hpp"
#include "xyginext/ecs/components/Transform.hpp"
#include "xyginext/ecs/components/QuadTreeItem.hpp"
#include "xyginext/util/Rectangle.hpp"
#include "xyginext/core/Log.hpp"
#include "xyginext/core/Assert.hpp"

using namespace xy;

QuadTreeNode::QuadTreeNode(sf::FloatRect area, std::int32_t level, QuadTreeNode* parent, QuadTree* quadTree)
    : m_parent      (parent),
    m_tree          (quadTree),
    m_hasChildren   (false),
    m_area          (area),
    m_level         (level),
    m_numEntsBelow  (0)
{
    XY_ASSERT(quadTree, "Must have valid quad tree");
}

//public
void QuadTreeNode::addEntity(xy::Entity entity)
{    
    //add to children if it fits
    if (m_hasChildren)
    {
        if (addToChildren(entity)) return;
    }
    //else try splitting
    else
    {
        if (m_entities.size() >= QuadTree::MaxNodeEntities
            && m_level < QuadTree::MaxLevels)
        {
            split();
            if (addToChildren(entity)) return;

            //std::cout << "Split node with " << m_entities.size() << " entities" << std::endl;
        }
    }

    //otherwise add to here because we're at the furthest branch
    addToThis(entity);
}

sf::FloatRect QuadTreeNode::getArea() const
{
    return m_area;
}

std::int32_t QuadTreeNode::getNumEntsBelow() const
{
    return m_numEntsBelow;
}

void QuadTreeNode::update(xy::Entity entity)
{
    //TODO we don't really want to call this on entities which
    //haven't changed, as we're basically removing it from the 
    //node then adding it back again, only with spurious checks
    m_entities.erase(std::remove(m_entities.begin(), m_entities.end(), entity), m_entities.end());
    entity.getComponent<xy::QuadTreeItem>().m_node = nullptr;

    auto entBounds = entity.getComponent<xy::Transform>().getWorldTransform().transformRect(entity.getComponent<xy::QuadTreeItem>().m_area);

    auto* currentNode = this;
    while (currentNode)
    {
        currentNode->m_numEntsBelow--;
        if (Util::Rectangle::contains(currentNode->m_area, entBounds))
        {
            //this is our node
            break;
        }

        //else walk up the tree
        currentNode = currentNode->m_parent;
    }

    if (!currentNode)
    {
        //we must have reached the top, so add to outside set
        auto& outsideSet = m_tree->getOutsideRootEnts();
        if (std::find(outsideSet.begin(), outsideSet.end(), entity) == outsideSet.end())
        {
            outsideSet.push_back(entity);
            //entity.getComponent<xy::QuadTreeItem>().m_node = nullptr;
        }
    }
    else
    {
        currentNode->addEntity(entity);
    }
}

void QuadTreeNode::removeEntity(xy::Entity entity)
{
    auto result = std::find(m_entities.begin(), m_entities.end(), entity);
    if (result == m_entities.end())
    {
        //auto node = entity.getComponent<xy::QuadTreeItem>().m_node;
        entity.getComponent<xy::QuadTreeItem>().m_node = nullptr;
        
        //xy::Logger::log("Entity not found in quad tree node when removing " + std::to_string(entity.getIndex()), Logger::Type::Error, Logger::Output::All);
        return;
    }
    
    entity.getComponent<xy::QuadTreeItem>().m_node = nullptr;
    m_entities.erase(result, m_entities.end());
    
    //m_entities.erase(std::remove(m_entities.begin(), m_entities.end(), entity));

    auto* currentNode = this;
    while (currentNode)
    {
        currentNode->m_numEntsBelow--;

        if (currentNode->m_numEntsBelow <= QuadTree::MinNodeEntities)
        {
            join();
            break;
        }
        currentNode = currentNode->m_parent;
    }
}

const std::vector<xy::Entity>& QuadTreeNode::getEntities() const
{
    return m_entities;
}

bool QuadTreeNode::hasChildren() const
{
    return m_hasChildren;
}

const std::array<QuadTreeNode::Ptr, 4u>& QuadTreeNode::getChildNodes() const
{
    return m_childNodes;
}

std::size_t QuadTreeNode::getEntityCount() const
{
    auto retVal = m_entities.size();
    if (m_hasChildren)
    {
        for (const auto& c : m_childNodes)
        {
            if(c) retVal += c->getEntityCount();
        }
    }
    return retVal;
}

#ifdef DDRAW
void QuadTreeNode::getVertices(std::vector<sf::Vertex>& vertices)
{
    sf::Color colour = (m_entities.size() > 0) ? sf::Color::Magenta : sf::Color::Cyan;
    vertices.emplace_back(sf::Vector2f(m_area.left, m_area.top), sf::Color::Transparent); //pad with breaks between quads
    vertices.emplace_back(sf::Vector2f(m_area.left, m_area.top), colour);
    vertices.emplace_back(sf::Vector2f(m_area.left + m_area.width, m_area.top), colour);
    vertices.emplace_back(sf::Vector2f(m_area.left + m_area.width, m_area.top + m_area.height), colour);
    vertices.emplace_back(sf::Vector2f(m_area.left, m_area.top + m_area.height), colour);
    vertices.emplace_back(sf::Vector2f(m_area.left, m_area.top), colour);
    vertices.emplace_back(sf::Vector2f(m_area.left, m_area.top), sf::Color::Transparent);

    colour = sf::Color::Yellow;

    //switch (m_level)
    //{
    //default:
    //    colour = sf::Color::Cyan;
    //    break;
    //case 0:
    //    colour = sf::Color::Yellow;
    //    break;
    //case 1:
    //    colour = sf::Color(255, 127, 0);
    //    break;
    //case 2:
    //    colour = sf::Color::Red;
    //    break;
    //}

    for (auto c : m_entities)
    {
        auto bounds = c.getComponent<Transform>().getWorldTransform().transformRect(c.getComponent<QuadTreeItem>().m_area);
        vertices.emplace_back(sf::Vector2f(bounds.left, bounds.top), sf::Color::Transparent);
        vertices.emplace_back(sf::Vector2f(bounds.left, bounds.top), colour);
        vertices.emplace_back(sf::Vector2f(bounds.left + bounds.width, bounds.top), colour);
        vertices.emplace_back(sf::Vector2f(bounds.left + bounds.width, bounds.top + bounds.height), colour);
        vertices.emplace_back(sf::Vector2f(bounds.left, bounds.top + bounds.height), colour);
        vertices.emplace_back(sf::Vector2f(bounds.left, bounds.top), colour);
        vertices.emplace_back(sf::Vector2f(bounds.left, bounds.top), sf::Color::Transparent);
    }

    if (m_hasChildren)
    {
        for (auto& c : m_childNodes)
        {
            if (c) c->getVertices(vertices);
        }
    }
}
#endif

//private
void QuadTreeNode::getSubEntities()
{
    //move all entities stored in any child nodes to this node
    std::vector<QuadTreeNode*> nodeList;
    nodeList.reserve(QuadTree::MaxLevels);
    nodeList.push_back(this);

    while (!nodeList.empty())
    {
        auto* currentNode = nodeList.back();
        nodeList.pop_back();

        if (currentNode != this) //don't duplicate our own entities
        {
            for (auto entity : currentNode->m_entities)
            {
                entity.getComponent<QuadTreeItem>().m_node = this;
                m_entities.push_back(entity);
            }
        }

        if (currentNode->m_hasChildren)
        {
            const auto& children = currentNode->m_childNodes;
            for (const auto& c : children)
            {
                if(c) nodeList.push_back(c.get());
            }
        }
    }
}

sf::Vector2i QuadTreeNode::getPossiblePosition(xy::Entity entity) const
{
    auto bounds = entity.getComponent<Transform>().getWorldTransform().transformRect(entity.getComponent<QuadTreeItem>().m_area);

    auto boundsCentre = Util::Rectangle::centre(bounds);
    auto areaCentre = Util::Rectangle::centre(m_area);
        
    return { boundsCentre.x > areaCentre.x ? 1 : 0, boundsCentre.y > areaCentre.y ? 1 : 0 };
}

void QuadTreeNode::addToThis(xy::Entity entity)
{
    entity.getComponent<QuadTreeItem>().m_node = this;
    if (std::find(m_entities.begin(), m_entities.end(), entity) == m_entities.end())
    {
        m_entities.push_back(entity);
        m_numEntsBelow++;
    }
}

bool QuadTreeNode::addToChildren(xy::Entity entity)
{
    XY_ASSERT(m_hasChildren, "No children belong to this node!");

    auto position = getPossiblePosition(entity);
    auto child = m_childNodes[position.x + position.y * 2].get();

    auto bounds = entity.getComponent<Transform>().getWorldTransform().transformRect(entity.getComponent<QuadTreeItem>().m_area);

    if (Util::Rectangle::contains(child->m_area, bounds))
    {
        child->addEntity(entity);
        m_numEntsBelow++;
        return true;
    }

    return false;
}

void QuadTreeNode::destroyChildren()
{
    for (auto& c : m_childNodes) c.reset();
    m_hasChildren = false;
}

void QuadTreeNode::split()
{
    XY_ASSERT(!m_hasChildren, "Node is already split!");

    sf::Vector2f areaHalfDims(m_area.width / 2.f, m_area.height / 2.f);
    sf::Vector2f areaPosition(m_area.left, m_area.top);
    sf::Vector2f areaCentre = Util::Rectangle::centre(m_area);

    std::int32_t nextLevel = m_level + 1;
    for (auto x = 0; x < 2; ++x)
    {
        for (auto y = 0; y < 2; ++y)
        {
            sf::Vector2f offset(x * areaHalfDims.x, y * areaHalfDims.y);
            sf::FloatRect newBounds = Util::Rectangle::fromBounds(areaPosition + offset, areaCentre + offset);

            sf::Vector2f newHalfDims(newBounds.width / 2.f, newBounds.height / 2.f);
            sf::Vector2f newCentre = Util::Rectangle::centre(newBounds);
            newBounds = Util::Rectangle::fromBounds(newCentre - newHalfDims, newCentre + newHalfDims);

            m_childNodes[x + y * 2] = std::make_unique<QuadTreeNode>(newBounds, nextLevel, this, m_tree);
        }
    }
    m_hasChildren = true;
}

void QuadTreeNode::join()
{
    if (m_hasChildren)
    {
        getSubEntities();
        destroyChildren();
    }
}
