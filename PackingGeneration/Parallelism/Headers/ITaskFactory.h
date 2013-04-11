// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Parallelism_Headers_ITaskFactory_h
#define Parallelism_Headers_ITaskFactory_h

#include <vector>
#include <boost/shared_ptr.hpp>
namespace Parallelism { class ITask; }

namespace Parallelism
{
    // Defines methods for tasks creation. Methods are not const, as they may change the inner state of the factory.
    class ITaskFactory
    {
    public:
        virtual void FillTasks(std::vector<boost::shared_ptr<ITask> >* tasks) = 0;

        virtual ~ITaskFactory(){};
    };
}


#endif /* Parallelism_Headers_ITaskFactory_h */
