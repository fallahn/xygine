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

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>

#include <cstdint>
#include <limits>
#include <cmath>
#include <array>

namespace xy
{
    namespace Util
    {
        namespace Net
        {
            static constexpr std::int16_t CompressionRange = std::numeric_limits<std::int16_t>::max();

            /*
            \brief Compresses a float value into a 16 bit fixed point representation to send over
            a network connection. This compression is lossy.

            \param input The floating point value to compress
            \param range The max/min value within which input is expected to fall

            Note that 'range' should be the max (and min when negative) value expected to be compressed
            by this function. Exceeding the range will cause the value to overflow. However, the smaller
            the range value, the more precision that is preserved, so choose this value carefully. Using
            a value divisible by 8 bits will help the compiler optimise the conversion with bitshift division.

            Based on the stack overflow answer given here: https://stackoverflow.com/a/1659563/

            \returns 16 bit integer containing the compressed value of the float
            \see decompressFloat()
            */
            static inline std::int16_t compressFloat(float input, std::int16_t range = 1024)
            {
                return static_cast<std::int16_t>(std::round(input * CompressionRange / range));
            }

            /*!
            \brief Decompresses the 16 bit value into a floating point number
            \param input The value to decompress
            \param range The range with which the value was compressed. Incorrect values will give unexpected
            results.
            \returns Decompressed floating point value.
            \see compressFloat()
            */
            static inline constexpr float decompressFloat(std::int16_t input, std::int16_t range = 1024)
            {
                return static_cast<float>(input) * range / CompressionRange;
            }

            /*!
            \brief Compresses a 3 component vector into an array of 3 16 bit integers
            \param v The vector to compress
            \param range The positive/negative range within which the component values
            are expected to fall
            \returns Array of 3 16 bit integers
            \see decompressVec3() compressFloat()
            */
            static inline std::array<std::int16_t, 3u> compressVec3(sf::Vector3f v, std::int16_t range = 256)
            {
                std::int16_t x = compressFloat(v.x, range);
                std::int16_t y = compressFloat(v.y, range);
                std::int16_t z = compressFloat(v.z, range);

                return { x,y,z };
            }

            /*!
            \brief Decompresses an array of 16 bit integers compressed with compressVec3
            \param v An array of 16 bit integers
            \param range The range value with which the vector was compressed.
            \returns A sf::Vector3f
            \see compressVec3() compressFloat()
            */
            static inline sf::Vector3f decompressVec3(std::array<std::int16_t, 3u> v, std::int16_t range = 256)
            {
                float x = decompressFloat(v[0], range);
                float y = decompressFloat(v[1], range);
                float z = decompressFloat(v[2], range);

                return { x, y, z };
            }

            /*!
            \brief Compresses a 2 component vector into an array of 2 16 bit integers
            \param v The vector to compress
            \param range The positive/negative range within which the component values
            are expected to fall
            \returns Array of 2 16 bit integers
            \see decompressVec2() compressFloat()
            */
            static inline std::array<std::int16_t, 2u> compressVec2(sf::Vector2f v, std::int16_t range = 256)
            {
                std::int16_t x = compressFloat(v.x, range);
                std::int16_t y = compressFloat(v.y, range);

                return { x,y };
            }

            /*!
            \brief Decompresses an array of 16 bit integers compressed with compressVec2
            \param v An array of 16 bit integers
            \param range The range value with which the vector was compressed.
            \returns sf::Vector2f
            \see compressVec2() compressFloat()
            */
            static inline sf::Vector2f decompressVec2(std::array<std::int16_t, 2u> v, std::int16_t range = 256)
            {
                float x = decompressFloat(v[0], range);
                float y = decompressFloat(v[1], range);

                return { x, y };
            }
        }
    }
}