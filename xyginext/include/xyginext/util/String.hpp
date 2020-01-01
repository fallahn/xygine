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

#include "xyginext/core/Assert.hpp"

#include <vector>
#include <string>
#include <algorithm>

namespace xy
{
    namespace Util
    {
        /*!
        \brief String manipulation functions
        */
        namespace String
        {
            /*!
            \brief Converts a comma delimited string of floats into an array
            */
            static inline std::vector<float> toFloatArray(const std::string& str)
            {
                std::vector<float> values;
                std::size_t start = 0u;
                auto next = str.find_first_of(',');
                while (next != std::string::npos && start < str.length())
                {
                    try
                    {
                        values.push_back(std::stof(str.substr(start, next)));
                    }
                    catch (...)
                    {
                        values.push_back(0.f);
                    }
                    start = ++next;
                    next = str.find_first_of(',', start);
                    if (next > str.length()) next = str.length();
                }
                return values;
            }

            /*!
            \brief Remove all isntances of c from line
            */
            static inline void removeChar(std::string& line, const char c)
            {
                line.erase(std::remove(line.begin(), line.end(), c), line.end());
            }

            /*!
            \brief Splits a string with a given token and returns a vector of results
            */
            static inline std::vector<std::string> tokenize(const std::string& str, char delim, bool keepEmpty = false)
            {
                XY_ASSERT(!str.empty(), "string empty");
                std::stringstream ss(str);
                std::string token;
                std::vector<std::string> output;
                while (std::getline(ss, token, delim))
                {
                    if (!token.empty() ||
                        (token.empty() && keepEmpty))
                    {
                        output.push_back(token);
                    }
                }
                return output;
            }

            /*!
            \brief Converts a string to all lower case
            */
            static inline std::string toLower(const std::string& str)
            {
                std::string result = str;
                std::transform(result.begin(), result.end(), result.begin(), ::tolower);
                return result;
            }

            /*!
            \brief Converts a string representation of an IPv4 address to a
            32bit uint in host byte order ie 127.0.0.1 == 0x7f000001
            */
            static inline std::uint32_t toHostOrderIPv4(const std::string& str)
            {
                //TODO sanity checks on input
                std::stringstream s(str);
                std::uint32_t a, b, c, d;
                char ch;
                s >> a >> ch >> b >> ch >> c >> ch >> d;
                return (a << 24) | (b << 16) | (c << 8) | (d);
            }

            /*!
            \brief Converts a 32bit IPv4 address to its string representation.
            \param bytes Uint32 value containing the ip address bytes in host
            byte order ie 127.0.0.1 == 0x7f000001
            */
            static inline std::string fromHostOrderIPv4(std::uint32_t bytes)
            {
                std::string ret = std::to_string((bytes & 0xFF000000) >> 24);
                ret += "." + std::to_string((bytes & 0x00FF0000) >> 16);
                ret += "." + std::to_string((bytes & 0x0000FF00) >> 8);
                ret += "." + std::to_string((bytes & 0x000000FF));
                return ret;
            }

            /*!
            \brief Converts a string representation of an IPv4 address to a
            32bit uint in network byte order ie 127.0.0.1 == 0x0100007f
            */
            static inline std::uint32_t toNetworkOrderIPv4(const std::string& str)
            {
                //TODO sanity checks on input
                std::stringstream s(str);
                std::uint32_t a, b, c, d;
                char ch;
                s >> a >> ch >> b >> ch >> c >> ch >> d;
                return (d << 24) | (c << 16) | (b << 8) | (a);
            }

            /*!
            \brief Converts a 32bit IPv4 address to its string representation.
            \param bytes Uint32 value containing the ip address bytes in network
            byte order ie 127.0.0.1 == 0x0100007f
            */
            static inline std::string fromNetworkOrderIPv4(std::uint32_t bytes)
            {
                std::string ret = std::to_string((bytes & 0x000000FF));
                ret += "." + std::to_string((bytes & 0x0000FF00) >> 8);
                ret += "." + std::to_string((bytes & 0x00FF0000) >> 16);
                ret += "." + std::to_string((bytes & 0xFF000000) >> 24);
                return ret;
            }

            /*!
            \brief Decode a UTF8 encoded string to a vector of uint32 codepoints.
            Donated by therocode https://github.com/therocode
            */
            static inline std::vector<uint32_t> getCodepoints(const std::string& str)
            {
                std::vector<uint32_t> codePoints;

                for (uint32_t i = 0; i < str.size(); ++i)
                {
                    if ((str[i] >> 7) == 0)
                    {
                        codePoints.push_back(uint32_t(str[i]));
                    }
                    else if (((uint8_t)str[i] >> 5) == 6)
                    {
                        // add "x"s to the unicode bits //
                        uint8_t character = str[i + 1];
                        // take away the "10" //
                        uint8_t bitmask = 127; // 0x01111111
                        uint32_t codePoint = uint32_t(character & bitmask);

                        // add "y"s to the unicode bits //
                        character = str[i];
                        // take away the "110" //
                        bitmask = 63; // 0x00111111
                        uint32_t yValues = uint32_t(character & bitmask);
                        codePoint = codePoint | (yValues << 6);

                        codePoints.push_back(codePoint);
                        ++i;
                    }
                    else if (((uint8_t)str[i] >> 4) == 14)
                    {
                        // add "x"s to the unicode bits //
                        uint8_t character = str[i + 2];
                        // take away the "10" //
                        uint8_t bitmask = 127; // 0x01111111
                        uint32_t codePoint = uint32_t(character & bitmask);

                        // add "y"s to the unicode bits //
                        character = str[i + 1];
                        // take away the "10" //
                        uint32_t yValues = uint32_t(character & bitmask);
                        codePoint = codePoint | (yValues << 6);

                        // add "z"s to the unicode bits //
                        character = str[i];
                        // take away the "1110" //
                        bitmask = 31; // 0x00011111
                        uint32_t zValues = uint32_t(character & bitmask);
                        codePoint = codePoint | (zValues << 12);

                        codePoints.push_back(codePoint);
                        i += 2;
                    }
                    else if (((uint8_t)str[i] >> 3) == 30)
                    {
                        // add "x"s to the unicode bits //
                        uint8_t character = str[i + 3];
                        // take away the "10" //
                        uint8_t bitmask = 127; // 0x01111111
                        uint32_t codePoint = uint32_t(character & bitmask);

                        // add "y"s to the unicode bits //
                        character = str[i + 2];
                        // take away the "10" //
                        uint32_t yValues = uint32_t(character & bitmask);
                        codePoint = codePoint | (yValues << 6);

                        // add "z"s to the unicode bits //
                        character = str[i + 1];
                        // take away the "10" //
                        uint32_t zValues = uint32_t(character & bitmask);
                        codePoint = codePoint | (zValues << 12);

                        // add "w"s to the unicode bits //
                        character = str[i];
                        // take away the "11110" //
                        bitmask = 7; // 0x00001111
                        uint32_t wValues = uint32_t(character & bitmask);
                        codePoint = codePoint | (wValues << 18);

                        codePoints.push_back(codePoint);
                        i += 3;
                    }
                }
                return codePoints;
            }
        }
    }
}
