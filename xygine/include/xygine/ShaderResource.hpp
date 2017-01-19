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

//resource for caching shaders

#ifndef XY_SHADER_RESOURCE_HPP_
#define XY_SHADER_RESOURCE_HPP_

#include <xygine/Config.hpp>

#include <SFML/Config.hpp>
#include <SFML/Graphics/Shader.hpp>

#include <map>
#include <memory>

namespace sf
{
    class Shader;
}

namespace xy
{
    /*!
    \brief Specialised resource manager for shaders
    */
    class XY_EXPORT_API ShaderResource final
    {
    public:

        using ID = sf::Int32;

        ShaderResource();
        ~ShaderResource() = default;
        ShaderResource(const ShaderResource&) = delete;
        const ShaderResource& operator = (const ShaderResource&) = delete;

        /*!
        \brief get a reference to the shader corresponding to the given ID

        Before a shader can be returned it must be preloaded and associated with
        the given ID. If using custom shaders along side the built in xygine shaders
        the the Unique ID should start at least Shader::Type::Count
        \see preLoad
        */
        sf::Shader& get(ID);
        /*!
        \brief Preloads a shader.

        Shaders need to be loaded and compiled before they can be used which
        can be a time consuming action. Preloading shaders when other resources
        are loaded allows shaders to be returned more efficiently at run time.

        \param ID A unique 32 bit integer to identify the loaded shader
        \param string A string containing the source for the vertex shader.
        The string can either be a constant stored within a source file, or loaded
        from an external file at run time.
        \param string A string containing the source for the fragment shader.
        The string can either be a constant stored within a source file, or loaded
        from an external file at run time.
        */
        void preload(ID, const std::string&, const std::string&);

        /*!
        \brief Preloads a shader.

        Shaders need to be loaded and compiled before they can be used which
        can be a time consuming action. Preloading shaders when other resources
        are loaded allows shaders to be returned more efficiently at run time.

        \param ID A unique 32 bit integer to identify the loaded shader
        \param string A string containing the source for a vertex or fragment shader.
        The string can either be a constant stored within a source file, or loaded
        from an external file at run time.
        \param sf::Shader::Type Shader type describing whether the source is a vertex
        or fragment shader.
        */
        void preload(ID, const std::string&, sf::Shader::Type);

        /*!
        \brief Returns true if the given ID is already in use
        */
        bool exists(ID id) { return m_shaders.find(id) != m_shaders.end(); }
    private:
        std::map<ID, std::unique_ptr<sf::Shader>> m_shaders;
    };
}
#endif //XY_SHADER_RESOURCE_HPP_