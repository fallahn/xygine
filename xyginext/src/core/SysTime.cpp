/*********************************************************************
(c) Matt Marchant 2017
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

#include <xyginext/core/SysTime.hpp>

#include <ctime>
#include <sstream>
#include <iomanip>

using namespace xy;

namespace
{
    SysTime::Data data;
}

SysTime::Data::Data()
{
    update();
}

void SysTime::Data::update() const
{
    auto time = std::time(nullptr);
    m_time = std::localtime(&time);
}

//public
int SysTime::Data::seconds() const
{
    update();
    return m_time->tm_sec;
}

int SysTime::Data::minutes() const
{
    update();
    return m_time->tm_min;
}

int SysTime::Data::hours() const
{
    update();
    return m_time->tm_hour;
}

int SysTime::Data::days() const
{
    update();
    return m_time->tm_mday;
}

int SysTime::Data::months() const
{
    update();
    return m_time->tm_mon + 1;
}

int SysTime::Data::year() const 
{
    update();
    return m_time->tm_year + 1900;
}

const SysTime::Data& SysTime::now()
{
    return data;
}

std::uint64_t SysTime::epoch()
{
    return std::time(nullptr);
}

std::string SysTime::dateString()
{
    Data data;
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << data.days() << "/"
        << std::setw(2) << std::setfill('0') << data.months() << "/"
        << data.year();
    
    return ss.str();
}

std::string SysTime::timeString()
{
    Data data;
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << data.hours() << ":"
        << std::setw(2) << std::setfill('0') << data.minutes() << ":"
        << std::setw(2) << std::setfill('0') << data.seconds();

    return ss.str();
}