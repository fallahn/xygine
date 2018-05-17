/*********************************************************************
(c) Matt Marchant 2017 - 2018
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

#include "xyginext/ecs/systems/TextRenderer.hpp"
#include "xyginext/ecs/components/Text.hpp"
#include "xyginext/ecs/components/Transform.hpp"
#include "xyginext/util/Rectangle.hpp"

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/OpenGL.hpp>

using namespace xy;

TextRenderer::TextRenderer(MessageBus& mb)
    : System(mb, typeid(TextRenderer))
{
    requireComponent<Transform>();
    requireComponent<Text>();
}

//public
void TextRenderer::process(float)
{
    auto addQuad = [](Text& text, sf::Vector2f position, const sf::Glyph& glyph)
    {
        float left = glyph.bounds.left;
        float top = glyph.bounds.top;
        float right = glyph.bounds.left + glyph.bounds.width;
        float bottom = glyph.bounds.top + glyph.bounds.height;

        float u1 = static_cast<float>(glyph.textureRect.left);
        float v1 = static_cast<float>(glyph.textureRect.top);
        float u2 = static_cast<float>(glyph.textureRect.left + glyph.textureRect.width);
        float v2 = static_cast<float>(glyph.textureRect.top + glyph.textureRect.height);

        auto& vertices = text.m_vertices;
        auto& colour = text.m_fillColour;

        vertices.push_back(sf::Vertex(sf::Vector2f(position.x + left, position.y + top), colour, sf::Vector2f(u1, v1)));
        vertices.push_back(sf::Vertex(sf::Vector2f(position.x + right, position.y + top), colour, sf::Vector2f(u2, v1)));
        vertices.push_back(sf::Vertex(sf::Vector2f(position.x + left, position.y + bottom), colour, sf::Vector2f(u1, v2)));
        vertices.push_back(sf::Vertex(sf::Vector2f(position.x + left, position.y + bottom), colour, sf::Vector2f(u1, v2)));
        vertices.push_back(sf::Vertex(sf::Vector2f(position.x + right, position.y + top), colour, sf::Vector2f(u2, v1)));
        vertices.push_back(sf::Vertex(sf::Vector2f(position.x + right, position.y + bottom), colour, sf::Vector2f(u2, v2)));
    };

    auto& entities = getEntities();
    m_texts.clear();
    m_texts.reserve(entities.size());
    m_croppedTexts.clear();
    m_croppedTexts.reserve(entities.size());

    for (auto& entity : entities)
    {
        auto& text = entity.getComponent<Text>();
        if (text.m_dirty)
        {
            text.m_dirty = false;

            text.m_vertices.clear();
            text.m_localBounds = {};

            //skip if nothing to build
            if (!text.m_font || text.m_string.isEmpty())
            {
                continue;
            }

            //update glyphs - TODO here we could check for bold fonts in the future
            auto font = text.m_font;
            float xOffset = static_cast<float>(font->getGlyph(L' ', text.m_charSize, false).advance);
            float yOffset = static_cast<float>(font->getLineSpacing(text.m_charSize));
            float x = 0.f;
            float y = static_cast<float>(text.m_charSize);

            float minX = y;
            float minY = y;
            float maxX = 0.f;
            float maxY = 0.f;

            sf::Uint32 prevChar = 0;
            const auto& string = text.m_string;
            for (auto i = 0u; i < string.getSize(); ++i)
            {
                sf::Uint32 currChar = string[i];

                x += font->getKerning(prevChar, currChar, text.m_charSize);
                prevChar = currChar;

                //whitespace chars
                if (currChar == ' ' || currChar == '\t' || currChar == '\n')
                {
                    minX = std::min(minX, x);
                    minY = std::min(minY, y);

                    switch (currChar)
                    {
                    default: break;
                    case ' ':
                        x += xOffset;
                        break;
                    case '\t':
                        x += xOffset * 4.f; //4 spaces for tab suckas
                        break;
                    case '\n':
                        y += yOffset;
                        x = 0.f;
                        break;
                    }

                    maxX = std::max(maxX, x);
                    maxY = std::max(maxY, y);

                    continue; //skip quad for whitespace
                }

                //create the quads.
                const auto& glyph = font->getGlyph(currChar, text.m_charSize, false);
                addQuad(text, sf::Vector2f(x, y), glyph);

                float left = glyph.bounds.left;
                float top = glyph.bounds.top;
                float right = glyph.bounds.left + glyph.bounds.width;
                float bottom = glyph.bounds.top + glyph.bounds.height;

                minX = std::min(minX, x + left);
                maxX = std::max(maxX, x + right);
                minY = std::min(minY, y + top);
                maxY = std::max(maxY, y + bottom);

                x += glyph.advance;
            }

            text.m_localBounds.left = minX;
            text.m_localBounds.top = minY;
            text.m_localBounds.width = maxX - minX;
            text.m_localBounds.height = maxY - minY;


            //check for alignment
            float offset = 0.f;
            if (text.m_alignment == Text::Alignment::Centre)
            {
                offset = text.m_localBounds.width / 2.f;
            }
            else if (text.m_alignment == Text::Alignment::Right)
            {
                offset = text.m_localBounds.width;
            }
            if (offset > 0)
            {
                for (auto& v : text.m_vertices)
                {
                    v.position.x -= offset;
                }
                text.m_localBounds.left -= offset;
            }

            //use the local bounds to see if we want cropping or not
            text.m_cropped = !Util::Rectangle::contains(text.m_croppingArea, text.m_localBounds);
        }

        //TODO - shouldn't this be in the dirty loop above?
        const auto& xForm = entity.getComponent<Transform>().getWorldTransform();

        //update world positions
        text.m_croppingWorldArea = xForm.transformRect(text.m_croppingArea);
        text.m_croppingWorldArea.top += text.m_croppingWorldArea.height;
        text.m_croppingWorldArea.height = -text.m_croppingWorldArea.height;

        text.m_globalBounds = xForm.transformRect(text.m_localBounds);

        //assign to relevant array
        (text.m_cropped) ? m_croppedTexts.push_back(entity) : m_texts.push_back(entity);
    }
}

void TextRenderer::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    auto viewSize = rt.getView().getSize();
    sf::FloatRect viewable(rt.getView().getCenter() - (viewSize / 2.f), viewSize);

    for (const auto& entity : m_texts)
    {
        const auto& text = entity.getComponent<Text>();

        if (text.m_globalBounds.intersects(viewable))
        {
            states = text.m_states;
            states.texture = &text.m_font->getTexture(text.m_charSize);
            states.transform = entity.getComponent<Transform>().getWorldTransform();

            rt.draw(text.m_vertices.data(), text.m_vertices.size(), sf::Triangles, states);
        }
    }

    glEnable(GL_SCISSOR_TEST);
    for (const auto& entity : m_croppedTexts)
    {
        const auto& text = entity.getComponent<Text>();

        if (text.m_globalBounds.intersects(viewable))
        {
            states = text.m_states;
            states.texture = &text.m_font->getTexture(text.m_charSize);
            states.transform = entity.getComponent<Transform>().getWorldTransform();

            //convert cropping area to target coords (remember this might not be a window!)
            sf::Vector2f start(text.m_croppingWorldArea.left, text.m_croppingWorldArea.top);
            sf::Vector2f end(start.x + text.m_croppingWorldArea.width, start.y + text.m_croppingWorldArea.height);

            auto scissorStart = rt.mapCoordsToPixel(start);
            auto scissorEnd = rt.mapCoordsToPixel(end);
            //Y coords are flipped...
            auto rtHeight = rt.getSize().y;
            scissorStart.y = rtHeight - scissorStart.y;
            scissorEnd.y = rtHeight - scissorEnd.y;

            glScissor(scissorStart.x, scissorStart.y, scissorEnd.x - scissorStart.x, scissorEnd.y - scissorStart.y);
            rt.draw(text.m_vertices.data(), text.m_vertices.size(), sf::Triangles, states);
        }
    }
    glDisable(GL_SCISSOR_TEST);
}

void TextRenderer::handleMessage(const xy::Message &msg)
{
    if (msg.id == Message::Type::ResourceMessage)
    {
        // Check if any fonts need to be reloaded
        auto data = msg.getData<Message::ResourceEvent>();
        for (auto& e : getEntities())
        {
            auto& t = e.getComponent<xy::Text>();
            if (t.getFontResourceID() == data.id)
            {
                t.setFont(*reinterpret_cast<sf::Font*>(data.resource));
            }
        }
    }
}

