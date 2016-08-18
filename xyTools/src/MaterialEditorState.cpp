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

#include <MaterialEditorState.hpp>

#include <xygine/App.hpp>
#include <xygine/imgui/imgui.h>
#include <xygine/imgui/CommonDialogues.hpp>
#include <xygine/Reports.hpp>

#include <xygine/components/Model.hpp>
#include <xygine/mesh/IQMBuilder.hpp>
#include <xygine/mesh/Mesh.hpp>

#include <SFML/Window/Event.hpp>

namespace
{
    bool drawDebug = false;
    xy::Entity* modelEntity = nullptr;
    sf::Int32 meshID = 0; //ugh we need to flush old meshes somehow

    float scale = 1.f;
    float rotX = 0.f;
    float rotY = 0.f;
    float rotZ = 0.f;

    sf::Vector2f lastMousePos;
}


MaterialEditorState::MaterialEditorState(xy::StateStack& stack, Context context)
    :xy::State      (stack, context),
    m_messageBus    (context.appInstance.getMessageBus()),
    m_scene         (m_messageBus),
    m_meshRenderer  ({ context.appInstance.getVideoSettings().VideoMode.width, context.appInstance.getVideoSettings().VideoMode.height }, m_scene)
{
    launchLoadingScreen();
    buildMenu();

    xy::Stats::clear();
    m_scene.setView(context.defaultView);
    m_scene.setAmbientColour({ 76, 76, 72 });
    m_scene.getSkyLight().setIntensity(0.6f);
    m_scene.getSkyLight().setDiffuseColour({ 255, 255, 250 });
    m_scene.getSkyLight().setSpecularColour({ 255, 255, 250 });
    m_scene.getSkyLight().setDirection({ 0.2f, 0.4f, -1.f });

    lastMousePos = xy::App::getMouseWorldPosition();

    quitLoadingScreen();
}

MaterialEditorState::~MaterialEditorState()
{
    xy::App::removeUserWindows(this);

    getContext().renderWindow.setTitle("xy tools");
}

//public
bool MaterialEditorState::update(float dt)
{
    m_scene.update(dt);
    m_meshRenderer.update();

    if (modelEntity)
    {
        auto model = modelEntity->getComponent<xy::Model>();
        model->setScale({ scale, scale, scale });
        model->setRotation({ rotX, rotY, rotZ });
    }

    return true;
}

void MaterialEditorState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.draw(m_scene);

    rw.setView(getContext().defaultView);
    rw.draw(m_meshRenderer);
}

bool MaterialEditorState::handleEvent(const sf::Event& evt)
{
    switch (evt.type)
    {
    default: break;
    case sf::Event::MouseMoved:
        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
        {
            auto position = xy::App::getMouseWorldPosition();
            auto movement = position - lastMousePos;
            lastMousePos = position;

            if (modelEntity) modelEntity->move(movement);
        }
        break;
    case sf::Event::MouseButtonPressed:
        if (evt.mouseButton.button == sf::Mouse::Button::Left)
        {
            lastMousePos = xy::App::getMouseWorldPosition();
        }
        break;
    }

    return true;
}

void MaterialEditorState::handleMessage(const xy::Message& msg)
{
    m_scene.handleMessage(msg);
    m_meshRenderer.handleMessage(msg);
}

//private
void MaterialEditorState::buildMenu()
{    
    xy::App::addUserWindow([this]()
    {        
        //load model / material
        static std::string selectedFile;
        
        nim::SetNextWindowSizeConstraints({ 200.f, 500.f }, { 800.f, 600.f });
        if (!nim::Begin("Model Viewer and Material Editor", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_ShowBorders))
        {
            nim::End();
            return;
        }

        if (nim::fileBrowseDialogue("Select Model", selectedFile, nim::Button("Open Model"))
            && !selectedFile.empty())
        {
            //attempt to load model
            loadModel(selectedFile);
        }

        selectedFile.clear();
        nim::SameLine();
        if (nim::fileBrowseDialogue("Select Material", selectedFile, nim::Button("Open Material"))
            && !selectedFile.empty())
        {
            //attempt to load material
        }

        //toggle debug draw
        bool debug = drawDebug;
        nim::Checkbox("Show Debug Output", &debug);
        if (debug != drawDebug)
        {
            m_scene.drawDebug(debug);
        }
        drawDebug = debug;


        //TODO loading images, saving materials
        //TODO loading / previewing animations


        //model controls
        nim::SliderFloat("Scale", &scale, 1.f, 50.f);
        nim::SliderAngle("Rotation X", &rotX);
        nim::SliderAngle("Rotation Y", &rotY);
        nim::SliderAngle("Rotation Z", &rotZ);
        if (nim::Button("Reset"))
        {
            scale = 1.f;
            rotX = 0.f;
            rotY = 0.f;
            rotZ = 0.f;

            if (modelEntity)
            {
                modelEntity->setPosition(xy::DefaultSceneSize / 2.f);
            }
        }

        if (modelEntity)
        {
            nim::Text("Info:");
            auto bb = modelEntity->getComponent<xy::Model>()->getMesh().getBoundingBox().asFloatRect();
            std::string txt("Bounding Box: " + std::to_string(bb.width) + ", " + std::to_string(bb.height));
            nim::Text(txt.c_str());
        }

        nim::End();
    }, this);
}

void MaterialEditorState::loadModel(const std::string& path)
{
    if (xy::FileSystem::getFileExtension(path) != ".iqm")
    {
        xy::Logger::log("Not an IQM file.", xy::Logger::Type::Error);
        return;
    }
    
    if (!modelEntity)
    {
        auto entity = xy::Entity::create(m_messageBus);
        entity->setPosition(xy::DefaultSceneSize / 2.f);
        modelEntity = m_scene.addEntity(entity, xy::Scene::Layer::FrontMiddle);
    }
    else
    {
        auto model = modelEntity->getComponent<xy::Model>();
        if (model) model->destroy();
    }

    xy::IQMBuilder ib(path);
    m_meshRenderer.loadModel(meshID, ib);
    xy::Logger::log("Loaded: " + path, xy::Logger::Type::Info);
    getContext().renderWindow.setTitle(xy::FileSystem::getFileName(path));

    auto model = m_meshRenderer.createModel(meshID++, m_messageBus);
    modelEntity->addComponent(model);

    rotX = rotY = rotZ = 0.f;
    scale = 1.f;
}