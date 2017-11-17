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

#include "ClientServerShared.hpp"
#include "sha1.hpp"
#include "CommandIDs.hpp"

#include <xyginext/ecs/Scene.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/QuadTreeItem.hpp>
#include <xyginext/ecs/components/CommandTarget.hpp>

#include <tmxlite/Object.hpp>

void createCollisionObject(xy::Scene& scene, const tmx::Object& obj, CollisionType::ID type)
{
    if (obj.getShape() == tmx::Object::Shape::Rectangle)
    {
        auto bounds = obj.getAABB();

        auto entity = scene.createEntity();
        entity.addComponent<xy::Transform>().setPosition(bounds.left, bounds.top);
        entity.addComponent<CollisionComponent>().addHitbox({ 0.f, 0.f, bounds.width, bounds.height }, type);
        entity.addComponent<xy::QuadTreeItem>().setArea({ 0.f, 0.f, bounds.width, bounds.height });
        entity.addComponent<xy::CommandTarget>().ID = CommandID::MapItem;

        switch (type)
        {
        default: break;
        case CollisionType::Platform:
            entity.getComponent<CollisionComponent>().setCollisionCategoryBits(CollisionFlags::Platform);
            entity.getComponent<CollisionComponent>().setCollisionMaskBits(
                CollisionFlags::Player | CollisionFlags::NPC | CollisionFlags::Fruit | CollisionFlags::Powerup | 
                CollisionFlags::Bonus | CollisionFlags::MagicHat | CollisionFlags::Crate);
            break;
        case CollisionType::Solid:
            entity.getComponent<CollisionComponent>().setCollisionCategoryBits(CollisionFlags::Solid);
            entity.getComponent<CollisionComponent>().setCollisionMaskBits(
                CollisionFlags::Player | CollisionFlags::Bubble | CollisionFlags::NPC | CollisionFlags::Fruit | 
                CollisionFlags::Powerup | CollisionFlags::Bonus | CollisionFlags::MagicHat | CollisionFlags::Crate);
            break;
        case CollisionType::Teleport:
            entity.getComponent<CollisionComponent>().setCollisionCategoryBits(CollisionFlags::Teleport);
            entity.getComponent<CollisionComponent>().setCollisionMaskBits(
                CollisionFlags::Player | CollisionFlags::NPC | CollisionFlags::Fruit | CollisionFlags::MagicHat | CollisionFlags::Crate);
            break;
        }
    }
}

std::string getSha(const std::string& path)
{
    std::string line;
    SHA1 checksum;
    
    std::ifstream file(path);
    if (file.good())
    {
        while (!file.eof())
        {
            std::getline(file, line);
            checksum.update(line);
        }
    }

    return checksum.final();
}