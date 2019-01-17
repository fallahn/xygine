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

MyFirstState::MyFirstState(xy::StateStack& ss, xy::State::Context ctx)
    : xy::State         (ss,ctx),
    m_scene             (ctx.appInstance.getMessageBus()),
    m_emitterSettings   (nullptr)
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
        xy::Nim::setNextWindowSize(400.f, 550.f);
        xy::Nim::setNextWindowConstraints(400.f, 550.f, 400.f, 550.f);
        xy::Nim::begin("Emitter Settings");

        xy::Nim::slider("Gravity X", m_emitterSettings->gravity.x, -1000.f, 1000.f);
        xy::Nim::slider("Gravity Y", m_emitterSettings->gravity.y, -1000.f, 1000.f);
        
        xy::Nim::slider("Velocity X", m_emitterSettings->initialVelocity.x, -1000.f, 1000.f);
        xy::Nim::slider("Velocity Y", m_emitterSettings->initialVelocity.y, -1000.f, 1000.f);

        xy::Nim::slider("Spread", m_emitterSettings->spread, 0.f, 360.f);
        xy::Nim::slider("Lifetime", m_emitterSettings->lifetime, 0.f, 10.f);
        xy::Nim::slider("Lifetime Variance", m_emitterSettings->lifetimeVariance, 0.f, 10.f);

        xy::Nim::slider("Rotation Speed", m_emitterSettings->rotationSpeed, 0.f, 500.f);
        xy::Nim::slider("Scale Affector", m_emitterSettings->scaleModifier, 0.f, 5.f);
        xy::Nim::slider("Size", m_emitterSettings->size, 0.f, 600.f);

        xy::Nim::slider("Emit Rate", m_emitterSettings->emitRate, 0.f, 150.f);
        //TODO numeric up/down for emit count

        xy::Nim::slider("Spawn Radius", m_emitterSettings->spawnRadius, 0.f, 500.f);
        xy::Nim::slider("Spawn Offset X", m_emitterSettings->spawnOffset.x, 0.f, 500.f);
        xy::Nim::slider("Spawn Offset Y", m_emitterSettings->spawnOffset.y, 0.f, 500.f);

        //TODO numeric up/down for release count

        xy::Nim::checkbox("Random Initial Rotation", &m_emitterSettings->randomInitialRotation);
        //TODO colour picker

        if (xy::Nim::button("Reset"))
        {
            entity.getComponent<xy::ParticleEmitter>().settings = xy::EmitterSettings();
        }

        //TODO texture browser
        //TODO blendmode drop down
        //TODO start/stop buttons
        //TODO load button
        //TODO save button        

        xy::Nim::end();
    };
    registerWindow(windowFunc);
}