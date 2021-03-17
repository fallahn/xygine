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

#include "../detail/GLCheck.hpp"

#include "xyginext/core/App.hpp"
#include "xyginext/ecs/Scene.hpp"
#include "xyginext/ecs/components/Camera.hpp"
#include "xyginext/ecs/components/Transform.hpp"
#include "xyginext/ecs/components/AudioListener.hpp"

#include <SFML/Window/Event.hpp>

using namespace xy;

namespace
{
    sf::FloatRect getDefaultViewport()
    {
        if (App::getRenderWindow())
        {
            auto winSize = sf::Vector2f(App::getRenderWindow()->getSize());

            float windowRatio = winSize.x / winSize.y;
            float viewRatio = DefaultSceneSize.x / DefaultSceneSize.y;

            float sizeY = windowRatio / viewRatio;
            float top = (1.f - sizeY) / 2.f;

            return { { 0.f, top },{ 1.f, sizeY } };
        }
        return {};
    }
}

Scene::Scene(MessageBus& mb, std::size_t poolSize)
    : m_messageBus      (mb),
    m_entityManager     (mb, m_componentManager, poolSize),
    m_systemManager     (*this, m_componentManager)
{
    auto defaultCamera = createEntity();
    defaultCamera.addComponent<Transform>().setPosition(xy::DefaultSceneSize / 2.f);
    defaultCamera.addComponent<Camera>();
    defaultCamera.getComponent<Camera>().setViewport(getDefaultViewport());
    defaultCamera.addComponent<AudioListener>();

    m_defaultCamera = defaultCamera;
    m_activeCamera = m_defaultCamera;
    m_activeListener = m_defaultCamera;

    currentRenderPath = [this](sf::RenderTarget& rt, sf::RenderStates states)
    {
        rt.setView(m_activeCamera.getComponent<Camera>().m_view);
        for (auto r : m_drawables)
        {
            rt.draw(*r, states);
        }
    };
}

//public
void Scene::update(float dt)
{
    //update directors first as they'll be working on data from the previous frame
    for (auto& d : m_directors)
    {
        d->process(dt);
    }

    //destroyed entities are double buffered so
    //that removeFromSystems() (which calls System::onEntityRemoved())
    //can safely delete further entities without modifying this vector
    m_destroyedEntities.swap(m_destroyedBuffer);
    for (auto entity : m_destroyedEntities)
    {
        m_systemManager.removeFromSystems(entity);
        m_entityManager.destroyEntity(entity);
    }
    m_destroyedEntities.clear();


    for (auto entity : m_pendingEntities)
    {
        m_systemManager.addToSystems(entity);
    }
    m_pendingEntities.clear();


    m_systemManager.process(dt);
    for (auto& p : m_postEffects) p->update(dt);
}

Entity Scene::createEntity()
{
    m_pendingEntities.push_back(m_entityManager.createEntity());
    return m_pendingEntities.back();
}

void Scene::destroyEntity(Entity entity)
{
    m_destroyedBuffer.push_back(entity);
    entity.m_destroyed = true;
}

Entity Scene::getEntity(Entity::ID id) const
{
    return m_entityManager.getEntity(id);
}

void Scene::setPostEnabled(bool enabled)
{
    if (enabled && !m_postEffects.empty())
    {
        currentRenderPath = std::bind(&Scene::postRenderPath, this, std::placeholders::_1, std::placeholders::_2);
        
        XY_ASSERT(App::getRenderWindow(), "no valid window");
        auto size = App::getRenderWindow()->getSize();
        m_sceneBuffer.create(size.x, size.y, sf::ContextSettings(24));
        for (auto& p : m_postEffects) p->resizeBuffer(size.x, size.y);
    }
    else
    {       
        currentRenderPath = [this](sf::RenderTarget& rt, sf::RenderStates states)
        {
            rt.setView(m_activeCamera.getComponent<Camera>().m_view);
            for (auto r : m_drawables)
            {
                rt.draw(*r, states);
            }
        };
    }
}

Entity Scene::getDefaultCamera() const
{
    return m_defaultCamera;
}

Entity Scene::setActiveCamera(Entity entity)
{
    XY_ASSERT(entity.hasComponent<Transform>() && entity.hasComponent<Camera>(), "Entity requires at least a transform and a camera component");
    XY_ASSERT(m_entityManager.owns(entity), "This entity must belong to this scene!");
    auto oldCam = m_activeCamera;
    m_activeCamera = entity;

    return oldCam;
}

Entity Scene::setActiveListener(Entity entity)
{
    XY_ASSERT(entity.hasComponent<Transform>() && entity.hasComponent<AudioListener>(), "Entity requires at least a transform and a camera component");
    XY_ASSERT(m_entityManager.owns(entity), "This entity must belong to this scene!");
    auto oldListener = m_activeListener;
    m_activeListener = entity;
    return oldListener;
}

Entity Scene::getActiveListener() const
{
    return m_activeListener;
}

Entity Scene::getActiveCamera() const
{
    return m_activeCamera;
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
            //update post effect buffers if they exist
            if (m_sceneBuffer.getTexture().getNativeHandle() > 0)
            {
                m_sceneBuffer.create(data.width, data.height, sf::ContextSettings(24));

                for (auto& b : m_postBuffers)
                {
                    if (b.getTexture().getNativeHandle() > 0)
                    {
                        b.create(data.width, data.height);
                    }
                }
            }
            //updates the view of the default camera
            m_defaultCamera.getComponent<Camera>().setViewport(getDefaultViewport());
        }
    }
   
}

//private
void Scene::postRenderPath(sf::RenderTarget& rt, sf::RenderStates states)
{
    auto activeView = m_activeCamera.getComponent<Camera>().m_view;

    m_sceneBuffer.setView(activeView);

    //TODO if rendering multiple/split views
    //this needs to be done for each camera
    //in this loop - this doesn't affect the
    //default render path, however
    //m_sceneBuffer.clear(sf::Color::Transparent);
    m_sceneBuffer.setActive();
    glCheck(glClearColor(0.f, 0.f, 0.f, 0.f));
    glCheck(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    for (auto r : m_drawables)
    {
        m_sceneBuffer.draw(*r, states);
    }
    m_sceneBuffer.display();

    sf::RenderTexture* inTex = &m_sceneBuffer;
    sf::RenderTexture* outTex = nullptr;

    for (auto i = 0u; i < m_postEffects.size() - 1; ++i)
    {
        outTex = &m_postBuffers[i % 2];
        outTex->clear();
        m_postEffects[i]->apply(*inTex, *outTex);
        outTex->display();
        inTex = outTex;
    }

    rt.setView(inTex->getDefaultView());
    m_postEffects.back()->apply(*inTex, rt);

    rt.setView(activeView);
}

void Scene::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    currentRenderPath(rt, states);
}
