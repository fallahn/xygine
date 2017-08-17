/*********************************************************************
(c) Matt Marchant 2017
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

template <typename T, typename... Args>
T& SystemManager::addSystem(Args&&... args)
{
    static_assert(std::is_base_of<System, T>::value, "This is not a system type");
    
    UniqueType type(typeid(T));
    auto result = std::find_if(std::begin(m_systems), std::end(m_systems), 
        [&type](const System::Ptr& sys)
    {
        return sys->getType() == type;
    });

    if (result != m_systems.end())
    {
        return *(dynamic_cast<T*>(result->get()));
    }

    m_systems.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
    m_systems.back()->setScene(m_scene);
    return *(dynamic_cast<T*>(m_systems.back().get()));
}

template <typename T>
void SystemManager::removeSystem()
{
    UniqueType type(typeid(T));
    m_systems.erase(std::remove_if(std::begin(m_systems), std::end(m_systems),
        [&type](const System::Ptr& sys) 
    {
        return sys->getType() == type;
    }), std::end(m_systems));
}

template <typename T>
T& SystemManager::getSystem()
{
    CRO_ASSERT(hasSystem<T>(), "System index out of range");

    UniqueType type(typeid(T));
    auto result = std::find_if(std::begin(m_systems), std::end(m_systems),
        [&type](const System::Ptr& sys)
    {
        return sys->getType() == type;
    });

    return *(dynamic_cast<T*>(result->get()));
}

template <typename T>
bool SystemManager::hasSystem() const
{
    UniqueType type(typeid(T));
    auto result = std::find_if(std::begin(m_systems), std::end(m_systems),
        [&type](const System::Ptr& sys)
    {
        return sys->getType() == type;
    });

    return result != m_systems.end();
}