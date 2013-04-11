// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/RandomLoadBalancer.h"
#include "Core/Headers/StlUtilities.h"

using namespace std;
using namespace Core;

namespace Parallelism
{
    RandomLoadBalancer::RandomLoadBalancer()
    {

    }

    void RandomLoadBalancer::BalanceTasks(const vector<ITask*>& tasks, int processesCount, vector<vector<ITask*> >* balancedTasks) const
    {
        vector<vector<ITask*> >& balancedTasksRef = *balancedTasks;
        balancedTasksRef.resize(processesCount);
        size_t tasksPerProcess = tasks.size() / processesCount;
        for (int i = 0; i < processesCount; ++i)
        {
            vector<ITask*>& processorTasks = balancedTasksRef[i];
            processorTasks.reserve(tasksPerProcess);
        }

        // Do not shuffle for a single process
        if (processesCount == 1)
        {
            vector<ITask*>& processorTasks = balancedTasksRef[0];
            StlUtilities::Append(tasks, &processorTasks);
            return;
        }

        BalanceTasksSafely(tasks, processesCount, balancedTasks);
    }

    void RandomLoadBalancer::BalanceTasksSafely(const vector<ITask*>& tasks, int processesCount, vector<vector<ITask*> >* balancedTasks) const
    {
        size_t tasksPerProcess = tasks.size() / processesCount;

        // Can not use time(NULL), as all of the processes should have equal seed to generate equal balancing schemes.
        // TODO: Broadcast the seed from the master node with MPI.
        // srand(time(NULL));
        srand(0);

        vector<ITask*> leftTasks(tasks.begin(), tasks.end());
        int currentProcessIndex = 0;

        vector<vector<ITask*> >& balancedTasksRef = *balancedTasks;
        // Do not use tasks->size(), as currentProcessIndex may exceed the allowed values then.
        for (size_t i = 0; i < tasksPerProcess * processesCount; ++i)
        {
            vector<ITask*>& processorTasks = balancedTasksRef[currentProcessIndex];

            int taskIndex = rand() % leftTasks.size();
            processorTasks.push_back(leftTasks[taskIndex]);
            StlUtilities::QuicklyRemoveAt(&leftTasks, taskIndex);

            if (processorTasks.size() == tasksPerProcess)
            {
                currentProcessIndex++;
            }
        }

        // Distribute left tasks. left tasks count will never exceed processes count (as a remainder of division).
        for (size_t i = 0; i < leftTasks.size(); ++i)
        {
            vector<ITask*>& processorTasks = balancedTasksRef[i];
            processorTasks.push_back(leftTasks[i]);
        }
    }
}

