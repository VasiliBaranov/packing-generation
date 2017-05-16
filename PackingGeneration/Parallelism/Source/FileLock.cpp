// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/FileLock.h"
#include <stdio.h>
#include <climits>
#include "Core/Headers/Exceptions.h"
#include "Core/Headers/MpiManager.h"
#include "Core/Headers/StlUtilities.h"
#include "Core/Headers/Math.h"
#include "Core/Headers/Path.h"
#include "Core/Headers/ScopedFile.h"
#include "Core/Headers/Utilities.h"

using namespace std;
using namespace Core;

namespace Parallelism
{
    const string FileLock::semaphorePostfix = ".sem";

    FileLock::FileLock(string filePath)
    {
        this->filePath = filePath;
        LockFile(filePath);
    }

    FileLock::~FileLock()
    {
        ReleaseFileLock(filePath);
    }

    void FileLock::LockFile(string filePath)
    {
        bool fileLocked = TryLockFile(filePath);

        while (!fileLocked)
        {
            Utilities::Sleep(200);
            fileLocked = TryLockFile(filePath);
        }
    }

    void FileLock::ReleaseFileLock(string filePath)
    {
        string semaphoreFilePath = GetSemaphoreFilePath(filePath);
        Path::DeleteFile(semaphoreFilePath);
    }

    string FileLock::GetSemaphoreFilePath(string filePath)
    {
        return filePath + "_" + Utilities::ConvertToString(MpiManager::GetInstance()->GetCurrentRank()) + semaphorePostfix;
    }

    bool FileLock::TryLockFile(string filePath)
    {
        // Get semaphore files for filePath
        vector<string> semaphoreFilePaths;
        FillSemaphoreFilePaths(filePath, &semaphoreFilePaths);

        // If exist, return false
        if (semaphoreFilePaths.size() > 0)
        {
            return false;
        }

        // Create a semaphore file
        string currentSemaphoreFilePath = GetSemaphoreFilePath(filePath);
        ScopedFile<ExceptionErrorHandler> semaphore(currentSemaphoreFilePath, FileOpenMode::Write);
        semaphore.Close();

        // Wait for 100ms
        Utilities::Sleep(100);

        // Get semaphore files for filePath
        FillSemaphoreFilePaths(filePath, &semaphoreFilePaths);
        if (semaphoreFilePaths.size() == 1) // the only semaphore is the created one
        {
            return true;
        }
        else // some other semaphores have been created while we were creating ours
        {
            bool currentSemaphoreIsTheEarliest = CurrentSemaphoreIsTheEarliest(currentSemaphoreFilePath, semaphoreFilePaths);
            if (!currentSemaphoreIsTheEarliest)
            {
                Path::DeleteFile(currentSemaphoreFilePath);
                return false;
            }
            else
            {
                // Ours is the earliest one
                return true;
            }
        }
    }

    int FileLock::GetProcessRankBySemaphorePath(std::string semaphorePath)
    {
        string fileName = Path::GetFileName(semaphorePath);
        int rankStartIndex = fileName.find_last_of("_");
        string rankAndPostfix = fileName.substr(rankStartIndex + 1);

        int postfixStartIndex = rankAndPostfix.find_last_of(".");
        string rankString = rankAndPostfix.substr(0, postfixStartIndex);
        return Utilities::ParseInt(rankString.c_str());
    }

    bool FileLock::CurrentSemaphoreIsTheEarliest(string currentSemaphoreFilePath, const vector<string>& semaphoreFilePaths)
    {
        int minRank = INT_MAX;
        for (size_t i = 0; i < semaphoreFilePaths.size(); ++i)
        {
            // Avoid actual reading of semaphore files, as the current semaphore might have been the true lock, and was already released.
            // Or this semaphore might have been not the earliest lock, and might have already been deleted by the owner.
            // We may also lock the file prior to deletion, etc.
            int rank = GetProcessRankBySemaphorePath(semaphoreFilePaths[i]);
            bool semaphoreIsEarlier = rank < minRank;
            if (semaphoreIsEarlier)
            {
                minRank = rank;
            }
        }

        return minRank == MpiManager::GetInstance()->GetCurrentRank();
    }

    void FileLock::FillSemaphoreFilePaths(string filePath, vector<string>* semaphoreFilePaths)
    {
        string folderPath = Path::GetParentPath(filePath);
        string fileNameToCheck = Path::GetFileName(filePath);
        vector<string> fileNames;
        Path::FillFileNames(folderPath, &fileNames);

        semaphoreFilePaths->clear();
        for (size_t i = 0; i < fileNames.size(); ++i)
        {
            string fileName = fileNames[i];
            if (Utilities::StringStartsWith(fileName, fileNameToCheck) && Utilities::StringEndsWith(fileName, semaphorePostfix))
            {
                semaphoreFilePaths->push_back(Path::Append(folderPath, fileName));
            }
        }
    }
}

