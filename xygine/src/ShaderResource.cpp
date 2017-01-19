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

#include <xygine/detail/GLExtensions.hpp>
#include <xygine/ShaderResource.hpp>
#include <xygine/detail/GLCheck.hpp>

#include <SFML/Graphics/Shader.hpp>

#include <xygine/Assert.hpp>

using namespace xy;

ShaderResource::ShaderResource(){}

//public
sf::Shader& ShaderResource::get(ShaderResource::ID type)
{
    auto result = m_shaders.find(type);
    XY_ASSERT(result != m_shaders.end(), "shader not loaded - did you forget to preload this shader?");

    return *result->second;
}

void ShaderResource::preload(ShaderResource::ID id, const std::string& vertShader, const std::string& fragShader)
{
    auto shader = std::make_unique<sf::Shader>();
#ifndef _DEBUG_
    shader->loadFromMemory(vertShader, fragShader);
#else
    XY_ASSERT(shader->loadFromMemory(vertShader, fragShader), "failed to create shader");
#endif //_DEBUG_

    m_shaders.insert(std::make_pair(id, std::move(shader)));
}

void ShaderResource::preload(ShaderResource::ID id, const std::string& src, sf::Shader::Type type)
{
    auto shader = std::make_unique<sf::Shader>();
#ifndef _DEBUG_
    shader->loadFromMemory(src, type);
#else
    XY_ASSERT(shader->loadFromMemory(src, type), "failed to create shader");
#endif //_DEBUG_

    m_shaders.insert(std::make_pair(id, std::move(shader)));
}