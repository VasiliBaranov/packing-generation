// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Parallelism_Headers_FileLock_h
#define Parallelism_Headers_FileLock_h

#include <vector>
#include <string>
#include "Core/Headers/Macros.h"

namespace Parallelism
{
    class FileLock
    {
    private:
        static const std::string semaphorePostfix;
        std::string filePath;

    public:
        FileLock(std::string filePath);

        ~FileLock();

    private:
        std::string GetSemaphoreFilePath(std::string filePath);

        void LockFile(std::string filePath);

        void ReleaseFileLock(std::string filePath);

        bool TryLockFile(std::string filePath);

        bool CurrentSemaphoreIsTheEarliest(std::string currentSemaphoreFilePath, const std::vector<std::string>& semaphoreFilePaths);

        int GetProcessRankBySemaphorePath(std::string semaphorePath);

        void FillSemaphoreFilePaths(std::string filePath, std::vector<std::string>* semaphoreFiles);

        DISALLOW_COPY_AND_ASSIGN(FileLock);
    };
}

#endif /* Parallelism_Headers_FileLock_h */

