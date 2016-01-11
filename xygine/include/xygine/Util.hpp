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

#ifndef XY_UTIL_HPP_
#define XY_UTIL_HPP_

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

#include <cmath>
#include <random>
#include <ctime>
#include <xygine/Assert.hpp>
#include <sstream>

namespace //is this not moot here as the anonymous namespace gets included in any TU which use this?
{
    static std::default_random_engine rndEngine(static_cast<unsigned long>(std::time(0)));
    const float PI = 3.1412f;
    const float degToRad = PI / 180.f;
    const float radToDeg = 180.f / PI;
    const float TAU = PI * 2.f;
    const float E = 2.71828f;
}

namespace xy
{
    namespace Util
    {
        namespace Wavetable
        {
            /*!
            \brief Creates a vector containing a series of floating point values
            representing a single cycle of a sine wave.

            \param frequency Frency of generated sine wave
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
                float step = TAU / stepCount;
                std::vector<float> wavetable;
                for (float i = 0.f; i < stepCount; ++i)
                {
                    wavetable.push_back(std::sin(step * i) * amplitude);
                }

                return std::move(wavetable);
            }
        }

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
                if (length != 0) source /= length;
                return source;
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
                const float rads = degrees * degToRad;
                auto ca = std::cos(rads);
                auto sa = std::sin(rads);
                return{ ca*v.x - sa*v.y, sa*v.x + ca*v.y };
            }

            /*!
            \brief Gets the rotation (in degrees) of a vector
            */
            static inline float rotation(const sf::Vector2f v)
            {
                return std::atan2(v.y, v.x) * radToDeg;
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

        namespace Position
        {
            /*!
            \brief Centres the origin of sf::Transformable types
            */
            template <typename T>
            static inline void centreOrigin(T& transformable)
            {
                static_assert(std::is_base_of<sf::Transformable, T>::value, "only transformable type allowed");
                sf::FloatRect bounds = transformable.getLocalBounds();
                transformable.setOrigin(std::floor(bounds.width / 2.f), std::floor(bounds.height / 2.f));
            }
        }

        namespace Random
        {
            /*!
            \brief Returns a pseudo random floating point value
            \param begin Minimum value
            \param end Maximum value
            */
            static inline float value(float begin, float end)
            {
                XY_ASSERT(begin < end, "first value is not less than last value");
                std::uniform_real_distribution<float> dist(begin, end);
                return dist(rndEngine);
            }
            /*!
            \brief Returns a pseudo random integer value
            \param begin Minimum value
            \param end Maximum value
            */
            static inline int value(int begin, int end)
            {
                XY_ASSERT(begin < end, "first value is not less than last value");
                std::uniform_int_distribution<int> dist(begin, end);
                return dist(rndEngine);
            }
        }

        namespace Math
        {
            /*!
            \brief Clamps a given value to the given upper and lower bounds
            */
            template <typename T>
            static inline T clamp(const T& n, const T& lower, const T& upper)
            {
                return std::max(lower, std::min(n, upper));
            }
            /*!
            \brief Rounds a floating point value to the nearest whole number
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
        }

        namespace Rectangle
        {
            /*!
            \brief Returns true if the second given FloatRect is fully contained within the first
            */
            static inline bool contains(const sf::FloatRect& first, const sf::FloatRect& second)
            {
                if (second.left < first.left) return false;
                if (second.top < first.top) return false;
                if (second.left + second.width > first.left + first.width) return false;
                if (second.top + second.height > first.top + first.height) return false;

                return true;
            }
            /*!
            \brief Returns the centre point of a given FloatRect
            */
            static inline sf::Vector2f centre(const sf::FloatRect& rect)
            {
                return sf::Vector2f(rect.left + (rect.width / 2.f), rect.top + (rect.height / 2.f));
            }
            /*!
            \brief Returns a new FloatRect from the given upper and lower bounds.
            */
            static inline sf::FloatRect fromBounds(const sf::Vector2f& lower, const sf::Vector2f& upper)
            {
                return sf::FloatRect(lower.x, lower.y, upper.x - lower.x, upper.y - lower.y);
            }
        }
    }
}
#endif //XY_UTIL_HPP_