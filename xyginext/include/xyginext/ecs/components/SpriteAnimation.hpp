/*********************************************************************
(c) Matt Marchant 2017 - 2020
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

#include "xyginext/Config.hpp"
#include <SFML/Config.hpp>

namespace xy
{
    /*!
    \brief Component which contains information about the currently
    playing sprite animation. Requires a SpriteAnimator system in the scene.
    */
    struct XY_EXPORT_API SpriteAnimation final
    {
        /*!
        \brief Play the animation at the given index if it exists
        */
        void play(std::size_t index) { m_id = index; m_playing = true; }

        /*!
        \brief Returns the currently playing animation index, or the
        last played index if the animation is currently not playing
        */
        std::size_t getAnimationIndex() const { return m_id; }

        /*!
        \brief Pause the playing animation, if there is one
        */
        void pause() { m_playing = false; }

        /*!
        \brief Stops the current animation if it is playing and
        rewinds it to the first frame if it is playing or paused
        */
        void stop() { m_playing = false; m_frameID = 0; }

        /*!
        \brief Returns true if the current animation has stopped playing
        */
        bool stopped() const { return !m_playing; }
        
        /*!
        \brief Returns the current frame ID
        */
        sf::Uint32 getFrameID() const { return m_frameID; }
        
        /*!
        \brief Set the current frame ID
        */
        void setFrameID(sf::Uint32 frameID) { m_frameID = frameID; }

    private:
        std::size_t m_id = 0;
        bool m_playing = false;
        float m_currentFrameTime = 0.f;
        sf::Uint32 m_frameID = 0;

        friend class SpriteAnimator;
    };
}
