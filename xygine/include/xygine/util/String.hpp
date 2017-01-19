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

#ifndef XY_UTIL_STRING_HPP_
#define XY_UTIL_STRING_HPP_

#include <xygine/Assert.hpp>

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
                auto start = 0u;
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
        }
    }
}

#endif //XY_UTIL_STRING_HPP_