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

#include <xygine/spriter/Model.hpp>
#include <xygine/spriter/Document.hpp>
#include <xygine/spriter/DocumentAttribute.hpp>
#include <xygine/spriter/DocumentElement.hpp>

#include <xygine/Resource.hpp>
#include <xygine/FileSystem.hpp>
#include <xygine/Log.hpp>

using namespace xy;
using namespace xy::Spriter;

Model::Model(TextureResource& tr)
    : m_textureResource(tr) {}


//public
bool Model::loadFromFile(const std::string& file)
{
    if (FileSystem::getFileExtension(file) == ".scml")
    {
        Detail::Document document;
        if (document.loadFromFile(file))
        {
            auto firstElement = document.firstElement("spriter_data");
            if (firstElement)
            {
                
            }
            else
            {
                LOG("Invalid or corrupt xml in: " + file, Logger::Type::Error);
                return false;
            }
        }
        else
        {
            LOG("Failed to open scml file: " + file, Logger::Type::Error);
            return false;
        }
    }
    LOG("Invalid file extension: must be *.scml", Logger::Type::Error);
    return false;
}

//private