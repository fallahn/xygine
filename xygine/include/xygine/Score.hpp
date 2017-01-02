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

//read / write score data

#ifndef XY_SCORES_HPP_
#define XY_SCORES_HPP_

#include <xygine/Difficulty.hpp>
#include <xygine/Config.hpp>

#include <vector>
#include <string>

namespace xy
{
    /*!
    \brief Provides ability to load and save scores as an external binary file

    Used internally by xygine
    */
    class XY_EXPORT_API Scores final
    {
    public:
        struct Chunk
        {
            unsigned int offset;
            unsigned int size;
        };

        struct Header
        {
            int ident;
            int version;
            Chunk chunks[3];
        };

        struct Item
        {
            char name[21];
            int score;
            std::size_t hash;
        };

        Scores() = default;
        ~Scores() = default;
        Scores(const Scores&) = delete;
        const Scores& operator = (const Scores&) = delete;

        void load();
        void save();
        int add(const std::string&, int, Difficulty);

        const std::vector<Item>& getScores(Difficulty) const;

    private:
        std::vector<Item> m_easyScores;
        std::vector<Item> m_mediumScores;
        std::vector<Item> m_hardScores;

        void createFallBack();
    };
}
#endif //XY_SCORES_HPP_