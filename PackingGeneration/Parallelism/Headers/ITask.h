// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Parallelism_Headers_ITask_h
#define Parallelism_Headers_ITask_h

#include "Core/Headers/Types.h"

namespace Parallelism
{
    // Defines methods for a parallel task (aka Command/Action pattern).
    class ITask
    {
    public:
        //Gets the estimated difficulty of task execution, i.e. its weight in the tasks list.
        virtual Core::FLOAT_TYPE GetWeight() const = 0;

        virtual void Execute() = 0;

        virtual ~ITask(){ };
    };
}


#endif /* Parallelism_Headers_ITask_h */

