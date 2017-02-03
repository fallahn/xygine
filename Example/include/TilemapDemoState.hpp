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

#ifndef TILEMAP_DEMO_STATE_HPP_
#define TILEMAP_DEMO_STATE_HPP_

#include <StateIds.hpp>

#include <xygine/State.hpp>
#include <xygine/Resource.hpp>
#include <xygine/ShaderResource.hpp>
#include <xygine/Scene.hpp>
#include <xygine/physics/World.hpp>
#include <xygine/tilemap/Map.hpp>

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <array>

class TilemapDemoState final : public xy::State
{
public:
    TilemapDemoState(xy::StateStack& stateStack, Context context);
    ~TilemapDemoState() = default;

    bool update(float dt) override;
    void draw() override;
    bool handleEvent(const sf::Event& evt) override;
    void handleMessage(const xy::Message&) override;
    xy::StateID stateID() const override
    {
        return States::ID::TilemapDemo;
    }
private:

    xy::MessageBus& m_messageBus;
    xy::Physics::World m_physWorld; //must live longer than scene
    xy::Scene m_scene;
    
    xy::TextureResource m_textureResource;
    xy::FontResource m_fontResource;
    xy::ShaderResource m_shaderResource;

    xy::tmx::Map m_tilemap;

    void buildScene();
};

#endif //DEFERRED_DEMO_STATE_HPP_