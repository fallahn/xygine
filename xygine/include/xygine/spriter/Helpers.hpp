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

#ifndef XY_SPRITER_HELPERS_HPP_
#define XY_SPRITER_HELPERS_HPP_

#include <vector>

namespace xy
{
    /*!
    \brief Classes for parsing Spriter animation files
    */
    namespace Spriter
    {
        namespace Detail
        {
            /*!
            \brief Used internally by Sprite file parser
            */
            //lists all files / sub files in a given directory
            class DirectoryLister final
            {
            public:
                DirectoryLister();
                ~DirectoryLister() = default;

                void addDirectory();
                void addFile();
                int getIndex(std::size_t dirIdx, std::size_t fileIdx);

            private:
                std::vector<std::vector<std::size_t>> m_directories;
                std::size_t m_fileCount;
            };
        }
    }
}

#endif //XY_SPRITER_HELPERS_HPP_