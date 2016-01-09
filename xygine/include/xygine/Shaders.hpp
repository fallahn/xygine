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

//source for shaders

#ifndef XY_SHADERS_HPP_
#define XY_SHADERS_HPP_

#include <string>

namespace xy
{
    namespace Shader
    {
        enum Type
        {
            BrightnessExtract,
            DownSample,
            GaussianBlur,
            AdditiveBlend,
            ChromeAb,
            Lightray,
            Count
        };

        namespace FullPass
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

        namespace PostBrightness
        {
            static const std::string fragment =
                "#version 120\n" \
                "uniform sampler2D u_sourceTexture;\n" \
                "const float threshold = 0.35;\n" \
                "const float factor = 4.0;\n" \

                "void main()\n" \
                "{\n" \
                "    vec4 sourceColour = texture2D(u_sourceTexture, gl_TexCoord[0].xy);\n" \
                "    float luminance = sourceColour.r * 0.2126 + sourceColour.g * 0.7152 + sourceColour.b * 0.0722;\n" \
                "    sourceColour *= clamp(luminance - threshold, 0.0 , 1.0) * factor;\n" \
                "    gl_FragColor = sourceColour;"
                "}\n";
        }

        //buns
        namespace PostDownSample
        {
            static const std::string fragment =
                "#version 120\n" \
                "uniform sampler2D u_sourceTexture;\n" \
                "uniform vec2 u_sourceSize;\n" \

                "void main()\n" \
                "{\n" \
                "    vec2 pixelSize = 1.0 / u_sourceSize;\n" \
                "    vec2 texCoords = gl_TexCoord[0].xy;\n" \
                "    vec4 colour = texture2D(u_sourceTexture, texCoords);\n" \
                "    colour += texture2D(u_sourceTexture, texCoords + vec2(1.0, 0.0) * pixelSize);\n" \
                "    colour += texture2D(u_sourceTexture, texCoords + vec2(-1.0, 0.0) * pixelSize);\n" \
                "    colour += texture2D(u_sourceTexture, texCoords + vec2(0.0, 1.0) * pixelSize);\n" \
                "    colour += texture2D(u_sourceTexture, texCoords + vec2(0.0, -1.0) * pixelSize);\n" \
                "    colour += texture2D(u_sourceTexture, texCoords + vec2(1.0, 1.0) * pixelSize);\n" \
                "    colour += texture2D(u_sourceTexture, texCoords + vec2(-1.0, -1.0) * pixelSize);\n" \
                "    colour += texture2D(u_sourceTexture, texCoords + vec2(1.0, -1.0) * pixelSize);\n" \
                "    colour += texture2D(u_sourceTexture, texCoords + vec2(-1.0, 1.0) * pixelSize);\n" \
                "    gl_FragColor = colour / 9.0;\n" \
                "}";
        }

        namespace PostGaussianBlur
        {
            static const std::string fragment =
                "#version 120\n" \
                "uniform sampler2D u_sourceTexture;\n" \
                "uniform vec2 u_offset;\n" \

                "void main()\n" \
                "{\n " \
                "    vec2 texCoords = gl_TexCoord[0].xy;\n" \
                "    vec4 colour = vec4(0.0);\n" \
                "    colour += texture2D(u_sourceTexture, texCoords - 4.0 * u_offset) * 0.0162162162;\n" \
                "    colour += texture2D(u_sourceTexture, texCoords - 3.0 * u_offset) * 0.0540540541;\n" \
                "    colour += texture2D(u_sourceTexture, texCoords - 2.0 * u_offset) * 0.1216216216;\n" \
                "    colour += texture2D(u_sourceTexture, texCoords - u_offset) * 0.1945945946;\n" \
                "    colour += texture2D(u_sourceTexture, texCoords) * 0.2270270270;\n" \
                "    colour += texture2D(u_sourceTexture, texCoords + u_offset) * 0.1945945946;\n" \
                "    colour += texture2D(u_sourceTexture, texCoords + 2.0 * u_offset) * 0.1216216216;\n" \
                "    colour += texture2D(u_sourceTexture, texCoords + 3.0 * u_offset) * 0.0540540541;\n" \
                "    colour += texture2D(u_sourceTexture, texCoords + 4.0 * u_offset) * 0.0162162162;\n" \
                "    gl_FragColor = colour;\n" \
                "}";
        }

        namespace PostAdditiveBlend
        {
            static const std::string fragment =
                "#version 120\n" \
                "uniform sampler2D u_sourceTexture;\n" \
                "uniform sampler2D u_bloomTexture;\n" \

                "void main()\n" \
                "{\n" \
                "    gl_FragColor = texture2D(u_sourceTexture, gl_TexCoord[0].xy) + texture2D(u_bloomTexture, gl_TexCoord[0].xy);\n" \
                "}";
        }

        namespace PostChromeAb
        {
            static const std::string fragment =
                "#version 120\n" \
                /*"#define BLUR\n" \*/
                "uniform sampler2D u_sourceTexture;\n" \
                "uniform float u_time;\n" \
                "uniform float u_lineCount = 6000.0;\n" \

                "const float noiseStrength = 0.7;\n" \

                "#if defined(BLUR)\n" \
                "const float maxOffset = 1.0 / 35.0;\n" \
                "#define KERNEL_SIZE 9\n" \
                "const vec2 kernel[KERNEL_SIZE] = vec2[KERNEL_SIZE]\n" \
                "(\n" \
                "    vec2(0.0000000000000000000, 0.04416589065853191),\n" \
                "    vec2(0.10497808951021347),\n" \
                "    vec2(0.0922903086524308425, 0.04416589065853191),\n" \
                "    vec2(0.0112445223775533675, 0.10497808951021347)," \
                "    vec2(0.40342407932501833),\n" \
                "    vec2(0.1987116566428735725, 0.10497808951021347),\n" \
                "    vec2(0.0000000000000000000, 0.04416589065853191),\n" \
                "    vec2(0.10497808951021347),\n" \
                "    vec2(0.0922903086524308425, 0.04416589065853191)\n" \
                ");\n" \
                "#else\n" \
                "const float maxOffset = 1.0 / 450.0;\n" \
                "#endif\n" \

                "const float centreDistanceSquared = 0.25;\n" \
                "float distanceSquared(vec2 coord)\n" \
                "{\n" \
                "    return dot(coord, coord);\n" \
                "}\n" \

                "void main()\n" \
                "{\n" \
                "    vec2 distortOffset = vec2(0.01, 0.01);\n" \
                "    vec2 texCoord = gl_TexCoord[0].xy;\n" \
                "    float distSquared = distanceSquared(0.5 - texCoord);\n" \
                "    if(distSquared > centreDistanceSquared) texCoord += ((vec2(0.5, 0.5) - texCoord) * (centreDistanceSquared - distSquared)) * 0.12;\n" \
                "    vec2 offset = vec2((maxOffset / 2.0) - (texCoord.x * maxOffset), (maxOffset / 2.0) - (texCoord.y * maxOffset));\n"
                "    vec3 colour = vec3(0.0);\n" \
                "#if defined(BLUR)\n" \
                "    for(int i = 0; i < KERNEL_SIZE; ++i)\n" \
                "    {\n" \
                "        colour.r += texture2D(u_sourceTexture, texCoord + (offset * kernel[i])).r;\n" \
                "        colour.g += texture2D(u_sourceTexture, texCoord).g;\n" \
                "        colour.b += texture2D(u_sourceTexture, texCoord - (offset * kernel[i])).b;\n" \
                "    }\n" \
                "    colour /= KERNEL_SIZE;\n"
                "#else\n" \
                "    colour.r = texture2D(u_sourceTexture, texCoord + offset).r;\n" \
                "    colour.g = texture2D(u_sourceTexture, texCoord).g;\n" \
                "    colour.b = texture2D(u_sourceTexture, texCoord - offset).b;\n" \
                "#endif\n" \

                /*noise*/
                "    float x = (texCoord.x + 4.0) * texCoord.y * u_time * 10.0;\n" \
                "    x = mod(x, 13.0) * mod(x, 123.0);\n" \
                "    float grain = mod(x, 0.01) - 0.005;\n" \
                "    vec3 result = colour + vec3(clamp(grain * 100.0, 0.0, 0.07));\n" \
                /*scanlines*/
                "    vec2 sinCos = vec2(sin(texCoord.y * u_lineCount), cos(texCoord.y * u_lineCount + u_time));\n" \
                "    result += colour * vec3(sinCos.x, sinCos.y, sinCos.x) * (noiseStrength * 0.08);\n" \
                "    colour += (result - colour) * noiseStrength;\n" \
                "    gl_FragColor = vec4(colour, 1.0);" \
                "}";
        }

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

        namespace NormalMapped
        {
            /*static const std::string textured =
                "#version 120\n#define TEXTURED\n";
                static const std::string coloured =
                "#version 120\n#define COLOURED\n";*/

            static const std::string vertex =
                "#version 120\n" \
                "uniform vec3 u_lightPosition = vec3(960.0, 540.0, 280.0);\n" \
                "uniform mat4 u_inverseWorldViewMatrix;\n" \

                "varying vec3 v_eyeDirection;\n" \
                "varying vec3 v_lightDirection;\n" \

                "const vec3 tangent = vec3(1.0, 0.0, 0.0);\n" \
                "const vec3 normal = vec3(0.0, 0.0, 1.0);\n" \

                "void main()\n" \
                "{\n" \
                "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n" \
                "    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;\n" \
                "    gl_FrontColor = gl_Color;\n" \

                "    vec3 n = normalize(gl_NormalMatrix * normal);\n" \
                "    vec3 t = normalize(gl_NormalMatrix * tangent);\n" \
                "    vec3 b = cross(n, t);\n" \

                "    vec3 viewVertex = vec3(gl_ModelViewMatrix * gl_Vertex);\n" \
                "    vec3 viewLightDirection = vec3(gl_ModelViewMatrix * (u_inverseWorldViewMatrix * vec4(u_lightPosition, 1.0))) - viewVertex;\n" \
                "    v_lightDirection.x = dot(viewLightDirection, t);\n" \
                "    v_lightDirection.y = dot(viewLightDirection, b);\n" \
                "    v_lightDirection.z = dot(viewLightDirection, n);\n" \
                
                "    v_eyeDirection.x = dot(-viewVertex, t);\n" \
                "    v_eyeDirection.y = dot(-viewVertex, b);\n" \
                "    v_eyeDirection.z = dot(-viewVertex, n);\n" \
                "}";

            static const std::string fragment =
                "#version 120\n" \
                "#define TEXTURED\n" \
                "#if defined(TEXTURED)\n" \
                "uniform sampler2D u_diffuseMap;\n" \
                "#endif\n" \
                "uniform sampler2D u_normalMap;\n" \
                "uniform float u_lightIntensity = 1.0;\n" \

                /*TODO make ambient colour match background*/
                "uniform vec3 u_ambientColour = vec3 (0.1, 0.1, 0.1);\n" \

                "varying vec3 v_eyeDirection;\n" \
                "varying vec3 v_lightDirection;\n" \

                "const vec3 lightColour = vec3(1.0, 0.98, 0.945);\n" \
                "const float inverseRange = 0.005;\n" \

                "void main()\n" \
                "{\n" \
                "#if defined(TEXTURED)\n" \
                "    vec4 diffuseColour = texture2D(u_diffuseMap, gl_TexCoord[0].xy);\n" \
                "#elif defined(COLOURED)\n" \
                "    vec4 diffuseColour = gl_Color;\n" \
                "#endif\n" \
                "    vec3 normalVector = texture2D(u_normalMap, gl_TexCoord[0].xy).rgb * 2.0 - 1.0;\n" \

                "    vec3 blendedColour = diffuseColour.rgb * u_ambientColour;\n" \
                "    float diffuseAmount = max(dot(normalVector, normalize(v_lightDirection)), 0.0);\n" \
                /*multiply by falloff*/
                "    vec3 falloffDirection = v_lightDirection * inverseRange;\n" \
                "    float falloff = clamp(1.0 - dot(falloffDirection, falloffDirection), 0.0, 1.0);\n" \
                "    blendedColour += (lightColour * u_lightIntensity) * diffuseColour.rgb * diffuseAmount;// * falloff;\n" \

                "    gl_FragColor.rgb = blendedColour;\n" \
                "    gl_FragColor.a = diffuseColour.a;\n" \
                "#if defined(TEXTURED)\n" \
                "    gl_FragColor *= gl_Color;\n" \
                "#endif\n" \
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
    }
}
#endif //XY_SHADERS_HPP_