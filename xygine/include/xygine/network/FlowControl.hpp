/*********************************************************************
Matt Marchant 2014 - 2017
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

#ifndef XY_FLOW_CONTROL_HPP_
#define XY_FLOW_CONTROL_HPP_

#include <xygine/Config.hpp>

namespace xy
{
    namespace Network
    {
        /*!
        \brief Used internally by xygine to calculate recommended send rate
        for a connection to prevent flooding it.
        */
        class XY_EXPORT_API FlowControl final
        {
        public:
            FlowControl();
            ~FlowControl() = default;
            FlowControl(const FlowControl&) = delete;
            FlowControl& operator = (const FlowControl&) = delete;

            /*!
            \brief Resets the control to default values
            */
            void reset();
            /*!
            \brief Updates the internal state of the flow control
            object.
            \param dt Delta time (in seconds) since last update
            \param rtt Time (in seconds) of the estimated round trip of a packet
            */
            void update(float dt, float rtt);
            /*!
            \brief Returns the current recommended send rate in packets per second
            */
            float getSendRate() const;

        private:
            float m_sendRate;
            float m_penaltyTime;
            float m_goodConditionTime;
            float m_penaltyReductionAccumulator;
        };
    }
}
#endif //XY_FLOW_CONTROL_HPP_