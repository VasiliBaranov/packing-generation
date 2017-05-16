// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/TaskManager.h"
#include <stdio.h>
#include "Core/Headers/Exceptions.h"
#include "Core/Headers/MpiManager.h"
#include "Core/Headers/StlUtilities.h"
#include "Core/Headers/Math.h"
#include "Core/Headers/Path.h"
#include "Core/Headers/ScopedFile.h"
#include "Core/Headers/Utilities.h"
#include "../Headers/ITask.h"
#include "../Headers/ITaskFactory.h"
#include "../Headers/FileLock.h"

using namespace std;
using namespace Core;

namespace Parallelism
{
    const string TaskManager::syncFolderName = "sync";
    const string TaskManager::finishedTasksFileName = "finished.txt";
    const string TaskManager::startedTasksFileName = "started.txt";

    TaskManager::TaskManager(ITaskFactory* taskFactory)
    {
        this->taskFactory = taskFactory;
    }

    ITaskFactory* TaskManager::GetTaskFactory() const
    {
        return taskFactory;
    }

    void TaskManager::SubmitTasks()
    {
        taskFactory->FillTasks(&tasks);

        // May be shuffle tasks in a parallel environment?

        LogInitialValues();

        ITask* task = GetFirstTask(tasks);

        while (task != NULL)
        {
            try
            {
                 task->Execute();
            }
            catch (Exception& e)
            {
                printf(e.GetMessage().c_str());
            }
            catch (std::exception& e)
            {
                printf(e.what());
            }
            catch (...)
            {
                // TODO: log
            }

            LogTaskFinish();
            task = GetNextTaskAndLogStart();
        }

        // If no task to execute, wait, periodically check finished tasks. If all tasks are finished, exit
        while (!AllTasksFinished())
        {
            const int secondsToWait = 30;
            Utilities::Sleep(secondsToWait * 1000);
        }

        ClearLogs();
    }

    void TaskManager::LogInitialValues()
    {
        if (!MpiManager::GetInstance()->IsParallel())
        {
            startedTasksCount = 1;
            finishedTasksCount = 0;
            return;
        }

        if (MpiManager::GetInstance()->IsMaster())
        {
            Path::EnsureClearDirectory(GetSyncFolderPath());

            ScopedFile<ExceptionErrorHandler> finishedFile(GetFinishedTasksPath(), FileOpenMode::Write);
            fprintf(finishedFile, "0");

            ScopedFile<ExceptionErrorHandler> startedFile(GetStartedTasksPath(), FileOpenMode::Write);
            fprintf(startedFile, "%d", MpiManager::GetInstance()->GetNumberOfProcesses());
        }

        // No tasks shall start operation before initial files are written
        MpiManager::GetInstance()->Barrier();
    }

    void TaskManager::LogTaskFinish()
    {
        if (!MpiManager::GetInstance()->IsParallel())
        {
            finishedTasksCount++;
            return;
        }

        string finishedTasksPath = GetFinishedTasksPath();
        FileLock fileLock(finishedTasksPath);
        ScopedFile<ExceptionErrorHandler> finishedTasksFile(finishedTasksPath, FileOpenMode::Read | FileOpenMode::Update);
        int finishedTasksCount;
        fscanf(finishedTasksFile, "%d", &finishedTasksCount);
        fseek(finishedTasksFile, 0, SEEK_SET);
        fprintf(finishedTasksFile, "%d", finishedTasksCount + 1);
    }

    ITask* TaskManager::GetNextTaskAndLogStart()
    {
        if (!MpiManager::GetInstance()->IsParallel())
        {
            if (static_cast<size_t>(startedTasksCount) < tasks.size())
            {
                ITask* task = tasks[startedTasksCount].get();
                startedTasksCount++;
                return task;
            }
            else
            {
                return NULL;
            }
        }

        string startedTasksPath = GetStartedTasksPath();
        FileLock fileLock(startedTasksPath);

        ScopedFile<ExceptionErrorHandler> startedTasksFile(startedTasksPath, FileOpenMode::Read | FileOpenMode::Update);
        int startedTasksCount;
        fscanf(startedTasksFile, "%d", &startedTasksCount);

        ITask* nextTask = NULL;
        if (static_cast<size_t>(startedTasksCount) < tasks.size())
        {
            nextTask = tasks[startedTasksCount].get();
            fseek(startedTasksFile, 0, SEEK_SET);
            fprintf(startedTasksFile, "%d", startedTasksCount + 1);
        }

        return nextTask;
    }

    bool TaskManager::AllTasksFinished()
    {
        if (!MpiManager::GetInstance()->IsParallel())
        {
            return static_cast<size_t>(finishedTasksCount) == tasks.size();
        }

        string finishedTasksPath = GetFinishedTasksPath();
        ScopedFile<ExceptionErrorHandler> finishedTasksFile(finishedTasksPath, FileOpenMode::Read);
        int finishedTasksCount;
        fscanf(finishedTasksFile, "%d", &finishedTasksCount);

        return static_cast<size_t>(finishedTasksCount) == tasks.size();
    }

    void TaskManager::ClearLogs()
    {
        MpiManager::GetInstance()->Barrier();

        if (MpiManager::GetInstance()->IsMaster())
        {
            Path::DeleteFolder(GetSyncFolderPath());
        }
    }

    string TaskManager::GetSyncFolderPath()
    {
        return Path::Append(Path::GetCurrentWorkingFolder(), syncFolderName);
    }

    string TaskManager::GetFinishedTasksPath()
    {
        return Path::Append(GetSyncFolderPath(), finishedTasksFileName);
    }

    string TaskManager::GetStartedTasksPath()
    {
        return Path::Append(GetSyncFolderPath(), startedTasksFileName);
    }

    ITask* TaskManager::GetFirstTask(const vector<boost::shared_ptr<ITask> >& tasks) const
    {
        size_t currentRank = MpiManager::GetInstance()->GetCurrentRank();
        if (currentRank >= tasks.size())
        {
            return NULL;
        }

        return tasks[currentRank].get();
    }
}

