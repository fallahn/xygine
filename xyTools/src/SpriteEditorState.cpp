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

#include <SpriteEditorState.hpp>

#include <xygine/App.hpp>
#include <xygine/imgui/imgui.h>

SpriteEditorState::SpriteEditorState(xy::StateStack& stack, Context context)
    :xy::State  (stack, context),
    m_messageBus(context.appInstance.getMessageBus())
{
    launchLoadingScreen();
    buildMenu();
    quitLoadingScreen();
}

SpriteEditorState::~SpriteEditorState()
{
    xy::App::removeUserWindows(this);
    getContext().renderWindow.setTitle("xy tools");
}

//public
bool SpriteEditorState::update(float)
{
    return true;
}

void SpriteEditorState::draw()
{

}

bool SpriteEditorState::handleEvent(const sf::Event&)
{
    return true;
}

void SpriteEditorState::handleMessage(const xy::Message&)
{

}

//private
void SpriteEditorState::buildMenu()
{
    xy::App::addUserWindow([]()
    {
        nim::Begin("Sprite Editor!");

        nim::End();
    }, this);
}