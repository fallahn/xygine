/*********************************************************************
(c) Matt Marchant 2017
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

#include <xyginext/core/App.hpp>
#include <xyginext/ecs/Scene.hpp>
#include <xyginext/ecs/components/Camera.hpp>
#include <xyginext/ecs/components/Transform.hpp>
//#include <xyginext/ecs/components/AudioListener.hpp>

using namespace xy;

namespace
{
    sf::FloatRect getDefaultViewport()
    {
        auto winSize = sf::Vector2f(App::getRenderWindow().getSize());

        float windowRatio = winSize.x / winSize.y;
        float viewRatio = DefaultSceneSize.x / DefaultSceneSize.y;

        float sizeY = windowRatio / viewRatio;
        float top = (1.f - sizeY) / 2.f;

        return { { 0.f, top },{ 1.f, sizeY } };
    }
}

Scene::Scene(MessageBus& mb)
    : m_messageBus      (mb),
    m_entityManager     (mb),
    m_systemManager     (*this)
{
    auto defaultCamera = createEntity();
    defaultCamera.addComponent<Transform>().setPosition(xy::DefaultSceneSize / 2.f);
    defaultCamera.addComponent<Camera>();
    defaultCamera.getComponent<Camera>().setViewport(getDefaultViewport());
    //defaultCamera.addComponent<AudioListener>();

    m_defaultCamera = defaultCamera.getIndex();
    m_activeCamera = m_defaultCamera;
    //m_activeListener = m_defaultCamera;

    currentRenderPath = [this](sf::RenderTarget& rt, sf::RenderStates states)
    {
        for (auto r : m_drawables)
        {
            rt.draw(*r, states);
        }
    };
}

//public
void Scene::update(float dt)
{
    //update directors first as they'll be working on data from the last frame
    for (auto& d : m_directors)
    {
        d->process(dt);
    }

    for (const auto& entity : m_pendingEntities)
    {
        m_systemManager.addToSystems(entity);
    }
    m_pendingEntities.clear();

    for (const auto& entity : m_destroyedEntities)
    {
        m_systemManager.removeFromSystems(entity);
        m_entityManager.destroyEntity(entity);
    }
    m_destroyedEntities.clear();


    m_systemManager.process(dt);
    //for (auto& p : m_postEffects) p->process(dt);
}

Entity Scene::createEntity()
{
    m_pendingEntities.push_back(m_entityManager.createEntity());
    return m_pendingEntities.back();
}

void Scene::destroyEntity(Entity entity)
{
    m_destroyedEntities.push_back(entity);
}

Entity Scene::getEntity(Entity::ID id) const
{
    return m_entityManager.getEntity(id);
}

//void Scene::setPostEnabled(bool enabled)
//{
//    if (enabled && !m_postEffects.empty())
//    {
//        currentRenderPath = std::bind(&Scene::postRenderPath, this);
//        auto size = App::getWindow().getSize();
//        m_sceneBuffer.create(size.x, size.y, true);
//        for (auto& p : m_postEffects) p->resizeBuffer(size.x, size.y);
//    }
//    else
//    {       
//        currentRenderPath = [this]()
//        {
            //for (auto r : m_drawables)
            //{
            //    rt.draw(*r, states);
            //}
//        };
//    }
//}

Entity Scene::getDefaultCamera() const
{
    return m_entityManager.getEntity(m_defaultCamera);
}

Entity Scene::setActiveCamera(Entity entity)
{
    XY_ASSERT(entity.hasComponent<Transform>() && entity.hasComponent<Camera>(), "Entity requires at least a transform and a camera component");
    XY_ASSERT(m_entityManager.owns(entity), "This entity must belong to this scene!");
    auto oldCam = m_entityManager.getEntity(m_activeCamera);
    m_activeCamera = entity.getIndex();

    return oldCam;
    //return m_entityManager.getEntity(m_defaultCamera);
}

Entity Scene::setActiveListener(Entity entity)
{
    //CRO_ASSERT(entity.hasComponent<Transform>() && entity.hasComponent<AudioListener>(), "Entity requires at least a transform and a camera component");
    //CRO_ASSERT(m_entityManager.owns(entity), "This entity must belong to this scene!");
    //auto oldListener = m_entityManager.getEntity(m_activeListener);
    //m_activeListener = entity.getIndex();
    //return oldListener;
    return m_entityManager.getEntity(m_activeListener);
}

Entity Scene::getActiveListener() const
{
    return m_entityManager.getEntity(m_activeListener);
}

Entity Scene::getActiveCamera() const
{
    return m_entityManager.getEntity(m_activeCamera);
}

void Scene::forwardEvent(const sf::Event& evt)
{
    for (auto& d : m_directors)
    {
        d->handleEvent(evt);
    }
}

void Scene::forwardMessage(const Message& msg)
{
    m_systemManager.forwardMessage(msg);
    for (auto& d : m_directors)
    {
        d->handleMessage(msg);
    }

    if (msg.id == Message::WindowMessage)
    {
        const auto& data = msg.getData<Message::WindowEvent>();
        if (data.type == Message::WindowEvent::Resized)
        {
            //resizes the post effect buffer if it is in use
            /*if (m_sceneBuffer.available())
            {
                m_sceneBuffer.create(data.width, data.height);
                for (auto& p : m_postEffects) p->resizeBuffer(data.width, data.height);
            }

            if (m_postBuffers[0].available())
            {
                m_postBuffers[0].create(data.width, data.height, false);
            }

            if (m_postBuffers[1].available())
            {
                m_postBuffers[1].create(data.width, data.height, false);
            }*/

            //updates the view of the default camera
            getEntity(m_defaultCamera).getComponent<Camera>().setViewport(getDefaultViewport());
        }
    }
}

//private
void Scene::postRenderPath()
{
    /*auto camera = m_entityManager.getEntity(m_activeCamera);
    
    m_sceneBuffer.clear();
    for (auto r : m_renderables) r->render(camera);
    m_sceneBuffer.display();

    RenderTexture* inTex = &m_sceneBuffer;
    RenderTexture* outTex = nullptr;

    for (auto i = 0u; i < m_postEffects.size() - 1; ++i)
    {
        outTex = &m_postBuffers[i % 2];
        outTex->clear();
        m_postEffects[i]->apply(*inTex);
        outTex->display();
        inTex = outTex;
    }


    auto vp = m_sceneBuffer.getDefaultViewport();
    glViewport(vp.left, vp.bottom, vp.width, vp.height);
    m_postEffects.back()->apply(*inTex);*/
}

void Scene::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    auto view = getEntity(m_activeCamera).getComponent<Camera>().m_view;
    DPRINT("View position", std::to_string(view.getCenter().x) + ", " + std::to_string(view.getCenter().y));
    
    rt.setView(view);
    currentRenderPath(rt, states);
}