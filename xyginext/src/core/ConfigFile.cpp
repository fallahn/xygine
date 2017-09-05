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

#include <xyginext/core/ConfigFile.hpp>
#include <xyginext/core/Assert.hpp>
#include <xyginext/util/String.hpp>

#include <iostream>
#include <sstream>
#include <algorithm>

using namespace xy;

namespace
{
    const std::string indentBlock("    ");
}

//--------------------//
ConfigProperty::ConfigProperty(const std::string& name, const std::string& value)
    : ConfigItem(name),
    m_value(value) {}

void ConfigProperty::setValue(const std::string& value)
{
    m_value = value;
}

void ConfigProperty::setValue(sf::Int32 value)
{
    m_value = std::to_string(value);
}

void ConfigProperty::setValue(float value)
{
    m_value = std::to_string(value);
}

void ConfigProperty::setValue(bool value)
{
    m_value = (value) ? "true" : "false";
}

void ConfigProperty::setValue(sf::Vector2f v)
{
    m_value = std::to_string(v.x) + "," + std::to_string(v.y);
}

void ConfigProperty::setValue(sf::Vector3f v)
{
    m_value = std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z);
}

std::vector<float> ConfigProperty::valueAsArray() const
{
    std::vector<float> retval;
    auto start = 0u;
    auto next = m_value.find_first_of(',');
    while (next != std::string::npos && start < m_value.length())
    {
        float val;
        std::istringstream is(m_value.substr(start, next));
        if (is >> val) retval.push_back(val);
        else retval.push_back(0.f);

        start = ++next;
        next = m_value.find_first_of(',', start);
        if (next > m_value.length()) next = m_value.length();
    }
    return retval;
}

//-------------------------------------

ConfigObject::ConfigObject(const std::string& name, const std::string& id)
    : ConfigItem	(name), m_id(id){}

bool ConfigObject::loadFromFile(const std::string& path)
{
    m_id = "";
    setName("");
    m_properties.clear();
    m_objects.clear();

    std::fstream file(path);
    if (file.fail())
    {
        Logger::log(path + " file invalid or not found.", Logger::Type::Error);
        return false;
    }

    //file not empty
    file.seekg(0, file.end);
    auto fileSize = file.tellg();
    if (fileSize <= 0)
    {
        Logger::log(path + " invalid file size.", Logger::Type::Error);
        return false;
    }
    file.seekg(file.beg);

    if (file)
    {
        //remove any opening comments
        std::string data;
        while (data.empty() && !file.eof())
        {
            std::getline(file, data);
            removeComment(data);       
        }
        //check config is not opened with a property
        if (isProperty(data))
        {
            Logger::log(path + ": Cannot start configuration file with a property", Logger::Type::Error);
            return false;
        }
        
        //make sure next line is a brace to ensure we have an object
        std::string lastLine = data;
        std::getline(file, data);
        removeComment(data);
        sf::Int32 braceCount = 0;
        
        ConfigObject* currentObject = this;

        if (data == "{")
        {
            //we have our opening object
            auto objectName = getObjectName(lastLine);
            setName(objectName.first);
            m_id = objectName.second;
            braceCount++;
        }
        else
        {
            Logger::log(path + " Invalid configuration header (missing '{' ?)", Logger::Type::Error);
            return false;
        }

        while (std::getline(file, data))
        {
            removeComment(data);
            if (!data.empty())
            {
                if (data == "}")
                {
                    //close current object and move to parent
                    braceCount--;
                    if (braceCount > 0)
                        currentObject = dynamic_cast<ConfigObject*>(currentObject->getParent());
                }
                else if (isProperty(data))
                {			
                    //insert name / value property into current object
                    auto prop = getPropertyName(data);
                    //TODO need to reinstate this and create a property
                    //capable of storing arrays
                    /*if (currentObject->findProperty(prop.first))
                    {
                        Logger::log("Property \'" + prop.first + "\' already exists in \'" + currentObject->getName() + "\', skipping entry...", Logger::Type::Warning);
                        continue;
                    }*/

                    if (prop.second.empty())
                    {
                        Logger::log("\'" + currentObject->getName() + "\' property \'" + prop.first + "\' has no valid value", Logger::Type::Warning);
                        continue;
                    }
                    currentObject->addProperty(prop.first, prop.second);
                }
                else
                {
                    //add a new object and make it current
                    std::string prevLine = data;
                    std::getline(file, data);

                    removeComment(data);
                    if (data == "{")
                    {
                        braceCount++;
                        auto name = getObjectName(prevLine);
                        if (currentObject->findObjectWithId(name.second))
                        {
                        //    Logger::log("Object with ID \'" + name.second + "\' already exists, skipping...", Logger::Type::Warning);
                        //    //object with duplicate id already exists
                        //    while (data.find('}') == std::string::npos
                        //        && readTotal < fileSize) //just incase of infinite loop
                        //    {
                        //        //skip all the object properties before continuing
                        //        data = std::string(Util::String::rwgets(dest, DEST_SIZE, rr.file, &readTotal));
                        //    }
                            braceCount--;
                            continue;
                        }

                        currentObject = currentObject->addObject(name.first, name.second);
                    }
                    else //last line was probably garbage
                    {
                        continue;
                    }
                }
            }		
        }

        if (braceCount != 0)
        {
            Logger::log("Brace count not at 0 after parsing \'" + path + "\'. Config data may not be correct.", Logger::Type::Warning);
        }
        return true;
    }
    
    Logger::log(path + " file invalid or not found.", Logger::Type::Error);
    return false;
}

const std::string& ConfigObject::getId() const
{
    return m_id;
}

ConfigProperty* ConfigObject::findProperty(const std::string& name) const
{
    auto result = std::find_if(m_properties.begin(), m_properties.end(),
        [&name](const ConfigProperty& p)
    {
        return (p.getName() == name);
    });

    if (result != m_properties.end())
    {
        return const_cast<ConfigProperty*>(&*result);
    }
    //recurse
    for (auto& o : m_objects)
    {
        auto p = o.findProperty(name);
        if (p) return p;
    }

    return nullptr;
}

ConfigObject* ConfigObject::findObjectWithId(const std::string& id) const
{
    auto result = std::find_if(m_objects.begin(), m_objects.end(),
        [&id](const ConfigObject& p)
    {
        auto s = p.getId();
        return (!s.empty() && s == id);
    });

    if (result != m_objects.end())
    {
        return const_cast<ConfigObject*>(&*result);
    }
    
    //recurse
    for (auto& o : m_objects)
    {
        auto p = o.findObjectWithId(id);
        if (p) return p;
    }

    return nullptr;
}

ConfigObject* ConfigObject::findObjectWithName(const std::string& name) const
{
    auto result = std::find_if(m_objects.begin(), m_objects.end(),
        [&name](const ConfigObject& p)
    {
        return (p.getName() == name);
    });

    if (result != m_objects.end())
    {
        return const_cast<ConfigObject*>(&*result);
    }

    //recurse
    for (auto& o : m_objects)
    {
        auto p = o.findObjectWithName(name);
        if (p) return p;
    }

    return nullptr;
}

const std::vector<ConfigProperty>& ConfigObject::getProperties() const
{
    return m_properties;
}

const std::vector<ConfigObject>& ConfigObject::getObjects() const
{
    return m_objects;
}

ConfigProperty& ConfigObject::addProperty(const std::string& name, const std::string& value)
{
    m_properties.emplace_back(name, value);
    m_properties.back().setParent(this);
    return m_properties.back();
}

ConfigObject* ConfigObject::addObject(const std::string& name, const std::string& id)
{
    m_objects.emplace_back(name, id);
    m_objects.back().setParent(this);
    //return a reference to newly added object so we can start adding properties / objects to it
    return &m_objects.back();
}

void ConfigObject::removeProperty(const std::string& name)
{
    auto result = std::find_if(m_properties.begin(), m_properties.end(),
        [&name](const ConfigProperty& p)
    {
        return (p.getName() == name);
    });

    if (result != m_properties.end()) m_properties.erase(result);
}

ConfigObject ConfigObject::removeObject(const std::string& name)
{
    auto result = std::find_if(m_objects.begin(), m_objects.end(),
        [&name](const ConfigObject& p)
    {
        return (p.getName() == name);
    });

    if (result != m_objects.end())
    {
        auto p = std::move(*result);
        p.setParent(nullptr);
        m_objects.erase(result);
        return p;
    }

    return {};
}

ConfigObject::NameValue ConfigObject::getObjectName(std::string line)
{
    Util::String::removeChar(line, '\t');
    auto start = line.find_first_not_of(' ');
    
    auto result = line.find_first_of(' ', start);
    if (result != std::string::npos)
    {
        std::string first = line.substr(start, result);
        std::string second = line.substr(result + 1);

        //remove trailing spaces (may append on first when there is no second)
        Util::String::removeChar(first, ' ');
        Util::String::removeChar(second, ' ');

        return std::make_pair(first, second);
    }
    return std::make_pair(line, "");
}

ConfigObject::NameValue ConfigObject::getPropertyName(std::string line)
{
    //remove tabs and skip whitespace
    Util::String::removeChar(line, '\t');
    auto start = line.find_first_not_of(' ');
    
    auto result = line.find_first_of('=');
    XY_ASSERT(result != std::string::npos, "");

    std::string first = line.substr(start, result - start);
    Util::String::removeChar(first, ' ');
    std::string second = line.substr(result + 1);
    
    //check for string literal
    result = second.find_first_of('\"');
    if (result != std::string::npos)
    {
        auto otherResult = second.find_last_of('\"');
        if (otherResult != std::string::npos
            && otherResult != result)
        {
            second = second.substr(result, otherResult);
            Util::String::removeChar(second, '\"');
            if (second[0] == '/') second = second.substr(1);
        }
        else
        {
            Logger::log("String property \'" + first + "\' has missing \'\"\', value may not be as expected", Logger::Type::Warning);
        }
    }
    else
    {
        Util::String::removeChar(second, ' ');
    }

    return std::make_pair(first, second);
}

bool ConfigObject::isProperty(const std::string& line)
{
    auto pos = line.find('=');
    return(pos != std::string::npos && pos > 1 && line.length() > 5);
}

void ConfigObject::removeComment(std::string& line)
{
    auto result = line.find_first_of("//");

    //make sure to only crop comments outside of string literals
    auto otherResult = line.find_first_of('\"');
    if (result != std::string::npos && (result < otherResult || otherResult == std::string::npos))
    {
        line = line.substr(0, result);
    }
    
    otherResult = line.find_last_of('\"');
    if (result != std::string::npos && (result > otherResult || otherResult == std::string::npos))
    {
        line = line.substr(0, result);
    }
    
    //remove any tabs while we're at it
    Util::String::removeChar(line, '\t');

    if (line.find('{') != std::string::npos
        || line.find('}') != std::string::npos)
    {
        Util::String::removeChar(line, ' ');
    }
}

bool ConfigObject::save(const std::string& path) const
{
    std::ofstream file(path);
    if (file.good())
    {
        write(file);
        return true;
    }
    else
    {
        Logger::log("failed to write configuration to: \'" + path + "\'", Logger::Type::Error);
        return false;
    }
    return false;
}

void ConfigObject::write(std::ofstream& file, sf::Uint16 depth) const
{
    //add the correct amount of indenting based on this objects's depth
    std::string indent;
    for (auto i = 0u; i < depth; ++i)
        indent += indentBlock;

    file << indent << getName() << " " << getId() << std::endl;
    file << indent << "{" << std::endl;
    for (const auto& p : m_properties)
    {
        file << indent << indentBlock << p.getName() << " = " << p.getValue<std::string>() << std::endl;
    }
    for (const auto& o : m_objects)
    {
        o.write(file, depth + 1);
    }
    file << indent << "}" << std::endl;
}

//--------------------//
ConfigItem::ConfigItem(const std::string& name)
    : m_parent	(nullptr),
    m_name		(name){}

ConfigItem* ConfigItem::getParent() const
{
    return m_parent;
}

const std::string& ConfigItem::getName() const
{
    return m_name;
}

void ConfigItem::setParent(ConfigItem* parent)
{
    m_parent = parent;
}

void ConfigItem::setName(const std::string& name)
{
    m_name = name;
}
