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

#ifndef XY_UTIL_CONFIGFILE_HPP_
#define XY_UTIL_CONFIGFILE_HPP_

#include <xygine/Config.hpp>

#include <string>
#include <unordered_map>

namespace xy
{
    namespace Util
    {
        /*!
        \brief Stores the value data of a ConfigFile property.
        Accessed via the the ConfigFile class
        \see ConfigFile
        */
        class XY_EXPORT_API ConfigValue final
        {
        public:

            ConfigValue(const std::string& v = std::string())
                :m_value(v) {}

            const std::string& asString() const; //! fetch the value as a string
            int asInt() const; //! fetch the value as an int if it is valid, else returns 0
            float asFloat() const; //! fetch the value as a float if it is valid else returns 0
            bool asBool() const; //! fetch the value as a boolean. Returns false if not a valid value

            ConfigValue& operator=(const std::string&);
            ConfigValue& operator=(const char*);
            ConfigValue& operator=(int);
            ConfigValue& operator=(float);
            ConfigValue& operator=(bool);
        private:

            std::string m_value;
        };

        /*!
        \brief Reads / writes configuration (*.xfg) files.
        ConfigFile stores name/value pairs which are written to a text
        file with the extension *.xfg. This is useful for any non-critical
        game settings which can vary by project and are generally fine
        stored as a human readable format.
        */
        class XY_EXPORT_API ConfigFile final
        {
        public:
            ConfigFile() = default;
            ~ConfigFile() = default;
            /*!
            \brief Attempts to load and parse the config file at the given path
            \returns true if successful, else false
            */
            bool load(const std::string&);
            /*!
            \brief Attempts to save the current set of name / value pairs
            to a config value at the given path.
            \returns true if successful, else returns false
            */
            bool save(const std::string&) const;
            /*!
            \brief Inserts a name / value pair.
            \param string name to store with value
            \param string value to store
            Note: existing name / value pairs will have their value overwritten
            */
            void insert(const std::string&, const std::string&);
            /*!
            \brief Inserts a name / value pair.
            \param string name to store with value
            \param string value to store
            Note: existing name / value pairs will have their value overwritten
            */
            void insert(const std::string&, const char*);
            /*!
            \brief Inserts a name / value pair.
            \param string name to store with value
            \param int value to store
            Note: existing name / value pairs will have their value overwritten
            */
            void insert(const std::string&, int);
            /*!
            \brief Inserts a name / value pair.
            \param string name to store with value
            \param float value to store
            Note: existing name / value pairs will have their value overwritten
            */
            void insert(const std::string&, float);
            /*!
            \brief Inserts a name / value pair.
            \param string name to store with value
            \param bool value to store
            Note: existing name / value pairs will have their value overwritten
            */
            void insert(const std::string&, bool);
            /*!
            \brief Removes the name/value pair with given name, if it is found
            */
            void remove(const std::string&);

            ConfigValue& operator[](const std::string&);

            /*!
            \brief Returns the numbers of key/value pairs currently loaded
            */
            std::size_t size() const { return m_data.size(); }

        private:
            mutable std::unordered_map<std::string, ConfigValue> m_data;
        };
    }
}

#endif //XY_UTIL_CONFIG_FILE_HPP_