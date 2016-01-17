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

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

#include <xygine/Assert.hpp>

namespace
{
    const sf::Int32 minimumNodeComponents = 3u;
    const sf::Uint32 maximumNodeComponents = 6u;
    const sf::Int32 maximumLevels = 20u;
}

using namespace xy;

QuadTree::QuadTree()
{

}

//public
void QuadTree::create(const sf::FloatRect& rootArea)
{
    m_rootNode = std::make_unique<QuadTreeNode>(rootArea, 0, nullptr, this);
}

bool QuadTree::created() const
{
    return (m_rootNode != nullptr);
}

void QuadTree::reset()
{
    m_rootNode.reset();
}

void QuadTree::add(QuadTreeComponent* qc)
{
    XY_ASSERT(created(), "quad tree not yet created");

    qc->setQuadTree(this);

    //if root area doesn't contain component then move outside
    if (Util::Rectangle::contains(m_rootNode->getArea(), qc->globalBounds()))
    {
        m_rootNode->add(qc);
        //LOG("added component to quad tree", Logger::Type::Info);
    }
    else
    {
        m_outsideRootSet.insert(qc);
        //LOG("component added ouside tree set", Logger::Type::Info);
    }
}

const sf::FloatRect& QuadTree::getRootArea() const
{
    XY_ASSERT(created(), "quad tree not yet created");
    return m_rootNode->getArea();
}

std::vector<QuadTreeComponent*> QuadTree::queryArea(const sf::FloatRect& area)
{
    XY_ASSERT(created(), "quad tree not yet created");

    std::vector<QuadTreeComponent*> retVal;

    //check components outside of the root
    for (auto qc : m_outsideRootSet)
    {
        if (qc && area.intersects(qc->globalBounds()))
        {
            retVal.push_back(qc);
        }
    }

    //recursively check children contained in area
    std::vector<QuadTreeNode*> nodeList;
    nodeList.push_back(m_rootNode.get());

    while (!nodeList.empty())
    {
        QuadTreeNode* currentNode = nodeList.back();
        nodeList.pop_back();

        if (area.intersects(currentNode->getArea()))
        {
            const auto& components = currentNode->getComponents();
            for (const auto& qc : components)
            {
                if (qc && area.intersects(qc->globalBounds()))
                {
                    retVal.push_back(qc);
                }
            }

            //check children
            if (currentNode->hasChildren())
            {
                const auto& children = currentNode->getChildren();
                for (const auto& c : children)
                {
                    if (c->getNumComponentsBelow() != 0)
                    {
                        nodeList.push_back(c.get());
                    }
                }
            }
        }
    }

    return retVal;
}

QuadTreeNode::Set& QuadTree::getOutsideRootSet()
{
    return m_outsideRootSet;
}

sf::Int32 QuadTree::minNodeComponents() const
{
    return minimumNodeComponents;
}

sf::Uint32 QuadTree::maxNodeComponents() const
{
    return maximumNodeComponents;
}

sf::Int32 QuadTree::maxLevels() const
{
    return maximumLevels;
}

//private
void QuadTree::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    if (!created()) return;
    std::vector<sf::Vertex> vertices;
    m_rootNode->getVertices(vertices);

    rt.draw(vertices.data(), vertices.size(), sf::PrimitiveType::LinesStrip);
}
