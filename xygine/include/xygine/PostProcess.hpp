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

//abstract base class for full screen post process effects

#ifndef XY_POST_PROCESS_HPP_
#define XY_POST_PROCESS_HPP_

#include <memory>

namespace sf
{
    class RenderTarget;
    class RenderTexture;
    class Shader;
}

#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Vector2.hpp>

namespace xy
{
    class PostProcess
    {
    public:
        using Ptr = std::unique_ptr<PostProcess>;
        PostProcess();
        virtual ~PostProcess() = default;
        PostProcess(const PostProcess&) = delete;
        const PostProcess& operator = (const PostProcess&) = delete;

        virtual void apply(const sf::RenderTexture&, sf::RenderTarget&) = 0;
        //override this if you need to update shader parameters for example
        virtual void update(float) {}

    protected:
        static void applyShader(const sf::Shader&, sf::RenderTarget&);

    };
}

#endif //XY_POST_PROCESS_HPP_