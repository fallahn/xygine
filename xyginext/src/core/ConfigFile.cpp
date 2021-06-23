/*********************************************************************
(c) Matt Marchant 2017 - 2021
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

#include "xyginext/core/ConfigFile.hpp"
#include "xyginext/core/Assert.hpp"
#include "xyginext/util/String.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

using namespace xy;

namespace
{
    const std::string indentBlock("    ");
}

//--------------------//
ConfigProperty::ConfigProperty(const std::string& name, const std::string& value)
    : ConfigItem(name),
    m_value(value), m_isStringTypeValue(false) {}

void ConfigProperty::setValue(const std::string& value)
{
    m_value = value;
    m_isStringTypeValue = true;
}

void ConfigProperty::setValue(std::int32_t value)
{
    m_value = std::to_string(value);
    m_isStringTypeValue = false;
}

void ConfigProperty::setValue(float value)
{
    m_value = std::to_string(value);
    m_isStringTypeValue = false;
}

void ConfigProperty::setValue(bool value)
{
    m_value = (value) ? "true" : "false";
    m_isStringTypeValue = false;
}

void ConfigProperty::setValue(sf::Vector2f v)
{
    m_value = std::to_string(v.x) + "," + std::to_string(v.y);
    m_isStringTypeValue = false;
}

void ConfigProperty::setValue(sf::Vector2i v)
{
    m_value = std::to_string(v.x) + "," + std::to_string(v.y);
    m_isStringTypeValue = false;
}

void ConfigProperty::setValue(sf::Vector2u v)
{
    m_value = std::to_string(v.x) + "," + std::to_string(v.y);
    m_isStringTypeValue = false;
}

void ConfigProperty::setValue(sf::Vector3f v)
{
    m_value = std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z);
    m_isStringTypeValue = false;
}

void ConfigProperty::setValue(sf::FloatRect v)
{
    m_value = std::to_string(v.left) + "," + std::to_string(v.top) + "," + std::to_string(v.width) + "," + std::to_string(v.height);
    m_isStringTypeValue = false;
}

void ConfigProperty::setValue(sf::Color v)
{
    m_value = std::to_string(v.r) + "," + std::to_string(v.g) + "," + std::to_string(v.b) + "," + std::to_string(v.a);
    m_isStringTypeValue = false;
}

void ConfigProperty::setValue(xy::Vector4f v)
{
    m_value = std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + "," + std::to_string(v.w);
    m_isStringTypeValue = false;
}

//private
std::vector<float> ConfigProperty::valueAsArray() const
{
    std::vector<float> retval;
    std::size_t start = 0u;
    auto next = m_value.find_first_of(',');
    std::size_t end = std::min(m_value.length(), std::size_t(20));

    while (next != std::string::npos && start < end)
    {
        float val;
        std::istringstream is(m_value.substr(start, next));
        if (is >> val) retval.push_back(val);
        else retval.push_back(0.f);

        start = ++next;
        next = m_value.find_first_of(',', start);
        if (next > m_value.length()) next = m_value.length();
    }

    //make sure we meet the minimum size
    while (retval.size() < 4)
    {
        retval.push_back(0.f);
    }
    return retval;
}

//-------------------------------------

ConfigObject::ConfigObject(const std::string& name, const std::string& id)
    : ConfigItem	(name), m_id(id){}

//public
bool ConfigObject::loadFromFile(const std::string& path)
{
    m_id = "";
    setName("");
    m_properties.clear();
    m_objects.clear();

    std::fstream file(path);
    if(file.is_open())
    {
        if (file.fail())
        {
            Logger::log(path + " file invalid or not found.", Logger::Type::Info);
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
            std::size_t lineNumber = 0;

            //remove any opening comments
            std::string data;
            while (data.empty() && !file.eof())
            {
                std::getline(file, data);
                lineNumber++;
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
            lineNumber++;
            removeComment(data);

            //tracks brace balance
            std::vector<ConfigObject*> objStack;
            
            if (data[0] == '{')
            {
                //we have our opening object
                auto objectName = getObjectName(lastLine);
                setName(objectName.first);
                m_id = objectName.second;

                objStack.push_back(this);
            }
            else
            {
                Logger::log(path + " Invalid configuration header (missing '{' ?)", Logger::Type::Error);
                return false;
            }

            while (std::getline(file, data))
            {
                lineNumber++;
                removeComment(data);
                if (!data.empty())
                {
                    if (data[0] == '}')
                    {
                        //close current object and move to parent
                        objStack.pop_back();
                    }
                    else if (isProperty(data))
                    {			
                        //insert name / value property into current object
                        auto prop = getPropertyName(data);
                        //TODO implement arrays and remove allowing duplicate proprties... although this will break all animated sprites!!
                        /*if (currentObject->findProperty(prop.first))
                        {
                            Logger::log("Property \'" + prop.first + "\' already exists in \'" + currentObject->getName() + "\', skipping entry...", Logger::Type::Warning);
                            continue;
                        }*/

                        if (prop.second.empty())
                        {
                            Logger::log("\'" + objStack.back()->getName() + ": " + objStack.back()->getId() +  "\' property \'" + prop.first + "\' has no valid value", Logger::Type::Warning);
                            continue;
                        }
                        objStack.back()->addProperty(prop.first, prop.second);
                    }
                    else
                    {
                        //add a new object and make it current
                        std::string prevLine = data;
                        std::getline(file, data);
                        lineNumber++;

                        removeComment(data);
                        if (data[0] == '{')
                        {
                            auto name = getObjectName(prevLine);
                            if (name.second.empty() || objStack.back()->findObjectWithId(name.second) == nullptr)
                            {
                                //safe to add new object as the name doesn't exist
                                objStack.push_back(objStack.back()->addObject(name.first, name.second));
                            }
                            else
                            {
                                xy::Logger::log("Object with ID " + name.second + " has already been added, duplicate is skipped...", xy::Logger::Type::Warning);

                                //fast forward to closing brace
                                while (data[0] != '}')
                                {
                                    std::getline(file, data);
                                    removeComment(data);
                                }
                            }
                        }
                        else if(!data.empty()) //last line was probably garbage
                        {
                            LogW << path << std::endl;
                            LogW << "Missing brace following object, or garbage property on or near line " << lineNumber << std::endl;
                            continue;
                        }
                    }
                }		
            }

            if (!objStack.empty())
            {
                Logger::log("Brace count not at 0 after parsing \'" + path + "\'. Config data may not be correct.", Logger::Type::Warning);
            }
            return true;
        }
    }
    
    Logger::log(path + " file invalid or not found.", Logger::Type::Info);
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
    if (id.empty())
    {
        return nullptr;
    }

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

void ConfigObject::addProperty(const ConfigProperty& prop)
{
    m_properties.push_back(prop);
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

void ConfigObject::addObject(const ConfigObject& object)
{
    m_objects.push_back(object);
}

const std::vector<ConfigProperty>& ConfigObject::getProperties() const
{
    return m_properties;
}

std::vector<ConfigProperty>& ConfigObject::getProperties()
{
    return m_properties;
}

const std::vector<ConfigObject>& ConfigObject::getObjects() const
{
    return m_objects;
}

std::vector<ConfigObject>& ConfigObject::getObjects()
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

ConfigObject ConfigObject::removeObject(const std::string& name, const std::string& id)
{
    auto result = std::find_if(m_objects.begin(), m_objects.end(),
        [&name, &id](const ConfigObject& p)
    {
        return (p.getName() == name && p.getId() == id);
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
    /*auto otherResult = line.find_first_of('\"');
    if (result != std::string::npos && (result < otherResult || otherResult == std::string::npos))
    {
        line = line.substr(0, result);
    }*/
    
    //for some reason it appears result also matches '/' so unquoted paths get truncated
    if (result < line.size() - 1 && line[result + 1] == '/')
    {
        auto otherResult = line.find_last_of('\"');
        if (result != std::string::npos && (result > otherResult || otherResult == std::string::npos))
        {
            line = line.substr(0, result);
        }
    }
    
    //remove any tabs while we're at it
    Util::String::removeChar(line, '\t');
    //and preceding spaces
    auto start = line.find_first_not_of(' ');
    if (start != std::string::npos)
    {
        line = line.substr(start);
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

void ConfigObject::write(std::ofstream& file, std::uint16_t depth) const
{
    //add the correct amount of indenting based on this objects's depth
    std::string indent;
    for (auto i = 0u; i < depth; ++i)
        indent += indentBlock;

    file << indent << getName() << " " << getId() << std::endl;
    file << indent << "{" << std::endl;
    for (const auto& p : m_properties)
    {
        file << indent << indentBlock << p.getName() << " = ";
        if (p.m_isStringTypeValue)
        {
            file << "\"" << p.getValue<std::string>() << "\"" << std::endl;
        }
        else
        {
            file << p.getValue<std::string>() << std::endl;
        }
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
