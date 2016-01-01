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

//affectors for particle systems

#ifndef XY_AFFECTORS_HPP_
#define XY_AFFECTORS_HPP_

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>

namespace xy
{
    struct Particle;
    struct ForceAffector
    {
        explicit ForceAffector(const sf::Vector2f& force);
        void operator()(Particle& p, float dt);
        void setRandom(const sf::Vector2f& rangeStart, const sf::Vector2f& rangeEnd);

    private:
        sf::Vector2f m_force;

        bool m_random;
        sf::Vector2f m_randomStart;
        sf::Vector2f m_randomEnd;
    };

    struct ColourAffector
    {
        ColourAffector(const sf::Color& start, const sf::Color& end, float duration);
        void operator() (Particle& p, float dt);

    private:
        float m_duration;
        sf::Color m_start;
        sf::Color m_end;
    };

    struct RotateAffector
    {
        explicit RotateAffector(float rotation);
        void operator() (Particle& p, float dt);

    private:
        float m_rotation;
    };

    struct ScaleAffector
    {
        explicit ScaleAffector(const sf::Vector2f& scale);
        void operator() (Particle& p, float dt);
    private:
        sf::Vector2f m_scale;
    };
}
#endif //XY_AFFECTORS_HPP_