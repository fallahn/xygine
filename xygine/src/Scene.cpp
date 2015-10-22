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

#include <xygine/Scene.hpp>
#include <xygine/MessageBus.hpp>
#include <xygine/TextDrawable.hpp>
#include <xygine/QuadTreeComponent.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>

namespace
{

}

using namespace xy;

Scene::Scene(MessageBus& mb, bool createBuffers)
    : m_sceneWidth  (0.f),
    m_messageBus    (mb),
    m_drawDebug     (false)
    
{
    reset();

    if (createBuffers)
    {
        m_sceneBufferA.create(1920u, 1080u);
        m_sceneBufferB.create(1920u, 1080u);

        m_bloomEffect = std::make_unique<PostBloom>();
        m_chromeAbEffect = std::make_unique<PostChromeAb>();
    }
}

//public
void Scene::update(float dt)
{    
    //add pending entities
    for (auto& p : m_pendingEntities)
    {
        if (QuadTreeComponent* qc = p.second->getComponent<QuadTreeComponent>(Component::UniqueType::QuadTreeComponent))
        {
            m_quadTree.add(qc);
        }
        
        m_layers[p.first]->addChild(p.second);
    }
    m_pendingEntities.clear();

    //execute commands
    while (!m_commandQueue.empty())
    {
        auto cmd = m_commandQueue.pop();
        for (auto& e : m_layers)
        {
            e->doCommand(cmd, dt);
        }
    }

    for (auto& e : m_layers)
        e->update(dt);

    if(m_chromeAbEffect) m_chromeAbEffect->update(dt);
}

void Scene::handleMessage(const Message& msg)
{
    for (auto& e : m_layers)
        e->handleMessage(msg);

    if (msg.type == Message::Type::Physics)
    {

    }
    else if (msg.type == Message::Type::UI)
    {
        switch (msg.ui.type)
        {
        case Message::UIEvent::MenuClosed:

            break;
        case Message::UIEvent::MenuOpened:

            break;
        default:break;
        }
    }
}

void Scene::addEntity(Entity::Ptr& entity, Layer layer)
{
    m_pendingEntities.push_back(std::make_pair(layer, std::move(entity)));
}

Entity* Scene::findEntity(sf::Uint64 id)
{
    for (const auto& l : m_layers)
    {
        auto e = l->findEntity(id);
        if (e) return e;
    }
    return nullptr;
}

Entity& Scene::getLayer(Layer l)
{
    return *m_layers[l];
}

void Scene::setView(const sf::View& v)
{
    m_sceneBufferA.setView(v);
}

const sf::View& Scene::getView() const
{
    return m_sceneBufferA.getView();
}

void Scene::sendCommand(const Command& cmd)
{
    m_commandQueue.push(cmd);
}

sf::FloatRect Scene::getVisibleArea() const
{
    auto view = getView();
    return sf::FloatRect(view.getCenter() - (view.getSize() / 2.f), view.getSize());
}

void Scene::setSceneWidth(float width)
{
    m_sceneWidth = width;
    m_quadTree.create({ {-250.f, -250.f}, { width + 500.f, 1580.f } });
}

float Scene::getSceneWidth() const
{
    return m_sceneWidth;
}

std::vector<QuadTreeComponent*> Scene::queryQuadTree(const sf::FloatRect& area)
{
    return m_quadTree.queryArea(area);
}

void Scene::reset()
{
    m_layers.clear();
    for (int i = 0; i < Layer::Count; ++i)
    {
        m_layers.emplace_back(std::make_unique<Entity>(m_messageBus));
        m_layers.back()->setScene(this);
    }

    m_quadTree.reset();
}

void Scene::drawDebug(bool draw)
{
    m_drawDebug = draw;
}

//private
void Scene::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    std::vector<sf::Vertex> entBounds;
        
    m_sceneBufferA.clear();

    for (const auto& e : m_layers)
    {
        e->getVertices(entBounds);
        m_sceneBufferA.draw(*e, states);
    }
#ifdef _DEBUG_
    if (m_drawDebug)
    {
        m_sceneBufferA.draw(entBounds.data(), entBounds.size(), sf::PrimitiveType::LinesStrip);
        m_sceneBufferA.draw(m_quadTree);
    }
#endif //_DEBUG_

    //check view and wrap if necessary
    auto view = getView();
    bool secondPass = false;

    if (view.getCenter().x < view.getSize().x / 2.f)
    {
        view.move(m_sceneWidth, 0.f);
        secondPass = true;
    }
    else if (view.getCenter().x > m_sceneWidth - (view.getSize().x / 2.f))
    {
        view.move(-m_sceneWidth, 0.f);
        secondPass = true;
    }
    if (secondPass)
    {
        auto oldView = m_sceneBufferA.getView();
        m_sceneBufferA.setView(view);

        for (const auto& e : m_layers)
            m_sceneBufferA.draw(*e, states);

#ifdef _DEBUG_
        if (m_drawDebug)
        {
            m_sceneBufferA.draw(entBounds.data(), entBounds.size(), sf::PrimitiveType::LinesStrip);
            m_sceneBufferA.draw(m_quadTree);
        }
#endif //_DEBUG_

        m_sceneBufferA.setView(oldView);
    }

    m_sceneBufferA.display();

#ifdef _DEBUG_
    if (m_drawDebug)
    {
        m_bloomEffect->apply(m_sceneBufferA, rt);
    }
    else
    {
#endif //_DEBUG_

        //apply post effects
        m_sceneBufferB.clear();
        m_bloomEffect->apply(m_sceneBufferA, m_sceneBufferB);
        m_sceneBufferB.display();
        m_chromeAbEffect->apply(m_sceneBufferB, rt);

#ifdef _DEBUG_
    }
#endif //_DEBUG_
}
