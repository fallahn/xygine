/*********************************************************************
© Matt Marchant 2014 - 2017
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

#ifndef XY_TMX_OBJECTGROUP_HPP_
#define XY_TMX_OBJECTGROUP_HPP_

#include <xygine/tilemap/Layer.hpp>
#include <xygine/tilemap/Object.hpp>

#include <SFML/Graphics/Color.hpp>

#include <vector>

namespace xy
{
    namespace tmx
    {
        /*!
        \brief ObjectGroup layers contain a series of Objects
        which may be made up of shapes or images.
        */
        class XY_EXPORT_API ObjectGroup final : public Layer
        {
        public:
            enum class DrawOrder
            {
                Index, //< objects should be drawn in the order in which they appear
                TopDown //< objects should be drawn sorted by their Y position
            };

            ObjectGroup();
            ~ObjectGroup() = default;

            Type getType() const override { return Layer::Type::Object; }
            void parse(const pugi::xml_node&) override;

            /*!
            \brief Returns the colour associated with this layer
            */
            const sf::Color& getColour() const { return m_colour; }
            /*!
            \brief Returns the DrawOrder for the objects in this group.
            Defaults to TopDown, where Objects are drawn sorted by Y position
            */
            DrawOrder getDrawOrder() const { return m_drawOrder; }
            /*!
            \brief Returns a reference to the vector of properties for
            the ObjectGroup
            */
            const std::vector<Property>& getProperties() const { return m_properties; }
            /*!
            \brief Returns a reference to the vector of Objects which belong to the group
            */
            const std::vector<Object>& getObjects() const { return m_objects; }

        private:
            sf::Color m_colour;
            DrawOrder m_drawOrder;

            std::vector<Property> m_properties;
            std::vector<Object> m_objects;
        };
    }
}

#endif //XY_TMX_OBJECTGROUP_HPP_