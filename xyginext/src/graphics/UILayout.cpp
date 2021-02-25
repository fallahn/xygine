/*********************************************************************
(c) Matt Marchant 2021
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

#include <xyginext/graphics/UILayout.hpp>

using namespace xy;

UILayout::UILayout()
{
    m_fallback.label = "Missing element";
}

bool UILayout::loadFromFile(const std::string& path)
{
    m_panels.clear();

    ConfigFile file;
    if (!file.loadFromFile(path))
    {
        //config file will report errors for us
        return false;
    }

    std::vector<const ConfigObject*> validObjects;
    const auto& objects = file.getObjects();
    for (const auto& obj : objects)
    {
        if (obj.getName() == "panel"
            && !obj.getId().empty())
        {
            validObjects.push_back(&obj);
        }
    }

    if (validObjects.empty())
    {
        LogE << path << ": no valid panel objects found" << std::endl;
        return false;
    }

    for (const auto* obj : validObjects)
    {
        std::vector<const ConfigObject*> validElements;
        const auto& panelObjs = obj->getObjects();

        for (const auto& element : panelObjs)
        {
            if (element.getName() == "input")
            {
                validElements.push_back(&element);
            }
        }

        if (validElements.empty())
        {
            LogW << "Panel " << obj->getId() << " contains no input elements" << std::endl;
        }
        else
        {
            ElementMap elements;
            for (const auto* element : validElements)
            {
                UIElement e;

                const auto& properties = element->getProperties();
                for (const auto& prop : properties)
                {
                    const auto& name = prop.getName();
                    if (name == "position")
                    {
                        e.position = prop.getValue<sf::Vector2f>();
                    }
                    else if (name == "bounds")
                    {
                        e.bounds = prop.getValue<sf::FloatRect>();
                    }
                    else if (name == "group")
                    {
                        e.groupID = static_cast<std::uint32_t>(prop.getValue<std::int32_t>());
                    }
                    else if (name == "label")
                    {
                        e.label = prop.getValue<std::string>();
                    }
                }

                auto elementName = element->getId();

                if (!elementName.empty()
                    && e.bounds.width > 0
                    && e.bounds.height > 0)
                {
                    if (elements.count(elementName) == 0)
                    {
                        elements.insert(std::make_pair(elementName, e));
                    }
                    else
                    {
                        LogE << elementName << ": already exists in panel " << obj->getId() << std::endl;
                    }
                }
                else
                {
                    LogW << "Skipped element, missing bounds or empty ID" << std::endl;
                }
            }

            if (!elements.empty())
            {
                if (m_panels.count(obj->getId()) == 0)
                {
                    m_panels.insert(std::make_pair(obj->getId(), elements));
                }
                else
                {
                    LogE << obj->getId() << ": panel already exists... skipping." << std::endl;
                }
            }
        }
    }

    return !m_panels.empty();
}

const UIElement& UILayout::getElement(const std::string& panel, const std::string& element) const
{
    if (m_panels.count(panel) == 0)
    {
        LogE << panel << ": doesn't exist in UILayout." << std::endl;
        return m_fallback;
    }
    else
    {
        const auto& elements = m_panels.at(panel);
        if (elements.count(element) == 0)
        {
            LogE << element << " does not exist in panel " << panel << std::endl;
            return m_fallback;
        }
        else
        {
            return elements.at(element);
        }
    }
    return m_fallback;
}