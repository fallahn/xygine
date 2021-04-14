/*********************************************************************
(c) Matt Marchant 2017 - 2021
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

#include <cmath>
#include <type_traits>

namespace xy
{
    namespace Util
    {
        /*!
        \brief Math util functions
        */
        namespace Math
        {
            /*!
            \brief Clamps a given value to the given upper and lower bounds
            */
            template <typename T>
            static inline T clamp(T n, T lower, T upper)
            {
                return std::max(lower, std::min(n, upper));
            }
            /*!
            \brief Rounds a floating point value to the nearest whole number
            Note since C++11 you should probably prefer std::round()
            */
            static inline float round(float v)
            {
                return std::floor(v + 0.5f);
            }
            /*!
            \brief finds the shortest rotation, in degrees, between the given start and end angles
            */
            static inline float shortestRotation(float start, float end)
            {
                float diff = end - start;
                if (diff > 180.f) diff -= 360.f;
                else if (diff < -180.f) diff += 360.f;
                return diff;
            }

            /*!
            \brief Returns the normlised sign of the given value
            \param an int convertible value
            \returns -1 if the given value is negative, 1 if it is positive or 0
            if it has no value.
            */
            template <typename T>
            static inline std::int32_t sgn(T val)
            {
                static_assert(std::is_convertible<T, std::int32_t>::value);
                return (T(0) < val) - (val < T(0));
            }
        }
    }
}
