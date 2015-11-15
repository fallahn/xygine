//==============================================================================
// RoboMower - Copyright (C) Matt Marchant; All Rights Reserved
// Unauthorized copying of this file via any medium is strictly prohibited
// Proprietary and confidential
// Written by Matt Marchant (matty_styles@hotmail.com) 2015
//==============================================================================

#ifndef MENU_OPTION_STATE_HPP_
#define MENU_OPTION_STATE_HPP_

#include <StateIds.hpp>

#include <xygine/State.hpp>
#include <xygine/ui/Container.hpp>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

namespace xy
{
    class MessageBus;
}
class MenuOptionState final : public xy::State
{
public:
    MenuOptionState(xy::StateStack& stateStack, Context context);
    ~MenuOptionState() = default;

    bool update(float dt) override;
    void draw() override;
    bool handleEvent(const sf::Event& evt) override;
    void handleMessage(const xy::Message&) override;
    xy::StateId stateID() const override
    {
        return States::ID::MenuOptions;
    }
private:
    xy::MessageBus& m_messageBus;
    sf::Sprite m_menuSprite;
    sf::Sprite m_cursorSprite;
    std::vector<sf::Text> m_texts;

    xy::ui::Container m_uiContainer;

    void buildMenu(const sf::Font&);
    void close();
};
#endif //MENU_OPTION_STATE_HPP_