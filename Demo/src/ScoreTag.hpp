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

#ifndef DEMO_SCORE_TAG_HPP_
#define DEMO_SCORE_TAG_HPP_

#include <xyginext/ecs/System.hpp>

struct ScoreTag final
{
    static constexpr float MaxLife = 2.f;
    float lifetime = MaxLife;
    sf::Color colour = { 155, 255, 55 };
};

class ScoreTagSystem final : public xy::System
{
public:
    explicit ScoreTagSystem(xy::MessageBus&);

    void process(float) override;

private:
};

#endif //DEMO_SCORE_TAG_HPP_