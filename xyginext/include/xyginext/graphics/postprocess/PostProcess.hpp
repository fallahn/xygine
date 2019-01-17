/*********************************************************************
(c) Matt Marchant 2017 - 2019
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

//abstract base class for full screen post process effects

#pragma once

#include "xyginext/Config.hpp"

#include <SFML/System/Vector2.hpp>
#include <SFML/Config.hpp>

#include <memory>
#include <functional>

namespace sf
{
    class RenderTarget;
    class RenderTexture;
    class Shader;
}

namespace xy
{
    /*!
    \brief Abstract base class for full screen post processes

    Post processes can be added to a scene so that they are applied
    after a scene is rendered. Multiple post processes can be chained
    but be aware that too many will affect performance.
    */
    class XY_EXPORT_API PostProcess
    {
    public:

        using Ptr = std::unique_ptr<PostProcess>;

        PostProcess() = default;
        virtual ~PostProcess() = default;
        PostProcess(const PostProcess&) = delete;
        PostProcess(PostProcess&&) = delete;
        PostProcess& operator = (const PostProcess&) = delete;
        PostProcess& operator = (PostProcess&&) = delete;

        /*!
        \brief Applies the effect
        \param RenderTexture A render Texture containing the scene to which the 
        effect should be applied.
        \param RenderTarget The target to which the effected scene is rendered.
        This may be the current render window, or another RenderTexture when
        chaining multiple effects together.

        The source and destinations are handled automatically by xygine when
        a post process effect is added to a scene, but must be specified
        maunally when using the effect outside the scene rendering.
        */
        virtual void apply(const sf::RenderTexture&, sf::RenderTarget&) = 0;
        
        /*!
        \brief Updates the effect.
        Some effects may require shader parameters to be updated over time.
        This optionally overridable function passes in the current frame time
        and allows updating those parameters. This is called automatically
        when the effect is added to a Scene, but will need to be called manually
        when using the effect on its own.
        */
        virtual void update(float) {}

        /*
        \brief Used by xygine to update the post process should the buffer be resized.
        This should not be called by the user.
        */
        void resizeBuffer(sf::Int32 w, sf::Int32 h);

    protected:
        static void applyShader(const sf::Shader&, sf::RenderTarget&);

        /*!
        \brief Called when the main output buffer resized.
        Implement this in post processes which contain intermediate buffers
        which need to be updated.
        */
        virtual void bufferResized() {}

        /*!
        \brief Returns the current buffer size
        */
        sf::Vector2i getBufferSize() const { return m_bufferSize; }

    private:
        sf::Vector2i m_bufferSize;
    };
}
