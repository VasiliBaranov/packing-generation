// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Parallelism_Headers_ILoadBalancer_h
#define Parallelism_Headers_ILoadBalancer_h

#include <vector>
namespace Parallelism { class ITask; }

namespace Parallelism
{
    // Defines methods for tasks balancing.
    class ILoadBalancer
    {
    public:
        // It's possible to return vector<vector<ITask*> >* (even better performance). But understanding of whether it is efficient requires knowledge of how std::vector stores its content.
        // Methods is not const, as it may change the inner state of the balancer (some heuristics, e.g.).
        virtual void BalanceTasks(const std::vector<ITask*>& tasks, int processesCount, std::vector<std::vector<ITask*> >* balancedTasks) const = 0;

        virtual ~ILoadBalancer(){};
    };
}


#endif /* Parallelism_Headers_ILoadBalancer_h */
