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

#include <xygine/mesh/MaterialDefinition.hpp>
#include <xygine/Log.hpp>
#include <xygine/FileSystem.hpp>
#include <xygine/util/Json.hpp>

#include <xygine/parsers/picojson.h>

#include <fstream>

using namespace xy;

namespace
{
    const std::string fileExtension = ".xym";
}

std::vector<MaterialDefinition> MaterialDefinition::loadFile(const std::string& path)
{
    if (xy::FileSystem::getFileExtension(path) != fileExtension)
    {
        xy::Logger::log(path + ": Not a material file", xy::Logger::Type::Error);
        return{};
    }
    
    std::ifstream file(path);
    if (!file.good() || !Util::File::validLength(file))
    {
        LOG("failed to open " + path + ", or file empty", Logger::Type::Error);
        file.close();
        return{};
    }

    std::string jsonString;
    while (!file.eof())
    {
        std::string temp;
        file >> temp;
        jsonString += temp;
    }
    if (jsonString.empty())
    {
        LOG(path + "failed to read, or file empty", Logger::Type::Error);
        file.close();
        return{};
    }
    file.close();

    std::vector<MaterialDefinition> retVal;

    pj::value pv;
    auto err = pj::parse(pv, jsonString);
    if (err.empty())
    {
        if (pv.is<pj::array>())
        {
            pj::array defs = pv.get<pj::array>();
            for (const auto& def : defs)
            {
                retVal.emplace_back();
                auto& outDef = retVal.back();

                if (def.get("shaderType").is<double>())
                {
                    outDef.shaderType = static_cast<MaterialDefinition::Type>(static_cast<int>(def.get("shaderType").get<double>()));
                }

                if (def.get("colour").is<pj::object>())
                {
                    if(def.get("colour").get("r").is<double>())
                    {
                        outDef.colour[0] = static_cast<sf::Uint8>(def.get("colour").get("r").get<double>());
                    }
                    if (def.get("colour").get("g").is<double>())
                    {
                        outDef.colour[1] = static_cast<sf::Uint8>(def.get("colour").get("g").get<double>());
                    }
                    if (def.get("colour").get("b").is<double>())
                    {
                        outDef.colour[2] = static_cast<sf::Uint8>(def.get("colour").get("b").get<double>());
                    }
                    if (def.get("colour").get("a").is<double>())
                    {
                        outDef.colour[3] = static_cast<sf::Uint8>(def.get("colour").get("a").get<double>());
                    }
                }

                if (def.get("textures").is<pj::object>())
                {
                    if (def.get("textures").get("diffuse").is<std::string>())
                    {
                        outDef.textures[0] = def.get("textures").get("diffuse").get<std::string>();
                    }

                    if (def.get("textures").get("mask").is<std::string>())
                    {
                        outDef.textures[1] = def.get("textures").get("mask").get<std::string>();
                    }

                    if (def.get("textures").get("normal").is<std::string>())
                    {
                        outDef.textures[2] = def.get("textures").get("normal").get<std::string>();
                    }
                }

                if (def.get("castShadows").is<bool>())
                {
                    outDef.castShadows = def.get("castShadows").get<bool>();
                }

                if (def.get("name").is<std::string>())
                {
                    outDef.name = def.get("name").get<std::string>();
                }
            }
        }
    }

    return retVal;
}

void MaterialDefinition::writeFile(const std::vector<MaterialDefinition>& matDefs, std::string outpath)
{
    //check matdefs isn't empty
    if (matDefs.empty())
    {
        xy::Logger::log("Vector of material definitions is empty - no file will be written to " + outpath, xy::Logger::Type::Error);
        return;
    }

    //check outpath and attempt to open file for writing
    if (xy::FileSystem::getFileExtension(outpath) != fileExtension)
    {
        outpath += fileExtension;
    }
    std::ofstream outfile(outpath, std::ios::out);
    if (!outfile.good() || outfile.fail())
    {
        xy::Logger::log("Failed opening " + outpath + " for writing.", xy::Logger::Type::Error);
        outfile.close();
        return;
    }

    //parse to json string and output to file
    pj::array defArray;
    for (const auto& def : matDefs)
    {
        pj::object matObject;
        matObject["shaderType"] = pj::value(static_cast<double>(def.shaderType));

        pj::object colour;
        colour["r"] = pj::value(static_cast<double>(def.colour[0]));
        colour["g"] = pj::value(static_cast<double>(def.colour[1]));
        colour["b"] = pj::value(static_cast<double>(def.colour[2]));
        colour["a"] = pj::value(static_cast<double>(def.colour[3]));
        matObject["colour"] = pj::value(colour);

        if (def.shaderType == MaterialDefinition::Textured)
        {
            pj::object textures;
            textures["diffuse"] = pj::value(def.textures[0]);
            textures["mask"] = pj::value(def.textures[1]);
            textures["normal"] = pj::value(def.textures[2]);
            matObject["textures"] = pj::value(textures);
        }
        matObject["castShadows"] = pj::value(def.castShadows);
        matObject["name"] = pj::value(def.name);

        defArray.push_back(pj::value(matObject));
    }

    auto json = pj::value(defArray).serialize();
    outfile.write(json.c_str(), json.size());

    //close file
    outfile.close();
}