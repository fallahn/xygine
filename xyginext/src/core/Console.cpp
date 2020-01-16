/*********************************************************************
(c) Matt Marchant 2017 - 2020
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

#include "xyginext/core/Console.hpp"
#include "xyginext/core/ConfigFile.hpp"
#include "xyginext/core/Log.hpp"
#include "xyginext/core/App.hpp"
#include "xyginext/core/SysTime.hpp"
#include "xyginext/core/Assert.hpp"
#include "xyginext/audio/Mixer.hpp"
#include "xyginext/gui/GuiClient.hpp"

#include "xyginext/gui/imgui.h"

#include <SFML/System/Err.hpp>

#include <list>
#include <unordered_map>
#include <algorithm>
#include <array>
#include <tuple>
#include <streambuf>

using namespace xy;
namespace ui = ImGui;

namespace
{
    //static vars used by the console renderer
    std::vector<std::string> m_debugLines;
    std::vector<std::pair<std::function<void()>, const GuiClient*>> m_statusControls;
    
    using ConsoleTab = std::tuple<std::string, std::function<void()>, const GuiClient*>;
    std::vector<ConsoleTab> m_consoleTabs;

    std::vector<sf::Vector2u> resolutions;
    //int currentAALevel = 0;
    int currentResolution = 0;
    std::vector<char> resolutionNames{};
    bool fullScreen = false;
    bool vSync = false;
    bool useFrameLimit = false;
    int frameLimit = 10;

    
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
int textEditCallback(ImGuiInputTextCallbackData* data);

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

    const auto& size = App::getRenderWindow()->getSize();
    for (auto i = 0u; i < resolutions.size(); ++i)
    {
        if (resolutions[i].x == size.x && resolutions[i].y == size.y)
        {
            currentResolution = i;
            break;
        }
    }

    const auto& settings = App::getActiveInstance()->getVideoSettings();
    fullScreen = (settings.WindowStyle & sf::Style::Fullscreen);

    vSync = settings.VSync;
    useFrameLimit = (settings.FrameLimit != 0);
    frameLimit = settings.FrameLimit;
}

bool Console::isVisible()
{
    return visible;
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

        convars.save(FileSystem::getConfigDirectory(App::getActiveInstance()->getApplicationName()) + convarName);
    }
}

const ConfigObject& Console::getConvars()
{
    return convars;
}

//private
void Console::addCommand(const std::string& name, const Command& command, const ConsoleClient* client = nullptr)
{
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
}

void Console::removeCommands(const ConsoleClient* client)
{
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
}

void Console::draw()
{
    if (!visible) return;
    //ImGui::ShowDemoWindow();
    ui::SetNextWindowSizeConstraints({ 640, 480 }, { 1024.f, 768.f });
    if (ui::Begin("Console", &visible, ImGuiWindowFlags_NoScrollbar))
    {
        if (ui::BeginTabBar("Tabs"))
        {
            // Console
            if (ui::BeginTabItem("Console"))
            {
                ui::BeginChild("ScrollingRegion", ImVec2(0, -ui::GetFrameHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);
                ui::TextUnformatted(output.c_str(), output.c_str() + output.size());
                ui::SetScrollHereY(1.f); //TODO track when the user scrolled and set to correct position
                ui::EndChild();

                ui::Separator();

                ui::PushItemWidth(620.f);

                bool focus = false;
                if ((focus = ImGui::InputText(" ", input, MAX_INPUT_CHARS,
                    ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory,
                    &textEditCallback)))
                {
                    doCommand(input);
                }

                ui::PopItemWidth();

                ImGui::SetItemDefaultFocus();
                if (focus || ImGui::IsItemHovered()
                    || (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) 
                        && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)))
                {
                    ImGui::SetKeyboardFocusHere(-1);
                }

                ui::EndTabItem();
            }

            // Video options
            if (ui::BeginTabItem("Video"))
            {
                ui::Combo("Resolution", &currentResolution, resolutionNames.data());

                XY_ASSERT(App::getRenderWindow(), "no valid window");

                ui::Checkbox("Full Screen", &fullScreen);

                ui::Checkbox("V-Sync", &vSync);
                if (vSync)
                {
                    useFrameLimit = false;
                }

                ui::Checkbox("Limit Framerate", &useFrameLimit);
                if (useFrameLimit)
                {
                    vSync = false;
                }

                ui::SameLine();
                ui::PushItemWidth(80.f);
                ui::InputInt("Frame Rate", &frameLimit);
                ui::PopItemWidth();
                frameLimit = std::max(10, std::min(frameLimit, 360));

                if (ui::Button("Apply", { 50.f, 20.f }))
                {
                    //apply settings
                    auto settings = App::getActiveInstance()->getVideoSettings();
                    settings.VideoMode.width = resolutions[currentResolution].x;
                    settings.VideoMode.height = resolutions[currentResolution].y;
                    settings.WindowStyle = (fullScreen) ? sf::Style::Fullscreen : sf::Style::Close;
                    settings.VSync = vSync;
                    settings.FrameLimit = useFrameLimit ? frameLimit : 0;

                    App::getActiveInstance()->applyVideoSettings(settings);
                }
                ui::EndTabItem();
            }

            // Audio
            if (ui::BeginTabItem("Audio"))
            {
                ui::Text("NOTE: only AudioSystem sounds are affected.");

                static float maxVol = AudioMixer::getMasterVolume();
                ui::SliderFloat("Master", &maxVol, 0.f, 1.f);
                AudioMixer::setMasterVolume(maxVol);

                static std::array<float, AudioMixer::MaxChannels> channelVol;
                for (auto i = 0u; i < AudioMixer::MaxChannels; ++i)
                {
                    channelVol[i] = AudioMixer::getVolume(i);
                    ui::SliderFloat(AudioMixer::getLabel(i).c_str(), &channelVol[i], 0.f, 1.f);
                    AudioMixer::setVolume(channelVol[i], i);
                }
                ui::EndTabItem();
            }

            // Stats
            if (ui::BeginTabItem("Stats"))
            {
                ui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ui::NewLine();
                for (auto& line : m_debugLines)
                {
                    ImGui::TextUnformatted(line.c_str());
                }
                ui::EndTabItem();
            }
            m_debugLines.clear();
            m_debugLines.reserve(10);

            //display any registered controls
            int count(0);
            for (const auto& func : m_statusControls)
            {
                if (ImGui::BeginTabItem(("Stat " + std::to_string(count)).c_str()))
                {
                    func.first();
                    ui::EndTabItem();
                }
            }

            //display registered tabs
            for (const auto& tab : m_consoleTabs)
            {
                const auto&[name, func, c] = tab;
                if (ImGui::BeginTabItem(name.c_str()))
                {
                    func();
                    ui::EndTabItem();
                }
            }

            ui::EndTabBar();
        }
    }
    ui::End();
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

        //auto totalLen = width.size() + height.size() + 4; // x \0
        //if (i >= resolutionNames.size() - totalLen)
        //{
        //    break;
        //}

        for (char c : width)
        {
            resolutionNames.push_back(c);
        }

        resolutionNames.push_back(' ');
        resolutionNames.push_back('x');
        resolutionNames.push_back(' ');

        for (char c : height)
        {
            //resolutionNames[i++] = c;
            resolutionNames.push_back(c);
        }
        resolutionNames.push_back(0);
    }
    resolutionNames.push_back(0);

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
    convars.loadFromFile(FileSystem::getConfigDirectory(App::getActiveInstance()->getApplicationName()) + convarName);
}

void Console::finalise()
{
    convars.save(FileSystem::getConfigDirectory(App::getActiveInstance()->getApplicationName()) + convarName);
}

int textEditCallback(ImGuiInputTextCallbackData* data)
{
    //use this to scroll up and down through command history

    switch (data->EventFlag)
    {
    default: break;
    case ImGuiInputTextFlags_CallbackCompletion: //user pressed tab to complete
        {
            // Example of TEXT COMPLETION
            
            // Locate beginning of current word
            const char* word_end = data->Buf + data->CursorPos;
            const char* word_start = word_end;
            while (word_start > data->Buf)
            {
                const char c = word_start[-1];
                if (c == ' ' || c == '\t' || c == ',' || c == ';')
                    break;
                word_start--;
            }
            
            // Build a list of candidates
            std::vector<std::string> candidates;
            for (auto& c : commands)
            {
                if (c.first.substr(0,word_end - word_start) == std::string(word_start,word_end - word_start))
                {
                    candidates.push_back(c.first);
                }
            }
            
            if (candidates.size() == 0)
            {
                // No match
                break;
            }
            else if (candidates.size() == 1)
            {
                // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing
                data->DeleteChars((int)(word_start-data->Buf), (int)(word_end-word_start));
                data->InsertChars(data->CursorPos, candidates[0].c_str());
                data->InsertChars(data->CursorPos, " ");
            }
            else
            {
                // Multiple matches. Complete as much as we can, so inputing "C" will complete to "CL" and display "CLEAR" and "CLASSIFY"
                int match_len = (int)(word_end - word_start);
                for (;;)
                {
                    int c = 0;
                    bool all_candidates_matches = true;
                    for (auto i = 0u; i < candidates.size() && all_candidates_matches; i++)
                        if (i == 0)
                            c = toupper(candidates[i][match_len]);
                        else if (c == 0 || c != toupper(candidates[i][match_len]))
                            all_candidates_matches = false;
                    if (!all_candidates_matches)
                        break;
                    match_len++;
                }
                
                if (match_len > 0)
                {
                    data->DeleteChars((int)(word_start - data->Buf), (int)(word_end-word_start));
                    data->InsertChars(data->CursorPos, candidates[0].c_str(), candidates[0].c_str() + match_len);
                }
            }
            
            break;
    }
    case ImGuiInputTextFlags_CallbackHistory:
    {
        const int prev_history_pos = historyIndex;
        if (data->EventKey == ImGuiKey_UpArrow)
        {
            if (historyIndex == -1)
            {
                historyIndex = static_cast<std::int32_t>(history.size()) - 1;
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
                if (++historyIndex >= static_cast<std::int32_t>(history.size()))
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

void Console::addStatusControl(const std::function<void ()> &func, const xy::GuiClient *c)
{
    m_statusControls.push_back(std::make_pair(func, c));
}

void Console::removeStatusControls(const GuiClient* c)
{
    
    m_statusControls.erase(std::remove_if(std::begin(m_statusControls),
                                          std::end(m_statusControls),
                                          [c](const std::pair<std::function<void()>, const GuiClient*>& pair)
                                          {
                                              return pair.second == c;
                                          }), std::end(m_statusControls));
}

void Console::addConsoleTab(const std::string& name, const std::function<void()>& f, const GuiClient* c)
{
    m_consoleTabs.push_back(std::make_tuple(name, f, c));
}

void Console::removeConsoleTab(const GuiClient* c)
{
    m_consoleTabs.erase(std::remove_if(std::begin(m_consoleTabs),
        std::end(m_consoleTabs),
        [c](const ConsoleTab& tab)
    {
        const auto&[name, f, p] = tab;
        return c == p;
    }), std::end(m_consoleTabs));
}

void Console::printStat(const std::string& name, const std::string& value)
{
    m_debugLines.push_back(name + ":" + value);
}

