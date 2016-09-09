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

#include <xygine/imgui/imgui.h>
#include <xygine/Console.hpp>
#include <xygine/Assert.hpp>
#include <xygine/Log.hpp>
#include <xygine/App.hpp>

#include <list>
#include <unordered_map>
#include <algorithm>
#include <fstream>

using namespace xy;

namespace
{
    bool showVideoOptions = false;
    bool fullScreen = false;

    std::vector<sf::VideoMode> modes;
    int currentResolution = 0;
    char resolutionNames[300];
    
    
    //TODO we can make each line separate so output such
    //as warnings or errors can be highlighted
    //see https://github.com/ocornut/imgui/blob/master/imgui_demo.cpp#L2076
    std::string output;

    constexpr std::size_t MAX_INPUT_CHARS = 400;
    char input[MAX_INPUT_CHARS];
    std::list<std::string> buffer;
    const std::size_t MAX_BUFFER = 100;

    std::list<std::string> history;
    const std::size_t MAX_HISTORY = 10;
    std::int32_t historyIndex = -1;

    bool visible = false;

    std::unordered_map<std::string, std::pair<Console::Command, const void*>> commands;
}
int textEditCallback(ImGuiTextEditCallbackData* data);


void Console::print(const std::string& line)
{
    buffer.push_back(line);
    if (buffer.size() > MAX_BUFFER)
    {
        buffer.pop_front();
    }

    output.clear();
    for (const auto& str : buffer)
    {
        //TODO prepend timestamp?
        output.append(str);
        output.append("\n");
    }
}

void Console::show()
{
    visible = !visible;
}

void Console::addCommand(const std::string& name, const Console::Command& command, const void* owner)
{
    XY_ASSERT(!name.empty(), "Command cannot have an empty string");
    if (commands.count(name) != 0)
    {
        Logger::log("Command \"" + name + "\" exists! Command has been overwritten!", Logger::Type::Warning, Logger::Output::All);
        commands[name] = std::make_pair(command, owner);
    }
    else
    {
        commands.insert(std::make_pair(name, std::make_pair(command, owner)));
    }
}

void Console::unregisterCommands(const void* owner)
{
    //make sure this isn't nullptr else most if not all commands will get removed..
    XY_ASSERT(owner, "You really don't want to do that");
    
    for (auto i = commands.begin(); i != commands.end();)
    {
        if (i->second.second == owner)
        {
            i = commands.erase(i);
        }
        else
        {
            ++i;
        }
    }
}

void Console::doCommand(const std::string& str)
{
    //store in history
    history.push_back(str);
    if (history.size() > MAX_HISTORY)
    {
        history.pop_front();
    }
    historyIndex = -1;
    
    //parse the command from the string
    std::string command(str);
    std::string params;
    //shave off parameters if they exist
    std::size_t pos = command.find_first_of(' ');
    if (pos != std::string::npos)
    {
        if (pos < command.size())
        {
            params = command.substr(pos + 1);
        }
        command.resize(pos);
    }

    //locate command and execute it passing in params
    auto cmd = commands.find(command);
    if (cmd != commands.end())
    {
        cmd->second.first(params);
    }
    else
    {
        print(command + ": command not found!");
    }

    input[0] = '\0';
}

//private
void Console::draw(App* app)
{
    if (!visible) return;
    
    nim::SetNextWindowSizeConstraints({ 640, 480 }, { 1024.f, 768.f });
    if (!nim::Begin("Console", &visible, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_ShowBorders))
    {
        //window is collapsed so save your effort..
        nim::End();
        return;
    }

    //options at top of window
    if (nim::BeginMenuBar())
    {
        if (nim::BeginMenu("Options"))
        {
            if (nim::MenuItem("Video", nullptr, &showVideoOptions))
            {
                //select active mode
                const auto& activeMode = app->getVideoSettings().VideoMode;
                for (auto i = 0u; i < modes.size(); ++i)
                {
                    if (modes[i] == activeMode)
                    {
                        currentResolution = i;
                        break;
                    }
                }
            }
            nim::EndMenu();
        }

        if (nim::BeginMenu("Quit"))
        {
            xy::App::quit();
            nim::EndMenu();
        }
        nim::EndMenuBar();
    }

    nim::BeginChild("ScrollingRegion", ImVec2(0, -nim::GetItemsLineHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);
    nim::TextUnformatted(output.c_str(), output.c_str() + output.size());
    nim::SetScrollHere();
    nim::EndChild();

    nim::Separator();

    nim::PushItemWidth(620.f);
    if (nim::InputText("", input, MAX_INPUT_CHARS,
        ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory,
        textEditCallback))
    {
        doCommand(input);
    }
    nim::PopItemWidth();

    if (nim::IsItemHovered() || (nim::IsRootWindowOrAnyChildFocused()
        && !nim::IsAnyItemActive() && !nim::IsMouseClicked(0)))
    {
        nim::SetKeyboardFocusHere(-1);
    }

    nim::End();

    //draw options window if visible
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
        app->applyVideoSettings(settings);
    }
    nim::End();
}

void Console::registerDefaultCommands(App* app)
{
    //sets up the video options
    modes = app->getVideoSettings().AvailableVideoModes;
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
    
    //list all available commands to the console
    addCommand("list_all",
        [](const std::string&)
    {
        Console::print("Available Commands:");
        for (const auto& c : commands)
        {
            Console::print(c.first);
        }
    });

    //quits xygine
    addCommand("quit",
        [](const std::string&)
    {
        App::quit();
    });

    //prints version number
    addCommand("xy_version",
        [](const std::string&)
    {
        Console::print("xygine version 0.0.1");
    });

    //attempts to execute a config file
    addCommand("exec",
        [](const std::string& filePath)
    {
        if (filePath.empty())
        {
            Console::print("Usage: exec <file> where <file> is the path relative to\nthe working directory of the configuration file to execute.");
            return;
        }

        //attempt to open the file and execute line by line
        //TODO we need to validate the string a bit
        std::ifstream file(filePath);
        std::string line;
        if (file.good() && file.is_open())
        {
            Console::print("Executing " + filePath + "...");
            while (std::getline(file, line))
            {
                if (line.find_first_of("//") == 0)
                {
                    //Console::print(line); //probably don't want to print comments
                    continue;
                }
                else if (line.find_first_of("exec") == 0)
                {
                    //would be nice to execute other files as 'includes'
                    //but including a file in itself or including a file
                    //which executes the currently executing file is an
                    //easy stack overflow contender
                    continue;
                }
                else
                {
                    Console::doCommand(line);
                }
            }
        }
        else
        {
            Console::print("Unable to open file " + filePath);
        }
        file.close();
    });

    print("type list_all to see a list of available commands");
}

int textEditCallback(ImGuiTextEditCallbackData* data)
{
    //use this to scroll up and down through command history
    //TODO create an auto complete thinger

    switch (data->EventFlag)
    {
    default: break;
    case ImGuiInputTextFlags_CallbackCompletion: //user pressed tab to complete
    case ImGuiInputTextFlags_CallbackHistory:
        {
            const int prev_history_pos = historyIndex;
            if (data->EventKey == ImGuiKey_UpArrow)
            {
                if (historyIndex == -1)
                {
                    historyIndex = history.size() - 1;
                }
                else if (historyIndex > 0)
                {
                    historyIndex--;
                }
            }
            else if (data->EventKey == ImGuiKey_DownArrow)
            {
                if (historyIndex != -1)
                {
                    if (++historyIndex >= history.size())
                    {
                        historyIndex = -1;
                    }
                }
            }

            //a better implementation would preserve the data on the current input line along with cursor position.
            if (prev_history_pos != historyIndex)
            {
                data->CursorPos = data->SelectionStart = data->SelectionEnd = data->BufTextLen = 
                    (int)snprintf(data->Buf, (size_t)data->BufSize, "%s", (historyIndex >= 0) ? std::next(history.begin(), historyIndex)->c_str() : "");
                data->BufDirty = true;
            }
        }
    break;
    }

    return 0;
}

void Console::kill()
{
    visible = false;
}