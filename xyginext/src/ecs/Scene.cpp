/*********************************************************************
(c) Matt Marchant 2017 - 2018
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

#include "xyginext/core/App.hpp"
#include "xyginext/core/Editor.hpp"
#include "xyginext/core/ConfigFile.hpp"
#include "xyginext/ecs/Scene.hpp"
#include "xyginext/ecs/components/Text.hpp"
#include "xyginext/ecs/components/Camera.hpp"
#include "xyginext/ecs/components/Drawable.hpp"
#include "xyginext/ecs/components/Transform.hpp"
#include "xyginext/ecs/components/AudioListener.hpp"
#include "xyginext/ecs/components/ParticleEmitter.hpp"
#include "xyginext/ecs/components/AudioEmitter.hpp"
#include "xyginext/ecs/components/SpriteAnimation.hpp"
#include "xyginext/ecs/components/UIHitBox.hpp"
#include "xyginext/ecs/components/Sprite.hpp"

#include "xyginext/ecs/systems/RenderSystem.hpp"
#include "xyginext/ecs/systems/CameraSystem.hpp"
#include "xyginext/ecs/systems/SpriteSystem.hpp"
#include "xyginext/ecs/systems/TextRenderer.hpp"
#include "xyginext/ecs/systems/UISystem.hpp"

#include "cereal/archives/binary.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/bitset.hpp"

#include <SFML/Window/Event.hpp>

#include "../imgui/imgui_dock.hpp"

using namespace xy;

namespace
{
    sf::FloatRect getDefaultViewport()
    {
        if (App::getRenderTarget())
        {
            auto winSize = sf::Vector2f(App::getRenderTarget()->getSize());

            float windowRatio = winSize.x / winSize.y;
            float viewRatio = DefaultSceneSize.x / DefaultSceneSize.y;

            float sizeY = windowRatio / viewRatio;
            float top = (1.f - sizeY) / 2.f;

            return { { 0.f, top },{ 1.f, sizeY } };
        }
        return {};
    }
}

Scene::Scene(MessageBus& mb)
    : m_messageBus      (mb),
    m_entityManager     (mb, m_componentManager),
    m_systemManager     (*this, m_componentManager)
{
    auto defaultCamera = createEntity();
    defaultCamera.addComponent<Transform>().setPosition(xy::DefaultSceneSize / 2.f);
    defaultCamera.addComponent<Camera>();
    defaultCamera.getComponent<Camera>().setViewport(getDefaultViewport());
    defaultCamera.addComponent<AudioListener>();

    m_defaultCamera = defaultCamera.getIndex();
    m_activeCamera = m_defaultCamera;
    m_activeListener = m_defaultCamera;

    currentRenderPath = [this](sf::RenderTarget& rt, sf::RenderStates states)
    {
        rt.setView(getEntity(m_activeCamera).getComponent<Camera>().m_view);
        for (auto r : m_drawables)
        {
            rt.draw(*r, states);
        }
    };
    
    // Silly/lazy hack to try and enforce component ID consistency.
    // This means scene loading (probably) won't work when the editor is not
    // being used. Quite essential that this is fixed
    m_componentManager.getID<Sprite>();
    m_componentManager.getID<Text>();
    m_componentManager.getID<ParticleEmitter>();
    m_componentManager.getID<AudioEmitter>();
    m_componentManager.getID<SpriteAnimation>();
    m_componentManager.getID<UIHitBox>();
}

Scene::Scene(MessageBus& mb, const std::string& path) :
Scene(mb)
{
    loadFromFile(path);
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
    for (auto& p : m_postEffects) p->update(dt);
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

void Scene::setPostEnabled(bool enabled)
{
    if (enabled && !m_postEffects.empty())
    {
        currentRenderPath = std::bind(&Scene::postRenderPath, this, std::placeholders::_1, std::placeholders::_2);
        
        XY_ASSERT(App::getRenderTarget(), "no valid window");
        auto size = App::getRenderTarget()->getSize();
        m_sceneBuffer.create(size.x, size.y, xy::App::getActiveInstance()->getVideoSettings().ContextSettings);
        for (auto& p : m_postEffects) p->resizeBuffer(size.x, size.y);
    }
    else
    {       
        currentRenderPath = [this](sf::RenderTarget& rt, sf::RenderStates states)
        {
            rt.setView(getEntity(m_activeCamera).getComponent<Camera>().m_view);
            for (auto r : m_drawables)
            {
                rt.draw(*r, states);
            }
        };
    }
}

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
}

Entity Scene::setActiveListener(Entity entity)
{
    XY_ASSERT(entity.hasComponent<Transform>() && entity.hasComponent<AudioListener>(), "Entity requires at least a transform and a camera component");
    XY_ASSERT(m_entityManager.owns(entity), "This entity must belong to this scene!");
    auto oldListener = m_entityManager.getEntity(m_activeListener);
    m_activeListener = entity.getIndex();
    return oldListener;
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
            //update post effect buffers if they exist
            if (m_sceneBuffer.getTexture().getNativeHandle() > 0)
            {
                m_sceneBuffer.create(data.width, data.height);

                for (auto& b : m_postBuffers)
                {
                    if (b.getTexture().getNativeHandle() > 0)
                    {
                        b.create(data.width, data.height);
                    }
                }
            }
            //updates the view of the default camera
            getEntity(m_defaultCamera).getComponent<Camera>().setViewport(getDefaultViewport());
        }
    }
}

bool Scene::saveToFile(const std::string &path)
{
    std::ofstream os(path, std::ios::binary);
    if (!os.is_open() || !os.good())
    {
        std::string error = strerror(errno);
        xy::Logger::log("Error saving scene: " + error, xy::Logger::Type::Error);
        return false;
    }
    cereal::BinaryOutputArchive archive(os);
    
    // Store systems
    auto& systems = m_systemManager.getSystems();
    archive(systems.size());
    for (auto& s : systems)
    {
        // flaky af
        archive(std::string(s->getType().name()));
    }
    
    auto entities = getSystem<EditorSystem>().getEntities();
    archive(entities.size());
    
    for (auto& e : entities)
    {
        // Ignore first entity - it's a default one and will already be created
        if (e.getIndex() == 0)
        {
            continue;
        }
        
        archive(e.getIndex());
        
        auto mask = e.getComponentMask();
        archive(mask);
        
        // Need a smarter/generic way of doing this for any/all component types
        for (auto i=0u; i < mask.size(); i++)
        {
            if (mask.test(i))
            {
                if (i == m_componentManager.getID<xy::Text>())
                {
                    archive(e.getComponent<xy::Text>());
                }
                else if (i == m_componentManager.getID<xy::Drawable>())
                {
                    archive(e.getComponent<xy::Drawable>());
                }

                else if (i == m_componentManager.getID<xy::Transform>())
                {
                    archive(e.getComponent<xy::Transform>());
                }
                else if (i == m_componentManager.getID<xy::Sprite>())
                {
                    archive(e.getComponent<xy::Sprite>());
                }
                else if (i == m_componentManager.getID<xy::Camera>())
                {
                    archive(e.getComponent<xy::Camera>());
                }
                else if (i == m_componentManager.getID<xy::AudioListener>())
                {
                    archive(e.getComponent<xy::AudioListener>());
                }
                else if (i == m_componentManager.getID<xy::ParticleEmitter>())
                {
                    archive(e.getComponent<xy::ParticleEmitter>());
                }
                else if (i == m_componentManager.getID<xy::AudioEmitter>())
                {
                    archive(e.getComponent<xy::AudioEmitter>());
                }
                else if (i == m_componentManager.getID<xy::SpriteAnimation>())
                {
                    archive(e.getComponent<xy::SpriteAnimation>());
                }
                else if (i == m_componentManager.getID<xy::UIHitBox>())
                {
                    archive(e.getComponent<xy::UIHitBox>());
                }
                else
                {
                    xy::Logger::log("Component id " + std::to_string(i) + " not able to save in scene");
                }
            }
        }
    }
    return true;
}

bool Scene::loadFromFile(const std::string &path)
{
    std::ifstream is(path, std::ios::binary);
    if (!is.good() || !is.is_open())
    {
        // Try loading from resource folder (MACOS)
        is.open(xy::FileSystem::getResourcePath() + path);
        if (!is.good() || !is.is_open())
        {
            std::string error = strerror(errno);
            xy::Logger::log("Error loading scene: " + error, xy::Logger::Type::Error);
            return false;
        }
    }
    
    cereal::BinaryInputArchive archive(is);
    
    size_t systemCount;
    archive(systemCount);
    while(systemCount--)
    {
        std::string name;
        archive(name);
     
        // this is madness
        auto& mb = App::getActiveInstance()->getMessageBus();
        RenderSystem rs(mb);
        SpriteSystem ss(mb);
        EditorSystem es(mb);
        TextRenderer tr(mb);
        CameraSystem cs(mb);
        UISystem     us(mb);
        if (rs.getType().name() == name)
        {
            addSystem<RenderSystem>(mb);
        }
        else if (ss.getType().name() == name)
        {
            addSystem<SpriteSystem>(mb);
        }
        else if (es.getType().name() == name)
        {
            addSystem<EditorSystem>(mb, path);
        }
        else if (tr.getType().name() == name)
        {
            addSystem<TextRenderer>(mb);
        }
        else if (cs.getType().name() == name)
        {
            addSystem<CameraSystem>(mb);
        }
        else if (us.getType().name() == name)
        {
            addSystem<UISystem>(mb);
        }
        else
        {
            xy::Logger::log("System " + name + " loaded from scene " + path + " not recognised");
        }
    }
    
    size_t entityCount;
    archive(entityCount);
    
    // less 1 entity, for default one
    entityCount -= 1;
    
    while(entityCount--)
    {
        Entity::ID idx;
        archive(idx);
        auto e = m_entityManager.createEntity(idx);
        
        ComponentMask mask;
        archive(mask);
        
        for (auto i=0u; i < mask.size(); i++)
        {
            if (mask.test(i))
            {
                if (i == m_componentManager.getID<xy::Text>())
                {
                    archive(e.addComponent<xy::Text>());
                }
                else if (i == m_componentManager.getID<xy::Drawable>())
                {
                    archive(e.addComponent<xy::Drawable>());
                }
                else if (i == m_componentManager.getID<xy::Transform>())
                {
                    archive(e.addComponent<xy::Transform>());
                }
                else if (i == m_componentManager.getID<xy::Sprite>())
                {
                    archive(e.addComponent<xy::Sprite>());
                }
                else if (i == m_componentManager.getID<xy::Camera>())
                {
                    archive(e.addComponent<xy::Camera>());
                }
                else if (i == m_componentManager.getID<xy::AudioListener>())
                {
                    archive(e.addComponent<xy::AudioListener>());
                }
                else if (i == m_componentManager.getID<xy::ParticleEmitter>())
                {
                    archive(e.addComponent<xy::ParticleEmitter>());
                }
                else if (i == m_componentManager.getID<xy::AudioEmitter>())
                {
                    archive(e.addComponent<xy::AudioEmitter>());
                }
                else if (i == m_componentManager.getID<xy::SpriteAnimation>())
                {
                    archive(e.addComponent<xy::SpriteAnimation>());
                }
                else if (i == m_componentManager.getID<xy::UIHitBox>())
                {
                    archive(e.addComponent<xy::UIHitBox>());
                }
                else
                {
                    xy::Logger::log("Component id " + std::to_string(i) + " not able to load from scene");
                }
            }
        }
        
        m_systemManager.addToSystems(e);
    }
    return true;
}

//private
void Scene::postRenderPath(sf::RenderTarget& rt, sf::RenderStates states)
{
    m_sceneBuffer.setView(getEntity(m_activeCamera).getComponent<Camera>().m_view);

    m_sceneBuffer.clear();
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
}

void Scene::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    currentRenderPath(rt, states);
}
