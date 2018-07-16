/*********************************************************************
(c) Matt Marchant 2017 - 2018
http://trederia.blogspot.com

xygineXT - Zlib license.

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

#pragma once

#include "xyginext/core/Log.hpp"
#include "xyginext/core/Assert.hpp"
#include "xyginext/resources/DejaVuSans.hpp"

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Font.hpp>

#include <unordered_map>
#include <vector>
#include <typeindex>

// Fiddling for experimental features
#if __has_include(<any>)

#include <any>
namespace stdx {
    using namespace ::std;
}
#elif __has_include(<experimental/any>)
#include <experimental/any>
namespace stdx {
    using namespace ::std;
    using namespace ::std::experimental;
}

#else
#error <experimental/any> and <any> not found
#endif

namespace xy
{
    /*!
     \brief Handle for a Resource
     
     Basically just an index to the resource in the vector
     */
    using ResourceHandle = std::uint16_t;
    
    /*!
     \brief Struct responsible for loading resources
     
     Contains a method to load from a path, and a fallback if the resource isn't found
     */
    struct ResourceLoader
    {
        std::function<stdx::any(const std::string&)>   loader;
        std::function<stdx::any()>                     fallback;
    };
    
    /*!
     \brief Class for generic resource management using handles
     
     The user must explicitly load resources, and store the returned handle.
     Then they can use the handle to retreive the resource when required
     
     */
    class XY_EXPORT_API ResourceHandler
    {
    public:
        
        // Constructor
        ResourceHandler()
        {
            // Add a texture loader
            ResourceLoader texLoader;
            texLoader.loader = [](const std::string& path)
            {
                sf::Texture tex;
                return tex.loadFromFile(path) ? tex : stdx::any();
            };
            
            texLoader.fallback = []()
            {
                return sf::Texture();
            };
            
            // And a font loader
            ResourceLoader fontLoader;
            fontLoader.loader = [](const std::string& path)
            {
                sf::Font font;
                return font.loadFromFile(path) ? font : stdx::any();
            };
            
            fontLoader.fallback = []()
            {
                sf::Font font;
                font.loadFromMemory(DejaVuSans_ttf.data(), DejaVuSans_Size);
                return font;
            };
            
            getLoader<sf::Texture>() = texLoader;
            getLoader<sf::Font>() = fontLoader;
        }
        
        /*!
         \brief Load a resource
         
         Load a resource from a path, returns a handle to the resource
         */
        template<class T>
        ResourceHandle load(const std::string& path)
        {
            // Find the correct loader for the type
            std::type_index ti = typeid(T);
            
            XY_ASSERT(m_loaders.count(ti), std::string("Resource loader not found for ") + std::string(ti.name()));
            
            m_resources.emplace_back(m_loaders[ti].loader(path));
            
            // If it wasn't loaded, use fallback
            #ifdef __APPLE__
            if (m_resources.back().empty())
            #else
            if (m_resources.back().has_value())
            #endif
            {
                m_resources.back() = m_loaders[ti].fallback();
            }
            return static_cast<std::uint16_t>(m_resources.size()-1);
        }
        
        /*!
         \brief Get a resource
         
         Get a resource from it's handle
         */
        template<class T>
        T& get(const ResourceHandle& resource)
        {
            XY_ASSERT(resource < m_resources.size(), "Invalid resource handle");
            return *stdx::any_cast<T>(&m_resources[resource]);
        }
        
        /*!
         \brief Get the loader for a resource type
         
         Could be getter & setter if preferred? I find this more convenient when
         you just want to modify the fallback or something...
         */
        template<class T>
        ResourceLoader& getLoader()
        {
            return m_loaders[typeid(T)];
        }
        
    private:
        // Container of the resources
        std::vector<stdx::any> m_resources;
        
        // Resource loaders mapped by their type index
        std::unordered_map<std::type_index, ResourceLoader> m_loaders;
    };
}
