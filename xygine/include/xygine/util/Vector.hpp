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

#ifndef XY_UTIL_VECTOR_HPP_
#define XY_UTIL_VECTOR_HPP_

#include <xygine/util/Const.hpp>
#include <xygine/util/String.hpp>
#include <xygine/Assert.hpp>

#include <SFML/System/Vector2.hpp>

namespace xy
{
    namespace Util
    {
        /*!
        \brief Vector math utility functions
        */
        namespace Vector
        {
            /*!
            \brief Calculates the dot product of 2 vectors
            */
            static inline float dot(const sf::Vector2f& lv, const sf::Vector2f& rv)
            {
                return lv.x * rv.x + lv.y * rv.y;
            }
            /*!
            \brief Returns a given vector with its length normalized to 1
            */
            static inline sf::Vector2f normalise(sf::Vector2f source)
            {
                float length = std::sqrt(dot(source, source));
                XY_ASSERT(length != 0, "Division by zero");
                return source /= length;
            }
            /*!
            \brief Returns length of a vector squared

            When comparing the length of two vectors it is less expensive to
            compare the squared lengths, which will return the same result
            */
            static inline float lengthSquared(const sf::Vector2f& source)
            {
                return dot(source, source);
            }
            /*!
            \brief Returns length of a given vector

            If comparing the length of two vectors it is mroe efficient
            to use the squared lengths
            \see lengthSquared
            */
            static inline float length(const sf::Vector2f& source)
            {
                return std::sqrt(lengthSquared(source));
            }

            /*!
            \brief Reflects a velocity vector about a given normal
            */
            static inline sf::Vector2f reflect(const sf::Vector2f& velocity, const sf::Vector2f& normal)
            {
                return -2.f * dot(velocity, normal) * normal + velocity;
            }

            /*!
            \brief Rotates a vector (not very accurately)
            \param v Vector to rotate
            \param degrees Number of degrees to rotate vector
            \returns Rotated vector
            */
            static inline sf::Vector2f rotate(const sf::Vector2f& v, float degrees)
            {
                const float rads = degrees * Const::degToRad;
                auto ca = std::cos(rads);
                auto sa = std::sin(rads);
                return{ ca*v.x - sa*v.y, sa*v.x + ca*v.y };
            }

            /*!
            \brief Gets the rotation (in degrees) of a vector
            */
            static inline float rotation(const sf::Vector2f v)
            {
                return std::atan2(v.y, v.x) * Const::radToDeg;
            }

            /*!
            \brief Converts a comma delimited string to vector 2
            */
            template <typename T>
            static inline sf::Vector2<T> vec2FromString(const std::string& str)
            {
                sf::Vector2<T> retVec;
                auto values = String::toFloatArray(str);
                switch (values.size())
                {
                case 2:
                    retVec.y = static_cast<T>(values[1]);
                case 1:
                    retVec.x = static_cast<T>(values[0]);
                    break;
                default: break;
                }
                return retVec;
            }
        }
    }
}

#endif //XY_UTIL_VECTOR_HPP_