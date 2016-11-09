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

#ifndef XY_TILEMAP_SHADER_HPP_
#define XY_TILEMAP_SHADER_HPP_

#include <xygine/shaders/Default.hpp>

#include <string>

namespace xy
{
    namespace Shader
    {
        namespace tmx
        {
            static const std::string vertex =
                "#version 120\n"

                "varying vec2 v_texCoord;\n"
                "varying vec4 v_colour;\n"

                "void main()\n"
                "{\n"
                "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
                "    v_texCoord = (gl_TextureMatrix[0] * gl_MultiTexCoord0).xy;"
                /*"    gl_FrontColor = gl_Color;"*/
                "    v_colour = gl_Color;\n"
                "}";

            static const std::string fragment =
                "#version 130\n"
                "#define FLIP_HORIZONTAL 8u\n"
                "#define FLIP_VERTICAL 4u\n"
                "#define FLIP_DIAGONAL 2u\n"

                "in vec2 v_texCoord;\n"
                "in vec4 v_colour;\n"

                "uniform usampler2D u_lookup;\n"
                "uniform sampler2D u_tileMap;\n"

                "uniform vec2 u_tileSize;\n"
                "uniform vec2 u_tilesetCount;\n"
                "uniform vec2 u_tilesetScale = vec2(1.0);\n"

                "uniform float u_opacity = 1.0;\n"

                "out vec4 colour;\n"
                /*fixes rounding imprecision on AMD cards*/
                "const float epsilon = 0.05;\n"

                "void main()\n"
                "{\n"
                "    uvec2 values = texture(u_lookup, v_texCoord / u_tileSize).rg;\n"
                "    if(values.r > 0u)\n"
                "    {\n"
                "        //colour = vec4(vec3(1.0), 0.33333);\n"
                "        float index = float(values.r) - 1.0;\n"
                "        vec2 position = vec2(mod(index, u_tilesetCount.x), floor((index / u_tilesetCount.x) + epsilon)) / u_tilesetCount;\n"
                "        vec2 offsetCoord = (v_texCoord * (textureSize(u_lookup, 0) * u_tilesetScale)) / u_tileSize;\n"

                "        vec2 offset = mod(offsetCoord, 1.0) / u_tilesetCount;\n"

                "        if(values.g != 0u)\n"
                "        {\n"
                "            vec2 tileSize = vec2(1.0) / u_tilesetCount;\n"
                "            if((values.g & FLIP_DIAGONAL) != 0u)\n"
                "            {\n"
                "                float temp = offset.x;\n"
                "                offset.x = offset.y;\n"
                "                offset.y = temp;\n"
                "                temp = tileSize.x / tileSize.y;\n"
                "                offset.x *= temp;\n"
                "                offset.y /= temp;\n"
                "                offset.x = tileSize.x - offset.x;\n"
                "                offset.y = tileSize.y - offset.y;\n"
                "            }\n"
                "            if((values.g & FLIP_VERTICAL) != 0u)\n"
                "            {\n"
                "                offset.y = tileSize.y - offset.y;\n"
                "            }\n"
                "            if((values.g & FLIP_HORIZONTAL) != 0u)\n"
                "            {\n"
                "                offset.x = tileSize.x - offset.x;\n"
                "            }\n"
                "        }\n"
                "        colour = texture(u_tileMap, position + offset);\n"
                "        colour.a = min(colour.a, u_opacity);\n"
                "    }\n"
                "    else\n"
                "    {\n"
                "         colour = vec4(0.0);"
                "    }\n"
                "    colour *= v_colour;\n"
                "}";
        } //namespace tmx
    } //namespace Shader
} //namespace xy

#endif //XY_TILEMAP_SHADER_HPP_