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

#ifndef XY_SYSTIME_HPP_
#define XY_SYSTIME_HPP_

#include <xygine/Config.hpp>

#include <string>

namespace xy
{
    /*!
    \brief Static functions for common date/time queries
    using the system clock.
    */
    class XY_EXPORT_API SysTime final
    {
    public:
        /*!
        \brief Struct containing data/time information
        */
        struct XY_EXPORT_API Data final
        {
            Data();

            int seconds() const; //!< Seconds after the minute
            int minutes() const; //!< Minutes after the hour
            int hours() const; //!< Hours after midnight
            int days() const; //!< Day of the month
            int months() const; //!< Months of the year
            int year() const; //!< Current year

        private:
            mutable tm* m_time = nullptr;
            void update() const;
        };

        /*!
        \brief Returns a reference to a struct containing
        the current system dat and time information
        */
        static const Data& now();

        /*!
        \brief Returns the time since the epoch, in seconds
        */
        static std::uint64_t epoch();

        /*!
        \brief Returns the current date as a dd/mm/yyyy
        formatted string
        */
        static std::string dateString();

        /*!
        \brief Returns the current system time as a HH:MM:ss
        formatted string
        */
        static std::string timeString();
    private:
        //static Data m_data;
    };
}

#endif //XY_SYSTIME_HPP_