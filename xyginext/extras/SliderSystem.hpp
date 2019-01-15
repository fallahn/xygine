/*********************************************************************
(c) Matt Marchant 2017 - 2019
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

#pragma once

#include <xyginext/ecs/System.hpp>

#include <SFML/System/Vector2.hpp>

/*
This slider system creates simple sliding animations
between two given points. Typically used when moving
UI elements around the screen to provide some fluis motion.

Add SliderSystem.cpp to your project and use it as any other
xygine system.
*/


/*
Slider component. Add this to an entity which is to be
animated. To use it set the target point in world coordinates
and set the active property to true. This will automatically
be set to false once the entity reaches its destination.
The speed property is a multiplier which should always be
positive. Negative numbers will cause backward motion and
the entity will never reach its target...
*/
struct Slider final
{
    sf::Vector2f target;
    bool active = false;
    float speed = 2.f;
};

/*
Slider system. Include Slider.hpp (this file) in the
file used to set up your Scene and then add this system
as any other. Requires an entity to have a Slider component
and a xy::Transform component.
*/
class SliderSystem final : public xy::System
{
public:
    explicit SliderSystem(xy::MessageBus&);

    void process(float) override;

private:

};
