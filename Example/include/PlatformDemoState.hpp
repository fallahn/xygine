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

#ifndef PLATFORM_DEMO_STATE_HPP_
#define PLATFORM_DEMO_STATE_HPP_

#include <StateIds.hpp>

#include <xygine/State.hpp>
#include <xygine/Resource.hpp>
#include <xygine/ShaderResource.hpp>
#include <xygine/Scene.hpp>
#include <xygine/physics/World.hpp>

//-------
#include <xygine/mesh/MeshRenderer.hpp>
#include <xygine/mesh/MeshResource.hpp>
#include <xygine/mesh/Material.hpp>
#include <xygine/mesh/MaterialResource.hpp>
//-------

#include <SFML/Graphics/Text.hpp>


class PlatformDemoState final : public xy::State
{
public:
    PlatformDemoState(xy::StateStack& stateStack, Context context);
    ~PlatformDemoState() = default;

    bool update(float dt) override;
    void draw() override;
    bool handleEvent(const sf::Event& evt) override;
    void handleMessage(const xy::Message&) override;
    xy::StateID stateID() const override
    {
        return States::ID::PlatformDemo;
    }
private:

    xy::MessageBus& m_messageBus;
    xy::Scene m_scene;

    xy::TextureResource m_textureResource;
    xy::FontResource m_fontResource;
    xy::ShaderResource m_shaderResource;

    xy::Physics::World m_physWorld;

    xy::MeshRenderer m_meshRenderer;

    enum MeshID
    {
        Cube = 0,
        Fixit,
        Platform,
        Batcat,
        Quad
    };

    enum MatId
    {
        Demo = 0,
        MrFixitBody,
        MrFixitHead,
        LightSource,
        Platform01,
        Platform04,
        BatcatMat
    };
    xy::MaterialResource m_materialResource;

    void cacheMeshes();
    void buildTerrain();
    void buildPhysics();
    void addItems();
    void addPlayer();
};

#endif //PLATFORM_DEMO_STATE_HPP_