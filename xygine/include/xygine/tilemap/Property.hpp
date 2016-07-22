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

#ifndef XY_TMX_PROPERTY_HPP_
#define XY_TMX_PROPERTY_HPP_

#include <xygine/Config.hpp>
#include <xygine/Assert.hpp>

#include <string>

namespace pugi
{
    class xml_node;
}

namespace xy
{
    namespace tmx
    {
        /*!
        \brief Represents a custom property.
        Tiles, objects and layers of a tmx may all have custom
        properties assigned to them. This class represents a 
        single property and provides access to its value, the
        type of which can be determined with getType()
        */
        class XY_EXPORT_API Property final
        {
        public:
            
            enum class Type
            {
                Boolean,
                Float,
                Int,
                String,
                Undef
            };
            
            Property();
            ~Property() = default;

            /*!
            \brief Attempts to parse the given node as a property
            */
            void parse(const pugi::xml_node&);
            /*!
            \brief Returns the type of data stored in the property.
            This should generally be called first before trying to
            read the proprty value, as reading the incorrect type
            will lead to undefined behaviour.
            */
            Type getType() const { return m_type; }
            /*!
            \brief Returns the name of this property
            */
            const std::string& getName() const { return m_name; }
            /*!
            \brief Returns the property's value as a boolean
            */
            bool getBoolValue() const { XY_ASSERT(m_type == Type::Boolean, "Check property's type with getType() first"); return m_boolValue; }
            /*!
            \brief Returns the property's value as a float
            */
            float getFloatValue() const { XY_ASSERT(m_type == Type::Float, "Check property's type with getType() first"); return m_floatValue; }
            /*!
            \brief Returns the property's value as an integer
            */
            int getIntValue() const { XY_ASSERT(m_type == Type::Int, "Check property's type with getType() first"); return m_intValue; }
            /*!
            \brief Returns the property's value as a string
            */
            const std::string& getStringValue() const { XY_ASSERT(m_type == Type::String, "Check property's type with getType() first"); return m_stringValue; }

        private:
            union
            {
                bool m_boolValue;
                float m_floatValue;
                int m_intValue;
            };
            std::string m_stringValue;
            std::string m_name;

            Type m_type;
        };
    }
}

#endif //XY_TMX_PROPERTY_HPP_