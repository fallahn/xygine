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

//static functions for cross platform file handling

#pragma once

#include "xyginext/Config.hpp"

#include <string>
#include <vector>

namespace xy
{
    /*!
    \brief Utilities for manipulating the current file system
    */
    class XY_EXPORT_API FileSystem final
    {
    public:
        /*!
        \brief Lists all the files in the given directory
        */
        static std::vector<std::string> listFiles(std::string path);
        /*!
        \brief Attempts to return a string containing the file extension
        of a given path, including the period (.)
        */
        static std::string getFileExtension(const std::string& path);
        /*!
        \brief Attempts to return the name of a file at the end of
        a given file path
        */
        static std::string getFileName(const std::string& path);
        /*!
        \brief Attempts to return the path of a given filepath without
        the file name, including trailing separator char.
        */
        static std::string getFilePath(const std::string& path);
        /*!
        \brief Returns true if a file exists with the name at the given path
        */
        static bool fileExists(const std::string&);
        /*!
        \brief Tries to create a directory relative to the executable
        or via an absolute path.
        \returns false if creation fails and attempts to log the reason,
        else returns true.
        \param std::string Path to create.
        */
        static bool createDirectory(const std::string&);
        /*!
        \brief Attempts to determine if a directory at the given path exists.
        \returns true if the directory exists, else false. Attempts to log any
        errors to the console.
        */
        static bool directoryExists(const std::string&);
        /*!
        \brief Returns a vector of strings containing the names of directories
        found in the given path.
        */
        static std::vector<std::string> listDirectories(const std::string&);
        /*!
        \brief Returns the absolute path of the current working directory
        */
        static std::string getCurrentDirectory();
        /*!
        \brief Sets the current working directory to the given absolute path
        \param path String containing the path to attempt to set to cwd
        \returns false on failure
        */
        static bool setCurrentDirectory(std::string path);

        /*! 
        \brief Attempts to convert the given absolute path to a path relative to the given root directory
        \param path Absolute path to convert
        \param root Absolute path to root directory to which the result should be relative
        */
        static std::string getRelativePath(std::string path, const std::string& root);

        /*!
        \brief Returns a path to the current user's config directory.
        Config files should generally be written to this directory, rather than the
        current working directory. Output is usually in the form of the following:
        \code
        Windows: C:\Users\squidward\AppData\Roaming\appname\
        Linux: /home/squidward/.config/appname/
        Mac: /Users/squidward/Library/Application Support/appname/
        \endcode

        WARNING some linux distros are known to return the current working directory
        instead. This should be considered when using configuration files with the same
        name as another which is expected to be stored in a unique directory.

        \param appName Name of the current application used to create the appname directory
        \returns Above formatted string, or an empty string if something went wrong
        */
        static std::string getConfigDirectory(const std::string& appName);

        /*!
        \brief Show a native file dialog to open a file
        \param defaultDir Default path to open (optional)
        \param filter File extention filter in the format "png,jpg,bmp"
        \returns path the path selected by the user
        */
        static std::string openFileDialogue(const std::string& defaultDir = "", const std::string& filter = "");

        /*!
        \brief Show a native file dialog to open a folder
        \returns path the path selected by the user
        */
        static std::string openFolderDialogue();
        
        /*!
        \brief Show a platform native file dialogue for saving files.
        \param defaultDir Default directory to save to - optional
        \param filter String containing file extension filter in the format "png,jpg,bmp"
        \returns string containing the selected file path
        */
        static std::string saveFileDialogue(const std::string& defaultDir = "", const std::string& filter = "");

        /*!
         \brief Currently only relevant on macOS when creating an app bundle.
         Basically a wrapper around the SFML resourcePath() function.
         Might potentially provide some portable way of bundling resources nicely in the future.
         \returns path to the resource directory
         */
        static std::string getResourcePath();

		/*!
		\brief Sets the resource directory relative to the working directory.
		When using getResourcePath() this path will be appended to the working directory.
		Used, for example, when setting a sub-directory as a resource directory
		*/
		static void setResourceDirectory(const std::string& path);

    private:
		static std::string m_resourceDirectory;
    };
}
