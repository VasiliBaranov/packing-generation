// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/Path.h"

#include <sys/types.h>
#include <sys/stat.h>
#include "../Headers/Constants.h"
#include "../Headers/ScopedFile.h"

#ifdef WINDOWS
    #include "../Headers/dirent.h"
    #include <direct.h>
    #define GetCurrentFolder _getcwd
#else
    #include <dirent.h>
    #include <unistd.h>
    #define GetCurrentFolder getcwd
#endif

using namespace std;

namespace Core
{
    string Path::GetCurrentWorkingFolder()
    {
        char currentPath[FILENAME_MAX];
        GetCurrentFolder(currentPath, sizeof(currentPath));
        string result(currentPath);
        return result;
    }

    string Path::Append(string basePath, string path)
    {
        bool basePathEndsWithSlash = basePath.size() > 0 && basePath[basePath.size() - 1] == '/';
        bool pathStartsFromSlash = path.size() > 0 && path[0] == '/';
        bool shouldAppendSlash = basePath.size() > 0 && path.size() > 0 && !basePathEndsWithSlash && !pathStartsFromSlash;
        string slash = shouldAppendSlash ? "/" : "";
        string result = basePath + slash + path;
        return result;
    }

    void Path::FillFileNames(string basePath, vector<string>* fileNames)
    {
        Path::FillFileOrFolderNames(basePath, false, fileNames);
    }

    void Path::FillFolderNames(string basePath, vector<string>* folderNames)
    {
        Path::FillFileOrFolderNames(basePath, true, folderNames);
    }

    bool Path::Exists(string path)
    {
        ScopedFile<EmptyErrorHandler> file(path, "r");
        return file.Exists();
    }

    bool Path::CreateFolder(string path)
    {
#ifdef WINDOWS
        int result = _mkdir(path.c_str());
#else
        const int readWriteExecuteForAllPermission = 0777;
        int result = mkdir(path.c_str(), readWriteExecuteForAllPermission);
#endif

        bool success = (result != -1);
        return success;
    }

    bool Path::EnsureDirectory(string path)
    {
        bool success = true;
        if (!Path::Exists(path))
        {
            success = Path::CreateFolder(path);
        }

        return success;
    }

    void Path::FillFileOrFolderNames(string basePath, bool searchFolders, vector<string>* fileOrFolderNames)
    {
        fileOrFolderNames->clear();

        struct dirent *entry;
        DIR *directoryPointer;
        struct stat status;
        string filePath;

        directoryPointer = opendir(basePath.c_str());
        if (directoryPointer == NULL)
        {
            return;
        }

        while ((entry = readdir(directoryPointer)) != NULL)
        {
            string entryName(entry->d_name);
            if (entryName == "." || entryName == "..")
            {
                continue;
            }

#ifdef WINDOWS
            fileOrFolderNames->push_back(entryName);
#else
            filePath = Path::Append(basePath, entryName);
            if (lstat(filePath.c_str(), &status) == -1)
            {
                continue;
            }

            // Is symbolic link
            if (S_ISLNK(status.st_mode))
            {
                continue;
            }

            bool entryIsCorrect = (searchFolders == S_ISDIR(status.st_mode));
            if (entryIsCorrect)
            {
                fileOrFolderNames->push_back(entryName);
            }
#endif
        }

        closedir(directoryPointer);
    }

    void Path::FillFoldersRecursively(string basePath, string fileNameToCheck, vector<string>* folderPaths)
    {
        folderPaths->clear();
        Path::AddFoldersRecursively(basePath, fileNameToCheck, folderPaths);
    }

    void Path::AddFoldersRecursively(string basePath, string fileNameToCheck, vector<string>* folderPaths)
    {
        vector<string> currentFolderFiles;
        Path::FillFileNames(basePath, &currentFolderFiles);
        for (vector<string>::iterator it = currentFolderFiles.begin(); it < currentFolderFiles.end(); ++it)
        {
            string fileName = *it;
            if (fileName == fileNameToCheck)
            {
                folderPaths->push_back(basePath);
                break;
            }
        }

        vector<string> subfolders;
        Path::FillFolderNames(basePath, &subfolders);
        for (vector<string>::iterator it = subfolders.begin(); it < subfolders.end(); ++it)
        {
            string subfolderName = *it;
            string subfolderPath = Path::Append(basePath, subfolderName);
            Path::AddFoldersRecursively(subfolderPath, fileNameToCheck, folderPaths);
        }
    }
}
