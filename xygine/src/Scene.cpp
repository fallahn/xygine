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

#include <xygine/Scene.hpp>
#include <xygine/MessageBus.hpp>
#include <xygine/components/QuadTreeComponent.hpp>
#include <xygine/components/AudioListener.hpp>
#include <xygine/components/PointLight.hpp>
#include <xygine/Reports.hpp>
#include <xygine/Console.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Audio/Listener.hpp>

namespace
{
    const sf::Uint32 bufferWidth = 1920u;
    const sf::Uint32 bufferHeight = 1080u;

    xy::Scene* m_instance = nullptr;
}

using namespace xy;
using namespace std::placeholders;

Scene::Scene(MessageBus& mb)
    : m_ambientColour   (51,51,51),
    m_defaultCamera     (nullptr),
    m_activeCamera      (nullptr),
    m_messageBus        (mb),
    m_drawDebug         (false)
{
    reset();
    setupConCommands();
    m_instance = this;
}

Scene::~Scene()
{
    m_instance = nullptr;
    Console::unregisterCommands(this);
}

//public
void Scene::update(float dt)
{    
    std::function<void(Entity::Ptr&)> setPartitioning = 
        [this](Entity::Ptr& p)
    {
        if (QuadTreeComponent* qc = p->getComponent<QuadTreeComponent>())
        {
            m_quadTree.add(qc);
        }

        //if we find a light add it to its own quad tree
        if (PointLight* pl = p->getComponent<PointLight>())
        {
            const float rad = pl->getRadius();
            auto qtc = xy::Component::create<QuadTreeComponent>(m_messageBus, sf::FloatRect({ -rad, -rad }, { rad * 2.f, rad * 2.f}));
            m_lightTree.add(p->addComponent(qtc));
        }
    };
    
    //add pending entities
    for (auto& p : m_pendingEntities)
    {
        setPartitioning(p.second);
        for (auto& c : p.second->getChildren())
        {
            setPartitioning(c);
        }

        p.second->setScene(this);
        m_layers[p.first]->addChild(p.second);
    }
    m_pendingEntities.clear();
    //REPORT("In Light Tree Set", std::to_string(m_lightTree.getComponentCount()));
    //REPORT("Out Light Tree Set", std::to_string(m_lightTree.getOutsideRootSet().size()));
    //REPORT("Quad Tree Size", std::to_string(m_quadTree.getComponentCount()));

    //execute commands
    REPORT("Commands this frame", std::to_string(m_commandQueue.size()));
    while (!m_commandQueue.empty())
    {
        auto cmd = m_commandQueue.pop();
        for (auto& e : m_layers)
        {
            if (e->doCommand(cmd, dt)) break;
        }
    }

    for (auto& e : m_layers)
    {
        e->update(dt);
    }

    for (auto& pass : m_renderPasses)
    {
        pass.postEffect->update(dt);
    }
}

void Scene::handleMessage(const Message& msg)
{
    for (auto& pp : m_renderPasses)
    {
        pp.postEffect->handleMessage(msg);
    }
    
    for (auto& e : m_layers)
    {
        e->handleMessage(msg);
    }

    if (msg.id == Message::Type::UIMessage)
    {
        auto& msgData = msg.getData<Message::UIEvent>();
        switch (msgData.type)
        {
        case Message::UIEvent::MenuClosed:

            break;
        case Message::UIEvent::MenuOpened:

            break;
        default:break;
        }
    }
    else if (msg.id == Message::ComponentSystemMessage)
    {
        auto& msgData = msg.getData<Message::ComponentEvent>();
        switch (msgData.action)
        {
        case Message::ComponentEvent::Deleted:
            if (msgData.ptr == m_activeCamera)
            {
                m_activeCamera = m_defaultCamera;
            }
            break;
        default: break;
        }
    }
    //else if (msg.id == Message::EntityMessage)
    //{
    //    auto& msgData = msg.getData<Message::EntityEvent>();
    //    
    //    if (msgData.action == Message::EntityEvent::AddedToScene)
    //    {
    //        if (QuadTreeComponent* qc = msgData.entity->getComponent<QuadTreeComponent>())
    //        {
    //            m_quadTree.add(qc);
    //        }

    //        //if we find a light add it to its own quad tree
    //        if (PointLight* pl = msgData.entity->getComponent<PointLight>())
    //        {
    //            const float rad = pl->getRadius();
    //            auto qtc = xy::Component::create<QuadTreeComponent>(m_messageBus, sf::FloatRect({ -rad, -rad }, { rad * 2.f, rad * 2.f }));
    //            m_lightTree.add(msgData.entity->addComponent(qtc));
    //        }
    //    }
    //}
}

Entity* Scene::addEntity(Entity::Ptr& entity, Layer layer)
{
    Entity* ret = entity.get();
    m_pendingEntities.push_back(std::make_pair(layer, std::move(entity)));
    return ret;
}

Entity* Scene::findEntity(sf::Uint64 id)
{
    for (const auto& e : m_pendingEntities)
    {
        if (e.second->getUID() == id)
        {
            return e.second.get();
        }
    }
       
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
    m_defaultCamera->setView(v);
    m_activeCamera = m_defaultCamera;

    //we don't want to letterbox render buffers
    if (!m_renderPasses.empty())
    {
        m_defaultCamera->setViewport({ { 0.f, 0.f },{ 1.f, 1.f } });
    }
}

sf::View Scene::getView() const
{
    return m_activeCamera->getView();
}

sf::FloatRect Scene::getVisibleArea() const
{
    const auto& view = getView();
    return sf::FloatRect(view.getCenter() - (view.getSize() / 2.f), view.getSize());
}

void Scene::setActiveCamera(const Camera* camera)
{
    if (camera) 
    {
        m_activeCamera = camera;
    }
    else
    {
        m_activeCamera = m_defaultCamera;
    }
    auto msg = m_messageBus.post<xy::Message::SceneEvent>(xy::Message::SceneMessage);
    msg->action = xy::Message::SceneEvent::CameraChanged;
}

void Scene::setClearColour(const sf::Color& colour)
{
    m_defaultCamera->setClearColour(colour);
}

void Scene::setSize(const sf::FloatRect& size)
{
    m_quadTree.create(size);
    Command cmd;
    cmd.category = Command::All;
    cmd.action = [this](Entity& entity, float)
    {
        if (auto qc = entity.getComponent<QuadTreeComponent>())
        {
            m_quadTree.add(qc);
        }
    };
    sendCommand(cmd);
}

void Scene::sendCommand(const Command& cmd)
{
    m_commandQueue.push(cmd);
}

std::vector<QuadTreeComponent*> Scene::queryQuadTree(const sf::FloatRect& area) const
{
    return std::move(m_quadTree.queryArea(area));
}

std::vector<PointLight*> Scene::getVisibleLights(const sf::FloatRect& area) const
{
    const auto result = m_lightTree.queryArea(area);
    std::vector<PointLight*> retval;
    retval.reserve(result.size());
    
    for (auto i = 0u; i < result.size(); ++i)
    {
        if (auto lc = result[i]->getEntity()->getComponent<PointLight>())
        {
            retval.push_back(lc);
        }
    }
    return std::move(retval);
}

void Scene::reset()
{
    m_layers.clear();
    for (int i = 0; i < Layer::Count; ++i)
    {
        m_layers.emplace_back(Entity::create(m_messageBus));
        m_layers.back()->setScene(this);
    }

    m_quadTree.reset();
    m_quadTree.create({ {0.f, 0.f}, DefaultSceneSize });

    m_lightTree.reset();
    m_lightTree.create({ {0.f, 0.f}, DefaultSceneSize });

    m_renderPasses.clear();
    m_renderPasses.reserve(10);
    m_currentRenderPath = std::bind(&Scene::defaultRenderPath, this, _1, _2);

    //default sound / camera entity
    auto entity = Entity::create(m_messageBus);
    auto al = Component::create<AudioListener>(m_messageBus);
    entity->addComponent(al);
    entity->setPosition(DefaultSceneSize / 2.f);

    auto camera = Component::create<Camera>(m_messageBus, sf::View(DefaultSceneSize / 2.f, DefaultSceneSize));
    m_defaultCamera = entity->addComponent(camera);
    m_layers[Layer::BackRear]->addChild(entity);

    m_activeCamera = m_defaultCamera;

    m_ambientColour = { 51, 51, 51 };

    m_reflectionTexture.create(static_cast<sf::Uint32>(DefaultSceneSize.x) / 8, static_cast<sf::Uint32>(DefaultSceneSize.y) / 8);
    m_reflectionTexture.setRepeated(true);
    m_reflectionTexture.setSmooth(true);
}

void Scene::addPostProcess(PostProcess::Ptr& pp)
{
    if (m_renderPasses.empty())
    {
        m_sceneBufferA.create(bufferWidth, bufferHeight);

        m_renderPasses.emplace_back();
        m_renderPasses.back().postEffect = std::move(pp);
        m_renderPasses.back().inBuffer = &m_sceneBufferA;

        m_currentRenderPath = std::bind(&Scene::postEffectRenderPath, this, _1, _2);

        m_defaultCamera->setViewport({ {0.f, 0.f},{1.f, 1.f} });
    }
    else
    {
        if (m_renderPasses.size() == 1)
        {
            m_sceneBufferB.create(bufferWidth, bufferHeight);
        }

        m_renderPasses.back().outBuffer = (m_renderPasses.back().inBuffer == &m_sceneBufferA) ? &m_sceneBufferB : &m_sceneBufferA;
        auto newIn = m_renderPasses.back().outBuffer;

        m_renderPasses.emplace_back();
        m_renderPasses.back().postEffect = std::move(pp);
        m_renderPasses.back().inBuffer = newIn;
    }
}

void Scene::drawDebug(bool draw)
{
    m_drawDebug = draw;
}

sf::Transform Scene::getViewMatrix()
{
    if (m_instance)
    {
        const auto& view = m_instance->getView();
        sf::Transform tx;
        tx.translate(view.getCenter() - (view.getSize() / 2.f));
        return tx.getInverse();
    }
    return sf::Transform();
}

//private
void Scene::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    sf::View v(m_activeCamera->getView().getCenter(), DefaultSceneSize);
    v.rotate(180.f);
    m_reflectionTexture.setView(v);
    m_reflectionTexture.clear(m_activeCamera->getClearColour());
    m_reflectionTexture.draw(*m_layers[Layer::BackRear]);
    m_reflectionTexture.draw(*m_layers[Layer::BackMiddle]);
    m_reflectionTexture.display();

    m_currentRenderPath(rt, states);
}

void Scene::defaultRenderPath(sf::RenderTarget& rt, sf::RenderStates states) const
{   
    rt.setView(m_activeCamera->getView());
 
#ifdef _DEBUG_
    std::vector<sf::Vertex> entBounds;
#endif //_DEBUG_

    for (const auto& e : m_layers)
    {
#ifdef _DEBUG_
        e->getVertices(entBounds);
#endif //_DEBUG_
        rt.draw(*e, states);
    }

#ifdef _DEBUG_
    if (m_drawDebug)
    {
        rt.draw(entBounds.data(), entBounds.size(), sf::PrimitiveType::LinesStrip);
        rt.draw(m_quadTree);
    }
#endif //_DEBUG_
    rt.setView(rt.getDefaultView());
}

void Scene::postEffectRenderPath(sf::RenderTarget& rt, sf::RenderStates states) const
{
    //-------render scene to first buffer-----///
    auto firstBuffer = m_renderPasses.front().inBuffer;
    firstBuffer->setView(m_activeCamera->getView());
    firstBuffer->clear(m_activeCamera->getClearColour());
#ifdef _DEBUG_
    std::vector<sf::Vertex> entBounds;
#endif //_DEBUG_

    for (const auto& e : m_layers)
    {
#ifdef _DEBUG_
        e->getVertices(entBounds);
#endif //_DEBUG_
        firstBuffer->draw(*e, states);
    }

#ifdef _DEBUG_
    if (m_drawDebug)
    {
        firstBuffer->draw(entBounds.data(), entBounds.size(), sf::PrimitiveType::LinesStrip);
        firstBuffer->draw(m_quadTree);
    }
#endif //_DEBUG_
    firstBuffer->display();


    //----apply all post processes-----//
    for (const auto& pass : m_renderPasses)
    {
        if (pass.outBuffer)
        {
            pass.outBuffer->setView(pass.outBuffer->getDefaultView());
            pass.outBuffer->clear(m_activeCamera->getClearColour());
            pass.postEffect->apply(*pass.inBuffer, *pass.outBuffer);
            pass.outBuffer->display();
        }
    }

    rt.setView(rt.getDefaultView());
    m_renderPasses.back().postEffect->apply(*m_renderPasses.back().inBuffer, rt);    
}

void Scene::setupConCommands()
{
    Console::addCommand("r_drawDebug",
        [this](const std::string& params)
    {
        if (params.find_first_of('0') == 0 ||
            params.find_first_of("false") == 0)
        {
            drawDebug(false);
        }
        else if (params.find_first_of('1') == 0 ||
            params.find_first_of("true") == 0)
        {
            drawDebug(true);
        }
        else
        {
            Console::print("r_drawDebug: valid parameters are 0, 1, false or true");
        }

    }, this);
}