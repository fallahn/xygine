/*********************************************************************
© Matt Marchant 2014 - 2017
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

#ifndef XY_BTEXT_HPP_
#define XY_BTEXT_HPP_

#include <xygine/Config.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <vector>
#include <string>

namespace xy
{
    class BitmapFont;
    /*!
    \brief Draws text using a given bitmap font.
    Bitmap texts support only limited ASCII character sets, but are useful when
    drawing fonts from texture atlases (using the xy::BitmapFont class) or alternate
    character sets such as emojis
    */
    class XY_EXPORT_API BitmapText final : public sf::Drawable, public sf::Transformable
    {
    public:
        /*!
        \brief Default contstructor
        */
        BitmapText();
        /*!
        \brief Constructor.
        \param BitmapFont to use for this text.
        BitmapText objects only store a reference to the given font, so
        the font's lifetime must be managed to match or exceed that of the
        BitmapText.
        */
        BitmapText(const BitmapFont&);
        ~BitmapText() = default;

        /*!
        /brief Sets the BitmapFont used to draw this text.
        BitmapText objects only store a reference to the given font, so
        the font's lifetime must be managed to match or exceed that of the
        BitmapText.
        */
        void setFont(const BitmapFont&);

        /*!
        \brief Sets the string displayed by the text object
        */
        void setString(const std::string&);

        /*!
        \brief Sets the colour with which the text object is multiplied
        */
        void setColour(sf::Color);

        /*!
        \brief Returns a pointer to the font used by this text
        */
        const BitmapFont* getFont() { return m_font; }

        /*!
        \brief Returns the string associated with this text
        */
        const std::string& getString() const { return m_string; }

        /*!
        \brief Returns the colour of the text
        */
        const sf::Color& getColour() const { return m_colour; }

        /*!
        \brief Returns the local (pre-transform) bounds of the BitmapText
        */
        sf::FloatRect getLocalBounds() const;

        /*!
        \brief Returns the AABB of the BitmapText including and transform applied to it
        */
        sf::FloatRect getGlobalBounds() const;



    private:

        const BitmapFont* m_font;
        const sf::Texture* m_texture;
        sf::FloatRect m_localBounds;
        std::string m_string;
        sf::Color m_colour;

        std::vector<sf::Vertex> m_vertices;
        void updateVerts();
        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };
}

#endif //XY_BTEXT_HPP_