/*********************************************************************
(c) Jonny Paton 2018

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

#include "xyginext/core/Assert.hpp"

#include <typeindex>
#include <unordered_map>
#include <any>

namespace xy
{
    /*!
    \brief Handle for a Resource
     
    Basically just an index to the resource in the vector
    */
    using ResourceHandle = std::size_t;
    
    /*!
    \brief Struct responsible for loading resources
     
    Contains a method to load from a path, and a fallback if the resource isn't found
    */
    struct ResourceLoader
    {
        std::function<std::any(const std::string&)>   loader;
        std::function<std::any()>                     fallback;
    };
    
    /*!
    \brief Class for generic resource management using handles
     
    The user must explicitly load resources, and store the returned handle.
    Then they can use the handle to retreive the resource when required
     
    */
    class XY_EXPORT_API ResourceHandler
    {
    public:
        
        //Constructor
        ResourceHandler();
        
        /*!
        \brief Load a resource
         
        Load a resource from a path, returns a handle to the resource
        */
        template<class T>
        ResourceHandle load(const std::string& path)
        {
            //Find the correct loader for the type
            std::type_index ti = typeid(T);
            
            XY_ASSERT(m_loaders.count(ti), std::string("Resource loader not found for ") + std::string(ti.name()));
            
            m_resources.emplace_back(m_loaders[ti].loader(path));
            
            //If it wasn't loaded, use fallback
            if (!m_resources.back().has_value())
            {
                m_resources.back() = m_loaders[ti].fallback();
            }
            return m_resources.size()-1;
        }
        
        /*!
        \brief Get a resource
         
        Get a resource from its handle
        */
        template<class T>
        T& get(ResourceHandle resource)
        {
            XY_ASSERT(resource < m_resources.size(), "Invalid resource handle");
            return *std::any_cast<T>(&m_resources[resource]);
        }
        
        /*!
        \brief Get the loader for a resource type
        */
        template<class T>
        ResourceLoader& getLoader()
        {
            return m_loaders[typeid(T)];
        }
        
        template<class T>
        const ResourceLoader& getLoader() const
        {
            const auto found = m_loaders.find(typeid(T));
            XY_ASSERT(found != m_loaders.end(), "Requested loader not found");
            return found->second;
        }
        
    private:
        //Container of the resources
        std::vector<std::any> m_resources;
        
        //Resource loaders mapped by their type index
        std::unordered_map<std::type_index, ResourceLoader> m_loaders;
    };

	//just because I keep typing this incorrectly
	using ResourceHolder = ResourceHandler;
}
