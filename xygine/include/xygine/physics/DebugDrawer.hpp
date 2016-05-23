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

//implements the box2D debug draw interface

#ifndef XY_PHYS_DEBUG_DRAW_HPP_
#define XY_PHYS_DEBUG_DRAW_HPP_

#include <Box2D/Common/b2Draw.h>

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/ConvexShape.hpp>

namespace sf
{
    class RenderTarget;
}

namespace xy
{
    namespace Physics
    {
        /*!
        \brief Implements the box2D debug draw interface

        This is used when drawing the physics world. You don't need to manually use this.
        */
        class DebugDraw final : public b2Draw
        {
        public:
            explicit DebugDraw(sf::RenderTarget&);
            ~DebugDraw() = default;

            DebugDraw(const DebugDraw&) = delete;
            DebugDraw& operator = (const DebugDraw&) = delete;

            void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& colour) override;
            void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& colour) override;
            void DrawCircle(const b2Vec2& centre, float32 radius, const b2Color& colour) override;
            void DrawSolidCircle(const b2Vec2& centre, float32 radius, const b2Vec2& axis, const b2Color& colour) override;
            void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& colour) override;
            void DrawTransform(const b2Transform& xf) override;
            void DrawPoint(const b2Vec2& p, float32 size, const b2Color& color) override;

        private:

            sf::RenderTarget& m_renderTarget;
            sf::CircleShape m_circleShape;
            sf::ConvexShape m_convexShape;
        };
    }
}

#endif //XY_PHYS_DEBUG_DRAW_HPP_
