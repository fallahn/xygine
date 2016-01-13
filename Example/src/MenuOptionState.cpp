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

#include <xygine/Util.hpp>
#include <xygine/App.hpp>
#include <xygine/Log.hpp>

#include <xygine/ui/Slider.hpp>
#include <xygine/ui/CheckBox.hpp>
#include <xygine/ui/Selection.hpp>
#include <xygine/ui/Button.hpp>
#include <xygine/ui/TextBox.hpp>

#include <SFML/Window/Event.hpp>

namespace
{
    
}

MenuOptionState::MenuOptionState(xy::StateStack& stateStack, Context context)
    : State     (stateStack, context),
    m_messageBus(context.appInstance.getMessageBus()),
    m_window    (context.renderWindow, m_fontResource.get("assets/fonts/VeraMono.ttf"), 1024u, 768u)
{
    //m_menuSprite.setTexture(context.appInstance.getTexture("assets/images/main_menu.png"));
    //m_menuSprite.setPosition(context.defaultView.getCenter());
    //Util::Position::centreOrigin(m_menuSprite);
    //m_menuSprite.move(0.f, -40.f);

    m_cursorSprite.setTexture(m_textureResource.get("assets/images/ui/cursor.png"));
    m_cursorSprite.setPosition(context.renderWindow.mapPixelToCoords(sf::Mouse::getPosition(context.renderWindow)));
    
    const auto& font = m_fontResource.get("assets/fonts/N_E_B.ttf");
    buildMenu(font);

    auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
    msg->type = xy::Message::UIEvent::MenuOpened;
    msg->stateId = States::ID::MenuOptions;
}

//public
bool MenuOptionState::update(float dt)
{
    m_window.update(dt);
    return true;
}

void MenuOptionState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.setView(getContext().defaultView);
    //rw.draw(m_menuSprite);

    rw.draw(m_window);

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

    m_window.handleEvent(evt, mousePos);
    m_cursorSprite.setPosition(mousePos);

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
void MenuOptionState::buildMenu(const sf::Font& font)
{
    xy::UI::Window::Palette palette;
    palette.background = { 0u, 20u, 120u, 190u };
    palette.borderActive = { 0u, 20u, 190u };
    palette.borderNormal = { 0u, 15u, 120u };

    m_window.setPalette(palette);
    m_window.setTitle("title");
    m_window.setPosition(448.f, 156.f);

    auto soundSlider = std::make_shared<xy::UI::Slider>(font, m_textureResource.get("assets/images/ui/slider_handle.png"), 375.f);
    soundSlider->setPosition(152.f, 314.f);
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
    m_window.addControl(soundSlider);

    auto muteCheckbox = std::make_shared<xy::UI::CheckBox>(font, m_textureResource.get("assets/images/ui/checkbox.png"));
    muteCheckbox->setPosition(622.f, 274.f);
    muteCheckbox->setText("Mute");
    muteCheckbox->addCallback([this](const xy::UI::CheckBox* checkBox)
    {
        auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
        msg->type = (checkBox->checked()) ? xy::Message::UIEvent::RequestAudioMute : xy::Message::UIEvent::RequestAudioUnmute;
    }, xy::UI::CheckBox::Event::CheckChanged);
    muteCheckbox->check(getContext().appInstance.getAudioSettings().muted);
    m_window.addControl(muteCheckbox);


    auto resolutionBox = std::make_shared<xy::UI::Selection>(font, m_textureResource.get("assets/images/ui/scroll_arrow.png"), 375.f);
    resolutionBox->setPosition(152.f, 354.f);

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

    m_window.addControl(resolutionBox);

    auto fullscreenCheckbox = std::make_shared<xy::UI::CheckBox>(font, m_textureResource.get("assets/images/ui/checkbox.png"));
    fullscreenCheckbox->setPosition(622.f, 354.f);
    fullscreenCheckbox->setText("Full Screen");
    fullscreenCheckbox->addCallback([this](const xy::UI::CheckBox*)
    {

    }, xy::UI::CheckBox::Event::CheckChanged);
    fullscreenCheckbox->check((getContext().appInstance.getVideoSettings().WindowStyle & sf::Style::Fullscreen) != 0);
    m_window.addControl(fullscreenCheckbox);

    auto difficultySelection = std::make_shared<xy::UI::Selection>(font, m_textureResource.get("assets/images/ui/scroll_arrow.png"), 375.f);
    difficultySelection->setPosition(152.f, 434.f);
    difficultySelection->addItem("Easy", static_cast<int>(xy::Difficulty::Easy));
    difficultySelection->addItem("Medium", static_cast<int>(xy::Difficulty::Medium));
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
    m_window.addControl(difficultySelection);

    auto controllerCheckbox = std::make_shared<xy::UI::CheckBox>(font, m_textureResource.get("assets/images/ui/checkbox.png"));
    controllerCheckbox->setPosition(622.f, 434.f);
    controllerCheckbox->setText("Enable Controller");
    controllerCheckbox->addCallback([this](const xy::UI::CheckBox* checkBox)
    {
        auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
        msg->type = (checkBox->checked()) ? xy::Message::UIEvent::RequestControllerEnable : xy::Message::UIEvent::RequestControllerDisable;

    }, xy::UI::CheckBox::Event::CheckChanged);
    controllerCheckbox->check(getContext().appInstance.getGameSettings().controllerEnabled);
    m_window.addControl(controllerCheckbox);

    auto applyButton = std::make_shared<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/button.png"));
    applyButton->setText("Apply");
    applyButton->setAlignment(xy::UI::Alignment::Centre);
    applyButton->setPosition(392.f, 614.f);
    applyButton->addCallback([fullscreenCheckbox, resolutionBox, this]()
    {
        auto res = resolutionBox->getSelectedValue();

        xy::App::VideoSettings settings;
        settings.VideoMode.width = res >> 16;
        settings.VideoMode.height = res & 0xFFFF;
        settings.WindowStyle = (fullscreenCheckbox->checked()) ? sf::Style::Fullscreen : sf::Style::Close;
        getContext().appInstance.applyVideoSettings(settings);

        auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
        msg->type = xy::Message::UIEvent::ResizedWindow;
    });
    m_window.addControl(applyButton);

    auto backButton = std::make_shared<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/button.png"));
    backButton->setText("Back");
    backButton->setAlignment(xy::UI::Alignment::Centre);
    backButton->setPosition(632.f, 614.f);
    backButton->addCallback([this]()
    {
        close();
        requestStackPush((getContext().appInstance.connected()) ? States::ID::MenuMain : States::ID::MenuMain);
    });
    m_window.addControl(backButton);
}

void MenuOptionState::close()
{
    requestStackPop();

    auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
    msg->type = xy::Message::UIEvent::MenuClosed;
    msg->stateId = States::ID::MenuOptions;
}