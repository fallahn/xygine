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

#ifndef XY_MATDEF_HPP_
#define XY_MATDEF_HPP_

#include <xygine/Config.hpp>

#include <array>
#include <string>
#include <functional>
#include <vector>

namespace xy
{
    /*!
    \brief Material Definition.
    Material definitions are used to describe properties of xy::Materials, which can be read and
    written you the json formatted *.xym file format. These files contain data about one or more
    materials used by a single model, and are output by the material editor in the xyTools application
    */
    struct XY_EXPORT_API MaterialDefinition
    {
        enum Type
        {
            Coloured = 0, VertexColoured, Textured
        }shaderType;
        std::array<float, 4> colour = { { 255, 255, 255, 255 } };
        std::array<std::string, 3> textures;
        bool castShadows = false;
        std::string name = "Untitled";

        /*!
        \brief Returns a unique ID for the definition, which can be used when caching
        materials with the MaterialResource.
        */
        std::uint32_t uid() const
        {
            auto hash = std::hash<std::string>();
            return 
                static_cast<std::uint32_t>(hash(textures[0]) + hash(textures[1]) + hash(textures[2])
                    + hash(name) + (colour[0] + colour[1] + colour[2] + colour[3]));
        }

        /*!
        \brief Returns a vector of material definitions loaded from a *.xym file
        \param path Path to file to attempt to load
        \returns Vector of MaterialDefition structs - empty if the file failed to load.
        Material definitions are listed in the order to which they should be applied to
        a model's submesh. For instance retVal[2] should be applied to model.subMesh(2)
        */
        static std::vector<MaterialDefinition> loadFile(const std::string& path);
        
        /*!
        \brief Writes one or more material definitions to a *.xym file 
        \param definitions A vector of definitons to write to the file
        \param outpath Path to destination, including file name (extention is automatically appended if it is not included)
        */
        static void writeFile(const std::vector<MaterialDefinition>& definitions, std::string outpath);
    };
}

#endif //XY_MATDEF_HPP_