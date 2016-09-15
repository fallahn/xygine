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

#include <MenuOptionState.hpp>

#include <xygine/App.hpp>
#include <xygine/Log.hpp>
#include <xygine/KeyBinds.hpp>

#include <xygine/ui/Slider.hpp>
#include <xygine/ui/CheckBox.hpp>
#include <xygine/ui/Selection.hpp>
#include <xygine/ui/Button.hpp>
#include <xygine/ui/TextBox.hpp>
#include <xygine/ui/KeyBinds.hpp>

#include <SFML/Window/Event.hpp>

namespace
{
    
}

MenuOptionState::MenuOptionState(xy::StateStack& stateStack, Context context)
    : State             (stateStack, context),
    m_messageBus        (context.appInstance.getMessageBus()),
    m_optionContainer   (m_messageBus),
    m_inputContainer    (m_messageBus),
    m_activeContainer   (&m_inputContainer)
{
    //m_menuSprite.setTexture(context.appInstance.getTexture("assets/images/main_menu.png"));
    //m_menuSprite.setPosition(context.defaultView.getCenter());
    //Util::Position::centreOrigin(m_menuSprite);
    //m_menuSprite.move(0.f, -40.f);

    m_cursorSprite.setTexture(m_textureResource.get("assets/images/ui/cursor.png"));
    m_cursorSprite.setPosition(context.renderWindow.mapPixelToCoords(sf::Mouse::getPosition(context.renderWindow)));
    
    const auto& font = m_fontResource.get("assets/fonts/N_E_B.ttf");
    buildOptionsMenu(font);
    buildControlMenu(font);

    auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
    msg->type = xy::Message::UIEvent::MenuOpened;
    msg->stateID = States::ID::MenuOptions;

    xy::App::setMouseCursorVisible(false);
}

//public
bool MenuOptionState::update(float dt)
{
    m_activeContainer->update(dt);
    return true;
}

void MenuOptionState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.setView(getContext().defaultView);

    rw.draw(*m_activeContainer);

    for (const auto& t : m_texts)
    {
        rw.draw(t);
    }

    rw.draw(m_cursorSprite);
}

bool MenuOptionState::handleEvent(const sf::Event& evt)
{
    if (evt.type == sf::Event::KeyReleased)
    {
        switch (evt.key.code)
        {
        case sf::Keyboard::Tab:

            return false;
        case sf::Keyboard::Space:

            return false;
        default: break;
        }
    }
    else if (evt.type == sf::Event::JoystickButtonReleased)
    {
        switch (evt.joystickButton.button)
        {
        case 7: //start on xbox

            return false;
        default: break;
        }
    }
    
    //pass remaining events to menu
    const auto& rw = getContext().renderWindow;
    auto mousePos = rw.mapPixelToCoords(sf::Mouse::getPosition(rw));
    m_cursorSprite.setPosition(mousePos);

    m_activeContainer->handleEvent(evt, mousePos);

    return false; //consume events
}

void MenuOptionState::handleMessage(const xy::Message& msg)
{
    switch (msg.id)
    {
    case xy::Message::Type::UIMessage:
    {
        auto& msgData = msg.getData<xy::Message::UIEvent>();
        switch (msgData.type)
        {
        case xy::Message::UIEvent::MenuClosed:

            break;
        default: break;
        }
        break;
    }
    default: break;
    }
}

//private
void MenuOptionState::buildOptionsMenu(const sf::Font& font)
{
    auto soundSlider = std::make_shared<xy::UI::Slider>(font, m_textureResource.get("assets/images/ui/slider_handle.png"), 375.f);
    soundSlider->setPosition(640.f, 314.f);
    soundSlider->setText("Volume");
    soundSlider->setMaxValue(1.f);
    soundSlider->addCallback([this](const xy::UI::Slider* slider)
    {
        //send volume setting command
        auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
        msg->type = xy::Message::UIEvent::RequestVolumeChange;
        msg->value = slider->getValue();

    }, xy::UI::Slider::Event::ValueChanged);
    soundSlider->setValue(getContext().appInstance.getAudioSettings().volume); //set this *after* callback is set
    m_optionContainer.addControl(soundSlider);

    auto muteCheckbox = std::make_shared<xy::UI::CheckBox>(font, m_textureResource.get("assets/images/ui/checkbox.png"));
    muteCheckbox->setPosition(1110.f, 274.f);
    muteCheckbox->setText("Mute");
    muteCheckbox->addCallback([this](const xy::UI::CheckBox* checkBox)
    {
        auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
        msg->type = (checkBox->checked()) ? xy::Message::UIEvent::RequestAudioMute : xy::Message::UIEvent::RequestAudioUnmute;
    }, xy::UI::CheckBox::Event::CheckChanged);
    muteCheckbox->check(getContext().appInstance.getAudioSettings().muted);
    m_optionContainer.addControl(muteCheckbox);

    auto resolutionBox = std::make_shared<xy::UI::Selection>(font, m_textureResource.get("assets/images/ui/scroll_arrow.png"), 375.f);
    resolutionBox->setPosition(640.f, 354.f);

    const auto& modes = getContext().appInstance.getVideoSettings().AvailableVideoModes;
    auto i = 0u;
    auto j = 0u;
    for (const auto& m : modes)
    {
        std::string name = std::to_string(m.width) + " x " + std::to_string(m.height);
        sf::Int32 val = (m.width << 16) | m.height;
        resolutionBox->addItem(name, val);
        //select currently active mode
        if (getContext().appInstance.getVideoSettings().VideoMode != m)
            i++;
        else
            j = i;
    }
    if (i < modes.size()) resolutionBox->setSelectedIndex(j);

    m_optionContainer.addControl(resolutionBox);

    auto fullscreenCheckbox = std::make_shared<xy::UI::CheckBox>(font, m_textureResource.get("assets/images/ui/checkbox.png"));
    fullscreenCheckbox->setPosition(1110.f, 354.f);
    fullscreenCheckbox->setText("Full Screen");
    fullscreenCheckbox->addCallback([this](const xy::UI::CheckBox*)
    {

    }, xy::UI::CheckBox::Event::CheckChanged);
    fullscreenCheckbox->check((getContext().appInstance.getVideoSettings().WindowStyle & sf::Style::Fullscreen) != 0);
    m_optionContainer.addControl(fullscreenCheckbox);

    auto difficultySelection = std::make_shared<xy::UI::Selection>(font, m_textureResource.get("assets/images/ui/scroll_arrow.png"), 375.f);
    difficultySelection->setPosition(640.f, 434.f);
    difficultySelection->addItem("Easy", static_cast<int>(xy::Difficulty::Easy));
    difficultySelection->addItem("Normal", static_cast<int>(xy::Difficulty::Normal));
    difficultySelection->addItem("Hard", static_cast<int>(xy::Difficulty::Hard));
    difficultySelection->selectItem(0);
    difficultySelection->setCallback([this](const xy::UI::Selection* s)
    {
        //send message with new difficulty
        auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
        msg->type = xy::Message::UIEvent::RequestDifficultyChange;
        msg->difficulty = static_cast<xy::Difficulty>(s->getSelectedValue());
    });
    difficultySelection->selectItem(static_cast<int>(getContext().appInstance.getGameSettings().difficulty));
    m_optionContainer.addControl(difficultySelection);

    auto controllerCheckbox = std::make_shared<xy::UI::CheckBox>(font, m_textureResource.get("assets/images/ui/checkbox.png"));
    controllerCheckbox->setPosition(1110.f, 434.f);
    controllerCheckbox->setText("Enable Controller");
    controllerCheckbox->addCallback([this](const xy::UI::CheckBox* checkBox)
    {
        auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
        msg->type = (checkBox->checked()) ? xy::Message::UIEvent::RequestControllerEnable : xy::Message::UIEvent::RequestControllerDisable;

    }, xy::UI::CheckBox::Event::CheckChanged);
    controllerCheckbox->check(getContext().appInstance.getGameSettings().controllerEnabled);
    m_optionContainer.addControl(controllerCheckbox);

    auto applyButton = std::make_shared<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/start_button.png"));
    applyButton->setText("Apply");
    applyButton->setAlignment(xy::UI::Alignment::Centre);
    applyButton->setPosition(xy::DefaultSceneSize.x / 2.f, 580.f);
    applyButton->addCallback([fullscreenCheckbox, resolutionBox, this]()
    {
        auto res = resolutionBox->getSelectedValue();

        xy::App::VideoSettings settings;
        settings.VideoMode.width = res >> 16;
        settings.VideoMode.height = res & 0xFFFF;
        settings.WindowStyle = (fullscreenCheckbox->checked()) ? sf::Style::Fullscreen : sf::Style::Close;
        getContext().appInstance.applyVideoSettings(settings);
        xy::App::setMouseCursorVisible(false);

        auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
        msg->type = xy::Message::UIEvent::ResizedWindow;
    });
    m_optionContainer.addControl(applyButton);

    auto controlButton = std::make_shared<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/button.png"));
    controlButton->setText("Controls");
    controlButton->setAlignment(xy::UI::Alignment::Centre);
    controlButton->setPosition(840.f, 770.f);
    controlButton->addCallback([this]()
    {
        m_activeContainer = &m_inputContainer;
    });
    m_optionContainer.addControl(controlButton);

    auto backButton = std::make_shared<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/button.png"));
    backButton->setText("Back");
    backButton->setAlignment(xy::UI::Alignment::Centre);
    backButton->setPosition(1080, 770.f);
    backButton->addCallback([this]()
    {
        close();
        requestStackPush(States::ID::MenuMain);
    });
    m_optionContainer.addControl(backButton);
}

void MenuOptionState::buildControlMenu(const sf::Font& font)
{
    auto inputs = xy::UI::create<xy::UI::KeyBinds>(font);
    inputs->setPosition(500.f, 120.f);
    m_inputContainer.addControl(inputs);
    
    auto optionButton = std::make_shared<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/button.png"));
    optionButton->setText("Options");
    optionButton->setAlignment(xy::UI::Alignment::Centre);
    optionButton->setPosition(840.f, 770.f);
    optionButton->addCallback([this]()
    {
        m_activeContainer = &m_optionContainer;
    });
    m_inputContainer.addControl(optionButton);

    auto backButton = std::make_shared<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/button.png"));
    backButton->setText("Back");
    backButton->setAlignment(xy::UI::Alignment::Centre);
    backButton->setPosition(1080, 770.f);
    backButton->addCallback([this]()
    {
        close();
        requestStackPush(States::ID::MenuMain);
    });
    m_inputContainer.addControl(backButton);
}

void MenuOptionState::close()
{
    xy::Input::save();
    
    requestStackPop();

    auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
    msg->type = xy::Message::UIEvent::MenuClosed;
    msg->stateID = States::ID::MenuOptions;
}