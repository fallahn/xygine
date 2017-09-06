/*********************************************************************
(c) Matt Marchant 2017
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

#include <xyginext/core/Console.hpp>
#include <xyginext/core/ConfigFile.hpp>
#include <xyginext/core/Log.hpp>
#include <xyginext/core/App.hpp>
#include <xyginext/core/SysTime.hpp>
#include <xyginext/core/Assert.hpp>
#include <xyginext/audio/Mixer.hpp>

#include "../imgui/imgui.h"

#include <list>
#include <unordered_map>
#include <algorithm>
#include <array>

using namespace xy;
namespace nim = ImGui;

//this is a fallover from crogine - I'm keeping this in case I decide to implement it
#define USE_IMGUI 1

namespace
{
    bool showVideoOptions = false;
    bool showAudioOptions = false;

    std::vector<sf::Vector2u> resolutions;
    //int currentAALevel = 0;
    int currentResolution = 0;
    std::array<char, 300> resolutionNames{};
    
    std::string output;

    constexpr std::size_t MAX_INPUT_CHARS = 400;
    char input[MAX_INPUT_CHARS];
    std::list<std::string> buffer;
    const std::size_t MAX_BUFFER = 50;

    std::list<std::string> history;
    const std::size_t MAX_HISTORY = 10;
    std::int32_t historyIndex = -1;

    bool visible = false;

    std::unordered_map<std::string, std::pair<Console::Command, const ConsoleClient*>> commands;

    ConfigFile convars;
    const std::string convarName("convars.cfg");
}
int textEditCallback(ImGuiTextEditCallbackData* data);

//public
void Console::print(const std::string& line)
{
    if (line.empty()) return;

    std::string timestamp("<" + SysTime::timeString() + "> ");

    buffer.push_back(timestamp + line);
    if (buffer.size() > MAX_BUFFER)
    {
        buffer.pop_front();
    }

    output.clear();
    for (const auto& str : buffer)
    {
        output.append(str);
        output.append("\n");
    }
}

void Console::show()
{
    visible = !visible;
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
        //check to see if we have a convar
        auto* convar = convars.findObjectWithName(command);
        if (convar)
        {
            if (!params.empty())
            {
                auto* value = convar->findProperty("value");
                if (value) value->setValue(params);
                //TODO trigger a callback so systems can act on new value
            }
            else
            {
                auto* help = convar->findProperty("help");
                if (help) print(help->getValue<std::string>());
            }
        }
        else
        {
            print(command + ": command not found!");
        }
    }

    input[0] = '\0';
}

void Console::addConvar(const std::string& name, const std::string& defaultValue, const std::string& helpStr)
{
    if (!convars.findObjectWithName(name))
    {
        auto* obj = convars.addObject(name);
        obj->addProperty("value", defaultValue);
        obj->addProperty("help", helpStr);
    }
}


//private
void Console::addCommand(const std::string& name, const Command& command, const ConsoleClient* client = nullptr)
{
#ifdef USE_IMGUI
    XY_ASSERT(!name.empty(), "Command cannot have an empty string");
    if (commands.count(name) != 0)
    {
        Logger::log("Command \"" + name + "\" exists! Command has been overwritten!", Logger::Type::Warning, Logger::Output::All);
        commands[name] = std::make_pair(command, client);
    }
    else
    {
        commands.insert(std::make_pair(name, std::make_pair(command, client)));
    }
#endif //USE_IMGUI
}

void Console::removeCommands(const ConsoleClient* client)
{
#ifdef USE_IMGUI
    //make sure this isn't nullptr else most if not all commands will get removed..
    XY_ASSERT(client, "You really don't want to do that");

    for (auto i = commands.begin(); i != commands.end();)
    {
        if (i->second.second == client)
        {
            i = commands.erase(i);
        }
        else
        {
            ++i;
        }
    }
#endif //USE_IMGUI
}

void Console::draw()
{
#ifdef USE_IMGUI
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
                //select active resolution
                const auto& size = App::getRenderWindow().getSize();
                for (auto i = 0u; i < resolutions.size(); ++i)
                {
                    if (resolutions[i].x == size.x && resolutions[i].y == size.y)
                    {
                        currentResolution = i;
                        break;
                    }
                }
            }

            nim::MenuItem("Audio", nullptr, &showAudioOptions);

            if (nim::MenuItem("Quit", nullptr))
            {
                App::quit();
            }

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
    if (showVideoOptions)
    {
        nim::SetNextWindowSize({ 305.f, 100.f });
        nim::Begin("Video Options", &showVideoOptions, ImGuiWindowFlags_ShowBorders);

        nim::Combo("Resolution", &currentResolution, resolutionNames.data());

        auto settings = App::getActiveInstance().getVideoSettings();
        static bool vSync = settings.VSync;
        nim::Checkbox("V-Sync", &vSync);

        nim::SameLine();
        static bool fullScreen = (settings.WindowStyle & sf::Style::Fullscreen);
        nim::Checkbox("Full Screen", &fullScreen);

        if (nim::Button("Apply", { 50.f, 20.f }))
        {
            //apply settings       
            settings.VideoMode.width = resolutions[currentResolution].x;
            settings.VideoMode.height = resolutions[currentResolution].y;
            settings.WindowStyle = (fullScreen) ? sf::Style::Fullscreen : sf::Style::Close;
            settings.VSync = vSync;

            App::getActiveInstance().applyVideoSettings(settings);
        }
        nim::End();
    }

    if (showAudioOptions)
    {
        nim::SetNextWindowSize({ 315.f, 440.f });
        nim::Begin("Audio Mixer", &showAudioOptions, ImGuiWindowFlags_ShowBorders);

        nim::Text("NOTE: only AudioSystem sounds are affected.");

        static float maxVol = AudioMixer::getMasterVolume();
        nim::SliderFloat("Master", &maxVol, 0.f, 1.f);
        AudioMixer::setMasterVolume(maxVol);

        static std::array<float, AudioMixer::MaxChannels> channelVol;
        for (auto i = 0u; i < AudioMixer::MaxChannels; ++i)
        {
            channelVol[i] = AudioMixer::getVolume(i);
            nim::SliderFloat(AudioMixer::getLabel(i).c_str(), &channelVol[i], 0.f, 1.f);
            AudioMixer::setVolume(channelVol[i], i);
        }

        nim::End();
    }
#endif //USE_IMGUI
}

void Console::init()
{
    auto modes = sf::VideoMode::getFullscreenModes();
    for (const auto& mode : modes)
    {
        if (mode.bitsPerPixel == 32)
        {
            resolutions.emplace_back(mode.width, mode.height);
        }
    }
    
    std::reverse(std::begin(resolutions), std::end(resolutions));

    int i = 0;
    for (auto r = resolutions.begin(); r != resolutions.end(); ++r)
    {
        std::string width = std::to_string(r->x);
        std::string height = std::to_string(r->y);

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

    //------default commands------//
    //list all available commands to the console
    addCommand("help",
        [](const std::string&)
    {
        Console::print("Available Commands:");
        for (const auto& c : commands)
        {
            Console::print(c.first);
        }

        Console::print("Available Variables:");
        const auto& objects = convars.getObjects();
        for (const auto& o : objects)
        {
            std::string str = o.getName();
            const auto& properties = o.getProperties();
            for (const auto& p : properties)
            {
                if (p.getName() == "help")
                {
                    str += " " + p.getValue<std::string>();
                }
            }
            Console::print(str);
        }
    });

    //search for a command
    addCommand("find",
        [](const std::string& param)
    {
        if (param.empty())
        {
            Console::print("Usage: find <command> where <command> is the name or part of the name of a command to find");
        }
        else
        {           
            auto term = param;
            std::size_t p = term.find_first_of(" ");
            if (p != std::string::npos)
            {
                term = term.substr(0, p);
            }
            auto searchterm = term;
            auto len = term.size();

            std::vector<std::string> results;
            while (len > 0)
            {
                for (const auto& c : commands)
                {
                    if (c.first.find(term) != std::string::npos
                        && std::find(results.begin(), results.end(), c.first) == results.end())
                    {
                        results.push_back(c.first);
                    }
                }
                term = term.substr(0, len--);
            }

            if (!results.empty())
            {
                Console::print("Results for: " + searchterm);
                for (const auto& str : results)
                {
                    Console::print(str);
                }
            }
            else
            {
                Console::print("No results for: " + searchterm);
            }
        }
    });

    //quits
    addCommand("quit",
        [](const std::string&)
    {
        App::quit();
    });


    //loads any convars which may have been saved
    convars.loadFromFile(FileSystem::getConfigDirectory(APP_NAME) + convarName);
    //TODO execute callback for each to make sure values are applied
}

void Console::finalise()
{
    convars.save(FileSystem::getConfigDirectory(APP_NAME) + convarName);
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

