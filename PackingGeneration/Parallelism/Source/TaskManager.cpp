// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/TaskManager.h"
#include <stdio.h>
#include "Core/Headers/Exceptions.h"
#include "Core/Headers/MpiManager.h"
#include "Core/Headers/MemoryUtility.h"
#include "../Headers/ITask.h"
#include "../Headers/ITaskFactory.h"
#include "../Headers/ILoadBalancer.h"

using namespace std;
using namespace Core;

namespace Parallelism
{
    TaskManager::TaskManager(ITaskFactory* taskFactory, ILoadBalancer* loadBalancer)
    {
        this->taskFactory = taskFactory;
        this->loadBalancer = loadBalancer;
    }

    ITaskFactory* TaskManager::GetTaskFactory() const
    {
        return taskFactory;
    }

    ILoadBalancer* TaskManager::GetLoadBalancer() const
    {
        return loadBalancer;
    }

    void TaskManager::SubmitTasks()
    {
        vector<boost::shared_ptr<ITask> > tasks;
        taskFactory->FillTasks(&tasks);

        vector<ITask*> unownedTasks;
        MemoryUtility::RemoveOwnership(tasks, &unownedTasks);

        int processCount = MpiManager::GetInstance()->GetNumberOfProcesses();
        int currentRank = MpiManager::GetInstance()->GetCurrentRank();

        vector<vector<ITask*> > tasksPerProcess;
        loadBalancer->BalanceTasks(unownedTasks, processCount, &tasksPerProcess);
        vector<ITask*>& currentTasks = tasksPerProcess[currentRank];

        for (vector<ITask*>::iterator it = currentTasks.begin() ; it < currentTasks.end(); it++)
        {
            ITask* task = *it;
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
        }
    }
}

