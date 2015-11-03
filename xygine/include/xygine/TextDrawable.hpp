/*********************************************************************
Matt Marchant 2014 - 2015
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

//makes a Text instance component compatible

#ifndef TEXT_DRAWABLE_HPP_
#define TEXT_DRAWABLE_HPP_

#include <xygine/Component.hpp>

#include <SFML/Graphics/Text.hpp>

namespace xy
{
    class TextDrawable final : public Component, public sf::Transformable, public sf::Drawable
    {
    public:
        explicit TextDrawable(MessageBus&);
        ~TextDrawable() = default;

        Component::Type type() const override;
        void entityUpdate(Entity&, float) override;
        void handleMessage(const Message&) override;

        //urrrgh this is a mess
        sf::FloatRect localBounds() const override;
        sf::FloatRect globalBounds() const override;

        sf::FloatRect getLocalBounds() const;
        sf::FloatRect getGlobalBounds() const;

        void setString(const std::string&);
        void setFont(const sf::Font&);
        void setCharacterSize(sf::Uint32);
        void setStyle(sf::Uint32);
        void setColour(const sf::Color&);
        const sf::String& getString() const;
        const sf::Font* getFont() const;
        sf::Uint32 getCharacterSize() const;
        sf::Uint32 getStyle() const;
        const sf::Color& getColour() const;
        sf::Vector2f findCharacterPos(std::size_t) const;

    private:

        sf::Text m_text;
        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };
}

#endif //TEXT_DRAWABLE_HPP_