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

#include <xygine/mesh/BoundingBox.hpp>

#include <array>
#include <functional>

using namespace xy;

void BoundingBox::transform(const glm::mat4& matrix)
{
    //calc corners.
    std::array<glm::vec3, 8u> corners =
    {
        glm::vec3(m_min.x, m_max.y, m_max.z),
        { m_min.x, m_min.y, m_max.z },
        { m_max.x, m_min.y, m_max.z },
        m_max,
        { m_max.x, m_max.y, m_min.z },
        { m_max.x, m_min.x, m_min.z },
        m_min,
        { m_min.x, m_max.y, m_min.z }
    };

    std::function<void(glm::vec3&, glm::vec3&, glm::vec3&)> update =
        [](glm::vec3& point, glm::vec3& min, glm::vec3& max)
    {
        if (point.x < min.x)
        {
            min.x = point.x;
        }
        if (point.x > max.x)
        {
            max.x = point.x;
        }
        if (point.y < min.y)
        {
            min.y = point.y;
        }
        if (point.y > max.y)
        {
            max.y = point.y;
        }
        if (point.z < min.z)
        {
            min.z = point.z;
        }
        if (point.z > max.z)
        {
            max.z = point.z;
        }
    };

    //transform the corners, recalculating the min and max points along the way.
    corners[0] = glm::vec3(matrix * glm::vec4(corners[0], 1.f));
    glm::vec3 newMin = corners[0];
    glm::vec3 newMax = corners[0];
    for (auto i = 1u; i < corners.size(); ++i)
    {
        corners[i] = glm::vec3(matrix * glm::vec4(corners[i], 1.f));
        update(corners[i], newMin, newMax);
    }
    m_min = newMin;
    m_max = newMax;
}

BoundingBox& BoundingBox::operator*= (const glm::mat4& matrix)
{
    transform(matrix);
    return *this;
}