/*********************************************************************
(c) Matt Marchant 2017
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

#ifndef XY_RENDERABLE_HPP_
#define XY_RENDERABLE_HPP_

#include <xyginext/Config.hpp>

#include <SFML/Config.hpp>
#include <SFML/graphics/Rect.hpp>

#include <array>

namespace xy
{
    class Entity;
    /*!
    \brief Renderable interface for systems which draw parts of the scene.
    Systems which implement this will be drawn by any scene to which they are added.
    */
    class XY_EXPORT_API Renderable
    {
    public:
        Renderable() = default;
        virtual ~Renderable() = default;

        /*!
        \brief Renders this system.
        \param camera Entity containing a camera and transform component, automatically
        passed in by the scene when this system is drawn. Use this to accurately draw the system.
        */
        virtual void render(Entity) = 0;

    protected:
        /*!
        \brief Applies the given normalised viewport.
        Use this to set the viewport for the given camera component when rendering.
        Usually one would restore the existing viewport when done rendering, for consistency.
        \returns Newly set viewport in window coords
        */
        sf::IntRect applyViewport(sf::FloatRect);

        /*!
        \brief Restores the previously active viewport after a call to applyViewport()
        */
        void restorePreviousViewport();

    private:
        std::array<sf::Int32, 4> m_previousViewport{};
    };
}

#endif //XY_RENDERABLE_HPP_