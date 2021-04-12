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

#pragma once

#include <xyginext/core/ConfigFile.hpp>

#include <unordered_map>

namespace xy
{
    /*!
    \brief contains the properties of a UI element
    as loaded from a UILayout file.

    Use these properties to define entities which are used
    with the UI system. UIElements contain positional data
    as well as information for the UIHitbox component - 
    such as the bounding box, group ID and a string which can
    be used as a label.
    */
    struct UIElement final
    {
        sf::Vector2f position; //!< The position of the element
        sf::FloatRect bounds; //!< Hitbox bounds of the element
        std::uint32_t groupID = 0; //!< Group ID of the element \see UIHitBox::setGroup()
        std::uint32_t selectionIndex = 0; //!< Defines the order in which inputs are selected \see UIHitBox::setSelectionIndex()
        std::string label; //!< A string which can be used as a label with xy::Text
    };

    /*!
    \brief Parses a UILayout file.
    UILayout files (extension *.xui) are xy::ConfigFile format
    documents containing information which can be used for laying
    out entities when creating a menu or interface the uses
    xy::UISystem and xy::UIHitbox components.

    UILayout files contain a single ui_layout root object which
    can contain multiple 'panel' objects. Each 'panel' can contain
    multiple 'input' objects with the following properties:

    \begincode
    input <input_name>
    {
        position = 0,0
        bounds = 0,0,120,20
        group = 0
        label = "some string"
        index = 3
    }
    \endcode

    Position and bounds properties are required, but group and
    label properties are optional. Groups will default to 0.
    Index properties a 1 based, and start from the beginning of
    the current panel. When moving the selection left/right in
    the UISystem the index defines in which order inputs are selected.
    If all inputs in a panel have the value 0, or no index property
    then the selection order is that which the inputs are added
    to the current panel.

    Panels my also contain a 'group' property to which any contained
    inputs will default. Inputs within a panel with a different
    group ID will override the panel ID value.

    \begincode
    panel <panel_name>
    {
        group = 1

        input <name>
        {
            position = 0,20
            bounds = 0,0, 50, 12
        }

        input <other_name>
        {
            position = 60, 20
            bounds= -10, -10, 100, 20
            group = 4
        }
    }
    \endcode

    UILayout files are useful for quickly iterating on a UI design
    as well as creating UI layouts with a graphical tool/editor.
    */

    class XY_API UILayout final
    {
    public:
        using ElementMap = std::unordered_map<std::string, UIElement>;

        /*!
        \breif Default constructor
        */
        UILayout();

        /*!
        \brief Loads a UILayout file from the given path

        \param path A path to the file to attempt to load
        \returns true if successful else returns false.

        Multiple files can be loaded with a single instance,
        any existing data which has previously been loaded
        will be overwritten.
        */
        bool loadFromFile(const std::string& path);

        /*!
        \brief Attempts to return a UIElement from the requested
        panel with the requested name.
        \param panel The name of the panel containing the requested element
        \param element The name of the requested element
        \returns The requested element if it is found, else an empty UIElement
        */
        const UIElement& getElement(const std::string& panel, const std::string& element) const;

        /*!
        \brief Returns a reference to the map of panels currently loaded.
        */
        const std::unordered_map<std::string, ElementMap>& getPanels() const { return m_panels; }

    private:
        std::unordered_map<std::string, ElementMap> m_panels;
        UIElement m_fallback;
    };
}
