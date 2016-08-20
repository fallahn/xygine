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
#include <Enumerations.hpp>

#include <xygine/App.hpp>
#include <xygine/imgui/imgui.h>
#include <xygine/imgui/CommonDialogues.hpp>
#include <xygine/Reports.hpp>
#include <xygine/util/Math.hpp>
#include <xygine/components/PointLight.hpp>
#include <xygine/components/Model.hpp>
#include <xygine/components/MeshDrawable.hpp>
#include <xygine/mesh/IQMBuilder.hpp>
#include <xygine/mesh/CubeBuilder.hpp>
#include <xygine/mesh/Mesh.hpp>
#include <xygine/mesh/SubMesh.hpp>
#include <xygine/mesh/shaders/DeferredRenderer.hpp>

#include <SFML/Window/Event.hpp>

namespace
{
    bool drawDebug = false;
    xy::Entity* modelEntity = nullptr;
    sf::Int32 meshID = 1; //ugh we need to flush old meshes somehow. 0 is reserved for background

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
    loadMaterials();
    buildMenu();

    xy::Stats::clear();
    m_scene.setView(context.defaultView);
    m_scene.setAmbientColour({ 76, 76, 72 });
    m_scene.getSkyLight().setIntensity(0.3f);
    m_scene.getSkyLight().setDiffuseColour({ 255, 255, 250 });
    m_scene.getSkyLight().setSpecularColour({ 255, 255, 250 });
    m_scene.getSkyLight().setDirection({ 0.1f, 0.75f, -0.45f });

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

    //rw.draw(m_meshRenderer);
}

bool MaterialEditorState::handleEvent(const sf::Event& evt)
{
    switch (evt.type)
    {
    default: break;
    case sf::Event::MouseMoved:
    {
        auto position = xy::App::getMouseWorldPosition();
        auto movement = position - lastMousePos;
        lastMousePos = position;        
        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
        {
            if (modelEntity) modelEntity->move(movement);
        }
        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle))
        {
            static const float multiplier = 0.01f;
            rotX -= movement.y * multiplier;
            rotX = xy::Util::Math::clamp(rotX, -360.f, 360.f);

            rotZ += movement.x * multiplier;
            rotZ = xy::Util::Math::clamp(rotZ, -360.f, 360.f);
        }
    }
        break;
    case sf::Event::MouseButtonPressed:
        if (evt.mouseButton.button == sf::Mouse::Button::Left)
        {
            lastMousePos = xy::App::getMouseWorldPosition();
        }
        break;
    case sf::Event::MouseWheelScrolled:
    {
        scale += evt.mouseWheelScroll.delta * 0.5f;
        scale = xy::Util::Math::clamp(scale, 1.f, 50.f);
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
void MaterialEditorState::loadMaterials()
{
    m_shaderResource.preload(ShaderID::Coloured, DEFERRED_COLOURED_VERTEX, DEFERRED_COLOURED_FRAGMENT);
    m_shaderResource.preload(ShaderID::VertColoured, DEFERRED_VERTCOLOURED_VERTEX, DEFERRED_VERTCOLOURED_FRAGMENT);
    m_shaderResource.preload(ShaderID::Shadow, SHADOW_VERTEX, xy::Shader::Mesh::ShadowFragment);
    m_shaderResource.preload(ShaderID::Textured, DEFERRED_TEXTURED_VERTEX, DEFERRED_TEXTURED_FRAGMENT);

    auto& defaultMaterial = m_materialResource.add(MaterialID::Default, m_shaderResource.get(ShaderID::Coloured));
    defaultMaterial.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    defaultMaterial.addProperty({ "u_colour", sf::Color::White });
    defaultMaterial.addRenderPass(xy::RenderPass::ID::ShadowMap, m_shaderResource.get(ShaderID::Shadow));
    defaultMaterial.getRenderPass(xy::RenderPass::ID::ShadowMap)->setCullFace(xy::CullFace::Front);

    auto& vertColouredMaterial = m_materialResource.add(MaterialID::VertexColoured, m_shaderResource.get(ShaderID::VertColoured));
    vertColouredMaterial.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    vertColouredMaterial.addRenderPass(xy::RenderPass::ID::ShadowMap, m_shaderResource.get(ShaderID::Shadow));
    vertColouredMaterial.getRenderPass(xy::RenderPass::ID::ShadowMap)->setCullFace(xy::CullFace::Front);

    auto& boxMaterial = m_materialResource.add(MaterialID::Box, m_shaderResource.get(ShaderID::Textured));
    boxMaterial.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    //boxMaterial.addProperty({ "u_colour", sf::Color(245, 255, 230) });
    boxMaterial.addProperty({ "u_diffuseMap", m_textureResource.get("assets/images/textures/cornell_diffuse.png") });
    boxMaterial.addProperty({ "u_maskMap", m_textureResource.get("assets/images/textures/cornell_mask.png") });
    
    //cornell box background
    xy::IQMBuilder ib("assets/models/cornell.iqm");
    m_meshRenderer.loadModel(0, ib);

    auto model = m_meshRenderer.createModel(0, m_messageBus);
    model->setBaseMaterial(boxMaterial);
    model->rotate(xy::Model::Axis::Y, 180.f);

    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(model);
    entity->setPosition(xy::DefaultSceneSize / 2.f);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontRear);

    auto light = xy::Component::create<xy::PointLight>(m_messageBus, 1800.f, 1400.f/*, sf::Color(255, 255, 100)*/);
    //light->enableShadowCasting(true);
    light->setIntensity(0.8f);
    light->setDepth(100.f);

    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(xy::DefaultSceneSize / 2.f);
    entity->move(0.f, -385.f);
    entity->addComponent(light);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontMiddle);

    //place the mesh renderer drawable in the scene so the debug layer draws over the top
    auto drawable = m_meshRenderer.createDrawable(m_messageBus);
    entity = xy::Entity::create(m_messageBus);
    entity->addComponent(drawable);
    m_scene.addEntity(entity, xy::Scene::Layer::BackFront);
}

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
        if (nim::Button("Reset Transform"))
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

    auto isVertexColoured = model->getMesh().getVertexLayout().getElementIndex(xy::VertexLayout::Element::Type::Colour);
    if (isVertexColoured == -1) //TODO check if we need skinned equivalent
    {
        model->setBaseMaterial(m_materialResource.get(MaterialID::Default));
    }
    else
    {
        model->setBaseMaterial(m_materialResource.get(MaterialID::VertexColoured));
    }   

    modelEntity->addComponent(model);

    rotX = rotY = rotZ = 0.f;
    scale = 1.f;
}