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

#ifndef XY_MATERIAL_RESOURCE_HPP_
#define XY_MATERIAL_RESOURCE_HPP_

#include <xygine/mesh/Material.hpp>

#include <SFML/Graphics/Shader.hpp>

#include <map>
#include <memory>

namespace xy
{
    /*!
    \brief Material resource management.
    Materials are a shared resource and require a managed lifetime.
    The MaterialResource class is designed to make this easier by
    providing a single point from which all materials can be retrieved.
    */
    class XY_EXPORT_API MaterialResource final
    {
    public:
        using ID = std::uint32_t;

        MaterialResource();
        ~MaterialResource() = default;

        /*!
        \brief Adds a Material to the resource manager.
        \returns Reference to newly created Material.
        Materials should be added once to the resource mananger
        by providing an integer ID (mapped via an enum for example)
        and a shader to be used by the Material. Shaders should be
        ready initialised before being passed to this function, and
        must out-live any instances of the MaterialResource in
        which they are used. IDs can be any unsigned integer value
        except 0xFFFF which is reserved for the default Material.
        As Materials are shared between models on which they are
        used the return value from this function can be used for
        immediate configuration of the Material (assigning
        UniformBuffers or texture parameters for example) which
        will persist throughout any references to the same 
        Material retrieved from the MaterialResource via get()
        */
        Material& add(ID, sf::Shader&);
        /*!
        \brief Gets an instance of the Material assigned the given
        ID if it exists, else returns the default material.
        */
        Material& get(ID);

        enum DefaultID
        {
            Skinned = 0xffff,
            Static  = 0xfffe
        };

    private:

        std::map<ID, std::unique_ptr<Material>> m_materials;
        sf::Shader m_defaultShader;
    };
}

#endif //XY_MATERIAL_RESOURCE_HPP_