/*********************************************************************
(c) Matt Marchant 2017 - 2020
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

#include "Const.hpp"

#include <cmath>

namespace xy
{
    namespace Util
    {
        /*!
        \brief Port of https://github.com/nicolausYes/easing-functions
        Each function takes a given amount of time / progress in a normalised
        0 - 1 range, and returns a value 0 - 1 based on the current position
        along a curve. Examples of each curve can be found at https://easings.net/en

        Easing curves are generally useful for creating interesting motion in 
        animated items such as UI objects. They can also be used to create tables
        of values where lookups are faster than repeatedly calling functions
        that use sin/cos/sqrt.
        */
        namespace Easing
        {
            static inline float easeInSine(float t)
            {
                return std::sin(1.5707963f * t);
            }

            static inline float easeOutSine(float t)
            {
                return 1 + std::sin(1.5707963f * (--t));
            }

            static inline float easeInOutSine(float t)
            {
                return 0.5f * (1 + std::sin(xy::Util::Const::PI * (t - 0.5f)));
            }

            static inline float easeInQuad(float t)
            {
                return t * t;
            }

            static inline float easeOutQuad(float t)
            {
                return t * (2.f - t);
            }

            static inline float easeInOutQuad(float t)
            {
                if (t < 0.5f)
                {
                    return 2.f * t * t;
                }
                else
                {
                    return t * (4.f - 2.f * t) - 1;
                }
            }

            static inline float easeInCubic(float t)
            {
                return t * t * t;
            }

            static inline float easeOutCubic(float t)
            {
                return 1.f + (--t) * t * t;
            }

            static inline float easeInOutCubic(float t)
            {
                if (t < 0.5f)
                {
                    return 4.f * t * t * t;
                }
                else
                {
                    return 1.f + (--t) * (2.f * (--t)) * (2.f * t);
                }
            }

            static inline float easeInQuart(float t)
            {
                t *= t;
                return t * t;
            }

            static inline float easeOutQuart(float t)
            {
                t = (--t) * t;
                return 1.f - t * t;
            }

            static inline float easeInOutQuart(float t)
            {
                if (t < 0.5f)
                {
                    t *= t;
                    return 8.f * t * t;
                }
                else
                {
                    t = (--t) * t;
                    return 1.f - 8.f * t * t;
                }
            }

            static inline float easeInQuint(float t)
            {
                float t2 = t * t;
                return t * t2 * t2;
            }

            static inline float easeOutQuint(float t)
            {
                float t2 = (--t) * t;
                return 1.f + t * t2 * t2;
            }

            static inline float easeInOutQuint(float t)
            {
                float t2;
                if (t < 0.5f)
                {
                    t2 = t * t;
                    return 16.f * t * t2 * t2;
                }
                else
                {
                    t2 = (--t) * t;
                    return 1.f + 16.f * t * t2 * t2;
                }
            }

            static inline float easeInExpo(float t)
            {
                return (std::pow(2.f, 8.f * t) - 1) / 255.f;
            }

            static inline float easeOutExpo(float t)
            {
                return 1.f - std::pow(2.f, -8.f * t);
            }

            static inline float easeInOutExpo(float t)
            {
                if (t < 0.5f)
                {
                    return (std::pow(2.f, 16.f * t) - 1.f) / 510.f;
                }
                else
                {
                    return 1.f - 0.5f * std::pow(2.f, -16.f * (t - 0.5f));
                }
            }

            static inline float easeInCirc(float t)
            {
                return 1.f - std::sqrt(1.f - t);
            }

            static inline float easeOutCirc(float t)
            {
                return std::sqrt(t);
            }

            static inline float easeInOutCirc(float t)
            {
                if (t < 0.5f)
                {
                    return (1.f - std::sqrt(1.f - 2.f * t)) * 0.5f;
                }
                else
                {
                    return (1.f + std::sqrt(2.f * t - 1.f)) * 0.5f;
                }
            }

            static inline float easeInBack(float t)
            {
                return t * t * (2.70158f * t - 1.70158f);
            }

            static inline float easeOutBack(float t)
            {
                return 1.f + (--t) * t * (2.70158f * t + 1.70158f);
            }

            static inline float easeInOutBack(float t)
            {
                if (t < 0.5f)
                {
                    return t * t * (7.f * t - 2.5f) * 2.f;
                }
                else
                {
                    return 1.f + (--t) * t * 2.f * (7.f * t + 2.5f);
                }
            }

            static inline float easeInElastic(float t)
            {
                float t2 = t * t;
                return t2 * t2 * std::sin(t * xy::Util::Const::PI * 4.5f);
            }

            static inline float easeOutElastic(float t)
            {
                float t2 = (t - 1.f) * (t - 1.f);
                return 1.f - t2 * t2 * std::cos(t * xy::Util::Const::PI * 4.5f);
            }

            static inline float easeInOutElastic(float t)
            {
                float t2;
                if (t < 0.45f)
                {
                    t2 = t * t;
                    return 8.f * t2 * t2 * std::sin(t * xy::Util::Const::PI * 9.f);
                }
                else if (t < 0.55f)
                {
                    return 0.5f + 0.75f * std::sin(t * xy::Util::Const::PI * 4.f);
                }
                else
                {
                    t2 = (t - 1.f) * (t - 1.f);
                    return 1.f - 8.f * t2 * t2 * std::sin(t * xy::Util::Const::PI * 9.f);
                }
            }

            static inline float easeInBounce(float t)
            {
                return std::pow(2.f, 6.f * (t - 1.f)) * std::abs(std::sin(t * xy::Util::Const::PI * 3.5f));
            }

            static inline float easeOutBounce(float t)
            {
                return 1.f - std::pow(2.f, -6.f * t) * std::abs(std::cos(t * xy::Util::Const::PI * 3.5f));
            }

            static inline float easeInOutBounce(float t)
            {
                if (t < 0.5f)
                {
                    return 8.f * std::pow(2.f, 8.f * (t - 1.f)) * std::abs(std::sin(t * xy::Util::Const::PI * 7.f));
                }
                else
                {
                    return 1.f - 8.f * std::pow(2.f, -8.f * t) * std::abs(std::sin(t * xy::Util::Const::PI * 7.f));
                }
            }
        }
    }
}