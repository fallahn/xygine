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

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Font.hpp>

using namespace xy;

TextRenderer::TextRenderer(MessageBus& mb)
    : System(mb, typeid(TextRenderer))
{
    LOG("DEPRECATED: use the text system and render system with text/drawable components", xy::Logger::Type::Warning);

    requireComponent<Transform>();
    requireComponent<Text>();
}

//public
void TextRenderer::process(float)
{
    //auto& entities = getEntities();
    //m_texts.clear();
    //m_texts.reserve(entities.size());
    //m_croppedTexts.clear();
    //m_croppedTexts.reserve(entities.size());

    //for (auto& entity : entities)
    //{
    //    auto& text = entity.getComponent<Text>();
    //    if (text.m_dirty)
    //    {
    //        text.updateVertices();
    //    }

    //    //TODO - shouldn't this be in the dirty loop above?
    //    const auto& xForm = entity.getComponent<Transform>().getWorldTransform();

    //    //update world positions
    //    text.m_croppingWorldArea = xForm.transformRect(text.m_croppingArea);
    //    text.m_croppingWorldArea.top += text.m_croppingWorldArea.height;
    //    text.m_croppingWorldArea.height = -text.m_croppingWorldArea.height;

    //    text.m_globalBounds = xForm.transformRect(text.m_localBounds);

    //    //assign to relevant array
    //    (text.m_cropped) ? m_croppedTexts.push_back(entity) : m_texts.push_back(entity);
    //}
}

void TextRenderer::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    //auto viewSize = rt.getView().getSize();
    //sf::FloatRect viewable(rt.getView().getCenter() - (viewSize / 2.f), viewSize);

    //for (const auto& entity : m_texts)
    //{
    //    const auto& text = entity.getComponent<Text>();

    //    if (text.m_globalBounds.intersects(viewable))
    //    {
    //        states = text.m_states;
    //        states.texture = &text.m_font->getTexture(text.m_charSize);
    //        states.transform = entity.getComponent<Transform>().getWorldTransform();

    //        rt.draw(text.m_vertices.data(), text.m_vertices.size(), sf::Triangles, states);
    //    }
    //}

    //glEnable(GL_SCISSOR_TEST);
    //for (const auto& entity : m_croppedTexts)
    //{
    //    const auto& text = entity.getComponent<Text>();

    //    if (text.m_globalBounds.intersects(viewable))
    //    {
    //        states = text.m_states;
    //        states.texture = &text.m_font->getTexture(text.m_charSize);
    //        states.transform = entity.getComponent<Transform>().getWorldTransform();

    //        //convert cropping area to target coords (remember this might not be a window!)
    //        sf::Vector2f start(text.m_croppingWorldArea.left, text.m_croppingWorldArea.top);
    //        sf::Vector2f end(start.x + text.m_croppingWorldArea.width, start.y + text.m_croppingWorldArea.height);

    //        auto scissorStart = rt.mapCoordsToPixel(start);
    //        auto scissorEnd = rt.mapCoordsToPixel(end);
    //        //Y coords are flipped...
    //        auto rtHeight = rt.getSize().y;
    //        scissorStart.y = rtHeight - scissorStart.y;
    //        scissorEnd.y = rtHeight - scissorEnd.y;

    //        glScissor(scissorStart.x, scissorStart.y, scissorEnd.x - scissorStart.x, scissorEnd.y - scissorStart.y);
    //        rt.draw(text.m_vertices.data(), text.m_vertices.size(), sf::Triangles, states);
    //    }
    //}
    //glDisable(GL_SCISSOR_TEST);
}
