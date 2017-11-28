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

#include <xyginext/core/FileSystem.hpp>
#include <xyginext/core/Log.hpp>

#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>
#include <algorithm>

//TODO check this macro works on all windows compilers
//(only tested in VC right now)
#ifdef _WIN32
#include <Windows.h>
#include <shlobj.h>
#define PATH_SEPARATOR_CHAR '\\'
#define PATH_SEPARATOR_STRING "\\"
#ifdef _MSC_VER
#include <direct.h> //gcc doesn't use this
#endif //_MSC_VER
#else
#include <libgen.h>
#include <dirent.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define PATH_SEPARATOR_CHAR '/'
#define PATH_SEPARATOR_STRING "/"

#ifdef __linux__
#define MAX_PATH 512
#include <string.h>
#include <stdlib.h>

#elif defined(__APPLE__)
#define MAX_PATH PATH_MAX
#include <CoreServices/CoreServices.h>
#endif

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

    return results;
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
    return results;
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
    return retVal;
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
    return retVal;
#else //this may not work on macOS
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
    return retVal;
}

std::string FileSystem::getConfigDirectory(const std::string& appName)
{
    if (appName.empty())
    {
        LOG("Unable to get configuration directory, app name cannot be empty", Logger::Type::Error);
        return{};
    }

    static constexpr std::size_t maxlen = MAX_PATH;
    char outStr[maxlen];
    const char* appname = appName.c_str();

    char* out = outStr;

#ifdef __linux__
    const char *out_orig = out;
    char *home = getenv("XDG_CONFIG_HOME");
    unsigned int config_len = 0;
    if (!home)
    {
        home = getenv("HOME");
        if (!home)
        {
            // Can't find home directory
            out[0] = 0;
            LOG("Unable to find HOME directory when creating confinguration directory", Logger::Type::Error);
            return {};
        }
        config_len = strlen(".config/");
    }

    unsigned int home_len = strlen(home);
    unsigned int appname_len = strlen(appname);

    /* first +1 is "/", second is trailing "/", third is terminating null */
    if (home_len + 1 + config_len + appname_len + 1 + 1 > maxlen)
    {
        out[0] = 0;
        return {};
    }

    memcpy(out, home, home_len);
    out += home_len;
    *out = '/';
    out++;
    if (config_len) 
    {
        memcpy(out, ".config/", config_len);
        out += config_len;
        /* Make the .config folder if it doesn't already exist */
        *out = '\0';
        mkdir(out_orig, 0755);
    }
    memcpy(out, appname, appname_len);
    out += appname_len;
    /* Make the .config/appname folder if it doesn't already exist */
    *out = '\0';
    mkdir(out_orig, 0755);
    *out = '/';
    out++;
    *out = 0;

#elif defined(_WIN32)
    if (maxlen < MAX_PATH) 
    {
        out[0] = 0;
        return {};
    }
    if (!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, out)))
    {
        out[0] = 0;
        return {};
    }
    /* We don't try to create the AppData folder as it always exists already */
    unsigned int appname_len = strlen(appname);
    if (strlen(out) + 1 + appname_len + 1 + 1 > maxlen) 
    {
        out[0] = 0;
        return {};
    }
    strcat(out, "\\");
    strcat(out, appname);
    /* Make the AppData\appname folder if it doesn't already exist */
    _mkdir(out);
    strcat(out, "\\");

#elif defined(__APPLE__)
    FSRef ref;
    FSFindFolder(kUserDomain, kApplicationSupportFolderType, kCreateFolder, &ref);
    char home[MAX_PATH];
    FSRefMakePath(&ref, (UInt8 *)&home, MAX_PATH);
    /* first +1 is "/", second is trailing "/", third is terminating null */
    if (strlen(home) + 1 + strlen(appname) + 1 + 1 > maxlen)
    {
        out[0] = 0;
        return {};
    }

    strcpy(out, home);
    strcat(out, PATH_SEPARATOR_STRING);
    strcat(out, appname);
    /* Make the .config/appname folder if it doesn't already exist */
    mkdir(out, 0755);
    strcat(out, PATH_SEPARATOR_STRING);
#endif

    return { out };
}
