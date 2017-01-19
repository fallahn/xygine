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

#ifndef XY_POST_OLDSCHOOL_HPP_
#define XY_POST_OLDSCHOOL_HPP_

#include <xygine/shaders/Default.hpp>

namespace xy
{
    namespace Shader
    {
        namespace PostOldSchool
        {
            const static std::string fragment =
                "#version 120\n"

                "uniform sampler2D u_sourceTexture;\n"

                "float brightness(vec3 colour)\n"
                "{\n"
                "    return dot(colour, vec3(0.299, 0.587, 0.114));\n"
                "}\n"

                "float brightness(vec4 colour)\n"
                "{\n"
                "    return dot(colour.rgb, vec3(0.299, 0.587, 0.114));\n"
                "}\n"


                "float dither8(vec2 fragCoord, float brightness)\n"
                "{\n"
                "    int x = int(mod(fragCoord.x, 8.0));\n"
                "    int y = int(mod(fragCoord.y, 8.0));\n"
                "    int index = x + y * 8;\n"
                "    float limit = 0.0;\n"

                "    if (x < 8)\n"
                "    {\n"
                "        if (index == 0) limit = 0.015625;\n"
                "        if (index == 1) limit = 0.515625;\n"
                "        if (index == 2) limit = 0.140625;\n"
                "        if (index == 3) limit = 0.640625;\n"
                "        if (index == 4) limit = 0.046875;\n"
                "        if (index == 5) limit = 0.546875;\n"
                "        if (index == 6) limit = 0.171875;\n"
                "        if (index == 7) limit = 0.671875;\n"
                "        if (index == 8) limit = 0.765625;\n"
                "        if (index == 9) limit = 0.265625;\n"
                "        if (index == 10) limit = 0.890625;\n"
                "        if (index == 11) limit = 0.390625;\n"
                "        if (index == 12) limit = 0.796875;\n"
                "        if (index == 13) limit = 0.296875;\n"
                "        if (index == 14) limit = 0.921875;\n"
                "        if (index == 15) limit = 0.421875;\n"
                "        if (index == 16) limit = 0.203125;\n"
                "        if (index == 17) limit = 0.703125;\n"
                "        if (index == 18) limit = 0.078125;\n"
                "        if (index == 19) limit = 0.578125;\n"
                "        if (index == 20) limit = 0.234375;\n"
                "        if (index == 21) limit = 0.734375;\n"
                "        if (index == 22) limit = 0.109375;\n"
                "        if (index == 23) limit = 0.609375;\n"
                "        if (index == 24) limit = 0.953125;\n"
                "        if (index == 25) limit = 0.453125;\n"
                "        if (index == 26) limit = 0.828125;\n"
                "        if (index == 27) limit = 0.328125;\n"
                "        if (index == 28) limit = 0.984375;\n"
                "        if (index == 29) limit = 0.484375;\n"
                "        if (index == 30) limit = 0.859375;\n"
                "        if (index == 31) limit = 0.359375;\n"
                "        if (index == 32) limit = 0.0625;\n"
                "        if (index == 33) limit = 0.5625;\n"
                "        if (index == 34) limit = 0.1875;\n"
                "        if (index == 35) limit = 0.6875;\n"
                "        if (index == 36) limit = 0.03125;\n"
                "        if (index == 37) limit = 0.53125;\n"
                "        if (index == 38) limit = 0.15625;\n"
                "        if (index == 39) limit = 0.65625;\n"
                "        if (index == 40) limit = 0.8125;\n"
                "        if (index == 41) limit = 0.3125;\n"
                "        if (index == 42) limit = 0.9375;\n"
                "        if (index == 43) limit = 0.4375;\n"
                "        if (index == 44) limit = 0.78125;\n"
                "        if (index == 45) limit = 0.28125;\n"
                "        if (index == 46) limit = 0.90625;\n"
                "        if (index == 47) limit = 0.40625;\n"
                "        if (index == 48) limit = 0.25;\n"
                "        if (index == 49) limit = 0.75;\n"
                "        if (index == 50) limit = 0.125;\n"
                "        if (index == 51) limit = 0.625;\n"
                "        if (index == 52) limit = 0.21875;\n"
                "        if (index == 53) limit = 0.71875;\n"
                "        if (index == 54) limit = 0.09375;\n"
                "        if (index == 55) limit = 0.59375;\n"
                "        if (index == 56) limit = 1.0;\n"
                "        if (index == 57) limit = 0.5;\n"
                "        if (index == 58) limit = 0.875;\n"
                "        if (index == 59) limit = 0.375;\n"
                "        if (index == 60) limit = 0.96875;\n"
                "        if (index == 61) limit = 0.46875;\n"
                "        if (index == 62) limit = 0.84375;\n"
                "        if (index == 63) limit = 0.34375;\n"
                "    }\n"
                "    return brightness < limit ? 0.0 : 1.0;\n"
                "}\n"

                "vec3 dither(vec2 fragCoord, vec3 colour)\n"
                "{\n"
                "    float dither = dither8(fragCoord, brightness(colour));\n"
                "    const float step = 1.0 / 12.0;\n"
                "    vec3 nearest = floor(colour / step);\n"
                "    colour = nearest * step;\n"
                "    if (dither > 0.0) colour += step;\n"
                "    else colour -= step;\n"
                "    return colour;\n"
                "}\n"

                "vec4 dither(vec2 fragCoord, vec4 colour)\n"
                "{\n"
                "    return vec4(dither(fragCoord, colour.rgb), colour.a);\n"
                "}\n"

                /*"float discretise(float f, float d)\n"
                "{\n"
                "    return floor(f * d + 0.5) / d;\n"
                "}\n"

                "vec2 discretise(vec2 v, float d)\n"
                "{\n"
                "    return vec2(discretise(v.x, d), discretise(v.y, d));\n"
                "}\n"*/

                "void main()\n"
                "{\n"
                /*"    vec2 texCoord = discretise(gl_TexCoord[0].xy, 384.0);\n"*/
                "    gl_FragColor = dither(gl_FragCoord.xy, texture2D(u_sourceTexture, gl_TexCoord[0].xy));\n"
                "}\n";
        }
    }
}

#endif //XY_POST_OLDSCHOOL_HPP_