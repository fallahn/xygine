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

#include <xygine/PostProcess.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Shader.hpp>

using namespace xy;

namespace
{
    sf::Vector2f outputSize;
    sf::Vector2u lastSize;
    sf::VertexArray vertexArray(sf::TrianglesStrip, 4);
    void updateQuad()
    {
        auto newY = outputSize.x / 16.f * 9.f;
        auto diff = (outputSize.y - newY) / 2.f;
        outputSize.y = newY + diff;

        vertexArray[0] = { { 0.f, diff },{ 0.f, 1.f } };
        vertexArray[1] = { { outputSize.x, diff },{ 1.f, 1.f } };
        vertexArray[2] = { { 0.f, outputSize.y }, sf::Vector2f() };
        vertexArray[3] = { outputSize,{ 1.f, 0 } };
    }
}

//PostProcess::PostProcess() {}

//public
void PostProcess::handleMessage(const Message& msg)
{
    for (auto& mh : m_messageHandlers)
    {
        if (mh.id == msg.id)
        {
            mh.action(msg);
        }
    }
}

void PostProcess::addMessageHandler(const MessageHandler& mh)
{
    m_messageHandlers.push_back(mh);
}

//protected
void PostProcess::applyShader(const sf::Shader& shader, sf::RenderTarget& dest)
{
    auto size = dest.getSize();
    if (lastSize != size)
    {
        outputSize = static_cast<sf::Vector2f>(size);
        lastSize = size;
        updateQuad();
    }

    //All the seagulls are belong to us.
    sf::RenderStates states;
    states.shader = &shader;
    states.blendMode = sf::BlendNone;

    dest.draw(vertexArray, states);
}
