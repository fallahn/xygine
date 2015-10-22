/*********************************************************************
Matt Marchant 2014 - 2015
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

//allows classes to report data to a single point

#ifndef REPORTS_HPP_
#define REPORTS_HPP_

#ifdef _DEBUG_
#define REPORT(name, value) xy::StatsReporter::reporter.report(name, value)
#else
#define REPORT(name, value)
#endif //_DEBUG_

#include <SFML/System/Mutex.hpp>
#include <SFML/System/Lock.hpp>

#include <string>
#include <unordered_map>

namespace xy
{
    class StatsReporter final
    {
    public:
        StatsReporter();
        ~StatsReporter() = default;
        StatsReporter(const StatsReporter&) = delete;
        StatsReporter& operator = (const StatsReporter&) = delete;

        void report(const std::string& name, const std::string& value);
        void remove(const std::string& name);

        const std::string& getString();

        static StatsReporter reporter;

    private:
        std::unordered_map<std::string, std::string> m_data;
        std::string m_string;
        bool m_rebuildString;

        sf::Mutex m_mutex;
    };
}
#endif //REPORTS_HPP_
