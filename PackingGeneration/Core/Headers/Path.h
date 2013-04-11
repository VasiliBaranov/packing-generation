// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Core_Headers_Path_h
#define Core_Headers_Path_h

#include <string>
#include <vector>

namespace Core
{
    // Provides basic operations with paths.
    // Used to avoid referencing boost::filesystem.
    class Path
    {
    public:
        static std::string GetCurrentWorkingFolder();

        static std::string Append(std::string basePath, std::string path);

        static void FillFoldersRecursively(std::string basePath, std::string fileNameToCheck, std::vector<std::string>* folderPaths);

        static void FillFileNames(std::string basePath, std::vector<std::string>* fileNames);

        static void FillFolderNames(std::string basePath, std::vector<std::string>* folderNames);

        static bool Exists(std::string path);

        static bool CreateFolder(std::string path);

        static bool EnsureDirectory(std::string path);

    private:
        static void FillFileOrFolderNames(std::string basePath, bool searchFolders, std::vector<std::string>* fileOrFolderNames);

        static void AddFoldersRecursively(std::string basePath, std::string fileNameToCheck, std::vector<std::string>* folderPaths);
    };
}

#endif /* Core_Headers_Path_h */
