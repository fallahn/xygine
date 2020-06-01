/*********************************************************************
(c) Jonny Paton 2018
(c) Matt Marchant 2020

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

#include "SpriteState.hpp"

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Camera.hpp>
#include <xyginext/ecs/components/Callback.hpp>
#include <xyginext/ecs/components/Drawable.hpp>

#include <xyginext/ecs/systems/CallbackSystem.hpp>
#include <xyginext/ecs/systems/RenderSystem.hpp>

#include <xyginext/gui/Gui.hpp>
#include <xyginext/core/FileSystem.hpp>
#include <xyginext/util/Vector.hpp>
#include <xyginext/core/App.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/System/Err.hpp>

namespace
{
    const float ItemWidth = 160.f;
    const float WindowWidth = 350.f;
    const float WindowHeight = 680.f;

    const std::string cfgPath = xy::FileSystem::getConfigDirectory("xy_editor") + "sprite.cfg";
}

SpriteState::SpriteState(xy::StateStack& ss, xy::State::Context ctx)
    : xy::State             (ss,ctx),
    m_scene                 (ctx.appInstance.getMessageBus())
{
    setup();

    registerConsoleTab("Help", [&]
    {
        
    });

    m_scene.getActiveCamera().getComponent<xy::Camera>().setView(ctx.defaultView.getSize());
    m_scene.getActiveCamera().getComponent<xy::Camera>().setViewport(ctx.defaultView.getViewport());
}

SpriteState::~SpriteState()
{
    //m_config.findProperty("bg_colour")->setValue(m_backgroundColour);
    //m_config.findProperty("working_dir")->setValue(m_workingDirectory);
    m_config.save(cfgPath);
}

bool SpriteState::handleEvent(const sf::Event& evt)
{
    if (evt.type == sf::Event::KeyReleased)
    {
        switch (evt.key.code)
        {
        default: break;
        case sf::Keyboard::F2:
            m_scene.showSystemInfo();
            break;
        }
    }

    m_scene.forwardEvent(evt);
    return true;
}

void SpriteState::handleMessage(const xy::Message& msg)
{
    m_scene.forwardMessage(msg);
}

bool SpriteState::update(float dt)
{
    m_scene.update(dt);
    return true;
}

void SpriteState::draw()
{
    auto rw = getContext().appInstance.getRenderWindow();
    rw->draw(m_scene);
}


//private
void SpriteState::setup()
{
    auto& mb = xy::App::getActiveInstance()->getMessageBus();

    m_scene.addSystem<xy::CallbackSystem>(mb);
    m_scene.addSystem<xy::RenderSystem>(mb);

    //registerWindow([&]() 
    //    {
    //        if (m_showBackgroundPicker)
    //        {
    //            ImGui::SetNextWindowPos({504.f, 20.f}, ImGuiCond_FirstUseEver);
    //            ImGui::SetNextWindowSize({ 412.f, 94.f }, ImGuiCond_FirstUseEver);
    //            if (ImGui::Begin("Options", &m_showBackgroundPicker))
    //            {
    //                if (xy::ui::colourPicker("Background Colour", m_backgroundColour))
    //                {
    //                    xy::App::setClearColour(m_backgroundColour);
    //                }

    //                if (xy::ui::button("Set Working Directory"))
    //                {
    //                    auto path = xy::FileSystem::openFolderDialogue();
    //                    if (!path.empty())
    //                    {
    //                        m_workingDirectory = path;

    //                        //try trimming the loaded texture path
    //                        if (!m_emitterSettings->texturePath.empty())
    //                        {
    //                            if (m_emitterSettings->texturePath.find(path) != std::string::npos)
    //                            {
    //                                m_emitterSettings->texturePath = m_emitterSettings->texturePath.substr(path.size());
    //                            }
    //                        }
    //                    }
    //                }
    //                xy::ui::sameLine(); xy::ui::showToolTip("Current working directory. Set this to your project directory and textures will be loaded and saved in a path relative to this");

    //                ImGui::End();
    //            }
    //        }
    //    
    //    });

    m_config.loadFromFile(cfgPath);

    //if (auto* prop = m_config.findProperty("working_dir"); prop)
    //{
    //    m_workingDirectory = prop->getValue<std::string>();
    //}
    //else
    //{
    //    m_config.addProperty("working_dir").setValue(m_workingDirectory);
    //}

    //if (auto* prop = m_config.findProperty("bg_colour"); prop)
    //{
    //    m_backgroundColour = prop->getValue<sf::Color>();
    //    xy::App::setClearColour(m_backgroundColour);
    //}
    //else
    //{
    //    m_config.addProperty("bg_colour").setValue(m_backgroundColour);
    //}
}