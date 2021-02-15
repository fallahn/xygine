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

#include "ParticleState.hpp"

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/ParticleEmitter.hpp>
#include <xyginext/ecs/components/Camera.hpp>
#include <xyginext/ecs/components/Callback.hpp>
#include <xyginext/ecs/components/Drawable.hpp>

#include <xyginext/ecs/systems/ParticleSystem.hpp>
#include <xyginext/ecs/systems/CallbackSystem.hpp>
#include <xyginext/ecs/systems/SpriteSystem.hpp>
#include <xyginext/ecs/systems/CameraSystem.hpp>
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
    const float WindowHeight = 720.f;

    const std::string cfgPath = xy::FileSystem::getConfigDirectory("xy_editor") + "particle.cfg";
}

ParticleState::ParticleState(xy::StateStack& ss, xy::State::Context ctx)
    : xy::State             (ss,ctx),
    m_scene                 (ctx.appInstance.getMessageBus()),
    m_emitterSettings       (nullptr),
    m_selectedBlendMode     (0),
    m_workingDirectory      ("None Selected..."),
    m_showBackgroundPicker  (false),
    m_backgroundColour      (sf::Color::Black),
    m_showSpriteBrowser     (false)
{
    setup();

    registerConsoleTab("Help", [&]
    {
        xy::ui::image(m_textures.get("assets/images/help.png"));
    });

    m_scene.getActiveCamera().getComponent<xy::Camera>().setView(ctx.defaultView.getSize());
    m_scene.getActiveCamera().getComponent<xy::Camera>().setViewport(ctx.defaultView.getViewport());
}

ParticleState::~ParticleState()
{
    m_config.findProperty("bg_colour")->setValue(m_backgroundColour);
    m_config.findProperty("working_dir")->setValue(m_workingDirectory);
    m_config.save(cfgPath);
}

bool ParticleState::handleEvent(const sf::Event& evt)
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

void ParticleState::handleMessage(const xy::Message& msg)
{
    m_scene.forwardMessage(msg);
}

bool ParticleState::update(float dt)
{
    m_scene.update(dt);
    return true;
}

void ParticleState::draw()
{
    auto rw = getContext().appInstance.getRenderWindow();
    rw->draw(m_scene);
}


//private
void ParticleState::setup()
{
    auto& mb = xy::App::getActiveInstance()->getMessageBus();

    m_scene.addSystem<xy::CallbackSystem>(mb);
    m_scene.addSystem<xy::SpriteSystem>(mb);
    m_scene.addSystem<xy::CameraSystem>(mb);
    m_scene.addSystem<xy::RenderSystem>(mb);
    m_scene.addSystem<xy::ParticleSystem>(mb);

    m_sprite = m_scene.createEntity();
    m_sprite.addComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
    m_sprite.getComponent<xy::Transform>().move(100.f, 0.f);
    m_sprite.addComponent<xy::Drawable>();
    m_sprite.addComponent<xy::Sprite>();

    auto entity = m_scene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
    entity.getComponent<xy::Transform>().move(100.f, 0.f);
    entity.addComponent<xy::ParticleEmitter>().start();
    m_emitterSettings = &entity.getComponent<xy::ParticleEmitter>().settings;
    entity.addComponent<sf::Vector2f>() = {0.707f, 0.707f};
    entity.addComponent<xy::Callback>().function = 
        [](xy::Entity e, float dt)
    {
        const float Speed = 400.f;
        auto vel = e.getComponent<sf::Vector2f>();
        auto& tx = e.getComponent<xy::Transform>();
        tx.move(vel * Speed * dt);

        auto pos = tx.getPosition();
        if (pos.x > xy::DefaultSceneSize.x)
        {
            vel = xy::Util::Vector::reflect(vel, { -1.f, 0.f });
            e.getComponent<sf::Vector2f>() = vel;
            pos.x = xy::DefaultSceneSize.x;
            tx.setPosition(pos);
            tx.setRotation(xy::Util::Vector::rotation(vel));
        }
        else if (pos.x < 0.f)
        {
            vel = xy::Util::Vector::reflect(vel, { 1.f, 0.f });
            e.getComponent<sf::Vector2f>() = vel;
            pos.x = 0.f;
            tx.setPosition(pos);
            tx.setRotation(xy::Util::Vector::rotation(vel));
        }

        if (pos.y < 0.f)
        {
            vel = xy::Util::Vector::reflect(vel, { 0.f, 1.f });
            e.getComponent<sf::Vector2f>() = vel;
            pos.y = 0.f;
            tx.setPosition(pos);
            tx.setRotation(xy::Util::Vector::rotation(vel));
        }
        else if (pos.y > xy::DefaultSceneSize.y)
        {
            vel = xy::Util::Vector::reflect(vel, { 0.f, -1.f });
            e.getComponent<sf::Vector2f>() = vel;
            pos.y = xy::DefaultSceneSize.y;
            tx.setPosition(pos);
            tx.setRotation(xy::Util::Vector::rotation(vel));
        }
    };

    auto& verts = entity.addComponent<xy::Drawable>().getVertices();
    verts.emplace_back(sf::Vector2f(0.f, -12.f), sf::Color::Green);
    verts.emplace_back(sf::Vector2f(12.f, 0.f), sf::Color::Green);
    verts.emplace_back(sf::Vector2f(0.f, 12.f), sf::Color::Green);
    entity.getComponent<xy::Drawable>().setPrimitiveType(sf::LineStrip);
    entity.getComponent<xy::Drawable>().updateLocalBounds();

    auto windowFunc = [&, entity]() mutable
    {
        //ImGui::ShowDemoWindow();
        if (ImGui::BeginMainMenuBar())
        {
            if (xy::ui::beginMenu("File"))
            {
                if (ImGui::MenuItem("Load"))
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
                            else if (!m_emitterSettings->texturePath.empty())
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
                if (ImGui::MenuItem("Save"))
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

                xy::ui::endMenu();
            }

            if (ImGui::BeginMenu("Options"))
            {
                ImGui::MenuItem("Preferences...", nullptr, &m_showBackgroundPicker);
                ImGui::MenuItem("Load Sprite", nullptr, &m_showSpriteBrowser);

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        xy::ui::setNextWindowSize(WindowWidth, WindowHeight);
        xy::ui::setNextWindowConstraints(WindowWidth, WindowHeight, WindowWidth, WindowHeight);
        if (ImGui::Begin("Emitter Settings"))
        {
            xy::ui::text("Working Directory:");
            xy::ui::text(m_workingDirectory);
            xy::ui::separator();

            xy::ui::slider("Gravity X", m_emitterSettings->gravity.x, -1000.f, 1000.f, ItemWidth);
            xy::ui::sameLine(); xy::ui::showToolTip("Gravitational force applied to the velocity");
            xy::ui::slider("Gravity Y", m_emitterSettings->gravity.y, -1000.f, 1000.f, ItemWidth);

            xy::ui::slider("Velocity X", m_emitterSettings->initialVelocity.x, -1000.f, 1000.f, ItemWidth);
            xy::ui::sameLine(); xy::ui::showToolTip("Initial velocity of the particle");
            xy::ui::slider("Velocity Y", m_emitterSettings->initialVelocity.y, -1000.f, 1000.f, ItemWidth);

            xy::ui::slider("Acceleration", m_emitterSettings->acceleration, 0.f, 2.f, ItemWidth);

            xy::ui::slider("Spread", m_emitterSettings->spread, 0.f, 360.f, ItemWidth);
            xy::ui::sameLine(); xy::ui::showToolTip("Spead, in degrees, applied to the inital velocity");
            xy::ui::slider("Lifetime", m_emitterSettings->lifetime, 0.1f, 10.f, ItemWidth);
            xy::ui::sameLine(); xy::ui::showToolTip("Lifetime of a particle in seconds");
            xy::ui::slider("Lifetime Variance", m_emitterSettings->lifetimeVariance, 0.f, 10.f, ItemWidth);
            xy::ui::sameLine(); xy::ui::showToolTip("Amount of random variation added to the lifetime of a particle, in seconds");

            xy::ui::slider("Rotation Speed", m_emitterSettings->rotationSpeed, 0.f, 15.f, ItemWidth);
            xy::ui::sameLine(); xy::ui::showToolTip("Rotation in degrees per second - textured particles only");
            xy::ui::slider("Scale Affector", m_emitterSettings->scaleModifier, -5.f, 5.f, ItemWidth);
            xy::ui::sameLine(); xy::ui::showToolTip("How rapidly a particle is scaled in size over its lifetime");
            xy::ui::slider("Size", m_emitterSettings->size, 0.1f, 100.f, ItemWidth);
            xy::ui::sameLine(); xy::ui::showToolTip("Initial size of a particle");

            xy::ui::slider("Emit Rate", m_emitterSettings->emitRate, 0.f, 150.f, ItemWidth);
            xy::ui::sameLine(); xy::ui::showToolTip("Number of particles emitted per second");
            std::int32_t count = m_emitterSettings->emitCount;
            xy::ui::input("Emit Count", count, ItemWidth);
            xy::ui::sameLine(); xy::ui::showToolTip("Number of particles emitted simultaneously");
            count = std::max(count, 0);
            m_emitterSettings->emitCount = count;

            xy::ui::slider("Spawn Radius", m_emitterSettings->spawnRadius, 0.f, 500.f, ItemWidth);
            xy::ui::sameLine(); xy::ui::showToolTip("Radius around the emitter position in which particles are spawned");
            xy::ui::slider("Spawn Offset X", m_emitterSettings->spawnOffset.x, -500.f, 500.f, ItemWidth);
            xy::ui::sameLine(); xy::ui::showToolTip("Offsets the particle spawn position from the emitter position in world units");
            xy::ui::slider("Spawn Offset Y", m_emitterSettings->spawnOffset.y, -500.f, 500.f, ItemWidth);

            count = m_emitterSettings->releaseCount;
            xy::ui::input("Release Count", count, ItemWidth);
            xy::ui::sameLine(); xy::ui::showToolTip("Total number of particles to release before automatically stopping the emitter. 0 emits indefinitely, restart the emitter for updated values to take effect");
            count = std::max(count, 0);
            m_emitterSettings->releaseCount = count;

            xy::ui::checkbox("Random Initial Rotation", &m_emitterSettings->randomInitialRotation);
            xy::ui::sameLine(); xy::ui::showToolTip("Applies a random initial rotation to spawned particles. Textured particles only");

            xy::ui::checkbox("Inherit Rotation", &m_emitterSettings->inheritRotation);
            xy::ui::sameLine(); xy::ui::showToolTip("Sets the initial rotation to that of the emitter");

            bool oldState = entity.getComponent<xy::Callback>().active;
            bool newState = oldState;
            xy::ui::checkbox("Animate Movement", &newState);
            xy::ui::sameLine(); xy::ui::showToolTip("Enable emitter movement");

            if (oldState != newState)
            {
                entity.getComponent<xy::Callback>().active = newState;
                float rotation = newState ? xy::Util::Vector::rotation(entity.getComponent<sf::Vector2f>()) : 0.f;
                entity.getComponent<xy::Transform>().setRotation(rotation);
            }

            xy::ui::colourPicker("Colour", m_emitterSettings->colour);

            xy::ui::separator();

            //blendmode drop down
            std::int32_t idx = m_selectedBlendMode;
            xy::ui::simpleCombo("Blend Mode", idx, "Alpha\0Add\0Multiply\0\0", ItemWidth);
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
                xy::ui::text("No texture loaded");
            }
            else
            {
                xy::ui::text(m_emitterSettings->texturePath);
            }

            if (xy::ui::button("Browse Texture"))
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
            xy::ui::sameLine(); xy::ui::showToolTip("For a relative path to a texture set the working directory, above");

            xy::ui::separator();

            if (xy::ui::button("Start"))
            {
                entity.getComponent<xy::ParticleEmitter>().start();
            }
            xy::ui::sameLine();
            if (xy::ui::button("Stop"))
            {
                entity.getComponent<xy::ParticleEmitter>().stop();
            }
            xy::ui::sameLine();
            if (xy::ui::button("Reset"))
            {
                entity.getComponent<xy::ParticleEmitter>().settings = xy::EmitterSettings();
                entity.getComponent<xy::Callback>().active = false;
                entity.getComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
                entity.getComponent<xy::Transform>().setRotation(0.f);
            }
            xy::ui::sameLine(); xy::ui::showToolTip("Reset the properties to their default values");

            xy::ui::separator();
        }
        ImGui::End();
    };
    registerWindow(windowFunc);

    registerWindow([&]() 
        {
            if (m_showBackgroundPicker)
            {
                ImGui::SetNextWindowPos({504.f, 20.f}, ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize({ 412.f, 94.f }, ImGuiCond_FirstUseEver);
                if (ImGui::Begin("Options", &m_showBackgroundPicker))
                {
                    if (xy::ui::colourPicker("Background Colour", m_backgroundColour))
                    {
                        xy::App::setClearColour(m_backgroundColour);
                    }

                    if (xy::ui::button("Set Working Directory"))
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
                    xy::ui::sameLine(); xy::ui::showToolTip("Current working directory. Set this to your project directory and textures will be loaded and saved in a path relative to this");

                    ImGui::End();
                }
            }
        });

    registerWindow([&]()
        {
            if (m_showSpriteBrowser)
            {
                ImGui::SetNextWindowPos({ 504.f, 20.f }, ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize({ 412.f, 494.f }, ImGuiCond_FirstUseEver);

                if (ImGui::Begin("Sprite Browser", &m_showSpriteBrowser))
                {
                    if (ImGui::Button("Open Sprite Sheet"))
                    {
                        auto path = xy::FileSystem::openFileDialogue(m_workingDirectory, "spt");
                        if (!path.empty())
                        {
                            if (!m_spriteSheet.loadFromFile(path, m_textures))
                            {
                                ImGui::OpenPopup("Invalid Sprite Sheet");
                            }

                            if (ImGui::BeginPopupModal("Invalid Sprite Sheet", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
                            {
                                ImGui::Text("Sprite Sheet Was Invalid Or Not Found");
                                ImGui::NewLine();

                                if (ImGui::Button("OK", ImVec2(100.f, 0.f)))
                                {
                                    ImGui::CloseCurrentPopup();
                                }
                                ImGui::EndPopup();
                            }
                        }
                    }

                    const auto& sprites = m_spriteSheet.getSprites();
                    if (!sprites.empty())
                    {
                        ImGui::NewLine();
                        ImGui::Separator();
                        ImGui::Text("Sprites:");
                        ImGui::NewLine();

                        //imgui doesn't really have a way of dealing with unordered_map
                        //so we'll fudge this with a list of buttons :)
                        for (const auto& [name, sprite] : sprites)
                        {
                            if (ImGui::Button(name.c_str(), { 120.f, 30.f }))
                            {
                                setSprite(sprite);
                            }
                        }
                    }

                    ImGui::End();
                }
            }
        });

    m_config.loadFromFile(cfgPath);

    if (auto* prop = m_config.findProperty("working_dir"); prop)
    {
        m_workingDirectory = prop->getValue<std::string>();
    }
    else
    {
        m_config.addProperty("working_dir").setValue(m_workingDirectory);
    }

    if (auto* prop = m_config.findProperty("bg_colour"); prop)
    {
        m_backgroundColour = prop->getValue<sf::Color>();
        xy::App::setClearColour(m_backgroundColour);
    }
    else
    {
        m_config.addProperty("bg_colour").setValue(m_backgroundColour);
    }
}

void ParticleState::setSprite(const xy::Sprite& sprite)
{
    m_sprite.getComponent<xy::Sprite>() = sprite;
    auto bounds = sprite.getTextureBounds();
    m_scene.getActiveCamera().getComponent<xy::Transform>().setPosition((xy::DefaultSceneSize / 2.f) + sf::Vector2f(bounds.width / 2.f, bounds.height / 2.f));
    //m_sprite.getComponent<xy::Transform>().setOrigin(bounds.width / 2.f, bounds.height / 2.f);


    //need to correct the texture path
    auto spritePath = m_spriteSheet.getTexturePath();
    if (!spritePath.empty())
    {
        auto texPath = m_workingDirectory;
        std::replace(texPath.begin(), texPath.end(), '\\', '/');
        if (texPath.back() != '/')
        {
            texPath += "/";
        }
        texPath += spritePath;
        m_sprite.getComponent<xy::Sprite>().setTexture(m_textures.get(texPath), false);
    }
}