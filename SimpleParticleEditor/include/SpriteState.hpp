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

#pragma once

#include "States.hpp"

#include <xyginext/core/State.hpp>
#include <xyginext/core/ConfigFile.hpp>
#include <xyginext/ecs/Scene.hpp>
#include <xyginext/gui/GuiClient.hpp>
#include <xyginext/resources/Resource.hpp>

#include <string>

class SpriteState final : public xy::State, public xy::GuiClient
{
public:
    SpriteState(xy::StateStack&, xy::State::Context);

    ~SpriteState();

    bool handleEvent(const sf::Event &evt) override;
    
    void handleMessage(const xy::Message &) override;
    
    bool update(float dt) override;
    
    void draw() override;
    
    xy::StateID stateID() const override { return States::SpriteState; }

private:    
    xy::Scene m_scene;

    xy::ConfigFile m_config;

    void setup();
};
