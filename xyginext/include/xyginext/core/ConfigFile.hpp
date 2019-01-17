/*********************************************************************
(c) Matt Marchant 2017 - 2019
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

#include "xyginext/Config.hpp"
#include "xyginext/core/Log.hpp"
#include "xyginext/core/Vector4.hpp"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector3.hpp>

#include <string>
#include <vector>
#include <sstream>

namespace xy
{
    /*!
    \brief Base class for all items loaded from a configuration file
    */
    class XY_EXPORT_API ConfigItem
    {
    public:
        ConfigItem(const std::string& name);
        virtual ~ConfigItem() = default;
        ConfigItem(const ConfigItem&) = default;
        ConfigItem& operator = (const ConfigItem&) = default;
        ConfigItem(ConfigItem&&) = default;
        ConfigItem& operator = (ConfigItem&&) = default;

        /*!
        \brief Returns the name of this item
        */
        const std::string& getName() const;

    protected:
        void setParent(ConfigItem* parent);
        ConfigItem* getParent() const;
        void setName(const std::string& name);
    private:
        ConfigItem* m_parent;
        std::string m_name;
    };
    
    /*!
    \brief Properties are a name / value pair which can only reside in 
    configuration objects.
    */
    class XY_EXPORT_API ConfigProperty final : public ConfigItem
    {
        friend class ConfigObject;
    public:
        ConfigProperty(const std::string& name, const std::string& value = "");

        /*!
        \brief Attempts to retrieve the value as the requested type.
        Valid types are : std::string, sf::Int32, float, bool, sf::Vector2f,
        sf::Vector3f, sf::FloatRect, sf::Color, xy::Vector4f
        */
        template <typename T>
        T getValue() const;// = delete;

        //sets the property's value
        void setValue(const std::string& v);
        void setValue(sf::Int32 v);
        void setValue(float v);
        void setValue(bool v);
        void setValue(sf::Vector2f v);
        void setValue(sf::Vector2i v);
        void setValue(sf::Vector2u v);
        void setValue(sf::Vector3f v);
        void setValue(sf::FloatRect);
        void setValue(sf::Color);
        void setValue(xy::Vector4f);
        
    private:
        std::string m_value;
        std::vector<float> valueAsArray() const;
    };

#include "ConfigFile.inl"

    /*!
    \brief A configuration object can hold one or more nested objects
    or configuration properties. All objects require a name,
    and may have an optional id as a string value.
    */
    class XY_EXPORT_API ConfigObject final : public ConfigItem
    {
    public:
        using NameValue = std::pair<std::string, std::string>;
        ConfigObject(const std::string& name = "", const std::string& id = "");

        /*! 
        \brief Get the id of the object
        */
        const std::string& getId() const;

        /*!
        \brief Returns a pointer to the property if found, else nullptr
        Pointers returned from this function should only be used within
        a small scope as adding or removing any properties from the parent
        object is likely to invalidate them.
        */
        ConfigProperty* findProperty(const std::string& name) const;

        /*!
        \brief Searches for a child object with given id and returns a pointer
        to it if found, else returns nullptr.
        Pointers returned from this function should only be used within
        a small scope as adding or removing any objects from the is likely to
        invalidate them.
        */
        ConfigObject* findObjectWithId(const std::string& id) const;

        /*!
        \brief Searches for a child object with the given name and returns
        a pointer to it if found, else returns nullptr.
        Pointers returned from this function should only be used within
        a small scope as adding or removing any objects from the is likely to
        invalidate them.
        */
        ConfigObject* findObjectWithName(const std::string& name)const;

        /*!
        \brief Returns a reference to the vector of properties owned by this object
        */
        const std::vector<ConfigProperty>& getProperties() const;

        /*!
        \brief Returns a reference to the vector objects owned by this object
        */
        const std::vector<ConfigObject>& getObjects() const;

        /*!
        \brief Adds a name / value property pair to this object
        WARNING this will most likely invalidate any pointers retreived
        with findProperty()
        */
        ConfigProperty& addProperty(const std::string& name, const std::string& value = "");

        /*!
        \brief Adds a copy of the given ConfigProperty to this objects
        list of properties.
        WARNING this will most likely invalidate any pointers retreived
        with findProperty()
        */
        void addProperty(const ConfigProperty& prop);

        /*!
        \brief Adds an object with the given name and optional id to this object
        and returns a pointer to it.
        WARNING this will likely invalidate any existing pointers to objects
        retreived with findObject()
        */
        ConfigObject* addObject(const std::string& name, const std::string& id = "");

        /*!
        \brief Adds a copy of an existing object to this object.
        WARNING this will likely invalidate any existing pointers to objects
        retreived with findObject()
        */
        void addObject(const ConfigObject& obj);

        /*!
        \brief Removes a property with the given name from this object if it exists
        */
        void removeProperty(const std::string& name);

        /*!
        \brief Removes an object (and all of its children) with the given name from
        this object if it exists
        */
        ConfigObject removeObject(const std::string& name);

        /*!
        \brief Writes this object and all its children to given path
        */
        bool save(const std::string& path) const;

        /*!
        \brief Attempts to load a config file into the object.
        \returns true on success, else false if something went wrong
        */
        bool loadFromFile(const std::string& path);

    private:
        std::string m_id;
        std::vector<ConfigProperty> m_properties;
        std::vector<ConfigObject> m_objects;

        static NameValue getObjectName(std::string line);
        static NameValue getPropertyName(std::string line);
        static bool isProperty(const std::string& line);
        static void removeComment(std::string& line);

        void write(std::ofstream& file, sf::Uint16 depth = 0u) const;
    };

    using ConfigFile = xy::ConfigObject;
    
}
