/*********************************************************************
Matt Marchant 2014 - 2017
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

#ifndef XY_SHADER_MISC_HPP_
#define XY_SHADER_MISC_HPP_

#include <xygine/shaders/Default.hpp>

namespace xy
{
    namespace Shader
    {
        namespace Scanline
        {
            static const std::string fragment =
                "#version 120\n" \

                "uniform sampler2D u_sourceTexture;\n" \

                "void main()\n" \
                "{\n" \
                "    vec3 colour = texture2D(u_sourceTexture, gl_TexCoord[0].xy).rgb;\n" \
                "    if(mod(floor(gl_FragCoord.y), 2) == 0) colour *= 0.5;\n" \
                "    gl_FragColor = vec4(colour, 0.5);\n" \
                "}";
        }

        namespace LightRay
        {
            static const std::string vertex =
                "#version 120\n" \
                "varying vec2 v_vertPosition;\n" \

                "void main()\n" \
                "{\n" \
                "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n" \
                "    v_vertPosition = gl_Vertex.xy;\n" \
                "    gl_FrontColor = gl_Color;\n" \
                "}";

            static const std::string fragment =
                "#version 120\n" \
                "uniform float u_alpha = 1.0;\n" \

                "varying vec2 v_vertPosition;\n" \

                "const float falloffStart = 800.0;\n" \
                "const float falloffDistance = 700.0;\n" \

                "void main()\n " \
                "{\n" \
                "    float falloffAlpha = 1.0;\n" \
                "    float length = length(v_vertPosition);\n" \
                "    if(length > falloffStart)\n" \
                "    {\n" \
                "        falloffAlpha -= min((length - falloffStart) / falloffDistance , 1.0);\n" \
                "    }\n" \
                /*"    gl_FragColor = vec4(falloffAlpha);\n" \*/
                "    gl_FragColor = gl_Color;\n" \
                "    gl_FragColor.a *= u_alpha * falloffAlpha;\n" \
                "}";
        }

        namespace ReflectionMap
        {
            static const std::string fragment =
                "#version 120\n" \
                "uniform sampler2D u_diffuseMap;\n" \
                "uniform sampler2D u_normalMap;\n" \
                "uniform sampler2D u_reflectionMap;\n" \
                "uniform vec2 u_reflectionCoords;\n" \

                "void main()\n" \
                "{\n" \
                "    vec4 diffuseColour = texture2D(u_diffuseMap, gl_TexCoord[0].xy) * gl_Color;\n" \
                "    vec3 normalVector = texture2D(u_normalMap, gl_TexCoord[0].xy).rgb * 2.0 - 1.0;\n" \
                "    vec4 reflectionColour = texture2D(u_reflectionMap, clamp(gl_TexCoord[0].xy + normalVector.xy, 0.0, 1.0));\n" \
                /*TODO calc mix amount based on angle*/
                "    gl_FragColor = diffuseColour;//mix(diffuseColour, reflectionColour, 0.5);\n" \
                "}\n" \
                "";
        }

        namespace Cropping
        {

            static const std::string fragment = //remember frag Y coord inverted
                "#version 120\n" \
                "#define TEXTURE\n" \
                "uniform vec2 u_position = vec2(50.0, 50.0);\n" \
                "uniform vec2 u_size = vec2(150.0, 300.0);\n" \

                "#if defined(TEXTURE)\n" \
                "uniform sampler2D u_texture;\n" \
                "#endif\n" \

                "bool contains(vec2 point)\n" \
                "{\n" \
                "    return (point.x > u_position.x && point.x < u_size.x + u_position.x && point.y > u_position.y && point.y < u_position.y + u_size.y);\n" \
                "}\n" \

                "void main()\n" \
                "{\n" \
                "    if(!contains(gl_FragCoord.xy)) discard;\n" \
                "#if defined(TEXTURE)\n" \
                "    vec4 colour = texture2D(u_texture, gl_TexCoord[0].xy);\n" \
                "    gl_FragColor = vec4(colour.rgb * gl_Color.rgb, colour.a);\n" \
                "#else\n" \
                "    gl_FragColor = gl_Color;\n" \
                "#endif\n" \
                "}";
        }
    }//namespace Shader
}//namespace xy

#endif //XY_SHADER_MISC_HPP_
