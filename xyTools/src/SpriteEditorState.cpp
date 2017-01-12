/*********************************************************************
Matt Marchant 2014 - 2017
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

#include <SpriteEditorState.hpp>
#include <AtlasWidow.hpp>

#include <xygine/App.hpp>
#include <xygine/Entity.hpp>
#include <xygine/components/AnimatedDrawable.hpp>
#include <xygine/imgui/CommonDialogues.hpp>
#include <xygine/imgui/imgui.h>

namespace
{
    enum CommandID
    {
        Atlas = 1,
        Animation = 2
    };
}

SpriteEditorState::SpriteEditorState(xy::StateStack& stack, Context context)
    :xy::State  (stack, context),
    m_messageBus(context.appInstance.getMessageBus()),
    m_scene     (m_messageBus)
{
    launchLoadingScreen();
    buildMenu();
    buildScene();
    context.appInstance.setWindowTitle("Sprite Editor");
    quitLoadingScreen();
}

SpriteEditorState::~SpriteEditorState()
{
    xy::App::removeUserWindows(this);
}

//public
bool SpriteEditorState::update(float dt)
{
    m_scene.update(dt);
    return true;
}

void SpriteEditorState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.draw(m_scene);
}

bool SpriteEditorState::handleEvent(const sf::Event&)
{
    return true;
}

void SpriteEditorState::handleMessage(const xy::Message& msg)
{
    m_scene.handleMessage(msg);

    if (msg.id == xy::Message::UIMessage)
    {
        const auto& data = msg.getData<xy::Message::UIEvent>();
        if (data.type == xy::Message::UIEvent::ResizedWindow)
        {
            m_scene.setView(getContext().defaultView);
        }
    }
}

//private
void SpriteEditorState::buildMenu()
{
    xy::App::addUserWindow([this]()
    {
        static std::string selectedFile;

        nim::Begin("Sprite Editor!");

        //browse sheet
        if (nim::fileBrowseDialogue("Select Model", selectedFile, nim::Button("Open Sprite Sheet"))
            && !selectedFile.empty())
        {
            xy::Command cmd;
            cmd.category = CommandID::Atlas;
            cmd.action = [this](xy::Entity& entity, float)
            {
                auto pv = entity.getComponent<AtlasWindow>();
                pv->setSpriteSheet(selectedFile);

                xy::Command cmd2;
                cmd2.category = CommandID::Animation;
                cmd2.action = [pv](xy::Entity& entity, float)
                {
                    auto an = entity.getComponent<xy::AnimatedDrawable>();
                    an->setTexture(pv->getTexture());
                };
                m_scene.sendCommand(cmd2);
            };
            m_scene.sendCommand(cmd);
        }

        nim::SameLine();
        static float scale = 1.f;
        float oldScale = scale;
        nim::DragFloat("Scale", &scale, 0.01f, 0.2f, 2.f);
        if (oldScale != scale)
        {
            xy::Command cmd;
            cmd.category = CommandID::Atlas;
            cmd.action = [](xy::Entity& entity, float)
            {
                entity.setScale(scale, scale);
            };
            m_scene.sendCommand(cmd);
        }


        //browse folder


        //frame count


        nim::End();
    }, this);
}

void SpriteEditorState::buildScene()
{
    m_scene.setView(getContext().defaultView);

    auto atlas = xy::Component::create<AtlasWindow>(m_messageBus, m_textureResource);
    auto entity = xy::Entity::create(m_messageBus);
    entity->setPosition(300.f, 20.f);
    entity->addCommandCategories(CommandID::Atlas);
    entity->addComponent(atlas);
    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);

    auto aniSprite = xy::Component::create<xy::AnimatedDrawable>(m_messageBus);
    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(1324.f, 20.f);
    entity->addCommandCategories(CommandID::Animation);
    entity->addComponent(aniSprite);
    m_scene.addEntity(entity, xy::Scene::Layer::UI);
}