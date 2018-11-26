/*********************************************************************
(c) Matt Marchant 2017 - 2018
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

#pragma once

#include <xyginext/core/Assert.hpp>

#include <array>

/*!
\brief Simple, fixed size, circular buffer
*/
template <typename T, std::size_t s>
class CircularBuffer final
{
public:
    CircularBuffer()
        : m_front(0), m_back(0), m_nextFree(0), m_size(0), m_data()
    {

    }

    T pop_front()
    {
        XY_ASSERT(m_size > 0, "Invalid buffer size");
        auto ret = front();

        m_front = (m_front + 1) % m_data.size();
        m_size--;

        return ret;
    }

    void push_back(const T& value)
    {
        XY_ASSERT(m_size < m_data.size(), "Buffer full");
        m_data[m_nextFree] = value;
        m_back = m_nextFree;
        m_nextFree = (m_back + 1) % m_data.size();

        m_size++;
    }

    T& front()
    {
        XY_ASSERT(m_size > 0, "Invalid buffer size");
        return m_data[m_front];
    }

    const T& front() const
    {
        XY_ASSERT(m_size > 0, "Invalid buffer size");
        return front();
    }

    T& back()
    {
        XY_ASSERT(m_size > 0, "Invalid buffer size");
        return m_data[m_back];
    }

    const T& back() const
    {
        XY_ASSERT(m_size > 0, "Invalid buffer size");
        return back();
    }

    std::size_t size() const
    {
        return m_size;
    }

    constexpr std::size_t capacity() const
    {
        return s;
    }

private:
    std::size_t m_front;
    std::size_t m_back;
    std::size_t m_nextFree;
    std::size_t m_size;
    std::array<T, s> m_data;
};
