/*********************************************************************
(c) Matt Marchant 2019

This file is part of the xygine tutorial found at
https://github.com/fallahn/xygine

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

#include <xyginext/ecs/System.hpp>

#include <SFML/System/Vector2.hpp>

//this is the struct used as the ball component
struct Ball final
{
    enum class State
    {
        Waiting, Active
    }state = State::Waiting;

    sf::Vector2f velocity = {0.f, -1.f};
    static constexpr float Speed = 800.f;
};

//this is a component added to the paddle entity
//allowing it to track any ball entity which may be waiting launch
struct Paddle final
{
    xy::Entity ball;
};

//and the system used to update all entities with a ball component
class BallSystem final : public xy::System
{
public:
    explicit BallSystem(xy::MessageBus&);

    void process(float) override;

private:

};