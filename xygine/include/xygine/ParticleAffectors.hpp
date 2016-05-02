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

#include <xygine/Config.hpp>

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>

namespace xy
{
    struct Particle;
    /*!
    \brief Applies a given force to each particle, each frame
    */
    struct XY_EXPORT_API ForceAffector
    {
        /*!
        \brief Constructor
        \param force Force to apply each frame
        */
        explicit ForceAffector(const sf::Vector2f& force);
        void operator()(Particle& p, float dt);
        /*!
        \brief Set the force affector to apply a ramdom force each frame
        \param rangeStart Minimal force to apply
        \param rangeEnd Maximum force to apply

        Calling this at least once will disable any value the affector 
        was constructed with
        */
        void setRandom(const sf::Vector2f& rangeStart, const sf::Vector2f& rangeEnd);

    private:
        sf::Vector2f m_force;

        bool m_random;
        sf::Vector2f m_randomStart;
        sf::Vector2f m_randomEnd;
    };

    /*!
    \brief Modifies a particle's colour over time
    */
    struct XY_EXPORT_API ColourAffector
    {
        /*!
        \brief Constructor
        \param start Initial particle colour
        \param end Final particle colour
        \param duration Amount of time to take to change from start colour to end colour
        */
        ColourAffector(const sf::Color& start, const sf::Color& end, float duration);
        void operator() (Particle& p, float dt);

    private:
        float m_duration;
        sf::Color m_start;
        sf::Color m_end;
    };
    /*!
    \brief Applies a given rotation in degrees per second to a particle
    */
    struct XY_EXPORT_API RotateAffector
    {
        /*!
        \brief Constructor
        \param rotation Rotation amount in degrees per second to apply to each particle
        */
        explicit RotateAffector(float rotation);
        void operator() (Particle& p, float dt);

    private:
        float m_rotation;
    };

    /*!
    \brief Scales a particle by a given amount each frame
    */
    struct XY_EXPORT_API ScaleAffector
    {
        /*!
        \brief Constructor
        \param scale Scale to apply to the particle each frame.
        */
        explicit ScaleAffector(const sf::Vector2f& scale);
        void operator() (Particle& p, float dt);
    private:
        sf::Vector2f m_scale;
    };

    /*!
    \brief Scales a particle's velocity by a given amount each frame.
    Particle direction is left unchanged, but velocity magnitude is
    scaled by the given amount allowing particles to move faster or
    slower over time.
    */
    struct XY_EXPORT_API VelocityAffector
    {
        explicit VelocityAffector(const sf::Vector2f& scale);
        void operator() (Particle& p, float dt);
    private:
        sf::Vector2f m_scale;
    };
}
#endif //XY_AFFECTORS_HPP_