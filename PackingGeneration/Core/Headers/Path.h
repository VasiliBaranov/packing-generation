// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Core_Headers_Path_h
#define Core_Headers_Path_h

#include <string>
#include <vector>
#include <sys/stat.h>

namespace Core
{
    // Provides basic operations with paths.
    // TODO: May be switch to Boost.Filesystem? But it consumes 6Mb!!
    class Path
    {
    public:
        static std::string GetCurrentWorkingFolder();

        static char GetSeparator();

        static std::string Append(std::string basePath, std::string path);

        static void FillFoldersRecursively(std::string basePath, std::string fileNameToCheck, std::vector<std::string>* folderPaths);

        static void FillFileNames(std::string basePath, std::vector<std::string>* fileNames);

        static void FillFolderNames(std::string basePath, std::vector<std::string>* folderNames);

        static bool Exists(std::string path);

        static bool CreateFolder(std::string path);

        static bool DeleteFile(std::string path);

        static void DeleteFolder(std::string path);

        static bool EnsureDirectory(std::string path);

        static bool EnsureClearDirectory(std::string path);

        static std::string GetParentPath(std::string path);

        static std::string GetFileName(std::string path);

        static std::string GetFileNameWithoutExtension(std::string path);

        static std::string GetExtension(std::string filePath);

        static off_t GetFileSize(std::string filePath);

        static void CopyFile(std::string sourcePath, std::string targetPath);

        static void CopyFolder(std::string sourcePath, std::string targetPath);

        static bool Rename(std::string oldPath, std::string newPath);

    private:
        static void FillFileOrFolderNames(std::string basePath, bool searchFolders, std::vector<std::string>* fileOrFolderNames);

        static void AddFoldersRecursively(std::string basePath, std::string fileNameToCheck, std::vector<std::string>* folderPaths);
    };
}

#endif /* Core_Headers_Path_h */
