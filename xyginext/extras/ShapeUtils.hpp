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

#include <xyginext/ecs/components/Drawable.hpp>
#include <xyginext/util/Const.hpp>

#include <cmath>

/*!
\brief Utility functions for creating drawable shapes.
Usage: pass in a drawable component attached to an entity
along with any extra parameters and the function will update
the component vertices to create the requested shape. This
is useful, for example, for drawing physics debug output
*/

namespace Shape
{
    /*!
    \brief Creates a circle shape around the origin point.
    \param radius Radius of the circle to create
    \param colour Colour to set the created circle
    \param pointCount Number of points to use to create the circle. Setting this
    to a lower number such as 5 or 6 can be useful for creating regulat polygons
    like hexagons or pentagons.
    */
    static inline void setCircle(xy::Drawable& drawable, float radius, sf::Color colour = sf::Color::Green, int pointCount = 10)
    {
        auto& verts = drawable.getVertices();
        verts.clear();

        float step = xy::Util::Const::TAU / pointCount;
        for (auto i = 0; i < pointCount; ++i)
        {
            sf::Vector2f p(std::sin(i * step), std::cos(i * step));
            p *= radius;
            verts.emplace_back(p, colour);
        }
        verts.push_back(verts.front());
        verts.emplace_back(sf::Vector2f(), colour);

        drawable.setPrimitiveType(sf::LineStrip);
        drawable.updateLocalBounds();
    }

    /*!
    \brief Creates a rectangle shape.
    \param size Size of the rectangle to create
    \param colour Colour to set the rectangle
    */
    static inline void setRectangle(xy::Drawable& drawable, sf::Vector2f size, sf::Color colour = sf::Color::Green)
    {
        auto& verts = drawable.getVertices();
        verts.clear();

        verts.emplace_back(sf::Vector2f(), colour);
        verts.emplace_back(sf::Vector2f(size.x, 0.f), colour);
        verts.emplace_back(size, colour);
        verts.emplace_back(sf::Vector2f(0.f, size.y), colour);
        verts.push_back(verts.front());

        drawable.setPrimitiveType(sf::LineStrip);
        drawable.updateLocalBounds();
    }

    /*!
    \brief Creates a chain of segments from the given list of points.
    \param points std::vector of sf::Vector2f used as points to create the line
    \param colour Colour to use when drawing the line
    */
    static inline void setPolyLine(xy::Drawable& drawable, const std::vector<sf::Vector2f>& points, sf::Color colour = sf::Color::Green)
    {
        auto& verts = drawable.getVertices();
        verts.clear();
        for (auto p : points)
        {
            verts.emplace_back(p, colour);
        }
        drawable.setPrimitiveType(sf::LineStrip);
        drawable.updateLocalBounds();
    }
}