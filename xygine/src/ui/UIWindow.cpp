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

#include <xygine/ui/Window.hpp>
#include <xygine/Log.hpp>
#include <xygine/shaders/Misc.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

#include <cmath>

using namespace xy;
using namespace UI;

namespace
{
    const float titleBarHeight = 34.f;
    const float cornerRadius = 14.f;
}

Window::Window(sf::RenderWindow& rw, xy::MessageBus& mb, const sf::Font& font, sf::Uint16 width, sf::Uint16 height, const Palette& palette)
    : m_backgroundShape ({ static_cast<float>(width), static_cast<float>(height) }),
    m_container         (mb),
    m_dragMask          (0),
    m_resizable         (true),
    m_renderWindow      (rw)
{
    setPalette(palette);
    m_backgroundShape.setOutlineThickness(-1.f);

    m_titleBar.setSize({ static_cast<float>(width), titleBarHeight });
    m_titleText.setFont(font);
    m_titleText.setPosition(6.f, 8.f);
    m_titleText.setCharacterSize(16u);

    const float radius = cornerRadius / 2.f;
    m_resizeHandle.setRadius(radius);
    m_resizeHandle.setOrigin(radius, radius);
    m_resizeHandle.setOutlineThickness(1.f);
    m_resizeHandle.setPosition(width - cornerRadius, height - cornerRadius);

    m_cropShader.loadFromMemory(Shader::Cropping::fragment, sf::Shader::Fragment);
    auto size = rw.mapCoordsToPixel(m_backgroundShape.getSize());
    m_cropShader.setUniform("u_size", sf::Vector2f(size));
}

//public
void Window::update(float dt)
{
    //we have to do this here to hook any repositioning
    //from inheriting sf::Transform
    auto position = m_renderWindow.mapCoordsToPixel(getPosition());
    m_cropShader.setUniform("u_position", sf::Vector2f(position));
    m_container.update(dt);
}

void Window::handleEvent(const sf::Event& evt, const sf::Vector2f& mousePos)
{
    switch (evt.type)
    {
    case sf::Event::MouseButtonPressed:
        if (getTransform().transformRect(m_titleBar.getGlobalBounds()).contains(mousePos))
        {
            m_dragMask |= All;
        }
        else if (m_resizable && getTransform().transformRect(m_resizeHandle.getGlobalBounds()).contains(mousePos))
        {
            m_dragMask |= Corner;
        }
        break;
    case sf::Event::MouseButtonReleased:
        m_dragMask = 0;
        break;
    case sf::Event::MouseMoved:
        if (m_dragMask & All)
        {
            move(mousePos - m_lastMousePos);
        }
        else if (m_dragMask & Corner)
        {
            m_backgroundShape.setSize(m_backgroundShape.getSize() + (mousePos - m_lastMousePos));
            m_resizeHandle.setPosition(m_backgroundShape.getSize() - sf::Vector2f(cornerRadius, cornerRadius));
            m_titleBar.setSize({ m_backgroundShape.getSize().x, titleBarHeight });

            auto size = m_renderWindow.mapCoordsToPixel(m_backgroundShape.getSize());
            m_cropShader.setUniform("u_size", sf::Vector2f(size));
        }
        break;
    default: break;
    }
    m_container.handleEvent(evt, getInverseTransform().transformPoint(mousePos));
    m_lastMousePos = mousePos;
}

void Window::setPalette(const Palette& p)
{
    m_palette = p;
    m_palette.borderActive.a = 255u;
    m_palette.borderNormal.a = 255u;

    m_backgroundShape.setFillColor(m_palette.background);
    m_backgroundShape.setOutlineColor(m_palette.borderActive);

    m_titleBar.setFillColor(m_palette.borderActive);

    m_resizeHandle.setFillColor(m_palette.background);
    m_resizeHandle.setOutlineColor(m_palette.borderNormal);

    m_titleText.setFillColor(m_palette.font);
}

void Window::setTitle(const std::string& str)
{
    m_titleText.setString(str);
}

void Window::addControl(Control::Ptr c)
{
    c->move(0.f, titleBarHeight);
    //c->setShader(&m_cropShader);
    m_container.addControl(c);
}

void Window::canResize(bool resize)
{
    m_resizable = resize;
}

bool Window::canResize() const
{
    return m_resizable;
}

//private
void Window::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= getTransform();
    rt.draw(m_backgroundShape, states);
    rt.draw(m_titleBar, states);
    rt.draw(m_resizeHandle, states);
    rt.draw(m_titleText, states);

    rt.draw(m_container, states);
}

namespace
{
    const sf::Uint8 radiusPoints = 4u;
    const float pi = 3.141592654f;
    const float radian = pi / 180.f;
}

//======class for drawing window background=======//
Window::BackgroundShape::BackgroundShape(const sf::Vector2f& size)
    : m_size        (size),
    m_minimumSize   (400.f, 400.f)
{
    clampSize();
    update();
}

void Window::BackgroundShape::setSize(const sf::Vector2f& size)
{
    m_size = size;
    clampSize();
    update();
}

void Window::BackgroundShape::setMinimumSize(const sf::Vector2f& size)
{
    m_minimumSize = size;
    setSize(m_size);
}

const sf::Vector2f& Window::BackgroundShape::getSize() const
{
    return m_size;
}

std::size_t Window::BackgroundShape::getPointCount() const
{
    return radiusPoints * 4u;
}

sf::Vector2f Window::BackgroundShape::getPoint(std::size_t index) const
{
    if (index >= getPointCount()) return sf::Vector2f();

    float deltaAngle = 90.f / (radiusPoints - 1);
    sf::Vector2f center;
    sf::Uint32 centerIndex = index / radiusPoints;

    switch (centerIndex)
    {
    case 0:
        center.x = m_size.x - cornerRadius;
        center.y = cornerRadius;
        break;
    case 1:
        center.x = cornerRadius;
        center.y = cornerRadius;
        break;
    case 2:
        center.x = cornerRadius;
        center.y = m_size.y - cornerRadius;
        break;
    case 3:
        center.x = m_size.x - cornerRadius;
        center.y = m_size.y - cornerRadius;
        break;
    default: break;
    }

    return
    {
        cornerRadius * std::cos(deltaAngle * (index - centerIndex) * radian) + center.x,
        -cornerRadius * std::sin(deltaAngle * (index - centerIndex) * radian) + center.y
    };
}

void Window::BackgroundShape::clampSize()
{
    m_size.x = std::max(m_size.x, m_minimumSize.x);
    m_size.y = std::max(m_size.y, m_minimumSize.y);
}
//===============================================//
