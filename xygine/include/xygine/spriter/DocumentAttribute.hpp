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

#ifndef XY_SPRITE_DOCUMENT_ATTRIBUTE_HPP_
#define XY_SPRITE_DOCUMENT_ATTRIBUTE_HPP_

#include <xygine/parsers/pugixml.hpp>
#include <xygine/Config.hpp>

#include <SFML/Config.hpp>

namespace xy
{
    namespace Spriter
    {
        namespace Detail
        {
            /*!
            \brief Used internally for parsing Spriter documents
            */
            class XY_EXPORT_API DocumentAttribute final
            {
            public:
                DocumentAttribute(const pugi::xml_attribute&);
                ~DocumentAttribute() = default;

                std::string getName() const;

                float valueAsFloat() const;
                sf::Int32 valueAsInt() const;
                std::string valueAsString() const;

                void advanceNext();

                explicit operator bool() const { return !m_attribute.empty(); }

            private:
                pugi::xml_attribute m_attribute;
            };
        }
    }
}

#endif //XY_SPRITE_DOCUMENT_ATTRIBUTE_HPP_