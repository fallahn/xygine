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

#include <xygine/mesh/MaterialResource.hpp>
#include <xygine/mesh/shaders/DeferredRenderer.hpp>

#include <xygine/Log.hpp>
#include <xygine/Assert.hpp>

using namespace xy;

MaterialResource::MaterialResource()
{
    if (!m_defaultShader.loadFromMemory(DEFERRED_COLOURED_VERTEX, DEFERRED_COLOURED_FRAGMENT))
    {
        Logger::log("Failed creating default shader for material resource", xy::Logger::Type::Error, xy::Logger::Output::All);
    }

    m_materials.insert(std::make_pair(DefaultID::Static, std::make_unique<Material>(m_defaultShader)));
}

//public
Material& MaterialResource::add(MaterialResource::ID id, sf::Shader& shader)
{
    XY_ASSERT(id != DefaultID::Static, "Cannot use this ID as it is a reserved value");
    XY_ASSERT(m_materials.count(id) == 0, "Material already exists with this ID");

    m_materials.insert(std::make_pair(id, std::make_unique<Material>(shader)));
    return *m_materials.find(id)->second;
}

Material& MaterialResource::get(ID id)
{
    auto result = m_materials.find(id);
    if (result != m_materials.end())
    {
        return *result->second;
    }
    LOG("Material with ID " + std::to_string(id) + " not found", xy::Logger::Type::Warning);
    return *m_materials.find(DefaultID::Static)->second;
}

bool MaterialResource::hasMaterial(ID id) const
{
    return (m_materials.find(id) != m_materials.end());
}