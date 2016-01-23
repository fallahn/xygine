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

#include <xygine/spriter/Helpers.hpp>
#include <xygine/Assert.hpp>

using namespace xy;
using namespace xy::Spriter::Detail;

DirectoryLister::DirectoryLister() : m_fileCount(0u) {}

void DirectoryLister::addDirectory()
{
    m_directories.emplace_back();
}

void DirectoryLister::addFile()
{
    XY_ASSERT(!m_directories.empty(), "Needs at least one directory");
    m_directories.back().push_back(m_fileCount++);
}

int DirectoryLister::getIndex(std::size_t dirIdx, std::size_t fileIdx)
{
    XY_ASSERT(dirIdx < m_directories.size(), "Index out of range");
    const auto& files = m_directories[dirIdx];

    XY_ASSERT(fileIdx < files.size(), "Index out of range");
    return files[fileIdx];

    return -1;
}