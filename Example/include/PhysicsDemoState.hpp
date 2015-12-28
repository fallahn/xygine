/*********************************************************************
Matt Marchant 2014 - 2015
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

//physics system demo state

#ifndef PHYSICS_DEMO_STATE_HPP_
#define PHYSICS_DEMO_STATE_HPP_

#include <StateIds.hpp>

#include <xygine/State.hpp>
#include <xygine/Resource.hpp>
#include <xygine/Scene.hpp>
#include <xygine/physics/World.hpp>
#include <xygine/Resource.hpp>

#include <SFML/Graphics/Text.hpp>

namespace sf
{
    class Color;
}

class PhysicsDemoState final : public xy::State
{
public:
    PhysicsDemoState(xy::StateStack& stateStack, Context context);
    ~PhysicsDemoState() = default;

    bool update(float dt) override;
    void draw() override;
    bool handleEvent(const sf::Event& evt) override;
    void handleMessage(const xy::Message&) override;
    xy::StateId stateID() const override
    {
        return States::ID::PhysicsDemo;
    }
private:
    xy::Physics::World m_physWorld;
    xy::MessageBus& m_messageBus;
    xy::SoundResource m_soundResource;
    xy::Scene m_scene;

    xy::TextureResource m_textureResource;
    xy::FontResource m_fontResource;

    sf::Text m_reportText;

    void createBodies();

    void randomBall();
};

#endif //PHYSICS_DEMO_STATE_HPP_