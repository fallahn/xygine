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
    entity.addComponent<xy::ParticleEmitter>().start();

    m_emitterSettings = &entity.getComponent<xy::ParticleEmitter>().settings;

}