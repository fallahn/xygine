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

#include <xyginext/core/Log.hpp>

#include <iostream>
#include <fstream>
#include <sstream>

using namespace xy;

sf::Mutex Logger::m_mutex;
std::list<std::string> Logger::m_buffer;
std::string Logger::m_stringOutput;
sf::Clock Logger::m_messageClock;

//public
std::ostream& Logger::log(Type type, Output output)
{
    static Detail::LogStream stream(type, output, "", true);
    stream.setType(type);
    stream.setOutput(output);

    switch (type)
    {
    case Type::Info:
    default:
        stream << "INFO: ";
        break;
    case Type::Error:
        stream << "ERROR: ";
        break;
    case Type::Warning:
        stream << "WARNING: ";
        break;
    }

    return stream;
}

void Logger::log(const std::string& message, Type type, Output output)
{
    std::string outstring;// (std::to_string(m_messageClock.restart().asSeconds()));
    switch (type)
    {
    case Type::Info:
    default:
        outstring += "INFO: " + message;
        break;
    case Type::Error:
        outstring += "ERROR: " + message;
        break;
    case Type::Warning:
        outstring += "WARNING: " + message;
        break;
    }

    if (outstring.back() != '\n')
    {
        outstring.push_back('\n');
    }

    logPreFormatted(outstring, type, output);
}

void Logger::logPreFormatted(const std::string& outstring, Type type, Output output)
{
    sf::Lock lock(m_mutex);
    if (output == Output::Console || output == Output::All)
    {
        (type == Type::Error) ?
            std::cerr << outstring << /*std::endl*/std::flush
            :
            std::cout << outstring << /*std::endl*/std::flush;
#ifndef NO_UI_LOG
        Console::print(outstring);
#endif //NO_UI_LOG
        const std::size_t maxBuffer = 30;
        m_buffer.push_back(outstring);
        if (m_buffer.size() > maxBuffer)m_buffer.pop_front();
        updateOutString(maxBuffer);

#ifdef _MSC_VER
        OutputDebugStringA(outstring.c_str());
#endif //_MSC_VER
    }
    if (output == Output::File || output == Output::All)
    {
        //output to a log file
        std::ofstream file("output.log", std::ios::app);
        if (file.good())
        {
            file << /*SysTime::dateString() << "-" << SysTime::timeString() << ": "<< */ outstring << std::flush;
            file.close();
        }
        else
        {
            logPreFormatted(outstring, type, Output::Console);
            log("Above message was intended for log file. Opening file probably failed.", Type::Warning, Output::Console);
        }
    }
}

//private
void Logger::updateOutString(std::size_t maxBuffer)
{
    static size_t count = 0;
    m_stringOutput.append(m_buffer.back());
    m_stringOutput.append("\n");
    count++;

    if (count > maxBuffer)
    {
        m_stringOutput = m_stringOutput.substr(m_stringOutput.find_first_of('\n') + 1, m_stringOutput.size());
        count--;
    }
}

//logger stream buffer used with LogStream
using namespace Detail;

LogBuf::LogBuf(Logger::Type type, Logger::Output output, const std::string& prefix, bool preFormatted)
    : m_type        (type),
    m_output        (output),
    m_prefix        (prefix),
    m_preFormatted  (preFormatted)
{
    static const std::size_t size = 128;
    char* buffer = new char[size];
    setp(buffer, buffer + size);
}

LogBuf::~LogBuf()
{
    sync();
    delete[] pbase();
}

//private
int LogBuf::overflow(int character)
{
    if ((character != EOF) && (pptr() != epptr()))
    {
        return sputc(static_cast<char>(character));
    }
    else if (character != EOF)
    {
        sync();
        return overflow(character);
    }
    else
    {
        return sync();
    }

    return 0;
}

int LogBuf::sync()
{
    if (pbase() != pptr())
    {
        //print the contents of the write buffer into the logger
        //TODO this would be better to write directly to the target
        //output here instead of forwarding to existing log functions
        std::size_t size = static_cast<int>(pptr() - pbase());

        std::stringstream ss;
        ss.write(pbase(), size);

        if (m_preFormatted)
        {
            Logger::logPreFormatted(ss.str(), m_type, m_output);
        }
        else
        {
            Logger::log(m_prefix + ss.str(), m_type, m_output);
        }
        setp(pbase(), epptr());
    }

    return 0;
}

//used to print to the logger with c++ streams
LogStream::LogStream(Logger::Type type, Logger::Output output, const std::string& prefix, bool preFormatted)
    : m_buffer  (type, output, prefix, preFormatted),
    std::ostream(&m_buffer)
{

}