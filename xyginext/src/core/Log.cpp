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

using namespace xy;

LogBuf::LogBuf(Logger::Type type, Logger::Output output, const std::string& prefix)
    : m_type(type),
    m_output(output),
    m_prefix(prefix)
{
    static const std::size_t size = 64;
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
        std::size_t size = static_cast<int>(pptr() - pbase());

        std::stringstream ss;
        ss.write(pbase(), size);
        Logger::log(m_prefix + ss.str(), m_type, m_output);

        setp(pbase(), epptr());
    }

    return 0;
}

LogStream::LogStream(Logger::Type type, Logger::Output output, const std::string& prefix)
    : m_buffer  (type, output, prefix),
    std::ostream(&m_buffer)
{

}