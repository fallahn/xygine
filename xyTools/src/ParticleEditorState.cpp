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

#include <ParticleEditorState.hpp>

#include <xygine/App.hpp>
#include <xygine/imgui/imgui.h>

#include <xygine/Entity.hpp>
#include <xygine/components/ParticleSystem.hpp>
#include <xygine/components/CallbackProvider.hpp>

namespace
{
    xy::ParticleSystem* particleSystem = nullptr;

    bool animated = false;
    float initialVelocity[2] = {0.f, 0.f};
}

ParticleEditorState::ParticleEditorState(xy::StateStack& stack, Context context)
    :xy::State  (stack, context),
    m_messageBus(context.appInstance.getMessageBus()),
    m_scene     (m_messageBus)
{
    launchLoadingScreen();
    setupScene();
    buildMenu();
    quitLoadingScreen();
}

ParticleEditorState::~ParticleEditorState()
{
    xy::App::removeUserWindows(this);
    getContext().renderWindow.setTitle("xy tools");
}

//public
bool ParticleEditorState::update(float dt)
{
    applySettings();
    m_scene.update(dt);
    return true;
}

void ParticleEditorState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.draw(m_scene);
}

bool ParticleEditorState::handleEvent(const sf::Event&)
{
    return true;
}

void ParticleEditorState::handleMessage(const xy::Message&)
{

}

//private
void ParticleEditorState::buildMenu()
{
    xy::App::addUserWindow([this]()
    {
        nim::SetNextWindowSizeConstraints({ 200.f, 200.f }, { 800.f, 600.f });
        nim::Begin("Particle Editor!");

        static bool animate = false;
        nim::Checkbox("Animate", &animate);
        if (animate != animated)
        {
            //TODO set animated parameter
        }
        animated = animate;

        nim::InputFloat2("Initial Velocity", initialVelocity);


        nim::End();
    }, this);
}

void ParticleEditorState::setupScene()
{
    auto ps = xy::Component::create<xy::ParticleSystem>(m_messageBus);
    ps->start();
    auto entity = xy::Entity::create(m_messageBus);
    entity->setPosition(xy::DefaultSceneSize / 2.f);
    particleSystem = entity->addComponent(ps);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontRear);
}

void ParticleEditorState::applySettings()
{
    XY_ASSERT(particleSystem, "particle system not yet created");
    particleSystem->setInitialVelocity({ initialVelocity[0], initialVelocity[1] });
}