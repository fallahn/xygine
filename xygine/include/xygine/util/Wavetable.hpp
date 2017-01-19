/*********************************************************************
© Matt Marchant 2014 - 2017
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

#ifndef XY_UTIL_WAVETABLE_HPP_
#define XY_UTIL_WAVETABLE_HPP_

#include <xygine/util/Const.hpp>
#include <xygine/Assert.hpp>

namespace xy
{
    namespace Util
    {
        namespace Wavetable
        {
            /*!
            \brief Creates a vector containing a series of floating point values
            representing a single cycle of a sine wave.

            \param frequency Frequency of generated sine wave
            \param amplitude Amplitude of generated sine wave
            \param updateRate Rate at which the wave table is likely to be stepped through, normally
            the same rate at which the framework is updated (60hz)

            Sinewaves are useful for a variety of things, including dictating the motion of animated
            objects. Precalculating a wavetable is more efficient than repeatedly calling sin()
            */
            static inline std::vector<float> sine(float frequency, float amplitude = 1.f, float updateRate = 60.f)
            {
                XY_ASSERT(frequency > 0 && amplitude > 0 && updateRate > 0, "");

                float stepCount = updateRate / frequency;
                float step = Const::TAU / stepCount;
                std::vector<float> wavetable;
                for (float i = 0.f; i < stepCount; ++i)
                {
                    wavetable.push_back(std::sin(step * i) * amplitude);
                }

                return std::move(wavetable);
            }
        }
    }
}


#endif //XY_UTIL_WAVETABLE_HPP_