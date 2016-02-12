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

#include <NetworkDemoState.hpp>
#include <NetworkDemoPacketIDs.hpp>

#include <xygine/App.hpp>
#include <xygine/ui/Button.hpp>
#include <xygine/ui/TextBox.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/System/Sleep.hpp>

namespace
{
    //const xy::PortNumber port = 5000;
}

using namespace std::placeholders;

NetworkDemoState::NetworkDemoState(xy::StateStack& stack, Context context)
    : State(stack, context)
{
    launchLoadingScreen();
    buildMenu();

    m_packetHandler = std::bind(&NetworkDemoState::handlePacket, this, _1, _2, _3);
    m_connection.setPacketHandler(m_packetHandler);



    quitLoadingScreen();
}


//public
bool NetworkDemoState::update(float dt)
{
    m_menu.update(dt);
    m_server.update(dt);
    return true;
}

bool NetworkDemoState::handleEvent(const sf::Event& evt)
{
    switch (evt.type)
    {
    case sf::Event::KeyReleased:
        switch (evt.key.code)
        {
        case sf::Keyboard::Escape:
        case sf::Keyboard::BackSpace:
            requestStackPop();
            requestStackPush(States::ID::MenuMain);
            break;
        default:break;
        }
    }
    
    m_menu.handleEvent(evt, getContext().appInstance.getMouseWorldPosition());
    return false;
}

void NetworkDemoState::handleMessage(const xy::Message& msg)
{
    
    
}

void NetworkDemoState::draw()
{
    auto& rw = getContext().renderWindow;

    rw.draw(m_menu);
}

//private
void NetworkDemoState::buildMenu()
{
    auto& font = m_fontResource.get("assets/fonts/Console.ttf");

    auto textbox = xy::UI::create<xy::UI::TextBox>(font);
    textbox->setLabelText("IP Address:");
    textbox->setPosition(960.f, 500.f);
    textbox->setAlignment(xy::UI::Alignment::Centre);
    textbox->setText("127.0.0.1");
    m_menu.addControl(textbox);


    auto button = xy::UI::create<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/start_button.png"));
    button->setText("Connect");
    button->setAlignment(xy::UI::Alignment::Centre);
    button->setPosition(960.f, 600.f);
    button->addCallback([textbox, this]()
    {
        //validate text/IP address
        auto address = textbox->getText();
        m_connection.setServerInfo({ address }, xy::Network::ServerPort);

        //connect to server
        if (address == "127.0.0.1" || address == "localhost")
        {
            m_server.start();
            sf::sleep(sf::seconds(1.f));           
        }

        m_connection.connect();

        //close menu
        m_menu.setVisible(false);
    });
    m_menu.addControl(button);

    getContext().renderWindow.setMouseCursorVisible(true);
}

void NetworkDemoState::handlePacket(xy::Network::PacketType type, sf::Packet& packet, xy::Network::ClientConnection* connection)
{
    switch (type)
    {
    default: break;
    case xy::Network::Connect:
    {
        sf::Packet newPacket;
        newPacket << xy::PacketID(PacketID::PlayerDetails);
        newPacket << m_connection.getClientID();
        newPacket << "Player One";
        connection->send(newPacket, true);
    }
        break;
    case PacketID::BallSpawned:

        break;
    case PacketID::PlayerSpawned:
        LOG("CLIENT - rx player spawn", xy::Logger::Type::Info);
        break;
    }
}