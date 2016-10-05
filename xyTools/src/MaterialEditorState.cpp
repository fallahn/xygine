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
#include <xygine/imgui/imgui_sfml.h>
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
    m_meshRenderer  ({ context.appInstance.getVideoSettings().VideoMode.width, context.appInstance.getVideoSettings().VideoMode.height }, m_scene),
    m_materials     (1),
    m_materialData  (&m_materials[0])
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
    if (msg.id == xy::Message::UIMessage)
    {
        const auto& data = msg.getData<xy::Message::UIEvent>();
        switch (data.type)
        {
        default:break;
        case xy::Message::UIEvent::ResizedWindow:
            m_scene.setView(getContext().defaultView);
            break;
        }
    }
    m_scene.handleMessage(msg);
    m_meshRenderer.handleMessage(msg);
}

//private
void MaterialEditorState::clearTexture(std::size_t idx, const sf::Color& colour)
{
    sf::Image img;
    img.create(2, 2, colour);
    m_textures[idx].loadFromImage(img);
    m_materialData->textures[idx].clear();
}

void MaterialEditorState::loadMaterials()
{
    m_shaderResource.preload(ShaderID::Coloured, DEFERRED_COLOURED_VERTEX, DEFERRED_COLOURED_FRAGMENT);
    m_shaderResource.preload(ShaderID::VertColoured, DEFERRED_VERTCOLOURED_VERTEX, DEFERRED_VERTCOLOURED_FRAGMENT);
    m_shaderResource.preload(ShaderID::Shadow, SHADOW_VERTEX, xy::Shader::Mesh::ShadowFragment);
    m_shaderResource.preload(ShaderID::Textured, DEFERRED_TEXTURED_VERTEX, DEFERRED_TEXTURED_FRAGMENT);
    m_shaderResource.preload(ShaderID::TexturedBumped, DEFERRED_TEXTURED_BUMPED_VERTEX, DEFERRED_TEXTURED_BUMPED_FRAGMENT);

    auto& defaultMaterial = m_materialResource.add(MaterialID::Default, m_shaderResource.get(ShaderID::Coloured));
    defaultMaterial.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    defaultMaterial.addProperty({ "u_colour", sf::Color::White });
    defaultMaterial.addRenderPass(xy::RenderPass::ID::ShadowMap, m_shaderResource.get(ShaderID::Shadow));
    defaultMaterial.getRenderPass(xy::RenderPass::ID::ShadowMap)->setCullFace(xy::CullFace::Front);

    auto& vertColouredMaterial = m_materialResource.add(MaterialID::VertexColoured, m_shaderResource.get(ShaderID::VertColoured));
    vertColouredMaterial.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    vertColouredMaterial.addRenderPass(xy::RenderPass::ID::ShadowMap, m_shaderResource.get(ShaderID::Shadow));
    vertColouredMaterial.getRenderPass(xy::RenderPass::ID::ShadowMap)->setCullFace(xy::CullFace::Front);


    for (auto& t : m_textures)t.setRepeated(true);

    clearTexture(0, sf::Color::White);
    clearTexture(1, sf::Color::Black);
    clearTexture(2, { 127, 127, 255 });

    auto& texturedMaterial = m_materialResource.add(MaterialID::Textured, m_shaderResource.get(ShaderID::TexturedBumped));
    texturedMaterial.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    texturedMaterial.addProperty({ "u_diffuseMap", m_textures[0] });
    texturedMaterial.addProperty({ "u_maskMap", m_textures[1] });
    texturedMaterial.addProperty({ "u_normalMap", m_textures[2] });
    texturedMaterial.addRenderPass(xy::RenderPass::ID::ShadowMap, m_shaderResource.get(ShaderID::Shadow));
    texturedMaterial.getRenderPass(xy::RenderPass::ID::ShadowMap)->setCullFace(xy::CullFace::Front);

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

    auto light = xy::Component::create<xy::PointLight>(m_messageBus, 1800.f, 1400.f);
    light->setIntensity(0.8f);
    light->setDepth(100.f);

    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(xy::DefaultSceneSize / 2.f);
    entity->move(0.f, -385.f);
    entity->addComponent(light);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontMiddle);

    light = xy::Component::create<xy::PointLight>(m_messageBus, 1800.f, 1400.f, sf::Color::Red);
    light->setIntensity(0.4f);
    light->setDepth(50.f);

    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(xy::DefaultSceneSize / 2.f);
    entity->move(285.f, 0.f);
    entity->addComponent(light);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontMiddle);

    light = xy::Component::create<xy::PointLight>(m_messageBus, 1800.f, 1400.f, sf::Color::Green);
    light->setIntensity(0.4f);
    light->setDepth(50.f);

    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(xy::DefaultSceneSize / 2.f);
    entity->move(-285.f, 0.f);
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
        
        nim::SetNextWindowSizeConstraints({ 200.f, 500.f }, { 800.f, 720.f });
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
            m_materials = xy::MaterialDefinition::loadFile(selectedFile);
            if (!m_materials.empty())
            {
                m_materialData = &m_materials[0];
                for (auto i = 0u; i < m_textures.size(); ++i)
                {
                    if (!m_materialData->textures[i].empty())
                    {
                        m_textures[i].loadFromFile(m_materialData->textures[i]);
                    }
                    else
                    {
                        sf::Color c;
                        switch (i)
                        {
                        case 0:
                        default:
                            c = sf::Color::White;
                            break;
                        case 1:
                            c = sf::Color::Black;
                            break;
                        case 2:
                            c = sf::Color(127, 127, 255);
                            break;
                        }
                        clearTexture(i, c);
                    }
                }
            }
            else
            {
                m_materials.emplace_back();
                clearTexture(0, sf::Color::White);
                clearTexture(1, sf::Color::Black);
                clearTexture(2, sf::Color(127, 127, 255));
            }
        }

        selectedFile.clear();
        nim::SameLine();
        if (nim::fileBrowseDialogue("Save as...", selectedFile, nim::Button("Save Material"))
            && !selectedFile.empty())
        {
            //attempt to save material
            //TODO confirmation dialogue
            xy::MaterialDefinition::writeFile(m_materials, selectedFile);
        }

        //toggle debug draw
        bool debug = drawDebug;
        nim::Checkbox("Show Debug Output", &debug);
        if (debug != drawDebug)
        {
            m_scene.drawDebug(debug);
        }
        drawDebug = debug;

        //TODO loading / previewing animations

        nim::Separator();
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
        nim::Separator();

        //material list
        static int subMeshIndex = 0;

        nim::NewLine();
        nim::Text("Materials");
        nim::SameLine();
        if (nim::Button("Add"))
        {
            //add sub material
            m_materials.emplace_back();
            m_materialData = &m_materials.back();
        }
        nim::SameLine();
        if (nim::Button("Remove") && m_materials.size() > 1u)
        {
            //remove selected material            
            m_materials.erase(m_materials.begin() + subMeshIndex);
            m_materialData = &m_materials[subMeshIndex];
        }

        nim::BeginChild("ScrollRegion", {0.f, 100.f}, false, ImGuiWindowFlags_HorizontalScrollbar);
        for (auto i = 0u; i < m_materials.size(); ++i)
        {
            std::string label = m_materials[i].name + "##" + std::to_string(i);
            if (nim::Selectable(label.c_str(), (i == subMeshIndex)))
            {
                m_materialData = &m_materials[i];
                subMeshIndex = i;
            }
        }
        nim::EndChild();

        //material properties
        nim::NewLine();
        nim::Text("Properties:");
        char materialName[20] = "";
        std::memcpy(materialName, m_materialData->name.c_str(), m_materialData->name.size());
        materialName[m_materialData->name.size()] = '\0';
        if (nim::InputText("Material Name", materialName, 20, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            m_materialData->name = materialName;
        }

        const char* shaders = "Coloured\0Vertex Coloured\0Textured\0";
        int selectedShader = m_materialData->shaderType;
        int lastShader = selectedShader;
        nim::Combo("Shader", &selectedShader, shaders);
        if (lastShader != selectedShader)
        {
            //set sub material based on active index
            int subMeshCount = 0;
            if (modelEntity)
            {
                subMeshCount = modelEntity->getComponent<xy::Model>()->getMesh().getSubMeshCount();
            }
            switch (selectedShader)
            {
            default:
            case 0:
                m_materialData->shaderType = m_materialData->Coloured;
                if (modelEntity)
                {
                    if (subMeshIndex > 0 && subMeshIndex < subMeshCount)
                    {
                        modelEntity->getComponent<xy::Model>()->setSubMaterial(m_materialResource.get(MaterialID::Default), subMeshIndex);
                    }
                    else
                    {
                        modelEntity->getComponent<xy::Model>()->setBaseMaterial(m_materialResource.get(MaterialID::Default));
                    }
                }
                break;
            case 1:
                m_materialData->shaderType = m_materialData->VertexColoured;
                if (modelEntity)
                {
                    if (subMeshIndex > 0 && subMeshIndex < subMeshCount)
                    {
                        modelEntity->getComponent<xy::Model>()->setSubMaterial(m_materialResource.get(MaterialID::VertexColoured), subMeshIndex);
                    }
                    else
                    {
                        modelEntity->getComponent<xy::Model>()->setBaseMaterial(m_materialResource.get(MaterialID::VertexColoured));
                    }
                }
                break;
            case 2:
                m_materialData->shaderType = m_materialData->Textured;
                if (modelEntity)
                {
                    if (subMeshIndex > 0 && subMeshIndex < subMeshCount)
                    {
                        modelEntity->getComponent<xy::Model>()->setSubMaterial(m_materialResource.get(MaterialID::Textured), subMeshIndex);
                    }
                    else
                    {
                        modelEntity->getComponent<xy::Model>()->setBaseMaterial(m_materialResource.get(MaterialID::Textured));
                    }
                }
                break;
            }
        }
        //TODO if model loaded use submesh count
        //to create index selection for material
        bool castShadows = m_materialData->castShadows;
        nim::Checkbox("Cast Shadows", &m_materialData->castShadows);
        if (castShadows != m_materialData->castShadows)
        {
            /*if (castShadows)
            {
                m_materialResource.get(MaterialID::Default).addRenderPass(xy::RenderPass::ID::ShadowMap, m_shaderResource.get(ShaderID::Shadow));
                m_materialResource.get(MaterialID::VertexColoured).addRenderPass(xy::RenderPass::ID::ShadowMap, m_shaderResource.get(ShaderID::Shadow));
                m_materialResource.get(MaterialID::Textured).addRenderPass(xy::RenderPass::ID::ShadowMap, m_shaderResource.get(ShaderID::Shadow));
            }
            else
            {
                m_materialResource.get(MaterialID::Default).removeRenderPass(xy::RenderPass::ID::ShadowMap);
                m_materialResource.get(MaterialID::VertexColoured).removeRenderPass(xy::RenderPass::ID::ShadowMap);
                m_materialResource.get(MaterialID::Textured).removeRenderPass(xy::RenderPass::ID::ShadowMap);
            }*/
        }

        if (selectedShader == 0)
        {
            //show colour picker
            float c[] = { m_materialData->colour[0] / 255.f, m_materialData->colour[1] / 255.f, m_materialData->colour[2] / 255.f };
            nim::ColorEdit3("Colour", c);
            m_materialData->colour[0] = static_cast<sf::Uint8>(c[0] * 255.f);
            m_materialData->colour[1] = static_cast<sf::Uint8>(c[1] * 255.f);
            m_materialData->colour[2] = static_cast<sf::Uint8>(c[2] * 255.f);
            m_materialData->colour[3] = 255;

            if (xy::MaterialProperty* mp = m_materialResource.get(MaterialID::Default).getProperty("u_colour"))
            {
                mp->setValue(sf::Color(m_materialData->colour[0], m_materialData->colour[1], m_materialData->colour[2]));
            }
        }
        else if (selectedShader == 2)
        {            
            static std::string diffuseName;
            static std::string maskName;
            static std::string normalName;
            
            //show texture list
            //DIFFUSE
            if (nim::fileBrowseDialogue("Select Diffuse Texture", m_materialData->textures[0], nim::Button("Diffuse Texture", { 120.f, 20.f })))
            {
                if (m_textures[0].loadFromFile(m_materialData->textures[0]))
                {
                    m_materialResource.get(MaterialID::Textured).getProperty("u_diffuseMap")->setValue(m_textures[0]);
                    diffuseName = xy::FileSystem::getFileName(m_materialData->textures[0]);
                }
            }
            nim::SameLine();
            if (nim::Button("Clear"))
            {
                clearTexture(0, sf::Color::White);
                diffuseName.clear();
            }
            nim::Image(m_textures[0], { 50.f, 50.f });          
            nim::Text(diffuseName.c_str());

            //MASK
            if (nim::fileBrowseDialogue("Select Mask Texture", m_materialData->textures[1], nim::Button("Mask Texture", { 120.f, 20.f })))
            {
                if (m_textures[1].loadFromFile(m_materialData->textures[1]))
                {
                    m_materialResource.get(MaterialID::Textured).getProperty("u_maskMap")->setValue(m_textures[1]);
                    maskName = xy::FileSystem::getFileName(m_materialData->textures[1]);
                }
            }
            nim::SameLine();
            if (nim::Button("Clear##M"))
            {
                clearTexture(1, sf::Color::Black);
                maskName.clear();
            }
            nim::Image(m_textures[1], { 50.f, 50.f });
            nim::Text(maskName.c_str());

            //NORMAL
            if (nim::fileBrowseDialogue("Select Normal Texture", m_materialData->textures[2], nim::Button("Normal Texture", { 120.f, 20.f })))
            {
                if (m_textures[2].loadFromFile(m_materialData->textures[2]))
                {
                    m_materialResource.get(MaterialID::Textured).getProperty("u_normalMap")->setValue(m_textures[2]);
                    normalName = xy::FileSystem::getFileName(m_materialData->textures[2]);
                }
            }
            nim::SameLine();
            if (nim::Button("Clear##N"))
            {
                clearTexture(2, { 127, 127, 255 });
                normalName.clear();
            }
            nim::Image(m_textures[2], { 50.f, 50.f });
            nim::Text(normalName.c_str());
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
 
    //TODO attempt to load material file, or reset if one not found

    switch (m_materialData->shaderType)
    {
    default:
    case 0:
        model->setBaseMaterial(m_materialResource.get(MaterialID::Default));
        break;
    case 1:
        model->setBaseMaterial(m_materialResource.get(MaterialID::VertexColoured));
        break;
    case 2:
        model->setBaseMaterial(m_materialResource.get(MaterialID::Textured));
        break;
    }

    modelEntity->addComponent(model);

    rotX = rotY = rotZ = 0.f;
    scale = 1.f;
}