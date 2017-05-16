// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/Path.h"

#include <fcntl.h>   // open
#include <sys/types.h>
#include <sys/stat.h>
#include "Core/Headers/Macros.h"
#include "Core/Headers/Constants.h"
#include "Core/Headers/ScopedFile.h"

#ifdef WINDOWS
    #define NOMINMAX
    #include <direct.h>
    // dirent.h above is a free implementation of POSIX dirent.h.
    // It includes windows.h, and windows.h defines some weird macros like max, min, DeleteFile, CopyFile,
    // which we have to undefine right now.
    // // see http://stackoverflow.com/questions/1904635/warning-c4003-and-errors-c2589-and-c2059-on-x-stdnumeric-limitsintmax
    #include "Core/Headers/dirent.h"
    #undef DeleteFile
    #undef CopyFile
    #undef GetMessage

    #define GetCurrentFolder _getcwd

    #include <io.h>
    #define open _open
    #define read _read
    #define write _write
    #define close _close
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

    char Path::GetSeparator()
    {
#ifdef WINDOWS
    return '\\';
#else
    return '/';
#endif
    }

    string Path::Append(string basePath, string path)
    {
        char separatorChar = GetSeparator();
        bool basePathEndsWithSlash = basePath.size() > 0 && basePath[basePath.size() - 1] == separatorChar;
        bool pathStartsFromSlash = path.size() > 0 && path[0] == separatorChar;
        bool shouldAppendSlash = basePath.size() > 0 && path.size() > 0 && !basePathEndsWithSlash && !pathStartsFromSlash;

        string result;
        if (shouldAppendSlash)
        {
            result = basePath + separatorChar + path;
        }
        else
        {
            result = basePath + path;
        }

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
        ScopedFile<EmptyErrorHandler> file(path, FileOpenMode::Read);
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

    bool Path::Rename(std::string oldPath, std::string newPath)
    {
        int result = rename(oldPath.c_str(), newPath.c_str());
        return result == 0;
    }

    void Path::DeleteFolder(std::string path)
    {
        vector<string> currentFolderFiles;
        Path::FillFileNames(path, &currentFolderFiles);
        for (vector<string>::iterator it = currentFolderFiles.begin(); it < currentFolderFiles.end(); ++it)
        {
            string fileName = *it;
            string filePath = Path::Append(path, fileName);
            Path::DeleteFile(filePath);
        }

        vector<string> subfolders;
        Path::FillFolderNames(path, &subfolders);
        for (vector<string>::iterator it = subfolders.begin(); it < subfolders.end(); ++it)
        {
            string subfolderName = *it;
            string subfolderPath = Path::Append(path, subfolderName);
            Path::DeleteFolder(subfolderPath);
        }

        DeleteFile(path);
    }

    bool Path::DeleteFile(std::string path)
    {
        bool success = remove(path.c_str()) == 0;
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

    bool Path::EnsureClearDirectory(std::string path)
    {
        bool success = true;
        if (Path::Exists(path))
        {
            Path::DeleteFolder(path);
        }
        success = Path::CreateFolder(path);

        return success;
    }

    string Path::GetParentPath(string path)
    {
        int index = path.find_last_of("/\\");
        return path.substr(0, index);
    }

    string Path::GetFileName(string path)
    {
        int index = path.find_last_of("/\\");
        return path.substr(index + 1);
    }

    string Path::GetFileNameWithoutExtension(string path)
    {
        int lastSlashIndex = path.find_last_of("/\\");
        int lastDotIndex = path.find_last_of(".");
        int fileNameWithoutExtensionLength = lastDotIndex - lastSlashIndex - 1;
        return path.substr(lastSlashIndex + 1, fileNameWithoutExtensionLength);
    }

    string Path::GetExtension(string filePath)
    {
        int index = filePath.find_last_of(".");
        return filePath.substr(index);
    }

    off_t Path::GetFileSize(string filePath)
    {
        struct stat statistics;

        if (stat(filePath.c_str(), &statistics) != 0)
        {
            throw InvalidOperationException("Error calling stat");
        }

        return statistics.st_size;
    }

    // See http://stackoverflow.com/a/1932861
    // or http://stackoverflow.com/questions/612097/how-can-i-get-a-list-of-files-in-a-directory-using-c-or-c
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

// #ifdef WINDOWS
//             fileOrFolderNames->push_back(entryName);
// #else
            filePath = Path::Append(basePath, entryName);
//            if (lstat(filePath.c_str(), &status) == -1)
            if (stat(filePath.c_str(), &status) == -1)
            {
                continue;
            }

//             This code doesn't probably work well with symbolic links.
//             On unix, lstat function may be used instead of stat, but lstat is not available on windows.
//             You may use complete windows code from http://stackoverflow.com/a/1932861 instead.
//             Is symbolic link
//            if (S_ISLNK(status.st_mode))
//            {
//                continue;
//            }

            bool entryIsCorrect = (searchFolders == S_ISDIR(status.st_mode));
            if (entryIsCorrect)
            {
                fileOrFolderNames->push_back(entryName);
            }
// #endif
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

    // See http://stackoverflow.com/questions/10195343/copy-a-file-in-an-sane-safe-and-efficient-way , LINUX-WAY
    void Path::CopyFile(string sourcePath, string targetPath)
    {
        char buf[BUFSIZ];
        size_t size;

        // On Windows, text and binary mode differ. If you do not specifiy O_BINARY in your flags, 
        // Windows will handle '\n' characters differently for this file. Likely it will return at the first '\n' encountered.
        // See http://stackoverflow.com/a/7183014
#ifdef WINDOWS
        int source = open(sourcePath.c_str(), O_RDONLY | O_BINARY, 0);
        int dest = open(targetPath.c_str(), O_WRONLY | O_CREAT | O_BINARY, 0644); // | O_TRUNC
#else
        int source = open(sourcePath.c_str(), O_RDONLY, 0);
        int dest = open(targetPath.c_str(), O_WRONLY | O_CREAT, 0644); // | O_TRUNC
#endif

        while ((size = read(source, buf, BUFSIZ)) > 0)
        {
            write(dest, buf, size);
        }

        close(source);
        close(dest);
    }

    void Path::CopyFolder(string sourcePath, string targetPath)
    {
        EnsureClearDirectory(targetPath);

        vector<string> fileNames;
        vector<string> folderNames;

        FillFileNames(sourcePath, &fileNames);
        FillFolderNames(sourcePath, &folderNames);

        for (size_t i = 0; i < fileNames.size(); ++i)
        {
            string fileSourcePath = Path::Append(sourcePath, fileNames[i]);
            string fileTargetPath = Path::Append(targetPath, fileNames[i]);

            CopyFile(fileSourcePath, fileTargetPath);
        }

        for (size_t i = 0; i < folderNames.size(); ++i)
        {
            string subfolderSourcePath = Path::Append(sourcePath, folderNames[i]);
            string subfolderTargetPath = Path::Append(targetPath, folderNames[i]);
            CopyFolder(subfolderSourcePath, subfolderTargetPath);
        }
    }
}
