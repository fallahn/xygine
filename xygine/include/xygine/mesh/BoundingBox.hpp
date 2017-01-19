/*********************************************************************
© Matt Marchant 2014 - 2017
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

#ifndef XY_BOUNDINGBOX_HPP_
#define XY_BOUNDINGBOX_HPP_

#include <xygine/Config.hpp>
#include <xygine/Assert.hpp>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <SFML/Graphics/Rect.hpp>

namespace xy
{
    /*!
    \brief BoundingBox class used to represent the AABB of model meshes
    */
    class XY_EXPORT_API BoundingBox final
    {
    public:
        BoundingBox(const glm::vec3& min = glm::vec3(-0.5f, -0.5f, -0.5f), const glm::vec3& max = glm::vec3(0.5f, 0.5f, 0.5f));
        ~BoundingBox() = default;
        BoundingBox(BoundingBox&&) noexcept = default;
        BoundingBox& operator = (BoundingBox&&) = default;
        BoundingBox(const BoundingBox&) = default;
        BoundingBox& operator = (const BoundingBox&) = default;

        /*!
        \brief Returns this bounding box as a 2D AABB using only
        the x and y coordinates of the box
        */
        sf::FloatRect asFloatRect() const
        {
            return m_floatRect;
        }

        /*!
        \brief Transforms the BoundingBox by the give matrix
        */
        void transform(const glm::mat4&);

        BoundingBox& operator *= (const glm::mat4&);

        /*!
        \brief Sets the relative scale of the bounding box
        when a model has a non-zero depth value
        */
        void setDepthScale(float);

    private:
        glm::vec3 m_min;
        glm::vec3 m_max;

        sf::FloatRect m_floatRect;
        float m_depthScale;

        void updateFloatRect();
    };
}


#endif //XY_BOUNDINGBOX_HPP_