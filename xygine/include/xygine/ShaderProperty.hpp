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

//allows quickly adding a shader property to drawable components

#ifndef XY_SHADER_PROPERTY_HPP_
#define XY_SHADER_PROPERTY_HPP_

#include <xygine/Config.hpp>

namespace sf
{
    class Shader;
}

namespace xy
{
    /*!
    \brief Allows quickly adding a shader property to a drawable component

    Inheriting from this when creating a new drawable component means a
    reference to a shader can easily be added.
    */
    class XY_EXPORT_API ShaderProperty
    {
    public:
        ShaderProperty() : m_shader(nullptr), m_activeShader(nullptr){};
        virtual ~ShaderProperty() = default;

        /*!
        \brief Set which shader should be used
        */
        void setShader(sf::Shader* shader) { m_shader = m_activeShader = shader; }
        /*!
        \brief Returns a pointer to the current shader if it exists, else nullptr
        */
        sf::Shader* getShader() const { return m_shader; }
        /*!
        \brief Returns a pointer to the active shader

        Use this to set the shader property of the RenderStates passed to the
        drawable component's draw function
        */
        const sf::Shader* getActiveShader() const { return m_activeShader; }
        /*!
        \brief Set whether or not this shader is currently active
        \param b true to set the shader active

        When the shader is inactive nullptr is returned by getActiveShader()
        maintaining a valid value in any sf::RenderState with which it is used.
        */
        void setShaderActive(bool b = true) { m_activeShader = (b) ? m_shader : nullptr; }

    private:
        sf::Shader* m_shader;
        sf::Shader* m_activeShader;
    };
}

#endif //XY_SHADER_PROPERTY_HPP_
