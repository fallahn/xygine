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

#include <MaterialEditorState.hpp>

#include <xygine/App.hpp>
#include <xygine/imgui/imgui.h>
#include <xygine/imgui/CommonDialogues.hpp>

MaterialEditorState::MaterialEditorState(xy::StateStack& stack, Context context)
    :xy::State  (stack, context),
    m_messageBus(context.appInstance.getMessageBus())
{
    launchLoadingScreen();
    buildMenu();
    quitLoadingScreen();
}

MaterialEditorState::~MaterialEditorState()
{
    xy::App::removeUserWindows(this);
}

//public
bool MaterialEditorState::update(float)
{
    return true;
}

void MaterialEditorState::draw()
{

}

bool MaterialEditorState::handleEvent(const sf::Event&)
{
    return true;
}

void MaterialEditorState::handleMessage(const xy::Message&)
{

}

//private
void MaterialEditorState::buildMenu()
{    
    xy::App::addUserWindow([]()
    {        
        static std::string selectedFile;
        
        nim::SetNextWindowSizeConstraints({ 200.f, 500.f }, { 800.f, 600.f });
        if (!nim::Begin("Model Viewer and Material Editor", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_ShowBorders))
        {
            nim::End();
            return;
        }

        if (nim::fileBrowseDialogue("Select Model", selectedFile, nim::Button("Open Model"))
            && !selectedFile.empty())
        {
            //attempt to load model
        }

        selectedFile.clear();
        nim::SameLine();
        if (nim::fileBrowseDialogue("Select Material", selectedFile, nim::Button("Open Material"))
            && !selectedFile.empty())
        {
            //attempt to load material
        }
        //TODO loading images, saving materials, switching on debug view

        nim::End();
    }, this);
}
