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

//allows classes to report data to a single point

#ifndef XY_REPORTS_HPP_
#define XY_REPORTS_HPP_

#ifdef _DEBUG_
#define REPORT(name, value) xy::StatsReporter::reporter.report(name, value)
#else
#define REPORT(name, value)
#endif //_DEBUG_

#include <SFML/System/Mutex.hpp>
#include <SFML/System/Lock.hpp>

#include <string>
#include <map>

namespace xy
{
    /*!
    \brief Utility class for reporting statistics

    Useful for reporting data such as message bus statistics or
    frame rate and printing it on screen
    */
    class StatsReporter final
    {
    public:
        StatsReporter();
        ~StatsReporter() = default;
        StatsReporter(const StatsReporter&) = delete;
        StatsReporter& operator = (const StatsReporter&) = delete;

        /*!
        \brief Reports a name/value pair
        \param name Name of the value to report
        \param value Value to report

        Normally used to report information such as frame rate or key bindings
        report("FPS", std::to_string(framerate));
        report("Space", "Jump");

        Once a name/value pair has been reported it will remain in the output
        even if the value is not updated. To update a value simply report
        the same name with a new value.
        */
        void report(const std::string& name, const std::string& value);
        /*!
        \brief Removes a report

        Should a name/value pair no longer need to be displayed use this
        passing the name of the report to remove to remove it from the output
        */
        void remove(const std::string& name);

        /*!
        \brief Returns a string containing all the reported data

        This returns the output of all the reported data as a string
        so that it may be displayed via any string supporting output
        eg sf::Text
        */
        const std::string& getString();

        /*!
        \brief static instance of a reporter

        Used as a global access point for the REPORT macro, available
        when build in debug mode.
        */
        static StatsReporter reporter;

    private:
        std::map<std::string, std::string> m_data;
        std::string m_string;
        bool m_rebuildString;

        sf::Mutex m_mutex;
    };
}
#endif //XY_REPORTS_HPP_
