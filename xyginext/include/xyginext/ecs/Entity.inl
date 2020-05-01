/*********************************************************************
(c) Matt Marchant 2017 - 2020
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

template <typename T>
void Entity::addComponent(const T& component)
{
    XY_ASSERT(isValid(), "Not a valid Entity");
    XY_ASSERT(!hasComponent<T>(), "Already has this component");
    m_entityManager->addComponent<T>(*this, component);
}

template <typename T, typename... Args>
T& Entity::addComponent(Args&&... args)
{
    XY_ASSERT(isValid(), "Not a valid Entity");
    XY_ASSERT(!hasComponent<T>(), "Already has this component");
    return m_entityManager->addComponent<T>(*this, std::forward<Args>(args)...);
}

//TODO this doesn't remove the entity from systems
//which previously required the component being removed..
//template <typename T>
//void Entity::removeComponent()
//{
//    XY_ASSERT(m_entityManager, "Not a valid Entity");
//    m_entityManager->removeComponent<T>(*this);
//}

template <typename T>
bool Entity::hasComponent() const
{
    XY_ASSERT(isValid(), "Not a valid Entity");
    return m_entityManager->hasComponent<T>(*this);
}

template <typename T>
T& Entity::getComponent()
{
    XY_ASSERT(isValid(), "Not a valid Entity");
    XY_ASSERT(hasComponent<T>(), std::string(typeid(T).name()) + ": Component does not exist!");
    return m_entityManager->getComponent<T>(*this);
}

template <typename T>
const T& Entity::getComponent() const
{
    XY_ASSERT(isValid(), "Not a valid Entity");
    XY_ASSERT(hasComponent<T>(), std::string(typeid(T).name()) + ": Component does not exist!");
    return m_entityManager->getComponent<T>(*this);
}