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

//allows quickly adding a shader property to drawable components

#ifndef SHADER_PROPERTY_HPP_
#define SHADER_PROPERTY_HPP_

namespace sf
{
    class Shader;
}

namespace xy
{
    class ShaderProperty
    {
    public:
        ShaderProperty() : m_shader(nullptr), m_activeShader(nullptr){};
        virtual ~ShaderProperty() = default;

        void setShader(sf::Shader* shader) { m_shader = m_activeShader = shader; }
        sf::Shader* getShader() const { return m_shader; }
        const sf::Shader* getActiveShader() const { return m_activeShader; }
        void setShaderActive(bool b = true) { m_activeShader = (b) ? m_shader : nullptr; }

    private:
        sf::Shader* m_shader;
        sf::Shader* m_activeShader;
    };
}

#endif //SHADER_PROPERTY_HPP_
