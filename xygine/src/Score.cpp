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

#include <xygine/Score.hpp>
#include <xygine/Log.hpp>
#include <xygine/util/Random.hpp>

#include <string>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <functional>

namespace
{
    const int ident = 0x534e5542;
    const int version = 5;
    const std::string scoreFile = "scores.dat";
    const std::size_t maxScores = 100;

    const std::vector<std::string> fallbackNames = 
    {
        "Jeff",
        "Spider",
        "Hannah",
        "Harry P.",
        "Martin",
        "Joanne",
        "Naomi",
        "Jim Beef",
        "Tony 'The Blank' Neals",
        "Steffi"
    };
}

using namespace xy;

void Scores::load()
{
    std::fstream file(scoreFile, std::ios::binary | std::ios::in);
    if (!file.good() || !file.is_open() || file.fail())
    {
        Logger::log("failed to open score data for reading", Logger::Type::Warning, Logger::Output::All);
        Logger::log("file probably missing - new file will be created", Logger::Type::Warning, Logger::Output::All);
        file.close();
        createFallBack();
        return;
    }

    file.seekg(0, std::ios::end);
    int fileSize = static_cast<int>(file.tellg());
    file.seekg(0, std::ios::beg);

    if (fileSize < static_cast<int>(sizeof(Header)))
    {
        Logger::log("unexpected file size for score data.", Logger::Type::Error, Logger::Output::All);
        file.close();
        return;
    }

    std::vector<char>fileData(fileSize);
    file.read(fileData.data(), fileSize);
    file.close();

    Scores::Header header;
    std::memcpy(&header, fileData.data(), sizeof(Header));
    if (header.ident != ident || header.version != version)
    {
        Logger::log("unusual ident found in score data, or incorrect version", Logger::Type::Error, Logger::Output::All);
        return;
    }

    m_easyScores.resize(header.chunks[0].size / sizeof(Item));
    std::memcpy(m_easyScores.data(), fileData.data() + header.chunks[0].offset, header.chunks[0].size);

    m_mediumScores.resize(header.chunks[1].size / sizeof(Item));
    std::memcpy(m_mediumScores.data(), fileData.data() + header.chunks[1].offset, header.chunks[1].size);

    m_hardScores.resize(header.chunks[2].size / sizeof(Item));
    std::memcpy(m_hardScores.data(), fileData.data() + header.chunks[2].offset, header.chunks[2].size);
}

void Scores::save()
{
    std::fstream file(scoreFile, std::ios::binary | std::ios::out);
    if (!file.good() || !file.is_open() || file.fail())
    {
        Logger::log("failed to open score data for writing", Logger::Type::Error, Logger::Output::All);
        file.close();
        return;
    }

    Header header;
    header.ident = ident;
    header.version = version;
    header.chunks[0].offset = sizeof(Header);
    header.chunks[0].size = sizeof(Item) * m_easyScores.size();

    header.chunks[1].offset = header.chunks[0].offset + header.chunks[0].size;
    header.chunks[1].size = sizeof(Item) * m_mediumScores.size();

    header.chunks[2].offset = header.chunks[1].offset + header.chunks[1].size;
    header.chunks[2].size = sizeof(Item) * m_hardScores.size();

    file.write((char*)&header, sizeof(header));
    file.write((char*)m_easyScores.data(), header.chunks[0].size);
    file.write((char*)m_mediumScores.data(), header.chunks[1].size);
    file.write((char*)m_hardScores.data(), header.chunks[2].size);
    file.close();
}

int Scores::add(const std::string& name, int value, Difficulty difficulty)
{
    std::vector<Item>* scores = nullptr;
    switch (difficulty)
    {
    default:
    case Difficulty::Easy:
        scores = &m_easyScores;
        break;
    case Difficulty::Normal:
        scores = &m_mediumScores;
        break;
    case Difficulty::Hard:
        scores = &m_hardScores;
        break;
    }

    scores->emplace_back();

    Scores::Item& score = scores->back();
    std::strcpy(score.name, name.c_str());
    score.score = value;

    auto hashFunc = std::hash<std::string>();
    auto hash = hashFunc(name) + static_cast<std::size_t>(value);
    score.hash = hash;

    std::sort(scores->begin(), scores->end(),
        [](const Scores::Item& sv1, const Scores::Item& sv2)
    {
        return sv1.score > sv2.score;
    });

    if (scores->size() > maxScores) scores->pop_back();

    auto result = std::find_if(scores->begin(), scores->end(),
        [hash](const Scores::Item& sv)
    {
        return (sv.hash == hash);
    });

    return (result == scores->end()) ? 0 : result - scores->begin();
}

const std::vector<Scores::Item>& Scores::getScores(Difficulty difficulty) const
{
    switch (difficulty)
    {
    default:
    case Difficulty::Easy:
        return m_easyScores;
    case Difficulty::Normal:
        return m_mediumScores;
    case Difficulty::Hard:
        return m_hardScores;
    }
}

//private
void Scores::createFallBack()
{
    for (const auto& n : fallbackNames)
    {
        add(n, xy::Util::Random::value(184, 2249), Difficulty::Easy);
        add(n, xy::Util::Random::value(365, 2864), Difficulty::Normal);
        add(n, xy::Util::Random::value(253, 2843), Difficulty::Hard);
    }
}