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

#ifndef XY_STATIC_CONSTS_HPP_
#define XY_STATIC_CONSTS_HPP_

#include <string>

namespace xy
{
    static const std::string VertexAttribPosition("a_position");
    static const std::string VertexAttribColour("a_colour");
    static const std::string VertexAttribNormal("a_normal");
    static const std::string VertexAttribTangent("a_tangent");
    static const std::string VertexAttribBitangent("a_bitangent");
    static const std::string VertexAttribUV0("a_texCoord0");
    static const std::string VertexAttribUV1("a_texCoord1");
    static const std::string VertexAttribBlendIndices("a_boneIndices");
    static const std::string VertexAttribBlendWeights("a_boneWeights");
}
#endif //XY_STATIC_CONSTS_HPP_
