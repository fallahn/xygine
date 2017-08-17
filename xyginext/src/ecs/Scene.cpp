/*-----------------------------------------------------------------------

Matt Marchant 2017
http://trederia.blogspot.com

crogine - Zlib license.

This software is provided 'as-is', without any express or
implied warranty.In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions :

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.

-----------------------------------------------------------------------*/

#include <xyginext/ecs/Scene.hpp>
//#include <xyginext/ecs/components/Camera.hpp>
//#include <xyginext/ecs/components/Transform.hpp>
//#include <xyginext/ecs/components/AudioListener.hpp>
#include <xyginext/ecs/Renderable.hpp>

#include <xyginext/core/App.hpp>


using namespace xy;

Scene::Scene(MessageBus& mb)
    : m_messageBus      (mb),
    m_entityManager     (mb),
    m_systemManager     (*this)
{
    //auto defaultCamera = createEntity();
    //defaultCamera.addComponent<Transform>();
    //defaultCamera.addComponent<Camera>();
    //defaultCamera.addComponent<AudioListener>();

    //m_defaultCamera = defaultCamera.getIndex();
    //m_activeCamera = m_defaultCamera;
    //m_activeListener = m_defaultCamera;

    currentRenderPath = [this]()
    {
        auto camera = m_entityManager.getEntity(m_activeCamera);
        for (auto r : m_renderables) r->render(camera);
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


    updateFrustum();

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
//            auto camera = m_entityManager.getEntity(m_activeCamera);
//            for (auto& r : m_renderables) r->render(camera);
//        };
//    }
//}

Entity Scene::getDefaultCamera() const
{
    return m_entityManager.getEntity(m_defaultCamera);
}

Entity Scene::setActiveCamera(Entity entity)
{
    //CRO_ASSERT(entity.hasComponent<Transform>() && entity.hasComponent<Camera>(), "Entity requires at least a transform and a camera component");
    //CRO_ASSERT(m_entityManager.owns(entity), "This entity must belong to this scene!");
    //auto oldCam = m_entityManager.getEntity(m_activeCamera);
    //m_activeCamera = entity.getIndex();
    //updateFrustum();
    //return oldCam;
    return m_entityManager.getEntity(m_defaultCamera);
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

    //if (msg.id == Message::WindowMessage)
    //{
    //    const auto& data = msg.getData<Message::WindowEvent>();
    //    if (data.event == SDL_WINDOWEVENT_SIZE_CHANGED)
    //    {
    //        //resizes the post effect buffer if it is in use
    //        if (m_sceneBuffer.available())
    //        {
    //            m_sceneBuffer.create(data.data0, data.data1);
    //            for (auto& p : m_postEffects) p->resizeBuffer(data.data0, data.data1);
    //        }

    //        if (m_postBuffers[0].available())
    //        {
    //            m_postBuffers[0].create(data.data0, data.data1, false);
    //        }

    //        if (m_postBuffers[1].available())
    //        {
    //            m_postBuffers[1].create(data.data0, data.data1, false);
    //        }
    //    }
    //}
}

void Scene::render()
{
    currentRenderPath();
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

void Scene::updateFrustum()
{
    //auto activeCamera = getActiveCamera();
    //auto& camComponent = activeCamera.getComponent<Camera>();
    //auto viewProj = camComponent.projection
    //    * glm::inverse(activeCamera.getComponent<Transform>().getWorldTransform());

    //camComponent.m_frustum =
    //{
    //    { Plane //left
    //    (
    //        viewProj[0][3] + viewProj[0][0],
    //        viewProj[1][3] + viewProj[1][0],
    //        viewProj[2][3] + viewProj[2][0],
    //        viewProj[3][3] + viewProj[3][0]
    //    ),
    //    Plane //right
    //    (
    //        viewProj[0][3] - viewProj[0][0],
    //        viewProj[1][3] - viewProj[1][0],
    //        viewProj[2][3] - viewProj[2][0],
    //        viewProj[3][3] - viewProj[3][0]
    //    ),
    //    Plane //bottom
    //    (
    //        viewProj[0][3] + viewProj[0][1],
    //        viewProj[1][3] + viewProj[1][1],
    //        viewProj[2][3] + viewProj[2][1],
    //        viewProj[3][3] + viewProj[3][1]
    //    ),
    //    Plane //top
    //    (
    //        viewProj[0][3] - viewProj[0][1],
    //        viewProj[1][3] - viewProj[1][1],
    //        viewProj[2][3] - viewProj[2][1],
    //        viewProj[3][3] - viewProj[3][1]
    //    ),
    //    Plane //near
    //    (
    //        viewProj[0][3] + viewProj[0][2],
    //        viewProj[1][3] + viewProj[1][2],
    //        viewProj[2][3] + viewProj[2][2],
    //        viewProj[3][3] + viewProj[3][2]
    //    ),
    //    Plane //far
    //    (
    //        viewProj[0][3] - viewProj[0][2],
    //        viewProj[1][3] - viewProj[1][2],
    //        viewProj[2][3] - viewProj[2][2],
    //        viewProj[3][3] - viewProj[3][2]
    //    ) }
    //};

    ////normalise the planes
    //for (auto& p : camComponent.m_frustum)
    //{
    //    const float factor = 1.f / std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
    //    p.x *= factor;
    //    p.y *= factor;
    //    p.z *= factor;
    //    p.w *= factor;
    //}
}