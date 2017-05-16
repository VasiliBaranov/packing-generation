// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Parallelism_Headers_ITaskManager_h
#define Parallelism_Headers_ITaskManager_h

namespace Parallelism { class ILoadBalancer; }
namespace Parallelism { class ITaskFactory; }

namespace Parallelism
{
    // Defines methods for task parallel execution management.
    class ITaskManager
    {
    public:
        // Do not simply pass the list of tasks to the manager, as task creation logic may become more complicated (e.g. create first ten tasks, wait for completion, create next ten tasks, etc).
        virtual ITaskFactory* GetTaskFactory() const = 0;

        virtual void SubmitTasks() = 0;

        virtual ~ITaskManager(){};
    };
}


#endif /* Parallelism_Headers_ITaskManager_h */
