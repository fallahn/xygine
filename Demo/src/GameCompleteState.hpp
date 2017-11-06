/*********************************************************************
(c) Matt Marchant 2017
http://trederia.blogspot.com

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

#ifndef DEMO_COMPLETE_STATE_HPP_
#define DEMO_COMPLETE_STATE_HPP_

#include "StateIDs.hpp"

#include <xyginext/core/State.hpp>
#include <xyginext/ecs/Scene.hpp>
#include <xyginext/resources/Resource.hpp>

struct SharedStateData;

class GameCompleteState final : public xy::State
{
public:
    GameCompleteState(xy::StateStack&, xy::State::Context, SharedStateData&);

    xy::StateID stateID() const override { return StateID::GameComplete; }

    bool handleEvent(const sf::Event&) override;
    void handleMessage(const xy::Message&) override;
    bool update(float) override;
    void draw() override;

private:

    SharedStateData& m_sharedData;

    xy::Scene m_scene;
    xy::TextureResource m_textureResource;
    xy::FontResource m_fontResource;
};

#endif //DEMO_COMPLETE_STATE_HPP_