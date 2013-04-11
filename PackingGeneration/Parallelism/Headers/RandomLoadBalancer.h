// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Parallelism_Headers_RandomLoadBalancer_h
#define Parallelism_Headers_RandomLoadBalancer_h

#include "Core/Headers/Macros.h"
#include "ILoadBalancer.h"

namespace Parallelism
{
    // Implements a default load balancer to split tasks per process in a random manner, thus minimizing execution time deviation.
    class RandomLoadBalancer : public ILoadBalancer
    {
    public:
        RandomLoadBalancer();

        OVERRIDE void BalanceTasks(const std::vector<ITask*>& tasks, int processesCount, std::vector<std::vector<ITask*> >* balancedTasks) const;

    private:
        void BalanceTasksSafely(const std::vector<ITask*>& tasks, int processesCount, std::vector<std::vector<ITask*> >* balancedTasks) const;

        DISALLOW_COPY_AND_ASSIGN(RandomLoadBalancer);
    };
}

#endif /* Parallelism_Headers_RandomLoadBalancer_h */

