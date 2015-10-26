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

#include <xygine/MessageBus.hpp>
#include <xygine/Log.hpp>

using namespace xy;

namespace
{
    const std::size_t bufferSize = 2048u;
}

MessageBus::MessageBus()
    : m_currentBuffer   (bufferSize),
    m_pendingBuffer     (bufferSize),
    m_currentPointer    (m_pendingBuffer.data())
{}

Message MessageBus::poll()
{
    Message m = m_currentMessages.front();
    m_currentMessages.pop();

    return m;
}

bool MessageBus::empty()
{
    if (m_currentMessages.empty())
    {
        m_currentMessages.swap(m_pendingMessages);
        m_currentBuffer.swap(m_pendingBuffer);
        m_currentPointer = m_pendingBuffer.data();
        return true;
    }
    return false;
}

std::size_t MessageBus::pendingMessageCount() const
{
    return m_currentMessages.size();
}