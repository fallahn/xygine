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

#include <MenuMainState.hpp>

#include <xygine/App.hpp>
#include <xygine/ui/Button.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/imgui/imgui.h>

#include <SFML/Window/Mouse.hpp>

namespace
{
    const float buttonSpacing = 60.f;
    const float buttonMargin = 30.f;

    bool showVideoOptions = false;
    bool fullScreen = false;

    std::vector<sf::VideoMode> modes;
    int currentResolution = 0;
    char resolutionNames[300];
}

MenuMainState::MenuMainState(xy::StateStack& stack, Context context)
    : State         (stack, context),
    m_messageBus    (context.appInstance.getMessageBus()),
    m_uiContainer   (m_messageBus)
{
    launchLoadingScreen();
    
    buildMenu();
    buildSettings();

    auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
    msg->type = xy::Message::UIEvent::MenuOpened;
    msg->value = 0.f;
    msg->stateID = States::ID::MenuMain;

    xy::App::setMouseCursorVisible(true);
    getContext().renderWindow.setTitle("xy tools");

    quitLoadingScreen();
}

//public
bool MenuMainState::update(float dt)
{
    m_uiContainer.update(dt);
    return false;
}

void MenuMainState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.setView(getContext().defaultView);

    rw.draw(m_uiContainer);
}

bool MenuMainState::handleEvent(const sf::Event& evt)
{
    const auto& rw = getContext().renderWindow;
    auto mousePos = rw.mapPixelToCoords(sf::Mouse::getPosition(rw));

    m_uiContainer.handleEvent(evt, mousePos);

    return false;
}

void MenuMainState::handleMessage(const xy::Message& msg)
{

}

//private
void MenuMainState::buildMenu()
{
    const auto& font = m_fontResource.get("assets/fonts/VeraMono.ttf");
    
    auto button = xy::UI::create<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/button.png"));
    button->setString("Settings");
    button->setPosition({ buttonMargin, buttonMargin });
    button->addCallback([this]()
    {
        showVideoOptions = !showVideoOptions;
        const auto& activeMode = getContext().appInstance.getVideoSettings().VideoMode;
        for (auto i = 0u; i < modes.size(); ++i)
        {
            if (modes[i] == activeMode)
            {
                currentResolution = i;
                break;
            }
        }
    });
    m_uiContainer.addControl(button);

    float currentSpacing = buttonSpacing + buttonMargin;
    button = xy::UI::create<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/button.png"));
    button->setString("Sprite Editor");
    button->setPosition({ buttonMargin, currentSpacing });
    button->addCallback([this]()
    {
        if (getStateCount() > 1)
        {
            requestStackPop();
        }
        requestStackPush(States::ID::SpriteEditor);
    });
    m_uiContainer.addControl(button);
    currentSpacing += buttonSpacing;

    button = xy::UI::create<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/button.png"));
    button->setString("Particle Editor");
    button->setPosition({ buttonMargin, currentSpacing });
    button->addCallback([this]()
    {
        if (getStateCount() > 1)
        {
            requestStackPop();
        }
        requestStackPush(States::ID::ParticleEditor);
    });
    m_uiContainer.addControl(button);
    currentSpacing += buttonSpacing;

    button = xy::UI::create<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/button.png"));
    button->setString("Model Viewer");
    button->setPosition({ buttonMargin, currentSpacing });
    button->addCallback([this]()
    {
        if (getStateCount() > 1)
        {
            requestStackPop();
        }
        requestStackPush(States::ID::MaterialEditor);
    });
    m_uiContainer.addControl(button);
    currentSpacing += buttonSpacing;

    button = xy::UI::create<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/button.png"));
    button->setString("Quit");
    button->setPosition({ buttonMargin, currentSpacing });
    button->addCallback([this]() 
    {
        xy::App::quit();
    });
    m_uiContainer.addControl(button);
    currentSpacing += buttonSpacing;
}

void MenuMainState::buildSettings()
{
    modes = getContext().appInstance.getVideoSettings().AvailableVideoModes;
    int i = 0;
    for (const auto& mode : modes)
    {
        if (mode.bitsPerPixel == 32u && mode.isValid())
        {
            std::string width = std::to_string(mode.width);
            std::string height = std::to_string(mode.height);

            for (char c : width)
            {
                resolutionNames[i++] = c;
            }
            resolutionNames[i++] = ' ';
            resolutionNames[i++] = 'x';
            resolutionNames[i++] = ' ';
            for (char c : height)
            {
                resolutionNames[i++] = c;
            }
            resolutionNames[i++] = '\0';
        }
    }

    xy::App::addUserWindow([this]()
    {
        if (!showVideoOptions) return;

        nim::SetNextWindowSize({ 300.f, 100.f });
        nim::Begin("Video Options", &showVideoOptions, ImGuiWindowFlags_ShowBorders);

        nim::Combo("Resolution", &currentResolution, resolutionNames);

        nim::Checkbox("Full Screen", &fullScreen);
        if (nim::Button("Apply", { 50.f, 20.f }))
        {
            //apply settings
            xy::App::VideoSettings settings;
            settings.WindowStyle = (fullScreen) ? sf::Style::Fullscreen : sf::Style::Close;
            settings.VideoMode = modes[currentResolution];
            getContext().appInstance.applyVideoSettings(settings);
        }
        nim::End();
    });
}