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

//resource for caching shaders

#ifndef SHADER_RESOURCE_HPP_
#define SHADER_RESOURCE_HPP_

#include <xygine/Shaders.hpp>

#include <map>
#include <memory>

namespace sf
{
    class Shader;
}

namespace xy
{
    class ShaderResource final
    {
    public:

        ShaderResource();
        ~ShaderResource() = default;
        ShaderResource(const ShaderResource&) = delete;
        const ShaderResource& operator = (const ShaderResource&) = delete;

        sf::Shader& get(Shader::Type);
        void preload(Shader::Type, const std::string&, const std::string&);

    private:
        std::map<Shader::Type, std::unique_ptr<sf::Shader>> m_shaders;
    };
}
#endif //SHADER_RESOURCE_HPP_