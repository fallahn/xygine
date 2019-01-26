/*********************************************************************
(c) Jonny Paton 2018
(c) Matt Marchant 2019

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

#include "MyFirstState.hpp"

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/ParticleEmitter.hpp>
#include <xyginext/ecs/components/Camera.hpp>

#include <xyginext/ecs/systems/ParticleSystem.hpp>
#include <xyginext/gui/Gui.hpp>
#include <xyginext/core/FileSystem.hpp>

namespace
{
    const float ItemWidth = 160.f;
    const float WindowWidth = 350.f;
    const float WindowHeight = 680.f;
}

MyFirstState::MyFirstState(xy::StateStack& ss, xy::State::Context ctx)
    : xy::State         (ss,ctx),
    m_scene             (ctx.appInstance.getMessageBus()),
    m_emitterSettings   (nullptr),
    m_selectedBlendMode (0),
    m_workingDirectory  ("Working Directory")
{
    setup();

    m_scene.getActiveCamera().getComponent<xy::Camera>().setView(ctx.defaultView.getSize());
    m_scene.getActiveCamera().getComponent<xy::Camera>().setViewport(ctx.defaultView.getViewport());
}

bool MyFirstState::handleEvent(const sf::Event& evt)
{
    m_scene.forwardEvent(evt);
    return true;
}

void MyFirstState::handleMessage(const xy::Message& msg)
{
    m_scene.forwardMessage(msg);
}

bool MyFirstState::update(float dt)
{
    m_scene.update(dt);
    return true;
}

void MyFirstState::draw()
{
    auto rw = getContext().appInstance.getRenderWindow();
    rw->draw(m_scene);
}


//private
void MyFirstState::setup()
{
    auto& mb = xy::App::getActiveInstance()->getMessageBus();

    m_scene.addSystem<xy::ParticleSystem>(mb);


    auto entity = m_scene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
    entity.getComponent<xy::Transform>().move(100.f, 0.f);
    entity.addComponent<xy::ParticleEmitter>().start();

    m_emitterSettings = &entity.getComponent<xy::ParticleEmitter>().settings;

    auto windowFunc = [&, entity]() mutable
    {
        xy::Nim::setNextWindowSize(WindowWidth, WindowHeight);
        xy::Nim::setNextWindowConstraints(WindowWidth, WindowHeight, WindowWidth, WindowHeight);
        xy::Nim::begin("Emitter Settings");

        xy::Nim::text(m_workingDirectory);
        if (xy::Nim::button("Browse Path"))
        {
            auto path = xy::FileSystem::openFolderDialogue();
            if (!path.empty())
            {
                m_workingDirectory = path;

                //try trimming the loaded texture path
                if (!m_emitterSettings->texturePath.empty())
                {
                    if (m_emitterSettings->texturePath.find(path) != std::string::npos)
                    {
                        m_emitterSettings->texturePath = m_emitterSettings->texturePath.substr(path.size());
                    }
                }
            }
        }
        xy::Nim::sameLine(); xy::Nim::showToolTip("Current working directory. Set this to your project directory and textures will be loaded and saved in a path relative to this");
        xy::Nim::separator();

        xy::Nim::slider("Gravity X", m_emitterSettings->gravity.x, -1000.f, 1000.f, ItemWidth);
        xy::Nim::sameLine(); xy::Nim::showToolTip("Gravitational force applied to the velocity");
        xy::Nim::slider("Gravity Y", m_emitterSettings->gravity.y, -1000.f, 1000.f, ItemWidth);
        
        xy::Nim::slider("Velocity X", m_emitterSettings->initialVelocity.x, -1000.f, 1000.f, ItemWidth);
        xy::Nim::sameLine(); xy::Nim::showToolTip("Initial velocity of the particle");
        xy::Nim::slider("Velocity Y", m_emitterSettings->initialVelocity.y, -1000.f, 1000.f, ItemWidth);

        xy::Nim::slider("Spread", m_emitterSettings->spread, 0.f, 360.f, ItemWidth);
        xy::Nim::sameLine(); xy::Nim::showToolTip("Spead, in degrees, applied to the inital velocity");
        xy::Nim::slider("Lifetime", m_emitterSettings->lifetime, 0.1f, 10.f, ItemWidth);
        xy::Nim::sameLine(); xy::Nim::showToolTip("Lifetime of a particle in seconds");
        xy::Nim::slider("Lifetime Variance", m_emitterSettings->lifetimeVariance, 0.f, 10.f, ItemWidth);
        xy::Nim::sameLine(); xy::Nim::showToolTip("Amount of random variation added to the lifetime of a particle, in seconds");

        xy::Nim::slider("Rotation Speed", m_emitterSettings->rotationSpeed, 0.f, 15.f, ItemWidth);
        xy::Nim::sameLine(); xy::Nim::showToolTip("Rotation in degrees per second - textured particles only");
        xy::Nim::slider("Scale Affector", m_emitterSettings->scaleModifier, -5.f, 5.f, ItemWidth);
        xy::Nim::sameLine(); xy::Nim::showToolTip("How rapidly a particle is scaled in size over its lifetime");
        xy::Nim::slider("Size", m_emitterSettings->size, 0.1f, 100.f, ItemWidth);
        xy::Nim::sameLine(); xy::Nim::showToolTip("Initial size of a particle");

        xy::Nim::slider("Emit Rate", m_emitterSettings->emitRate, 0.f, 150.f, ItemWidth);
        xy::Nim::sameLine(); xy::Nim::showToolTip("Number of particles emitted per second");
        std::int32_t count = m_emitterSettings->emitCount;
        xy::Nim::input("Emit Count", count, ItemWidth);
        xy::Nim::sameLine(); xy::Nim::showToolTip("Number of particles emitted simultaneously");
        count = std::max(count, 0);
        m_emitterSettings->emitCount = count;

        xy::Nim::slider("Spawn Radius", m_emitterSettings->spawnRadius, 0.f, 500.f, ItemWidth);
        xy::Nim::sameLine(); xy::Nim::showToolTip("Radius around the emitter position in which particles are spawned");
        xy::Nim::slider("Spawn Offset X", m_emitterSettings->spawnOffset.x, 0.f, 500.f, ItemWidth);
        xy::Nim::sameLine(); xy::Nim::showToolTip("Offsets the particle spawn position from the emitter position in world units");
        xy::Nim::slider("Spawn Offset Y", m_emitterSettings->spawnOffset.y, 0.f, 500.f, ItemWidth);

        count = m_emitterSettings->releaseCount;
        xy::Nim::input("Release Count", count, ItemWidth);
        xy::Nim::sameLine(); xy::Nim::showToolTip("Total number of particles to release before automatically stopping the emitter. 0 emits indefinitely, restart the emitter for updated values to take effect");
        count = std::max(count, 0);
        m_emitterSettings->releaseCount = count;

        xy::Nim::checkbox("Random Initial Rotation", &m_emitterSettings->randomInitialRotation);
        xy::Nim::sameLine(); xy::Nim::showToolTip("Applies a random initial rotation to spawned particles. Textured particles only");
        
        xy::Nim::colourPicker("Colour", m_emitterSettings->colour);

        xy::Nim::separator();

        //blendmode drop down
        std::int32_t idx = m_selectedBlendMode;
        xy::Nim::simpleCombo("Blend Mode", idx, "Alpha\0Add\0Multiply\0\0", ItemWidth);
        if (idx != m_selectedBlendMode)
        {
            m_selectedBlendMode = idx;
            switch (idx)
            {
            case 0:
                m_emitterSettings->blendmode = sf::BlendAlpha;
                break;
            case 1:
                m_emitterSettings->blendmode = sf::BlendAdd;
                break;
            case 2:
                m_emitterSettings->blendmode = sf::BlendMultiply;
            }
        }
        if (m_emitterSettings->texturePath.empty())
        {
            xy::Nim::text("No texture loaded");
        }
        else
        {
            xy::Nim::text(m_emitterSettings->texturePath);
        }

        if (xy::Nim::button("Browse Texture"))
        {
            auto path = xy::FileSystem::openFileDialogue("png,jpg,bmp");
            if (!path.empty())
            {
                m_emitterSettings->texture = &m_textures.get(path);

                //try correcting with current working directory
                if (!m_workingDirectory.empty())
                {
                    if (path.find(m_workingDirectory) != std::string::npos)
                    {
                        path = path.substr(m_workingDirectory.size());
                    }
                }
                m_emitterSettings->texturePath = path;
            }
        }
        xy::Nim::sameLine(); xy::Nim::showToolTip("For a relative path to a texture set the working directory, above");

        xy::Nim::separator();

        if (xy::Nim::button("Start"))
        {
            entity.getComponent<xy::ParticleEmitter>().start();
        }
        xy::Nim::sameLine();
        if (xy::Nim::button("Stop"))
        {
            entity.getComponent<xy::ParticleEmitter>().stop();
        }
        xy::Nim::sameLine();
        if (xy::Nim::button("Reset"))
        {
            entity.getComponent<xy::ParticleEmitter>().settings = xy::EmitterSettings();
        }
        xy::Nim::sameLine(); xy::Nim::showToolTip("Reset the properties to their default values");

        xy::Nim::separator();

        //load button
        //save button
        if (xy::Nim::button("Load"))
        {
            auto path = xy::FileSystem::openFileDialogue("xyp");
            if (!path.empty())
            {
                entity.getComponent<xy::ParticleEmitter>().settings = xy::EmitterSettings();
                m_textures.setFallbackColour(sf::Color::White);
                m_emitterSettings->loadFromFile(path, m_textures);
                {
                    if (m_workingDirectory.empty())
                    {
                        xy::Logger::log("Working directory not set, textures may not be loaded");
                    }
                    else if(!m_emitterSettings->texturePath.empty())
                    {
                        xy::Logger::log("Trying to correct for texture path...");
                        auto texPath = m_workingDirectory;
                        std::replace(texPath.begin(), texPath.end(), '\\', '/');
                        if (texPath.back() != '/')
                        {
                            texPath += "/";
                        }
                        texPath += m_emitterSettings->texturePath;
                        m_emitterSettings->texture = &m_textures.get(texPath);
                    }
                }
                entity.getComponent<xy::ParticleEmitter>().stop();
            }
        }
        xy::Nim::sameLine();
        if (xy::Nim::button("Save"))
        {
            auto path = xy::FileSystem::saveFileDialogue("xyp");
            if (!path.empty())
            {
                if (xy::FileSystem::getFileExtension(path) != ".xyp")
                {
                    path += ".xyp";
                }
                m_emitterSettings->saveToFile(path);
            }
        }

        xy::Nim::end();
    };
    registerWindow(windowFunc);
}