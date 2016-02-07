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

#include <xygine/Reports.hpp>

#include <xygine/Assert.hpp>
#include <algorithm>

using namespace xy;

namespace
{
    const std::string interweebl(": ");
    StatsReporter reporter;
}

//---exported functions---//

const std::string& Stats::getString()
{
    return reporter.getString();
}

void Stats::report(const std::string& name, const std::string& value)
{
    reporter.report(name, value);
}

//-----------------------------//

StatsReporter::StatsReporter()
    : m_rebuildString(true)
{

}

//public
void StatsReporter::report(const std::string& name, const std::string& value)
{
    XY_ASSERT(!name.empty() && !value.empty(), "empty string values are not allowed");
    sf::Lock lock(m_mutex);
    m_data[name] = value;
    m_rebuildString = true;
}

void StatsReporter::remove(const std::string& name)
{
    sf::Lock lock(m_mutex);
    m_data.erase(name);
    m_rebuildString = true;
}

const std::string& StatsReporter::getString()
{
    sf::Lock lock(m_mutex);
    if (m_rebuildString)
    {
        m_rebuildString = false;

        m_string.clear();
        for (const auto& p : m_data)
        {
            m_string.append(p.first);
            m_string.append(interweebl);
            m_string.append(p.second);
            m_string.append("\n");
        }
    }

    return m_string;
}

//StatsReporter StatsReporter::reporter;