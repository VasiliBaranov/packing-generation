// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Parallelism_Headers_TaskManager_h
#define Parallelism_Headers_TaskManager_h

#include "ITaskManager.h"
#include "Core/Headers/Macros.h"

namespace Parallelism
{
    // Implements a default load balancer to split tasks per process in a random manner, thus minimizing execution time deviation.
    class TaskManager : public ITaskManager
    {
    private:
        ITaskFactory* taskFactory;
        ILoadBalancer* loadBalancer;

    public:
        TaskManager(ITaskFactory* taskFactory, ILoadBalancer* loadBalancer);

        OVERRIDE ITaskFactory* GetTaskFactory() const;

        OVERRIDE ILoadBalancer* GetLoadBalancer() const;

        OVERRIDE void SubmitTasks();

    private:
        DISALLOW_COPY_AND_ASSIGN(TaskManager);
    };
}

#endif /* Parallelism_Headers_TaskManager_h */

