/*********************************************************************
Matt Marchant 2014 - 2017
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

#include <xygine/util/ConfigFile.hpp>
#include <xygine/util/String.hpp>
#include <xygine/Log.hpp>
#include <xygine/FileSystem.hpp>

#include <fstream>

using namespace xy;
using namespace xy::Util;

namespace
{
    const std::string fileExt(".xfg");
    const std::size_t maxStrLen = 50;
    std::string snakify(const std::string& str)
    {
        auto ret = xy::Util::String::toLower(str);
        std::replace(ret.begin(), ret.end(), ' ', '_');
        return std::move(ret.substr(0, maxStrLen));
    }
}

bool ConfigFile::load(const std::string& path)
{
    m_data.clear();

    std::ifstream file(path, std::ios::in);
    if (!file.is_open() || !file.good())
    {
        xy::Logger::log("failed opening " + path + " for reading", xy::Logger::Type::Error);
        file.close();
        return false;
    }

    std::string line;
    while (std::getline(file, line))
    {
        auto parts = xy::Util::String::tokenize(line, '=');
        if (parts.size() > 1)
        {
            if (parts[0].size() > 1 || parts[1].size() > 1)
            {
                if (parts[0].back() == ' ') parts[0].pop_back();
                if (parts[1].front() == ' ')parts[1] = parts[1].substr(1);
            }
            m_data.insert(std::make_pair(snakify(parts[0]), ConfigValue(snakify(parts[1]))));
        }
    }
    file.close();

    return !m_data.empty();
}

bool ConfigFile::save(const std::string& path) const
{   
    std::string fPath = path;
    if (xy::FileSystem::getFileExtension(fPath) != fileExt)
    {
        fPath += fileExt;
    }
    
    std::ofstream file(fPath, std::ios::out);

    if (!file.is_open() || !file.good())
    {
        xy::Logger::log("failed opening " + path + " for writing", xy::Logger::Type::Error);
        file.close();
        return false;
    }
    
    for (const auto& v : m_data)
    {
        file << v.first << " = " << v.second.asString() << std::endl;
    }
    file.close();

    return true;
}

void ConfigFile::insert(const std::string& name, const std::string& value)
{
    auto key = snakify(name);
    if (m_data.count(key) == 0)
    {
        m_data.insert(std::make_pair(key, ConfigValue(snakify(value))));
    }
    else
    {
        m_data[key] = value;
    }
}

void ConfigFile::insert(const std::string& name, const char* value)
{
    auto key = snakify(name);
    if (m_data.count(key) == 0)
    {
        m_data.insert(std::make_pair(key, ConfigValue(snakify(value))));
    }
    else
    {
        m_data[key] = value;
    }
}

void ConfigFile::insert(const std::string& name, int value)
{
    auto key = snakify(name);
    if (m_data.count(key) == 0)
    {
        m_data.insert(std::make_pair(key, ConfigValue(std::to_string(value))));
    }
    else
    {
        m_data[key] = value;
    }
}

void ConfigFile::insert(const std::string& name, float value)
{
    auto key = snakify(name);
    if (m_data.count(key) == 0)
    {
        m_data.insert(std::make_pair(key, ConfigValue(std::to_string(value))));
    }
    else
    {
        m_data[key] = value;
    }
}

void ConfigFile::insert(const std::string& name, bool value)
{
    auto key = snakify(name);
    if (m_data.count(key) == 0)
    {
        m_data.insert(std::make_pair(key, ConfigValue(value ? "true" : "false")));
    }
    else
    {
        m_data[key] = value;
    }
}

void ConfigFile::remove(const std::string& name)
{
    auto key = snakify(name);
    if (m_data.count(key) != 0)
    {
        m_data.erase(key);
    }
}

ConfigValue& ConfigFile::operator[](const std::string& name)
{
    return m_data[snakify(name)];
}

//----ConfigValue----//
const std::string& ConfigValue::asString() const
{
    return m_value;
}

int ConfigValue::asInt() const
{
    try
    {
        return std::stoi(m_value);
    }
    catch (...)
    {
        LOG(m_value + " not a valid integer", xy::Logger::Type::Error);
        return 0;
    }
}

float ConfigValue::asFloat() const
{
    try
    {
        return std::stof(m_value);
    }
    catch (...)
    {
        LOG(m_value + " not a valid float", xy::Logger::Type::Error);
        return 0.f;
    }
}

bool ConfigValue::asBool() const
{
    return (String::toLower(m_value) == "true");
}

ConfigValue& ConfigValue::operator=(const std::string& str)
{
    m_value = str;
    return *this;
}

ConfigValue& ConfigValue::operator=(const char* c)
{
    m_value = std::string(c);
    return *this;
}

ConfigValue& ConfigValue::operator=(int v)
{
    m_value = std::to_string(v);
    return *this;
}

ConfigValue& ConfigValue::operator=(float v)
{
    m_value = std::to_string(v);
    return *this;
}

ConfigValue& ConfigValue::operator=(bool v)
{
    m_value = (v) ? "true" : "false";
    return *this;
}