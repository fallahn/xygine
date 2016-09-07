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

#include <xygine/QuadTree.hpp>
#include <xygine/util/Rectangle.hpp>
#include <xygine/Log.hpp>
#include <xygine/Assert.hpp>

using namespace xy;

QuadTreeNode::QuadTreeNode()
    : m_parent              (nullptr),
    m_quadTree              (nullptr),
    m_hasChildren           (false),
    m_level                 (0),
    m_numComponentsBelow    (0){}

QuadTreeNode::QuadTreeNode(const sf::FloatRect& area, sf::Int32 level, QuadTreeNode* parent, QuadTree* quadTree)
    : m_parent              (parent),
    m_quadTree              (quadTree),
    m_hasChildren           (false),
    m_area                  (area),
    m_level                 (level),
    m_numComponentsBelow    (0){}

//public
void QuadTreeNode::create(const sf::FloatRect& area, sf::Int32 level, QuadTreeNode* parent, QuadTree* quadTree)
{
    m_parent = parent;
    m_quadTree = quadTree;
    m_area = area;
    m_level = level;
}

QuadTree* QuadTreeNode::getTree() const
{
    return m_quadTree;
}

void QuadTreeNode::add(QuadTreeComponent* qc)
{
    XY_ASSERT(qc, "component pointer is null");

    //add to children if fits
    if (m_hasChildren)
    {
        if (addToChildren(qc)) return;
    }
    else //try adding to new partition
    {
        if (m_components.size() >= m_quadTree->maxNodeComponents()
            && m_level < m_quadTree->maxLevels())
        {
            split();
        
            if (addToChildren(qc)) return;
        }
    }

    //else add here as we are at our recursive limit
    addToThis(qc);
}

const sf::FloatRect& QuadTreeNode::getArea() const
{
    return m_area;
}

sf::Int32 QuadTreeNode::getNumComponentsBelow() const
{
    return m_numComponentsBelow;
}

void QuadTreeNode::update(QuadTreeComponent* qc)
{
    if (qc)
    {
        //remove from here
        if (!m_components.empty()) m_components.erase(qc);
        
        //move upwards looking for room
        QuadTreeNode* currentNode = this;
        while (currentNode)
        {
            currentNode->m_numComponentsBelow--;
            if (Util::Rectangle::contains(currentNode->m_area, qc->globalBounds())) break;

            currentNode = currentNode->m_parent;
        }

        //if no node found add to quad tree outside root set
        if (!currentNode)
        {
            XY_ASSERT(m_quadTree, "quad tree is null");

            auto& outsideSet = m_quadTree->getOutsideRootSet();
            if (outsideSet.find(qc) != outsideSet.end()) return; //already added

            outsideSet.insert(qc);
            qc->setQuadTreeNode(nullptr);
        }
        else //add to found node
        {
            currentNode->add(qc);
        }
    }
}

void QuadTreeNode::remove(QuadTreeComponent* qc)
{
    //XY_ASSERT(!m_components.empty(), "No components to remove. Make sure your root area is large enough (Scene::setSize())");
    XY_WARNING(m_components.empty(), "Components list is empty!");

    m_components.erase(qc);

    if (!qc) return;

    QuadTreeNode* currentNode = this;
    while (currentNode)
    {
        currentNode->m_numComponentsBelow--;

        if (currentNode->m_numComponentsBelow >= m_quadTree->minNodeComponents())
        {
            join();
            break;
        }
        currentNode = currentNode->m_parent;
    }
}

const QuadTreeNode::Set& QuadTreeNode::getComponents() const
{
    return m_components;
}

bool QuadTreeNode::hasChildren() const
{
    return m_hasChildren;
}

const std::array<QuadTreeNode::Ptr, 4u>& QuadTreeNode::getChildren() const
{
    return m_children;
}

void QuadTreeNode::getVertices(std::vector<sf::Vertex>& vertices)
{
    sf::Color colour = (m_components.size() > 0) ? sf::Color::Red : sf::Color::Green;
    vertices.emplace_back(sf::Vector2f(m_area.left, m_area.top), sf::Color::Transparent); //pad with breaks between quads
    vertices.emplace_back(sf::Vector2f(m_area.left, m_area.top), colour);
    vertices.emplace_back(sf::Vector2f(m_area.left + m_area.width, m_area.top), colour);
    vertices.emplace_back(sf::Vector2f(m_area.left + m_area.width, m_area.top + m_area.height), colour);
    vertices.emplace_back(sf::Vector2f(m_area.left, m_area.top + m_area.height), colour);
    vertices.emplace_back(sf::Vector2f(m_area.left, m_area.top), colour);
    vertices.emplace_back(sf::Vector2f(m_area.left, m_area.top),sf::Color::Transparent);

    colour = sf::Color::Yellow;
    for (const auto& c : m_components)
    {
        auto bounds = c->globalBounds();
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
        for (auto& c : m_children)
        {
            if(c) c->getVertices(vertices);
        }
    }
}

std::size_t QuadTreeNode::getComponentCount() const
{
    auto retval = m_components.size();
    if (m_hasChildren)
    {
        for (const auto& c : m_children)
        {
            retval += c->getComponentCount();
        }
    }
    return retval;
}

//private
void QuadTreeNode::getSubComponents()
{
    //get all components stored in children and move them to this node
    std::vector<QuadTreeNode*> nodeList;
    nodeList.push_back(this);

    while (!nodeList.empty())
    {
        auto currentNode = nodeList.back();
        nodeList.pop_back();

        for (auto qc : currentNode->m_components)
        {
            if (qc != nullptr)
            {
                qc->setQuadTreeNode(this);
                m_components.insert(qc);
            }
        }

        if (currentNode->m_hasChildren)
        {
            for (const auto& c : currentNode->m_children)
            {
                nodeList.push_back(c.get());
            }
        }
    }
}

sf::Vector2i QuadTreeNode::getPossiblePosition(QuadTreeComponent* qc)
{
    auto componentCentre = Util::Rectangle::centre(qc->globalBounds());
    auto areaCentre = Util::Rectangle::centre(m_area);

    return{ componentCentre.x > areaCentre.x ? 1 : 0, componentCentre.y > areaCentre.y ? 1 : 0 };
}

void QuadTreeNode::addToThis(QuadTreeComponent* qc)
{
    qc->setQuadTreeNode(this);
    if (m_components.find(qc) == m_components.end())
        m_components.insert(qc);
}

bool QuadTreeNode::addToChildren(QuadTreeComponent* qc)
{
    XY_ASSERT(m_hasChildren, "node has no children");

    auto position = getPossiblePosition(qc);

    auto child = m_children[position.x + position.y * 2].get();

    if (Util::Rectangle::contains(child->m_area, qc->globalBounds()))
    {
        child->add(qc);
        m_numComponentsBelow++;
        return true;
    }
    return false;
}

void QuadTreeNode::destroyChildren()
{
    for (auto& c : m_children) c.reset();
    m_hasChildren = false;
}

void QuadTreeNode::split()
{
    XY_ASSERT(!m_hasChildren, "node has children");

    sf::Vector2f areaHalfDims(m_area.width / 2.f, m_area.height / 2.f);
    sf::Vector2f areaPosition(m_area.left, m_area.top);
    sf::Vector2f areaCentre = Util::Rectangle::centre(m_area);

    sf::Int32 nextLevel = m_level - 1;
    for (auto x = 0; x < 2; ++x)
    {
        for (auto y = 0; y < 2; ++y)
        {
            sf::Vector2f offset(x * areaHalfDims.x, y * areaHalfDims.y);
            sf::FloatRect newBounds = Util::Rectangle::fromBounds(areaPosition + offset, areaCentre + offset);

            sf::Vector2f newHalfDims(newBounds.width / 2.f, newBounds.height / 2.f);
            sf::Vector2f newCentre = Util::Rectangle::centre(newBounds);
            newBounds = Util::Rectangle::fromBounds(newCentre - newHalfDims, newCentre + newHalfDims);

            m_children[x + y * 2] = std::make_unique<QuadTreeNode>(newBounds, nextLevel, this, m_quadTree);
        }
    }
    m_hasChildren = true;
}

void QuadTreeNode::join()
{
    if (m_hasChildren)
    {
        getSubComponents();
        destroyChildren();
    }
}

void QuadTreeNode::clearDestroyed()
{
    for (auto it = m_components.begin(); it != m_components.end();)
    {
        ((*it) == nullptr) ? it++ : it = m_components.erase(it);
    }

    if (m_hasChildren)
    {
        for (auto& c : m_children)
        {
            c->clearDestroyed();
        }
    }
}