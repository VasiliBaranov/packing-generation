// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Parallelism_Headers_TaskManager_h
#define Parallelism_Headers_TaskManager_h

#include <vector>
#include <boost/shared_ptr.hpp>
#include "ITaskManager.h"
#include "Core/Headers/Macros.h"
#include "Core/Headers/Types.h"
namespace Parallelism { class ITask; }

namespace Parallelism
{
    // Implements a default load balancer to split tasks per process in a random manner, thus minimizing execution time deviation.
    class TaskManager : public virtual ITaskManager
    {
    private:
        ITaskFactory* taskFactory;
        std::vector<boost::shared_ptr<ITask> > tasks;

        // Values used only in single-process environment to avoid writing to the file system and file locking.
        int startedTasksCount;
        int finishedTasksCount;

        static const std::string syncFolderName;
        static const std::string finishedTasksFileName;
        static const std::string startedTasksFileName;

    public:
        TaskManager(ITaskFactory* taskFactory);

        OVERRIDE ITaskFactory* GetTaskFactory() const;

        OVERRIDE void SubmitTasks();

    private:
        ITask* GetFirstTask(const std::vector<boost::shared_ptr<ITask> >& tasks) const;

        void LogTaskFinish();

        void LogInitialValues();

        ITask* GetNextTaskAndLogStart();

        bool AllTasksFinished();

        void ClearLogs();

        std::string GetSyncFolderPath();

        std::string GetFinishedTasksPath();

        std::string GetStartedTasksPath();

        DISALLOW_COPY_AND_ASSIGN(TaskManager);
    };
}

#endif /* Parallelism_Headers_TaskManager_h */

