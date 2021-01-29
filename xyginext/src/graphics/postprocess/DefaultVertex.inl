/*********************************************************************
(c) Matt Marchant 2017 - 2021
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

#ifndef XY_SHADER_DEFAULT_HPP_
#define XY_SHADER_DEFAULT_HPP_

namespace Default
{
    static const std::string vertex =
        "#version 120\n" \
        "void main()\n" \
        "{\n" \
        "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n" \
        "    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;\n" \
        "    gl_FrontColor = gl_Color;\n" \
        "}";
}

#endif //XY_SHADER_DEFAULT_HPP_