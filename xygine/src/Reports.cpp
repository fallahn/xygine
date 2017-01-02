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

#include <xygine/Reports.hpp>
#include <xygine/Config.hpp>
#include <xygine/imgui/imgui.h>
#include <xygine/Assert.hpp>

#include <SFML/System/Clock.hpp>

#include <algorithm>
#include <array>

using namespace xy;

namespace
{
    const std::string interweebl(": ");
    StatsReporter reporter;
    bool visible = false;
    sf::Clock frameTimer;
}

//---exported functions---//

const std::string& Stats::getString()
{
    return reporter.getString();
}

void Stats::report(const std::string& name, const std::string& value)
{
    reporter.report(name, value);
}

void Stats::clear()
{
    reporter.clear();
}

//-----------------------------//

StatsReporter::StatsReporter()
    : m_rebuildString(true)
{

}

//public
void StatsReporter::report(const std::string& name, const std::string& value)
{
    XY_ASSERT(!name.empty() && !value.empty(), "empty string values are not allowed");
    sf::Lock lock(m_mutex);
    m_data[name] = value;
    m_rebuildString = true;
}

void StatsReporter::remove(const std::string& name)
{
    sf::Lock lock(m_mutex);
    m_data.erase(name);
    m_rebuildString = true;
}

const std::string& StatsReporter::getString()
{
    sf::Lock lock(m_mutex);
    if (m_rebuildString)
    {
        m_rebuildString = false;

        m_string.clear();
        for (const auto& p : m_data)
        {
            m_string.append(p.first);
            m_string.append(interweebl);
            m_string.append(p.second);
            m_string.append("\n");
        }
    }

    return m_string;
}

void StatsReporter::clear()
{
    m_string.clear();
    m_data.clear();
}

void StatsReporter::show()
{
    visible = !visible;
}

//private
void StatsReporter::draw()
{
    if (!visible) return;

    nim::SetNextWindowSizeConstraints({ 300.f, 200.f }, { 400.f, 1000.f });
    nim::Begin("Stats:", &visible, ImGuiWindowFlags_ShowBorders);

    static std::array<float, 90> timeValues = { 0.f };
    static std::array<float, 90> frameValues = { 0.f };
    static std::size_t valuesIndex = 0;

    const float elapsed = frameTimer.restart().asSeconds();
    const float frameTime = elapsed * 1000.f;
    const float frameRate = 1.f / elapsed;

    static float avgTime = 0.f;
    avgTime -= avgTime / timeValues.size();
    avgTime += frameTime / timeValues.size();
    std::string avgTStr = "Avg: " + std::to_string(avgTime) + " (ms)";

    static float avgFrame = 0.f;
    avgFrame -= avgFrame / frameValues.size();
    avgFrame += frameRate / frameValues.size();
    std::string avgFStr = "Avg: " + std::to_string(avgFrame);

    timeValues[valuesIndex] = frameTime;
    frameValues[valuesIndex] = frameRate;
    valuesIndex = (valuesIndex + 1) % timeValues.size();
    nim::PlotLines("Frame Time", timeValues.data(), timeValues.size(), valuesIndex, avgTStr.c_str(), 0.f, std::max(33.f, frameTime), ImVec2(0, 40));
    nim::PlotLines("Frame Rate", frameValues.data(), frameValues.size(), valuesIndex, avgFStr.c_str(), 0.f, std::max(950.f, frameRate), ImVec2(0, 40));

    nim::BeginChild("ScrollingRegion", ImVec2(0, -nim::GetItemsLineHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);
    nim::TextUnformatted(Stats::getString().c_str(), Stats::getString().c_str() + Stats::getString().size());
    nim::SetScrollHere();
    nim::EndChild();
    nim::End();

    //nim::ShowTestWindow(&visible);
}

void StatsReporter::kill()
{
    visible = false;
}