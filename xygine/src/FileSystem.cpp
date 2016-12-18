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

#include <xygine/FileSystem.hpp>
#include <xygine/Log.hpp>

#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>
#include <algorithm>

//TODO check this macro works on all windows compilers
//(only tested in VC right now)
#ifdef _WIN32
#include <Windows.h>
#ifdef _MSC_VER
#include <direct.h> //gcc doesn't use this
#endif //_MSC_VER
#else
#include <libgen.h>
#include <dirent.h>

#include <fcntl.h>
#include <unistd.h>

#endif //_WIN32

using namespace xy;

std::vector<std::string> FileSystem::listFiles(std::string path)
{
    std::vector<std::string> results;

#ifdef _WIN32
    if (path.back() != '/')
        path.append("/*");
    else
        path.append("*");

    //convert to wide chars for windows
    std::basic_string<TCHAR> wPath;
    wPath.assign(path.begin(), path.end());

    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile(wPath.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        std::cout << "Failed to find file data, invalid file handle returned" << std::endl;
        return results;
    }

    do
    {
        if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) //not a directory
        {
            //convert from wide char
            std::basic_string<TCHAR> wName(findData.cFileName);
            std::string fileName;
            fileName.assign(wName.begin(), wName.end());
            results.push_back(fileName);
        }

    }while (FindNextFile(hFind, &findData) != 0);
    FindClose(hFind);

    return std::move(results);
#else
    if (path.back() != '/')
        path.append("/.");
    else
        path.append(".");

    struct dirent* dp;
    DIR* dir = opendir(path.c_str());

    if (dir)
    {
        while ((dp = readdir(dir)) != nullptr)
        {
            std::string workingPath(path);
            workingPath.append("/");
            workingPath.append((dp->d_name));

            struct stat buf;
            if (!stat(workingPath.c_str(), &buf))
            {
                if (!S_ISDIR(buf.st_mode))
                {
                    results.emplace_back(dp->d_name);
                }
            }
        }
        closedir(dir);
    }
    return std::move(results);
#endif //_WIN32
}

std::string FileSystem::getFileExtension(const std::string& path)
{
    if (path.find_last_of(".") != std::string::npos)
        return path.substr(path.find_last_of("."));
    else
        return "";
}

std::string FileSystem::getFileName(const std::string& path)
{
    //TODO this doesn't actually check that there is a file at the
    //end of the path, or that it's even a valid path...
    
    static auto searchFunc = [](const char seperator, const std::string& path)->std::string
    {
        std::size_t i = path.rfind(seperator, path.length());
        if (i != std::string::npos)
        {
            return(path.substr(i + 1, path.length() - i));
        }

        return "";
    };
    

#ifdef _WIN32 //try windows formatted paths first
    std::string retVal = searchFunc('\\', path);
    if (!retVal.empty()) return retVal;
#endif

    return searchFunc('/', path);
}

std::string FileSystem::getFilePath(const std::string& path)
{
    //TODO this doesn't actually check that there is a file at the
    //end of the path, or that it's even a valid path...

    static auto searchFunc = [](const char seperator, const std::string& path)->std::string
    {
        std::size_t i = path.rfind(seperator, path.length());
        if (i != std::string::npos)
        {
            return(path.substr(0, i + 1));
        }

        return "";
    };


#ifdef _WIN32 //try windows formatted paths first
    std::string retVal = searchFunc('\\', path);
    if (!retVal.empty()) return retVal;
#endif

    return searchFunc('/', path);
}

bool FileSystem::fileExists(const std::string& path)
{
    std::ifstream file(path);
    bool exists = (file.is_open() && file.good());
    file.close();
    return exists;
}

bool FileSystem::createDirectory(const std::string& path)
{
    //TODO regex this or at least check for illegal chars
#ifdef _WIN32
    if (_mkdir(path.c_str()) == 0)
    {
        return true;
    }
    else
    {
        auto result = errno;
        if (result == EEXIST)
        {
            xy::Logger::log(path + " directory already exists!", xy::Logger::Type::Info);
        }
        else if (result == ENOENT)
        {
            xy::Logger::log("Unable to create " + path + " directory not found.", xy::Logger::Type::Error);
        }
    }
    return false;
#else
    if (mkdir(path.c_str(), 0777) == 0)
    {
        return true;
    }
    else
    {
        auto result = errno;
        switch (result)
        {
        case EEXIST:
            {
                xy::Logger::log(path + " directory already exists!", xy::Logger::Type::Info);
            }
            break;
        case ENOENT:
            {
                xy::Logger::log("Unable to create " + path + " directory not found.", xy::Logger::Type::Error);
            }
            break;
        case EFAULT:
            {
                xy::Logger::log("Unable to create " + path + ". Reason: EFAULT", xy::Logger::Type::Error);
            }
            break;
        case EACCES:
            {
                xy::Logger::log("Unable to create " + path + ". Reason: EACCES", xy::Logger::Type::Error);
            }
            break;
        case ENAMETOOLONG:
            {
                xy::Logger::log("Unable to create " + path + ". Reason: ENAMETOOLONG", xy::Logger::Type::Error);
            }
            break;
        case ENOTDIR:
            {
                xy::Logger::log("Unable to create " + path + ". Reason: ENOTDIR", xy::Logger::Type::Error);
            }
            break;
        case ENOMEM:
            {
                xy::Logger::log("Unable to create " + path + ". Reason: ENOMEM", xy::Logger::Type::Error);
            }
            break;
        }
    }
    return false;
#endif //_WIN32
}

bool FileSystem::directoryExists(const std::string& path)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
    {
        xy::Logger::log(path + " access denied, or doesn't exist", xy::Logger::Type::Warning);
        return false;
    }
    else if (info.st_mode & S_IFDIR)
    {
        return true;
    }
    return false;
}

std::vector<std::string> FileSystem::listDirectories(const std::string& path)
{
    std::vector<std::string> retVal;
    std::string fullPath = path;
    std::replace(fullPath.begin(), fullPath.end(), '\\', '/');

    //make sure the given path is relative to the working directory
    /*std::string fullPath = getCurrentDirectory();
    std::replace(fullPath.begin(), fullPath.end(), '\\', '/');
    if (workingPath.empty() || workingPath[0] != '/') fullPath.push_back('/');
    fullPath += workingPath;*/

#ifdef _WIN32

    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    char fullpath[MAX_PATH];
    GetFullPathName(fullPath.c_str(), MAX_PATH, fullpath, 0);
    std::string fp(fullpath);

    hFind = FindFirstFile((LPCSTR)(fp + "\\*").c_str(), &findFileData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((findFileData.dwFileAttributes | FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY
                && (findFileData.cFileName[0] != '.'))
            {
                retVal.emplace_back(findFileData.cFileName);
            }
        } while (FindNextFile(hFind, &findFileData) != 0);
    }
#else
    DIR *dp;
    struct dirent *dirp;
    if ((dp = opendir(fullPath.c_str())) == nullptr)
    {
        Logger::log("Error(" + std::to_string(errno) + ") opening " + path, Logger::Type::Error);
        return retVal;
    }

    while ((dirp = readdir(dp)) != nullptr)
    {
        std::string str(dirp->d_name);
        if (str != "." && str != "..")
        {
            retVal.emplace_back(std::move(str));
        }
    }
    closedir(dp);

#endif //_WIN32
    return std::move(retVal);
}

std::string FileSystem::getCurrentDirectory()
{
#ifdef _WIN32
    TCHAR output[FILENAME_MAX];
    if (GetCurrentDirectory(FILENAME_MAX, output) == 0)
    {
        Logger::log("Failed to find the current working directory, error: " + std::to_string(GetLastError()), Logger::Type::Error);
        return{};
    }
    std::string retVal(output);
    std::replace(retVal.begin(), retVal.end(), '\\', '/');
    return std::move(retVal);
#else //this may not work on OSx
    char output[FILENAME_MAX];
    if (getcwd(output, FILENAME_MAX) == 0)
    {
        Logger::log("Failed to find the current working directory, error: " + std::to_string(errno), Logger::Type::Error);
        return{};
    }
    return{ output };
#endif //_WIN32
}

std::string FileSystem::getRelativePath(std::string path, const std::string& root)
{
    auto currentPath = root;
    std::replace(std::begin(path), std::end(path), '\\', '/');
    std::replace(std::begin(currentPath), std::end(currentPath), '\\', '/');
    
    int i = -1;
    auto pos = std::string::npos;
    auto length = 0;
    auto currentPos = std::string::npos;

    do
    {
        pos = path.find(currentPath);
        length = currentPath.size();

        currentPos = currentPath.find_last_of('/');
        if (currentPos != std::string::npos)
        {
            currentPath = currentPath.substr(0, currentPos);
        }
        i++;
    } while (pos == std::string::npos && currentPos != std::string::npos);

    std::string retVal;
    while (i-- > 0)
    {
        retVal += "../";
    }
    retVal += path.substr(pos + length + 1); //extra 1 for trailing '/'
    return std::move(retVal);
}