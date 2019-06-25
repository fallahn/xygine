/*********************************************************************
(c) Matt Marchant 2019
http://trederia.blogspot.com

xygineXT Shader Editor - Zlib license.

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

#include "WindowFlags.hpp"

#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Shader.hpp>

#include <array>
#include <memory>
#include <bitset>

class Renderer final : public sf::Drawable, public sf::Transformable
{
public:
    Renderer();

    void compileShader(const std::string&, std::bitset<WindowFlags::Count>&);

private:

    std::size_t m_shaderIndex;
    std::array<std::unique_ptr<sf::Shader>, 2u> m_shaders;

    std::array<sf::Vertex, 4u> m_vertices;

    void draw(sf::RenderTarget&, sf::RenderStates) const override;
};