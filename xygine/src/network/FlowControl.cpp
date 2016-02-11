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

#include <xygine/network/FlowControl.hpp>

#include <xygine/Log.hpp>

using namespace xy::Network;

namespace
{
    const float GOOD = 30.f;
    const float BAD = 10.f;
    const float RTT_Threshold = 250.0f;
}

FlowControl::FlowControl()
    : m_sendRate                    (BAD),
    m_penaltyTime                   (4.f),
    m_goodConditionTime             (0.f),
    m_penaltyReductionAccumulator   (0.f) {}

//public
void FlowControl::reset()
{
    m_sendRate = BAD;
    m_penaltyTime = 4.f;
    m_goodConditionTime = 0.f;
    m_penaltyReductionAccumulator = 0.f;
}

void FlowControl::update(float dt, float rtt)
{
    if (m_sendRate == GOOD)
    {
        if (rtt > RTT_Threshold)
        {
            LOG("Flow Control: decreasing send rate", xy::Logger::Type::Info);
            m_sendRate = BAD;
            if (m_goodConditionTime < 10.f && m_penaltyTime < 60.f)
            {
                m_penaltyTime *= 2.f;
                if (m_penaltyTime > 60.f) m_penaltyTime = 60.f;
            }
            m_goodConditionTime = 0.f;
            m_penaltyReductionAccumulator = 0.f;
            return;
        }

        m_goodConditionTime += dt;
        m_penaltyReductionAccumulator += dt;

        if (m_penaltyReductionAccumulator > 10.f && m_penaltyTime > 1.f)
        {
            m_penaltyTime /= 2.f;
            if (m_penaltyTime < 1.f) m_penaltyTime = 1.f;

            m_penaltyReductionAccumulator = 0.f;
        }
    }

    if (m_sendRate == BAD)
    {
        if (rtt <= RTT_Threshold)
        {
            m_goodConditionTime += dt;
        }
        else
        {
            m_goodConditionTime = 0.f;
        }

        if (m_goodConditionTime > m_penaltyTime)
        {
            LOG("Flow Control: Increasing send rate", xy::Logger::Type::Info);
            m_goodConditionTime = 0.f;
            m_penaltyReductionAccumulator = 0.f;
            m_sendRate = GOOD;
            return;
        }
    }
}

float FlowControl::getSendRate() const
{
    return m_sendRate;
}