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

#ifndef XY_SPRITER_DOCUMENT_ELEMENT_HPP_
#define XY_SPRITER_DOCUMENT_ELEMENT_HPP_

#include <xygine/parsers/pugixml.hpp>
#include <xygine/Config.hpp>

namespace xy
{
    namespace Spriter
    {
        namespace Detail
        {
            class DocumentAttribute;
            /*!
            \brief Used internally for parsing Spriter documents
            */
            class DocumentElement final
            {
            public:
                DocumentElement(const pugi::xml_node&);
                ~DocumentElement() = default;

                std::string getName() const;

                void advanceNext();
                void advanceNextSameName();

                DocumentAttribute firstAttribute() const;
                DocumentAttribute firstAttribute(const std::string&) const;

                DocumentElement firstElement() const;
                DocumentElement firstElement(const std::string&) const;

                DocumentElement nextSibling() const;

                explicit operator bool() const { return !m_node.empty(); }

            private:
                pugi::xml_node m_node;

                pugi::xml_attribute findAttribute(const std::string&) const;

            };
        }
    }
}

#endif //XY_SPRITER_DOCUMENT_ELEMENT_HPP_