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

#include "xyginext/core/FileSystem.hpp"
#include "xyginext/core/Console.hpp"
#include "xyginext/core/SysTime.hpp"

#include <SFML/System/Lock.hpp>
#include <SFML/System/Mutex.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Graphics/Rect.hpp>

#include <string>
#include <list>
#include <sstream>

#ifdef _MSC_VER
#define NOMINMAX
#include <Windows.h>
#endif //_MSC_VER

namespace xy
{
    namespace Detail
    {
        class LogBuf;
    }

    /*!
    \brief Class to allowing messages to be logged to a combination
    of one or more destinations such as the console, log file or
    output window in Visual Studio
    */
    class XY_EXPORT_API Logger final
    {
    public:
        enum class Output
        {
            Console,
            File,
            All
        };

        enum class Type
        {
            Info,
            Warning,
            Error
        };

        /*!
        \brief Outputs to the log via the stream operator <<
        Simplified output interface wo allow logging similar to cout or cerr
        \param type Type of output. Defaults to Info
        \param output Output destination. Defaults to Console.
        Example:
        \code
        Logger::log() << "Hello\n";
        Logger::log(Logger::Type::Error) << "something is wrong!" << std::endl;
        \endcode
        */
        static std::ostream& log(Type = Type::Info, Output = Output::Console);

        /*!
        \brief Logs a message to a given destination.
        \param message Message to log
        \param type Whether this message gets tagged as information, a warning or an error
        \param output Destination for the message. Can be the console via cout, a log file on disk, or both
        */
        static void log(const std::string& message, Type type = Type::Info, Output output = Output::Console);

    private:
        static sf::Mutex m_mutex;
        static std::list<std::string> m_buffer;
        static std::string m_stringOutput;
        static sf::Clock m_messageClock; //< number of seconds since the last message

        static void updateOutString(std::size_t maxBuffer);

        static void logPreFormatted(const std::string&, Type, Output);

        friend class xy::Detail::LogBuf;
    };

    namespace Detail
    {
        //used for custom log streams such as
        //redirecting sf::err() to the console
        class LogBuf : public std::streambuf
        {
        public:
            LogBuf(xy::Logger::Type, xy::Logger::Output, const std::string&, bool preFormatted);
            ~LogBuf();

            void setType(xy::Logger::Type type) { m_type = type; }
            void setOutput(xy::Logger::Output output) { m_output = output; }

        private:
            xy::Logger::Type m_type;
            xy::Logger::Output m_output;
            std::string m_prefix;
            bool m_preFormatted;

            int overflow(int character) override;
            int sync() override;
        };

        class LogStream : public std::ostream
        {
        public:
            LogStream(xy::Logger::Type = xy::Logger::Type::Info, xy::Logger::Output = xy::Logger::Output::Console, const std::string& prefix = "", bool preFormatted = false);

            void setType(xy::Logger::Type type) { m_buffer.setType(type); }
            void setOutput(xy::Logger::Output output) { m_buffer.setOutput(output); }

        private:
            LogBuf m_buffer;
        };
    }
}

template <typename T>
std::ostream& operator << (std::ostream& stream, sf::Vector2<T> v)
{
    stream << "{ " << v.x << ", " << v.y << " }";
    return stream;
}

template <typename T>
std::ostream& operator << (std::ostream& stream, sf::Vector3<T> v)
{
    stream << "{ " << v.x << ", " << v.y << ", " << v.z << " }";
    return stream;
}

template <typename T>
std::ostream& operator << (std::ostream& stream, sf::Rect<T> r)
{
    stream << "{ " << r.left << ", " << r.top << ", " << r.width << ", " << r.height << " }";
    return stream;
}

#define LogI xy::Logger::log(xy::Logger::Type::Info)
#define LogW xy::Logger::log(xy::Logger::Type::Warning)
#define LogE xy::Logger::log(xy::Logger::Type::Error)


#ifndef XY_DEBUG
#define LOG(message, type)
#else
#define LOG(message, type) {\
std::string fileName(__FILE__); \
fileName = xy::FileSystem::getFileName(fileName); \
std::stringstream ss; \
ss << message << " (" << fileName << ", " << __LINE__ << ")"; \
xy::Logger::log(ss.str(), type);}
#endif //XY_DEBUG
